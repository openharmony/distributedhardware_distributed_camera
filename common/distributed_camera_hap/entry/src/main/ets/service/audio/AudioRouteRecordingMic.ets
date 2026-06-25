import { AppConstants } from '../../common/constants/AppConstants';
import { AudioSourceType } from '../../common/types/AudioTypes';
import { AudioRoutePreset, AudioRouteTarget } from '../../common/types/CreationSessionTypes';
import { isLocalDeviceId } from '../../common/types/DeviceTypes';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { DeviceManagerService } from '../DeviceManagerService';
import { PeerSignalService } from '../PeerSignalService';
import { DhDiag } from '../../common/util/DhDiag';
import { AudioRouteHelpers } from './AudioRouteHelpers';
import { AudioRouteState } from './AudioRouteState';
import { EndRecordingRemoteMicOptions, RemoteRecordingAudioOptions } from './AudioRouteTypes';
import { AudioRouteService } from '../AudioRouteService';

/** 远端录像麦 enable / 等待 / 停录恢复 */
export class AudioRouteRecordingMic {
  private readonly host: AudioRouteService;

  constructor(host: AudioRouteService) {
    this.host = host;
  }

  private get st(): AudioRouteState { return this.host.state; }

  public isRemoteMicRouted(networkId: string): boolean {
    const input = this.st.routeConfig.inputDevice;
    if (!input || input.sourceType !== AudioSourceType.DISTRIBUTED) {
      return false;
    }
    if (AudioRouteHelpers.networkIdMatches(input.networkId, networkId) ||
      AudioRouteHelpers.networkIdMatches(input.address, networkId)) {
      return true;
    }
    const remotes = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .filter(d => !isLocalDeviceId(d.deviceId));
    return remotes.length === 1 &&
      AudioRouteHelpers.networkIdMatches(remotes[0].networkId, networkId);
  }

  public hasRecordingMicSetupInFlight(networkId: string): boolean {
    return AudioRouteHelpers.networkIdMatches(this.st.recordingMicSetupNetId, networkId) &&
      this.st.recordingMicSetupPromise !== null;
  }

  public async waitForRecordingMicRoute(networkId: string, maxMs: number): Promise<boolean> {
    if (!networkId) {
      return false;
    }
    if (this.isRemoteMicRouted(networkId)) {
      return true;
    }
    const setup = AudioRouteHelpers.networkIdMatches(this.st.recordingMicSetupNetId, networkId) ?
      this.st.recordingMicSetupPromise : null;
    const deadline = Date.now() + maxMs;
    while (Date.now() < deadline) {
      if (this.isRemoteMicRouted(networkId)) {
        DhDiag.info('AUDIO', 'waitForRecordingMicRoute OK');
        return true;
      }
      if (setup) {
        await Promise.race([setup, AudioRouteHelpers.delay(200)]);
      } else {
        await AudioRouteHelpers.delay(150);
      }
    }
    const pending = AudioRouteHelpers.networkIdMatches(this.st.recordingMicSetupNetId, networkId) ?
      this.st.recordingMicSetupPromise : null;
    if (pending && !this.isRemoteMicRouted(networkId)) {
      DhDiag.info('AUDIO', 'waitForRecordingMicRoute — await in-flight mic setup');
      await pending;
      if (this.isRemoteMicRouted(networkId)) {
        return true;
      }
    }
    if (!this.isRemoteMicRouted(networkId) && !this.hasRecordingMicSetupInFlight(networkId)) {
      DhDiag.warn('AUDIO', `waitForRecordingMicRoute timeout ${maxMs}ms — quick bind once`);
      await this.tryQuickBindDistributedMic(networkId);
    }
    return this.isRemoteMicRouted(networkId);
  }

  public async tryQuickBindDistributedMic(networkId: string): Promise<boolean> {
    if (!networkId || !this.host.isReady()) {
      return false;
    }
    const peer = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .find(d => AudioRouteHelpers.networkIdMatches(d.networkId, networkId));
    const inputs = await this.host.enumerateInputDevices();
    const picked = this.host.deviceOps.pickDevice(inputs, AudioRouteTarget.REMOTE, networkId, peer);
    if (!picked || picked.sourceType !== AudioSourceType.DISTRIBUTED) {
      return false;
    }
    DhDiag.info('AUDIO', `tryQuickBindDistributedMic ${picked.deviceName}`);
    const bindTask = this.host.selectInputDeviceForRecording(picked);
    const timeout = new Promise<boolean>((resolve) => {
      setTimeout(() => resolve(false), AppConstants.REMOTE_MIC_QUICK_BIND_TIMEOUT_MS);
    });
    const ok = await Promise.race([bindTask, timeout]);
    if (!ok) {
      DhDiag.warn('AUDIO',
        `tryQuickBindDistributedMic timeout/fail ${AppConstants.REMOTE_MIC_QUICK_BIND_TIMEOUT_MS}ms`);
    }
    return ok;
  }

