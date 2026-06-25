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
import { AppConstants } from '../../common/constants/AppConstants';
import { AudioDeviceInfo, AudioDeviceKind, AudioSourceType } from '../../common/types/AudioTypes';
import { isLocalDeviceId, TrustDeviceInfo } from '../../common/types/DeviceTypes';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { DeviceManagerService } from '../DeviceManagerService';

/** 音频路由静态工具 */
export class AudioRouteHelpers {
  public static readonly SAMPLE_RATE: number = 48000;
  public static readonly TONE_HZ: number = 440;
  public static readonly TONE_SEC: number = 0.35;

  public static delay(ms: number): Promise<void> {
    return new Promise(resolve => setTimeout(resolve, ms));
  }

  /** 供 MusicPlayback 等模块匹配 networkId / address */
  public static networkIdMatches(a: string | undefined, b: string | undefined): boolean {
    if (!a || !b) {
      return false;
    }
    if (a === b) {
      return true;
    }
    const minLen = 8;
    if (a.length >= minLen && b.length >= minLen) {
      return a.startsWith(b.substring(0, minLen)) || b.startsWith(a.substring(0, minLen));
    }
    return a.includes(b) || b.includes(a);
  }

  /** AudioKit 分布式设备可见：networkId/设备名匹配，或系统包仅一台对端时认 distributed=1 */
  public static distributedDeviceVisible(list: AudioDeviceInfo[], networkId: string,
    peer?: TrustDeviceInfo): boolean {
    const dist = list.filter(d => d.sourceType === AudioSourceType.DISTRIBUTED);
    if (dist.length === 0) {
      return false;
    }
    if (dist.some(d => AudioRouteHelpers.networkIdMatches(d.networkId, networkId) ||
      AudioRouteHelpers.networkIdMatches(d.address, networkId))) {
      return true;
    }
    if (peer && dist.some(d => d.deviceName.includes(peer.deviceName))) {
      return true;
    }
    if (HardwareManagerAdapter.isKitAvailable()) {
      const remotes = DeviceManagerService.getInstance().getBoundRemoteDevices()
        .filter(d => !isLocalDeviceId(d.deviceId));
      if (remotes.length === 1) {
        return true;
      }
    }
    return false;
  }

  public static expandMonoToStereo(mono: ArrayBuffer): ArrayBuffer {
    const inView = new DataView(mono);
    const sampleCount = mono.byteLength / 2;
    const stereo = new ArrayBuffer(sampleCount * 4);
    const outView = new DataView(stereo);
    for (let i = 0; i < sampleCount; i++) {
      const sample = inView.getInt16(i * 2, true);
      outView.setInt16(i * 4, sample, true);
      outView.setInt16(i * 4 + 2, sample, true);
    }
    return stereo;
  }

  public static probeRetries(): number {
    return HardwareManagerAdapter.isKitAvailable() ?
      AppConstants.REMOTE_AUDIO_PROBE_RETRIES_SYSTEM :
      AppConstants.REMOTE_AUDIO_PROBE_RETRIES;
  }

  public static probeIntervalMs(): number {
    return HardwareManagerAdapter.isKitAvailable() ?
      AppConstants.REMOTE_AUDIO_PROBE_INTERVAL_MS_SYSTEM :
      AppConstants.REMOTE_AUDIO_PROBE_INTERVAL_MS;
  }

  public static distributedDeviceFlag(kind: AudioDeviceKind): audio.DeviceFlag {
    const flag = kind === AudioDeviceKind.MIC ?
      AppConstants.AUDIO_FLAG_DISTRIBUTED_INPUT :
      AppConstants.AUDIO_FLAG_DISTRIBUTED_OUTPUT;
    return flag as audio.DeviceFlag;
  }

  public static streamLevelToPercent(current: number, min: number, max: number): number {
    if (max <= min) {
      return current;
    }
    return Math.round((current - min) / (max - min) * 100);
  }
}
