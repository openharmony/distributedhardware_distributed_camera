import { audio } from '@kit.AudioKit';
import { AppConstants } from '../../common/constants/AppConstants';
import { AudioRouteTarget } from '../../common/types/CreationSessionTypes';
import { DhDiag } from '../../common/util/DhDiag';
import { AudioRouteHelpers } from './AudioRouteHelpers';
import { AudioRouteState } from './AudioRouteState';
import { AudioRouteService } from '../AudioRouteService';

/** 预览返听 Capturer → Renderer */
export class AudioRoutePreviewMonitor {
  private readonly host: AudioRouteService;

  constructor(host: AudioRouteService) {
    this.host = host;
  }

  private get st(): AudioRouteState { return this.host.state; }

  public async ensurePreviewMonitorForStudio(): Promise<void> {
    if (!AppConstants.SHOW_LIVE_MONITOR_UI) {
      return;
    }
    if (this.st.recordMicLockedForAvRecorder) {
      DhDiag.info('AUDIO-RECORD', 'ensurePreviewMonitor skip — recording locks mic');
      return;
    }
    if (this.isPreviewMonitorHeld()) {
      DhDiag.info('AUDIO', 'ensurePreviewMonitorForStudio skip — held for photo/music');
      return;
    }
    if (!this.host.isReady()) {
      const ok = await this.host.init();
      if (!ok) {
        return;
      }
    }
    if (!this.st.routeConfig.inputDevice) {
      await this.host.applyRouteForSession(
        { input: AudioRouteTarget.LOCAL, output: AudioRouteTarget.LOCAL },
        undefined,
        { skipDhEnable: true, discoverMaxRetries: 2, discoverIntervalMs: 200 });
      return;
    }
    await this.restartPreviewMonitor();
  }

  public async restartPreviewMonitor(): Promise<void> {
    if (!AppConstants.SHOW_LIVE_MONITOR_UI) {
      return;
    }
    if (this.st.recordMicLockedForAvRecorder) {
      DhDiag.info('AUDIO-RECORD', 'restartPreviewMonitor skip — recording');
      return;
    }
    await this.stopPreviewMonitor();
    await AudioRouteHelpers.delay(AppConstants.PREVIEW_MONITOR_RESTART_DELAY_MS);
    await this.startPreviewMonitor();
  }

  public async startPreviewMonitor(): Promise<void> {
    if (!AppConstants.SHOW_LIVE_MONITOR_UI) {
      return;
    }
    if (this.st.recordMicLockedForAvRecorder) {
      DhDiag.info('AUDIO-RECORD', 'startPreviewMonitor skip — recording');
      return;
    }
    if (!this.st.routeConfig.inputDevice || !this.st.routeConfig.outputDevice) {
      DhDiag.warn('AUDIO', 'startPreviewMonitor skip — 未配置输入/输出设备');
      return;
    }
    if (!this.host.isReady()) {
      const ok = await this.host.init();
      if (!ok) {
        return;
      }
    }
    this.st.previewMonitorDesired = true;
    const loopId = ++this.st.previewMonitorLoopId;
    try {
      await this.host.pipelines.ensureCapturer();
      await this.host.pipelines.ensureRenderer();
      const capturer = this.st.audioCapturer;
      const renderer = this.st.audioRenderer;
      if (!capturer || !renderer) {
        DhDiag.warn('AUDIO', 'startPreviewMonitor skip — Capturer/Renderer 未创建');
        return;
      }
      await capturer.start();
      await renderer.start();
      this.st.previewMonitorRunning = true;
      const inName = this.st.routeConfig.inputDevice.deviceName;
      const outName = this.st.routeConfig.outputDevice.deviceName;
      DhDiag.info('AUDIO', `preview monitor started in=${inName} out=${outName}`);
      void this.runPreviewMonitorLoop(loopId, capturer, renderer);
    } catch (err) {
      this.st.previewMonitorRunning = false;
      DhDiag.warn('AUDIO', `startPreviewMonitor failed ${JSON.stringify(err)}`);
    }
  }

