/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { audio } from '@kit.AudioKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { AppConstants } from '../../common/constants/AppConstants';
import {
  AudioDeviceInfo,
  AudioDeviceKind,
  AudioRouteConfig,
  AudioSourceType
} from '../../common/types/AudioTypes';
import { AudioRouteTarget } from '../../common/types/CreationSessionTypes';
import { isLocalDeviceId, TrustDeviceInfo } from '../../common/types/DeviceTypes';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { DeviceManagerService } from '../DeviceManagerService';
import { DhDiag } from '../../common/util/DhDiag';
import { AudioRouteHelpers } from './AudioRouteHelpers';
import { AudioRouteState } from './AudioRouteState';
import { AudioDescriptorExt, AudioRoutingManagerExt, BusinessError } from './AudioRouteTypes';
import { AudioRouteService } from '../AudioRouteService';

const TAG = AppConstants.TAG;

/** 设备枚举 / 发现 / 路由绑定 */
export class AudioRouteDeviceOps {
  private readonly host: AudioRouteService;

  constructor(host: AudioRouteService) {
    this.host = host;
  }

  private get st(): AudioRouteState { return this.host.state; }

  public async discoverDevicesForRoute(
    kind: AudioDeviceKind,
    needRemote: boolean,
    remoteNetworkId?: string,
    peer?: TrustDeviceInfo,
    maxRetries?: number,
    intervalMs?: number
  ): Promise<AudioDeviceInfo[]> {
    if (!needRemote) {
      return kind === AudioDeviceKind.MIC ?
        await this.enumerateInputDevices() : await this.enumerateOutputDevices();
    }
    const retries = maxRetries ?? AudioRouteHelpers.probeRetries();
    const interval = intervalMs ?? AudioRouteHelpers.probeIntervalMs();
    for (let i = 0; i < retries; i++) {
      const list = kind === AudioDeviceKind.MIC ?
        await this.enumerateInputDevices() : await this.enumerateOutputDevices();
      const remoteCnt = list.filter(d => d.sourceType === AudioSourceType.DISTRIBUTED).length;
      DhDiag.info('AUDIO', `discoverAudio ${kind} attempt=${i + 1} total=${list.length} distributed=${remoteCnt}`);
      const matched = list.filter(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
        (AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId) ||
          (peer && d.deviceName.includes(peer.deviceName))));
      if (matched.length > 0 || (HardwareManagerAdapter.isKitAvailable() && remoteCnt > 0)) {
        return list;
      }
      if (i < retries - 1) {
        await AudioRouteHelpers.delay(interval);
      }
    }
    return kind === AudioDeviceKind.MIC ?
      await this.enumerateInputDevices() : await this.enumerateOutputDevices();
  }

  public pickDevice(list: AudioDeviceInfo[], target: AudioRouteTarget, remoteNetworkId?: string,
    peer?: TrustDeviceInfo): AudioDeviceInfo | null {
    if (target === AudioRouteTarget.LOCAL) {
      return list.find(d => d.sourceType === AudioSourceType.LOCAL) ?? list[0] ?? null;
    }
    if (remoteNetworkId) {
      const matched = list.find(d =>
        d.sourceType === AudioSourceType.DISTRIBUTED &&
        (AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId) ||
          AudioRouteHelpers.networkIdMatches(d.address, remoteNetworkId)));
      if (matched) {
        return matched;
      }
    }
    if (peer) {
      const byName = list.find(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
        d.deviceName.includes(peer.deviceName));
      if (byName) {
        return byName;
      }
    }
    const anyRemote = list.find(d => d.sourceType === AudioSourceType.DISTRIBUTED);
    if (anyRemote) {
      return anyRemote;
    }
    return null;
  }

  public pickDistributedSpeaker(outputs: AudioDeviceInfo[], remoteNetworkId: string,
    peer?: TrustDeviceInfo): AudioDeviceInfo | null {
    return outputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
      (AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId) ||
        AudioRouteHelpers.networkIdMatches(d.address, remoteNetworkId))) ??
      outputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
        (peer ? d.deviceName.includes(peer.deviceName) : false)) ??
      outputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED) ?? null;
  }

  public pickDistributedMic(inputs: AudioDeviceInfo[], remoteNetworkId: string,
    peer?: TrustDeviceInfo): AudioDeviceInfo | null {
    return inputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
      (AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId) ||
        AudioRouteHelpers.networkIdMatches(d.address, remoteNetworkId))) ??
      inputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED &&
        (peer ? d.deviceName.includes(peer.deviceName) : false)) ??
      inputs.find(d => d.sourceType === AudioSourceType.DISTRIBUTED) ?? null;
  }

  public getRouteConfig(): AudioRouteConfig {
    return this.st.routeConfig;
  }

  private async fetchDescriptors(flag: audio.DeviceFlag): Promise<audio.AudioDeviceDescriptor[]> {
    if (this.st.routingManager) {
      try {
        return await this.st.routingManager.getDevices(flag);
      } catch (err) {
        DhDiag.warn('AUDIO', `routingManager.getDevices flag=${flag} failed ${JSON.stringify(err)}`);
      }
    }
    if (this.st.audioManager) {
      return await this.st.audioManager.getDevices(flag);
    }
    return [];
  }

  private mergeDeviceDescriptors(base: audio.AudioDeviceDescriptor[],
    extra: audio.AudioDeviceDescriptor[]): audio.AudioDeviceDescriptor[] {
    const byId = new Map<number, audio.AudioDeviceDescriptor>();
    for (const d of base) {
      byId.set(d.id, d);
    }
    for (const d of extra) {
      byId.set(d.id, d);
    }
    return Array.from(byId.values());
  }

  private async collectDescriptors(kind: AudioDeviceKind): Promise<audio.AudioDeviceDescriptor[]> {
    this.st.distributedNativeIds.clear();
    const localFlag = kind === AudioDeviceKind.MIC ?
      audio.DeviceFlag.INPUT_DEVICES_FLAG : audio.DeviceFlag.OUTPUT_DEVICES_FLAG;
    let list = await this.fetchDescriptors(localFlag);
    if (HardwareManagerAdapter.isKitAvailable()) {
      const distFlag = AudioRouteHelpers.distributedDeviceFlag(kind);
      const distList = await this.fetchDescriptors(distFlag);
      for (const d of distList) {
        this.st.distributedNativeIds.add(d.id);
      }
      list = this.mergeDeviceDescriptors(list, distList);
      DhDiag.info('AUDIO',
        `collectDescriptors ${kind} local=${list.length - distList.length} distFlag=${distList.length}`);
    }
    return list;
  }

  public async enumerateOutputDevices(): Promise<AudioDeviceInfo[]> {
    if (!this.st.audioManager) {
      return [];
    }
    try {
      const descriptors = await this.collectDescriptors(AudioDeviceKind.SPEAKER);
      const mapped = descriptors
        .filter(d => d.deviceRole === audio.DeviceRole.OUTPUT_DEVICE)
        .map(d => this.mapDescriptor(d, AudioDeviceKind.SPEAKER));
      const dist = mapped.filter(d => d.sourceType === AudioSourceType.DISTRIBUTED).length;
      DhDiag.info('AUDIO', `enumerateOutput total=${mapped.length} distributed=${dist}`);
      return mapped;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'enumerateOutputDevices failed: %{public}s', JSON.stringify(err));
      return [];
    }
  }

  public async enumerateInputDevices(): Promise<AudioDeviceInfo[]> {
    if (!this.st.audioManager) {
      return [];
    }
    try {
      const descriptors = await this.collectDescriptors(AudioDeviceKind.MIC);
      const mapped = descriptors
        .filter(d => d.deviceRole === audio.DeviceRole.INPUT_DEVICE)
        .map(d => this.mapDescriptor(d, AudioDeviceKind.MIC));
      const dist = mapped.filter(d => d.sourceType === AudioSourceType.DISTRIBUTED).length;
      DhDiag.info('AUDIO', `enumerateInput total=${mapped.length} distributed=${dist}`);
      return mapped;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'enumerateInputDevices failed: %{public}s', JSON.stringify(err));
      return [];
    }
  }

  private mapDescriptor(desc: audio.AudioDeviceDescriptor, kind: AudioDeviceKind): AudioDeviceInfo {
    const ext = desc as AudioDescriptorExt;
    const trustDevices = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .concat(DeviceManagerService.getInstance().getTrustDevices().filter(d => isLocalDeviceId(d.deviceId)));
    const matchedTrust = this.matchTrustByDescriptor(ext, trustDevices);
    const isDistributed = this.isDistributedDescriptor(ext, matchedTrust);
    const deviceId = `audio-${kind}-${desc.id}`;
    this.st.deviceDescriptorCache.set(deviceId, desc);
    if (isDistributed) {
      const netLabel: string = matchedTrust?.networkId ?? ext.networkId ?? desc.address ?? '';
      hilog.info(AppConstants.LOG_DOMAIN, TAG,
        'distributed audio dev id=%{public}d name=%{public}s networkId=%{public}s',
        desc.id, desc.name ?? '', netLabel);
    }
    return {
      deviceId: deviceId,
      deviceName: matchedTrust?.deviceName ?? desc.displayName ?? desc.name ??
        (isDistributed ? '远端音频' : '本机'),
      kind: kind,
      sourceType: isDistributed ? AudioSourceType.DISTRIBUTED : AudioSourceType.LOCAL,
      networkId: matchedTrust?.networkId ?? ext.networkId ?? desc.address,
      isAvailable: true,
      nativeDeviceId: desc.id,
      address: desc.address
    };
  }

  private isDistributedDescriptor(desc: AudioDescriptorExt, matchedTrust?: TrustDeviceInfo): boolean {
    if (this.st.distributedNativeIds.has(desc.id)) {
      return true;
    }
    if (matchedTrust && !isLocalDeviceId(matchedTrust.deviceId)) {
      return true;
    }
    const net = desc.networkId ?? '';
    if (net.length > 0 && net !== 'LocalDevice' && !net.toLowerCase().includes('local')) {
      return true;
    }
    const label = `${desc.displayName ?? ''}${desc.name ?? ''}${desc.address ?? ''}`;
    const remotes = DeviceManagerService.getInstance().getBoundRemoteDevices();
    for (const peer of remotes) {
      if (label.includes(peer.deviceName)) {
        return true;
      }
      if (desc.address && AudioRouteHelpers.networkIdMatches(desc.address, peer.networkId)) {
        return true;
      }
    }
    return false;
  }

  private matchTrustByDescriptor(desc: AudioDescriptorExt,
    trustDevices: TrustDeviceInfo[]): TrustDeviceInfo | undefined {
    const label = `${desc.displayName ?? ''}${desc.name ?? ''}`;
    const remotes = trustDevices.filter(d => !isLocalDeviceId(d.deviceId));
    for (const peer of remotes) {
      if (label.includes(peer.deviceName)) {
        return peer;
      }
      if (desc.networkId && AudioRouteHelpers.networkIdMatches(desc.networkId, peer.networkId)) {
        return peer;
      }
      if (desc.address && AudioRouteHelpers.networkIdMatches(desc.address, peer.networkId)) {
        return peer;
      }
    }
    if (!desc.address && !desc.networkId && remotes.length === 0) {
      return trustDevices.find(d => isLocalDeviceId(d.deviceId));
    }
    return undefined;
  }

  public async selectOutputDeviceForMusic(device: AudioDeviceInfo): Promise<boolean> {
    hilog.info(AppConstants.LOG_DOMAIN, TAG, 'selectOutputDeviceForMusic: %{public}s', device.deviceName);
    if (!this.host.isReady()) {
      return false;
    }
    try {
      const routed = await this.trySelectRoutingDevice(device, false);
      if (!routed) {
        return false;
      }
      this.st.routeConfig.outputDevice = device;
      DhDiag.info('AUDIO', `selectOutputDeviceForMusic OK ${device.deviceName}`);
      return true;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'selectOutputDeviceForMusic failed: %{public}s', JSON.stringify(err));
      return false;
    }
  }

  public async selectOutputDevice(device: AudioDeviceInfo): Promise<boolean> {
    hilog.info(AppConstants.LOG_DOMAIN, TAG, 'selectOutputDevice: %{public}s', device.deviceName);
    if (device.sourceType === AudioSourceType.DISTRIBUTED && device.networkId) {
      await this.host.dhService.enableSinkAudioBounded(device.networkId, 'speaker',
        AppConstants.MUSIC_ENABLE_SINK_TIMEOUT_MS);
    }
    if (!this.host.isReady()) {
      return false;
    }
    try {
      const routed = await this.trySelectRoutingDevice(device, false);
      if (!routed && HardwareManagerAdapter.isKitAvailable() &&
        device.sourceType === AudioSourceType.DISTRIBUTED) {
        return false;
      }
      await this.host.pipelines.resetOutputPipeline();
      await this.host.pipelines.ensureRenderer();
      this.st.routeConfig.outputDevice = device;
      return true;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'selectOutputDevice failed: %{public}s', JSON.stringify(err));
      return false;
    }
  }

  public async selectInputDeviceForRecording(device: AudioDeviceInfo): Promise<boolean> {
    hilog.info(AppConstants.LOG_DOMAIN, TAG, 'selectInputDeviceForRecording: %{public}s', device.deviceName);
    const nativeId = device.nativeDeviceId ?? -1;
    const skipSinkEnable = device.sourceType === AudioSourceType.DISTRIBUTED &&
      (this.st.distributedNativeIds.has(nativeId) ||
        (device.networkId?.length ?? 0) > 0);
    if (!skipSinkEnable && device.sourceType === AudioSourceType.DISTRIBUTED && device.networkId) {
      await this.host.dhService.enableSinkAudioBounded(device.networkId, 'mic',
        AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
    } else if (skipSinkEnable) {
      DhDiag.info('AUDIO', `selectInputDeviceForRecording skip enableSink ${device.deviceName}`);
    }
    if (!this.host.isReady()) {
      return false;
    }
    try {
      const routed = await this.trySelectRoutingDevice(device, true);
      if (!routed && HardwareManagerAdapter.isKitAvailable() &&
        device.sourceType === AudioSourceType.DISTRIBUTED) {
        this.st.lastRouteError = 'selectInputDeviceForRecording 失败';
        return false;
      }
      this.st.routeConfig.inputDevice = device;
      DhDiag.info('AUDIO', `selectInputDeviceForRecording OK ${device.deviceName}`);
      return true;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'selectInputDeviceForRecording failed: %{public}s',
        JSON.stringify(err));
      return false;
    }
  }

  public async selectInputDevice(device: AudioDeviceInfo): Promise<boolean> {
    hilog.info(AppConstants.LOG_DOMAIN, TAG, 'selectInputDevice: %{public}s', device.deviceName);
    if (device.sourceType === AudioSourceType.DISTRIBUTED && device.networkId) {
      await this.host.dhService.enableSinkAudioBounded(device.networkId, 'mic',
        AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
    }
    if (!this.host.isReady()) {
      return false;
    }
    try {
      const routed = await this.trySelectRoutingDevice(device, true);
      if (!routed && HardwareManagerAdapter.isKitAvailable() &&
        device.sourceType === AudioSourceType.DISTRIBUTED) {
        this.st.lastRouteError = 'selectInputDevice 失败，请确认系统签名与 DH 权限';
        return false;
      }
      await this.host.pipelines.resetInputPipeline();
      await this.host.pipelines.ensureCapturer();
      this.st.routeConfig.inputDevice = device;
      return true;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'selectInputDevice failed: %{public}s', JSON.stringify(err));
      return false;
    }
  }

  public async trySelectRoutingDevice(device: AudioDeviceInfo, isInput: boolean): Promise<boolean> {
    const desc = this.st.deviceDescriptorCache.get(device.deviceId);
    if (!desc || !this.st.routingManager) {
      return !HardwareManagerAdapter.isKitAvailable() ||
        device.sourceType !== AudioSourceType.DISTRIBUTED;
    }
    const rm = this.st.routingManager as AudioRoutingManagerExt;
    const needSysApi = HardwareManagerAdapter.isKitAvailable() &&
      device.sourceType === AudioSourceType.DISTRIBUTED;
    try {
      if (isInput && rm.selectInputDevice) {
        await rm.selectInputDevice([desc]);
        DhDiag.info('AUDIO',
          `selectInputDevice OK id=${desc.id} net=${(desc as AudioDescriptorExt).networkId ?? ''}`);
        return true;
      }
      if (!isInput && rm.selectOutputDevice) {
        await rm.selectOutputDevice([desc]);
        DhDiag.info('AUDIO',
          `selectOutputDevice OK id=${desc.id} net=${(desc as AudioDescriptorExt).networkId ?? ''}`);
        return true;
      }
      if (needSysApi) {
        DhDiag.error('AUDIO', 'routingManager 无 selectInput/Output 方法');
        return false;
      }
      return true;
    } catch (err) {
      const biz = err as BusinessError;
      DhDiag.warn('AUDIO', `routing select failed code=${biz?.code ?? 'n/a'} ${biz?.message ?? ''}`);
      return !needSysApi;
    }
  }
}
