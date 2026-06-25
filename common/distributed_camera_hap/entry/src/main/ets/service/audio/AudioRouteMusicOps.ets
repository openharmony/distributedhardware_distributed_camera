import { hilog } from '@kit.PerformanceAnalysisKit';
import { AppConstants } from '../../common/constants/AppConstants';
import { AudioDeviceInfo, AudioSourceType } from '../../common/types/AudioTypes';
import { isLocalDeviceId, TrustDeviceInfo } from '../../common/types/DeviceTypes';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { DeviceManagerService } from '../DeviceManagerService';
import { PeerSignalService } from '../PeerSignalService';
import { DhDiag } from '../../common/util/DhDiag';
import { AudioRouteHelpers } from './AudioRouteHelpers';
import { AudioRouteState } from './AudioRouteState';
import { RouteSpeakerForMusicOptions } from './AudioRouteTypes';
import { AudioRouteService } from '../AudioRouteService';

const TAG = AppConstants.TAG;

/** 播歌 / 演示录音 / 边录边听路由 */
export class AudioRouteMusicOps {
  private readonly host: AudioRouteService;

  constructor(host: AudioRouteService) {
    this.host = host;
  }

  private get st(): AudioRouteState { return this.host.state; }

  public maybePrewarmRemoteSpeaker(_inputs: AudioDeviceInfo[]): void {
  }

  public isSpeakerRouting(): boolean {
    return this.host.dhService.isSpeakerRouting();
  }

  public async routeSpeakerForMusic(playLocal: boolean, remoteNetworkId?: string,
    opts?: RouteSpeakerForMusicOptions): Promise<boolean> {
    this.host.dhService.setSpeakerRoutingActive(true);
    try {
      return await this.routeSpeakerForMusicInner(playLocal, remoteNetworkId, opts);
    } finally {
      this.host.dhService.setSpeakerRoutingActive(false);
    }
  }