  public cancelRecordingRemoteMicSetup(): void {
    this.st.recordingMicEpoch++;
    this.st.recordingRemoteMicNetId = '';
    this.st.recordingAlsoEnabledSpeaker = false;
    this.st.recordingMicSetupPromise = null;
    this.st.recordingMicSetupNetId = '';
    DhDiag.info('AUDIO', `cancelRecordingRemoteMicSetup epoch=${this.st.recordingMicEpoch}`);
  }

  private isRecordingMicEpochStale(epoch: number): boolean {
    return this.st.recordingMicEpoch !== epoch;
  }

  public async beginRecordingRemoteMic(networkId: string, peerDeviceName?: string): Promise<boolean> {
    if (!networkId) {
      return false;
    }
    const epoch = this.st.recordingMicEpoch;
    DhDiag.info('AUDIO', `beginRecordingRemoteMic net=${networkId.substring(0, 12)}... epoch=${epoch}`);
    try {
      if (this.isRecordingMicEpochStale(epoch)) {
        return false;
      }
      await this.host.previewOps.prepareExclusiveMicForAvRecorder();
      if (this.isRecordingMicEpochStale(epoch)) {
        return false;
      }
      if (this.isRemoteMicRouted(networkId)) {
        this.st.recordingRemoteMicNetId = networkId;
        return true;
      }
      if (peerDeviceName && peerDeviceName.length > 0) {
        PeerSignalService.getInstance().publishMicRequestOnce(peerDeviceName);
      }
      if (this.isRecordingMicEpochStale(epoch)) {
        return false;
      }
      let setup = this.hasRecordingMicSetupInFlight(networkId) ? this.st.recordingMicSetupPromise : null;
      if (!setup) {
        this.st.recordingMicSetupNetId = networkId;
        this.st.recordingMicSetupPromise = this.ensureRecordingMicInputOnly(networkId,
          AppConstants.MUSIC_ENABLE_SINK_PROBE_RETRIES);
        setup = this.st.recordingMicSetupPromise;
      }
      const ok = await setup;
      if (this.isRecordingMicEpochStale(epoch)) {
        DhDiag.info('AUDIO', 'beginRecordingRemoteMic aborted — stop/cancel won');
        await this.host.dhService.stopAudio(networkId, 'mic');
        return false;
      }
      if (ok) {
        this.st.recordingRemoteMicNetId = networkId;
      } else {
        DhDiag.warn('AUDIO', 'beginRecordingRemoteMic — 对端麦未就绪，录像可能无对端音轨');
      }
      return ok;
    } catch (err) {
      DhDiag.logCatchWarn('AUDIO', 'beginRecordingRemoteMic', err);
      return false;
    }
  }

  public async warmRemoteMicForRecording(networkId: string, peerDeviceName?: string): Promise<boolean> {
    if (!networkId || this.st.recordMicLockedForAvRecorder || this.isRecordingRemoteMicSession()) {
      return this.isRemoteMicRouted(networkId);
    }
    if (this.isRemoteMicRouted(networkId)) {
      return true;
    }
    if (this.hasRecordingMicSetupInFlight(networkId) && this.st.recordingMicSetupPromise) {
      return await this.st.recordingMicSetupPromise;
    }
    if (peerDeviceName && peerDeviceName.length > 0) {
      PeerSignalService.getInstance().publishMicRequestOnce(peerDeviceName);
    }
    this.st.recordingMicSetupNetId = networkId;
    this.st.recordingMicSetupPromise = this.ensureRecordingMicInputOnly(networkId,
      AppConstants.REMOTE_AUDIO_PROBE_RETRIES_FAST);
    const ok = await this.st.recordingMicSetupPromise;
    DhDiag.info('AUDIO-RECORD', `warmRemoteMicForRecording ok=${ok ? 1 : 0}`);
    return ok;
  }