  public async stopPreviewMonitor(): Promise<void> {
    this.st.previewMonitorDesired = false;
    this.st.previewMonitorLoopId++;
    this.st.previewMonitorRunning = false;
    await this.stopPreviewMonitorPipelines();
    DhDiag.info('AUDIO', 'preview monitor stopped');
  }

  public suppressPreviewMonitor(): void {
    this.st.previewMonitorSuppressed = true;
  }

  public resumePreviewMonitor(): void {
    this.st.previewMonitorSuppressed = false;
    this.st.previewMonitorHoldUntilMs = 0;
  }

  public holdPreviewMonitor(ms: number): void {
    this.st.previewMonitorSuppressed = true;
    this.st.previewMonitorHoldUntilMs = Date.now() + ms;
  }

  public isPreviewMonitorHeld(): boolean {
    return this.st.previewMonitorSuppressed || Date.now() < this.st.previewMonitorHoldUntilMs;
  }

  public async prepareForMusicPlayback(): Promise<void> {
    this.st.previewMonitorDesired = false;
    this.st.previewMonitorLoopId++;
    this.st.previewMonitorRunning = false;
    await this.stopPreviewMonitorPipelines();
    await this.host.pipelines.resetInputPipeline();
    await this.host.pipelines.resetOutputPipeline();
    DhDiag.info('AUDIO', 'prepareForMusicPlayback — preview pipelines released');
  }

  public async prepareExclusiveMicForAvRecorder(): Promise<void> {
    this.st.previewMonitorDesired = false;
    this.st.previewMonitorLoopId++;
    this.st.previewMonitorRunning = false;
    this.st.previewMonitorSuppressed = true;
    await this.stopPreviewMonitorPipelines();
    await this.host.pipelines.resetInputPipeline();
    DhDiag.info('AUDIO-RECORD', 'prepareExclusiveMicForAvRecorder — input capturer released');
  }

  public async stopPreviewMonitorPipelines(): Promise<void> {
    try {
      if (this.st.audioCapturer) {
        await this.st.audioCapturer.stop();
      }
    } catch (err) {
      DhDiag.logCatchWarn('AUDIO', 'stopPreviewMonitor capturer.stop', err);
    }
    try {
      if (this.st.audioRenderer) {
        await this.st.audioRenderer.stop();
      }
    } catch (err) {
      DhDiag.logCatchWarn('AUDIO', 'stopPreviewMonitor renderer.stop', err);
    }
  }

  public isPreviewMonitorActive(): boolean {
    return this.st.previewMonitorDesired && this.st.previewMonitorRunning;
  }

  public isLiveMonitorActive(): boolean {
    return this.st.previewMonitorRunning;
  }

  private async runPreviewMonitorLoop(loopId: number, capturer: audio.AudioCapturer,
    renderer: audio.AudioRenderer): Promise<void> {
    const readBytes = AppConstants.PREVIEW_MONITOR_READ_BYTES;
    while (this.st.previewMonitorDesired && this.st.previewMonitorLoopId === loopId) {
      if (this.st.previewMonitorSuppressed || this.st.isPlayingTone) {
        await AudioRouteHelpers.delay(40);
        continue;
      }
      try {
        const mono: ArrayBuffer = await capturer.read(readBytes, false);
        if (!this.st.previewMonitorDesired || this.st.previewMonitorLoopId !== loopId) {
          break;
        }
        if (mono.byteLength > 0) {
          const playBuf = AudioRouteHelpers.expandMonoToStereo(mono);
          await renderer.write(playBuf);
        }
      } catch (err) {
        if (this.st.previewMonitorDesired && this.st.previewMonitorLoopId === loopId) {
          DhDiag.warn('AUDIO', `preview monitor read/write ${JSON.stringify(err)}`);
          await AudioRouteHelpers.delay(80);
        }
      }
      await AudioRouteHelpers.delay(5);
    }
    if (this.st.previewMonitorLoopId === loopId) {
      this.st.previewMonitorRunning = false;
    }
  }
}