  private async routeSpeakerForMusicInner(playLocal: boolean, remoteNetworkId?: string,
    opts?: RouteSpeakerForMusicOptions): Promise<boolean> {
    DhDiag.info('AUDIO-MUSIC',
      `routeSpeakerForMusic begin local=${playLocal ? 1 : 0} net=${remoteNetworkId?.substring(0, 12) ?? 'n/a'}... releaseMic=${opts?.forceReleaseMic ? 1 : 0}`);
    let peer: TrustDeviceInfo | undefined = undefined;
    let micDhWarm = opts?.forceReleaseMic === true;
    if (!playLocal && remoteNetworkId) {
      peer = DeviceManagerService.getInstance().getBoundRemoteDevices()
        .find(d => !isLocalDeviceId(d.deviceId) &&
          AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId));
    }
    await this.host.previewOps.prepareForMusicPlayback();
    const outputs = await this.host.enumerateOutputDevices();
    let target: AudioDeviceInfo | null = null;
    if (playLocal) {
      const locals = outputs.filter(d => d.sourceType === AudioSourceType.LOCAL);
      const ordered = this.orderLocalSpeakersForMusic(locals);
      for (const candidate of ordered) {
        if (await this.host.deviceOps.selectOutputDeviceForMusic(candidate)) {
          target = candidate;
          break;
        }
        DhDiag.warn('AUDIO', `routeSpeakerForMusic local try failed name=${candidate.deviceName} id=${candidate.nativeDeviceId ?? -1}`);
      }
      if (!target) {
        DhDiag.warn('AUDIO', 'routeSpeakerForMusic local — no selectOutput success, AVPlayer default');
        return true;
      }
    } else if (remoteNetworkId) {
      if (!peer) {
        peer = DeviceManagerService.getInstance().getBoundRemoteDevices()
          .find(d => !isLocalDeviceId(d.deviceId) &&
            AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId));
      }
      const preInputs = await this.host.enumerateInputDevices();
      micDhWarm = micDhWarm || this.host.deviceOps.pickDistributedMic(preInputs, remoteNetworkId, peer) !== null;
      target = this.host.deviceOps.pickDistributedSpeaker(outputs, remoteNetworkId, peer);
      const sinkNet = remoteNetworkId.length > 0 ? remoteNetworkId :
        (target?.networkId ?? target?.address ?? '');
      if (sinkNet.length > 0 && sinkNet !== 'local' && !sinkNet.toLowerCase().includes('localdevice')) {
        if (target?.sourceType === AudioSourceType.DISTRIBUTED) {
          if (!this.host.dhService.isSinkAudioPrepared(sinkNet, 'speaker')) {
            void this.host.dhService.enableSinkAudioBounded(sinkNet, 'speaker',
              AppConstants.MUSIC_ENABLE_SINK_BACKGROUND_MS);
          }
          DhDiag.info('AUDIO-MUSIC', 'dist speaker visible — selectOutput first, enableSink in background');
        } else {
          target = await this.warmRemoteSpeakerForMusic(sinkNet, remoteNetworkId, peer, micDhWarm);
        }
      } else {
        DhDiag.warn('AUDIO-MUSIC', 'routeSpeakerForMusic skip enableSink — no sink networkId');
      }
    }
    if (!target) {
      this.st.lastRouteError = playLocal ? '未枚举到本机 Speaker' :
        '未枚举到对端 Speaker · 请确认双机同账号在线后重试';
      DhDiag.warn('AUDIO', `routeSpeakerForMusic miss local=${playLocal ? 1 : 0}`);
      if (!playLocal) {
        PeerSignalService.getInstance().stopHeartbeat();
      }
      return playLocal;
    }
    if (playLocal) {
      DhDiag.info('AUDIO', `routeSpeakerForMusic done ok=1 out=${target.deviceName}`);
      return true;
    }
    DhDiag.info('AUDIO', `routeSpeakerForMusic target=${target.deviceName} dist=${target.sourceType === AudioSourceType.DISTRIBUTED ? 1 : 0}`);
    const ok = await this.host.deviceOps.selectOutputDeviceForMusic(target);
    const sinkNet = remoteNetworkId ?? target.networkId ?? '';
    const sinkPrepared = sinkNet.length > 0 && this.host.dhService.isSinkAudioPrepared(sinkNet, 'speaker');
    if (!ok) {
      DhDiag.warn('AUDIO-MUSIC', 'selectOutput failed');
      return false;
    }
    if (!sinkPrepared) {
      this.st.lastRouteError = '扬声器路由已切换，DH 仍在后台就绪（若无声请稍后再点一次）';
      DhDiag.warn('AUDIO-MUSIC', `routeSpeakerForMusic select OK but sink warming net=${sinkNet.substring(0, 12)}...`);
    } else {
      this.st.lastRouteError = '';
    }
    DhDiag.info('AUDIO-MUSIC',
      `routeSpeakerForMusic done out=${target.deviceName} select=1 sinkPrepared=${sinkPrepared ? 1 : 0}`);
    PeerSignalService.getInstance().stopHeartbeat();
    return true;
  }

  private async warmRemoteSpeakerForMusic(sinkNet: string, remoteNetworkId: string,
    peer?: TrustDeviceInfo, micDhWarm: boolean = false): Promise<AudioDeviceInfo | null> {
    const peerName = peer?.deviceName ?? '';
    DhDiag.info('AUDIO-MUSIC',
      `warmRemoteSpeaker micWarm=${micDhWarm ? 1 : 0} kit=${HardwareManagerAdapter.isKitAvailable() ? 1 : 0}`);
    await this.host.dhService.ensureRemoteSpeakerForMusic(sinkNet, peerName, micDhWarm, 'playback');
    const found = await this.host.waitForDistributedAudio(remoteNetworkId, 'speaker',
      AppConstants.MUSIC_SPEAKER_PROBE_RETRIES,
      AppConstants.MUSIC_SPEAKER_PROBE_INTERVAL_MS);
    if (!found) {
      DhDiag.warn('AUDIO-MUSIC', 'warmRemoteSpeaker waitForDistributedAudio miss');
      return null;
    }
    const outs = await this.host.enumerateOutputDevices();
    const target = this.host.deviceOps.pickDistributedSpeaker(outs, remoteNetworkId, peer);
    if (target) {
      DhDiag.info('AUDIO-MUSIC', 'warmRemoteSpeaker OK');
    }
    return target;
  }

  public async routeMicForRecording(useLocalMic: boolean, remoteNetworkId?: string): Promise<boolean> {
    DhDiag.info('AUDIO-RECORD',
      `routeMicForRecording begin local=${useLocalMic ? 1 : 0} net=${remoteNetworkId?.substring(0, 12) ?? 'n/a'}...`);
    const inputs = await this.host.enumerateInputDevices();
    let target: AudioDeviceInfo | null = null;
    if (useLocalMic) {
      const locals = inputs.filter(d => d.sourceType === AudioSourceType.LOCAL);
      target = locals[0] ?? null;
      if (!target) {
        this.st.lastRouteError = '未枚举到本机 Mic';
        DhDiag.warn('AUDIO', 'routeMicForRecording miss local mic');
        return false;
      }
    } else if (remoteNetworkId) {
      const peer = DeviceManagerService.getInstance().getBoundRemoteDevices()
        .find(d => !isLocalDeviceId(d.deviceId) &&
          AudioRouteHelpers.networkIdMatches(d.networkId, remoteNetworkId));
      target = this.host.deviceOps.pickDistributedMic(inputs, remoteNetworkId, peer);
      const sinkNet = remoteNetworkId.length > 0 ? remoteNetworkId :
        (target?.networkId ?? target?.address ?? '');
      if (sinkNet.length > 0 && !target) {
        DhDiag.info('AUDIO-RECORD', 'dist mic not in list — short probe');
        await this.host.dhService.enableSinkAudioBounded(sinkNet, 'mic', AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
        await this.host.waitForDistributedAudio(sinkNet, 'mic',
          AppConstants.MUSIC_ENABLE_SINK_PROBE_RETRIES,
          AppConstants.REMOTE_AUDIO_PROBE_INTERVAL_MS_FAST);
        const inputsAfter = await this.host.enumerateInputDevices();
        target = this.host.deviceOps.pickDistributedMic(inputsAfter, remoteNetworkId, peer);
      } else if (target?.sourceType === AudioSourceType.DISTRIBUTED && sinkNet.length > 0) {
        void this.host.dhService.enableSinkAudioBounded(sinkNet, 'mic', AppConstants.MUSIC_ENABLE_SINK_BACKGROUND_MS);
      }
    }
    if (!target) {
      this.st.lastRouteError = useLocalMic ? '未枚举到本机 Mic' : '未枚举到对端 Mic';
      DhDiag.warn('AUDIO', `routeMicForRecording miss local=${useLocalMic ? 1 : 0}`);
      return false;
    }
    const ok = await this.host.selectInputDeviceForRecording(target);
    if (!ok) {
      this.st.lastRouteError = this.st.lastRouteError || '切换录音路由失败';
    }
    DhDiag.info('AUDIO', `routeMicForRecording done ok=${ok ? 1 : 0} in=${target.deviceName}`);
    return ok;
  }

  public async startLiveMonitor(micLocal: boolean, speakerLocal: boolean,
    remoteNetworkId?: string): Promise<boolean> {
    if (!AppConstants.SHOW_LIVE_MONITOR_UI) {
      this.st.lastRouteError = '边录边听功能已暂时关闭';
      return false;
    }
    await this.host.previewOps.prepareForMusicPlayback();
    const micOk = await this.routeMicForRecording(micLocal, micLocal ? undefined : remoteNetworkId);
    if (!micOk) {
      return false;
    }
    const spkOk = await this.routeSpeakerForMusic(speakerLocal, speakerLocal ? undefined : remoteNetworkId);
    if (!spkOk && !speakerLocal) {
      this.st.lastRouteError = this.st.lastRouteError || '扬声器路由失败';
      return false;
    }
    await this.host.previewOps.restartPreviewMonitor();
    if (!this.st.previewMonitorRunning) {
      this.st.lastRouteError = '返听启动失败，请检查 Mic/Speaker 权限与路由';
      return false;
    }
    DhDiag.info('AUDIO', `live monitor started micLocal=${micLocal ? 1 : 0} spkLocal=${speakerLocal ? 1 : 0}`);
    return true;
  }

  public async stopLiveMonitor(): Promise<void> {
    await this.host.previewOps.stopPreviewMonitor();
  }

  public async suspendForDistributedCamera(): Promise<void> {
    await this.stopLiveMonitor();
    this.host.previewOps.holdPreviewMonitor(15000);
    if (!this.host.recordingMicOps.isRecordingRemoteMicActive()) {
      await this.host.previewOps.prepareForMusicPlayback();
    } else {
      this.st.previewMonitorDesired = false;
      this.st.previewMonitorLoopId++;
      this.st.previewMonitorRunning = false;
      await this.host.previewOps.stopPreviewMonitorPipelines();
      DhDiag.info('AUDIO', 'suspendForDistributedCamera — skip full prepare, remote mic recording');
    }
    DhDiag.info('AUDIO', 'suspendForDistributedCamera done');
  }

  private orderLocalSpeakersForMusic(locals: AudioDeviceInfo[]): AudioDeviceInfo[] {
    const remotes = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .filter(d => !isLocalDeviceId(d.deviceId));
    const remoteNames = remotes.map(d => d.deviceName);
    const score = (d: AudioDeviceInfo): number => {
      let s = 0;
      const name = d.deviceName.toLowerCase();
      if (/speaker|扬声器|builtin|内置/.test(name)) {
        s += 100;
      }
      if (remoteNames.some(rn => d.deviceName.includes(rn))) {
        s -= 50;
      }
      const nid = (d.networkId ?? '').toLowerCase();
      if (nid.length === 0 || nid.includes('local')) {
        s += 20;
      }
      s -= (d.nativeDeviceId ?? 99);
      return s;
    };
    return [...locals].sort((a, b) => score(b) - score(a));
  }

  public async reapplyMusicOutputRoute(): Promise<boolean> {
    const out = this.st.routeConfig.outputDevice;
    if (!out || out.sourceType !== AudioSourceType.DISTRIBUTED) {
      return false;
    }
    return this.host.deviceOps.selectOutputDeviceForMusic(out);
  }
}