  public async endRecordingRemoteMic(options?: EndRecordingRemoteMicOptions): Promise<void> {
    this.st.recordingMicEpoch++;
    this.st.recordingMicSetupPromise = null;
    this.st.recordingMicSetupNetId = '';
    const netId = this.st.recordingRemoteMicNetId;
    this.st.recordingRemoteMicNetId = '';
    const openedSpeaker = this.st.recordingAlsoEnabledSpeaker;
    this.st.recordingAlsoEnabledSpeaker = false;

    const input = this.st.routeConfig.inputDevice;
    let closeNet: string = netId;
    if (closeNet.length === 0 && options?.remoteNetworkId) {
      closeNet = options.remoteNetworkId;
    }
    if (closeNet.length === 0 && input?.sourceType === AudioSourceType.DISTRIBUTED) {
      closeNet = input.networkId ?? input.address ?? '';
    }

    const restorePreset = options?.restorePreset ??
      { input: AudioRouteTarget.LOCAL, output: AudioRouteTarget.LOCAL };
    const routeRemoteNet = (options?.remoteNetworkId && options.remoteNetworkId.length > 0) ?
      options.remoteNetworkId : (closeNet.length > 0 ? closeNet : undefined);

    try {
      const hadRecordMic = netId.length > 0 ||
        input?.sourceType === AudioSourceType.DISTRIBUTED;
      if (closeNet.length > 0 && closeNet !== 'local') {
        if (hadRecordMic) {
          DhDiag.info('AUDIO', `endRecordingRemoteMic stop DH mic net=${closeNet.substring(0, 12)}...`);
          await this.host.dhService.stopAudio(closeNet, 'mic');
        }
        if (openedSpeaker && restorePreset.output !== AudioRouteTarget.REMOTE) {
          DhDiag.info('AUDIO', `endRecordingRemoteMic stop DH speaker net=${closeNet.substring(0, 12)}...`);
          await this.host.dhService.stopAudio(closeNet, 'speaker');
        }
      }

      const needRestore = hadRecordMic || options?.restorePreset !== undefined;
      if (needRestore) {
        DhDiag.info('AUDIO', 'endRecordingRemoteMic — restore route after camera stop');
        await this.host.previewOps.stopPreviewMonitor();
        const skipDhEnable = restorePreset.input !== AudioRouteTarget.REMOTE &&
          restorePreset.output !== AudioRouteTarget.REMOTE;
        const discoverRetries = (restorePreset.input === AudioRouteTarget.REMOTE ||
          restorePreset.output === AudioRouteTarget.REMOTE) ?
          AppConstants.REMOTE_AUDIO_PROBE_RETRIES_FAST : 2;
        await this.host.applyRouteForSession(restorePreset, routeRemoteNet, {
          allowInputFallback: !HardwareManagerAdapter.isKitAvailable(),
          allowOutputFallback: !HardwareManagerAdapter.isKitAvailable(),
          skipDhEnable: skipDhEnable,
          discoverMaxRetries: discoverRetries,
          discoverIntervalMs: AppConstants.REMOTE_AUDIO_PROBE_INTERVAL_MS_FAST
        });
        DhDiag.info('AUDIO',
          `endRecordingRemoteMic — 已恢复场景声场 in=${restorePreset.input} out=${restorePreset.output}`);
      }
    } catch (err) {
      DhDiag.logCatchWarn('AUDIO', 'endRecordingRemoteMic', err);
      try {
        await this.host.previewOps.stopPreviewMonitor();
      } catch (stopErr) {
        DhDiag.logCatchWarn('AUDIO', 'endRecordingRemoteMic stopPreviewMonitor', stopErr);
      }
    }
  }

  public isRecordingRemoteMicSession(): boolean {
    return this.st.recordingRemoteMicNetId.length > 0;
  }

  public async lockRecordMicForAvRecorder(lock: boolean): Promise<void> {
    this.st.recordMicLockedForAvRecorder = lock;
    try {
      if (lock) {
        this.st.previewMonitorSuppressed = true;
        await this.host.previewOps.prepareExclusiveMicForAvRecorder();
        DhDiag.info('AUDIO-RECORD', 'lockRecordMic — mic reserved for AVRecorder');
      } else {
        this.st.previewMonitorSuppressed = false;
        DhDiag.info('AUDIO-RECORD', 'unlockRecordMic — preview may restart');
      }
    } catch (err) {
      DhDiag.logCatchWarn('AUDIO-RECORD', 'lockRecordMicForAvRecorder', err);
    }
  }

  public isRecordMicLockedForAvRecorder(): boolean {
    return this.st.recordMicLockedForAvRecorder;
  }

  public async reassertRecordingMicRoute(networkId: string): Promise<boolean> {
    const input = this.st.routeConfig.inputDevice;
    if (!input || input.sourceType !== AudioSourceType.DISTRIBUTED) {
      DhDiag.warn('AUDIO-RECORD', 'reassertRecordingMic skip — no distributed input');
      return false;
    }
    const ok = await this.host.selectInputDeviceForRecording(input);
    DhDiag.info('AUDIO-RECORD', `reassertRecordingMic ${input.deviceName} ok=${ok ? 1 : 0}`);
    return ok && this.isRemoteMicRouted(networkId);
  }

  public async ensureRemoteRecordingAudio(networkId: string,
    options?: RemoteRecordingAudioOptions): Promise<boolean> {
    if (!networkId) {
      this.st.lastRouteError = '缺少对端 networkId';
      return false;
    }
    const micOnly = options?.micOnly !== false;
    const inflightKey = `${networkId}:${micOnly ? 1 : 0}`;
    const existing = this.st.ensureRemoteRecordingInflight.get(inflightKey);
    if (existing) {
      DhDiag.info('AUDIO', 'ensureRemoteRecordingAudio join in-flight');
      return existing;
    }
    const task = this.ensureRemoteRecordingAudioInner(networkId, options);
    this.st.ensureRemoteRecordingInflight.set(inflightKey, task);
    try {
      return await task;
    } finally {
      this.st.ensureRemoteRecordingInflight.delete(inflightKey);
    }
  }

  private async ensureRemoteRecordingAudioInner(networkId: string,
    options?: RemoteRecordingAudioOptions): Promise<boolean> {
    if (!this.host.isReady()) {
      const ok = await this.host.init();
      if (!ok) {
        return false;
      }
    }

    const micOnly = options?.micOnly !== false;
    if (!micOnly) {
      this.st.recordingAlsoEnabledSpeaker = true;
    }
    if (micOnly && this.isRemoteMicRouted(networkId)) {
      DhDiag.info('AUDIO', 'ensureRemoteRecordingAudio skip — remote mic already routed');
      this.st.lastRouteError = '';
      return true;
    }

    const fastProbe = HardwareManagerAdapter.isKitAvailable();
    const probeRetries = options?.probeRetries ?? (fastProbe ?
      AppConstants.REMOTE_AUDIO_PROBE_RETRIES_FAST : AppConstants.REMOTE_AUDIO_PROBE_RETRIES);
    DhDiag.info('AUDIO', `ensureRemoteRecordingAudio start net=${networkId.substring(0, 12)}... micOnly=${micOnly ? 1 : 0}`);

    await this.host.dhService.enableSinkAudioBounded(networkId, 'mic', AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
    if (!micOnly) {
      await this.host.dhService.enableSinkAudioBounded(networkId, 'speaker', AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
    }
    await this.host.waitForDistributedAudio(networkId, 'mic', probeRetries);
    if (!micOnly) {
      await this.host.waitForDistributedAudio(networkId, 'speaker', probeRetries);
    }

    if (micOnly) {
      const micOk = await this.ensureRecordingMicInputOnly(networkId, probeRetries);
      if (micOk && this.st.routeConfig.inputDevice?.sourceType === AudioSourceType.DISTRIBUTED) {
        DhDiag.info('AUDIO',
          `ensureRemoteRecordingAudio OK in=${this.st.routeConfig.inputDevice.deviceName} (input-only)`);
        this.st.lastRouteError = '';
        return true;
      }
      if (micOk && this.st.routeConfig.inputDevice?.sourceType === AudioSourceType.LOCAL) {
        this.st.lastRouteError = '对端 Mic 不可用，录像已降级本机麦克风';
        DhDiag.warn('AUDIO', this.st.lastRouteError);
        return true;
      }
      DhDiag.error('AUDIO', `ensureRemoteRecordingAudio failed ${this.st.lastRouteError}`);
      return false;
    }

    const audioPreset: AudioRoutePreset = { input: AudioRouteTarget.REMOTE, output: AudioRouteTarget.REMOTE };
    const systemDh = HardwareManagerAdapter.isKitAvailable();
    const ok = await this.host.applyRouteForSession(audioPreset, networkId, {
      allowInputFallback: !systemDh,
      allowOutputFallback: !systemDh,
      skipDhEnable: true,
      discoverMaxRetries: AppConstants.REMOTE_AUDIO_PROBE_RETRIES_FAST,
      discoverIntervalMs: AppConstants.REMOTE_AUDIO_PROBE_INTERVAL_MS_FAST
    });

    if (ok && this.st.routeConfig.inputDevice?.sourceType === AudioSourceType.DISTRIBUTED) {
      DhDiag.info('AUDIO',
        `ensureRemoteRecordingAudio OK in=${this.st.routeConfig.inputDevice.deviceName} out=${this.st.routeConfig.outputDevice?.deviceName ?? 'n/a'}`);
      this.st.lastRouteError = '';
      return true;
    }
    if (ok && this.st.routeConfig.inputDevice?.sourceType === AudioSourceType.LOCAL) {
      this.st.lastRouteError = '对端 Mic 不可用，录像已降级本机麦克风';
      DhDiag.warn('AUDIO', this.st.lastRouteError);
      return true;
    }
    DhDiag.error('AUDIO', `ensureRemoteRecordingAudio failed ${this.st.lastRouteError}`);
    return false;
  }

  private async ensureRecordingMicInputOnly(networkId: string, probeRetries?: number): Promise<boolean> {
    if (!networkId) {
      this.st.lastRouteError = '缺少对端 networkId';
      return false;
    }
    if (this.isRemoteMicRouted(networkId)) {
      return true;
    }
    if (!this.host.isReady()) {
      const ok = await this.host.init();
      if (!ok) {
        return false;
      }
    }
    const peer = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .find(d => AudioRouteHelpers.networkIdMatches(d.networkId, networkId));
    let inputs = await this.host.enumerateInputDevices();
    let picked = this.host.deviceOps.pickDevice(inputs, AudioRouteTarget.REMOTE, networkId, peer);
    const sinkPrepared = this.host.dhService.isSinkAudioPrepared(networkId, 'mic');
    if (picked?.sourceType === AudioSourceType.DISTRIBUTED && sinkPrepared) {
      DhDiag.info('AUDIO-RECORD', 'ensureRecordingMic — dist mic visible + sink prepared');
    } else {
      if (picked?.sourceType === AudioSourceType.DISTRIBUTED && !sinkPrepared) {
        DhDiag.warn('AUDIO-RECORD', 'dist mic visible but sink NOT prepared — enableSink mic');
      }
      await this.host.dhService.enableSinkAudioBounded(networkId, 'mic', AppConstants.RECORD_ENABLE_SINK_TIMEOUT_MS);
      const retries = probeRetries ?? AppConstants.REMOTE_AUDIO_PROBE_RETRIES_FAST;
      await this.host.waitForDistributedAudio(networkId, 'mic', retries);
      inputs = await this.host.enumerateInputDevices();
      picked = this.host.deviceOps.pickDevice(inputs, AudioRouteTarget.REMOTE, networkId, peer);
    }
    if ((!picked || picked.sourceType !== AudioSourceType.DISTRIBUTED) && HardwareManagerAdapter.isKitAvailable()) {
      const allowFb = true;
      if (allowFb) {
        picked = this.host.deviceOps.pickDevice(inputs, AudioRouteTarget.LOCAL, networkId, peer);
        if (picked) {
          DhDiag.warn('AUDIO', 'ensureRecordingMicInputOnly — 降级本机 Mic');
          this.st.lastRouteError = '对端 Mic 不可用，录像已降级本机麦克风';
        }
      }
    }
    if (!picked) {
      this.st.lastRouteError = '未找到对端 Mic';
      return false;
    }
    const inOk = await this.host.selectInputDeviceForRecording(picked);
    if (inOk && picked.sourceType === AudioSourceType.DISTRIBUTED) {
      DhDiag.info('AUDIO-RECORD',
        `ensureRecordingMic OK in=${picked.deviceName} sink=${this.host.dhService.isSinkAudioPrepared(networkId, 'mic') ? 1 : 0}`);
      this.st.lastRouteError = '';
      return true;
    }
    return inOk;
  }

  public isRecordingRemoteMicActive(): boolean {
    return this.st.recordingRemoteMicNetId.length > 0 || this.st.recordMicLockedForAvRecorder;
  }
}
