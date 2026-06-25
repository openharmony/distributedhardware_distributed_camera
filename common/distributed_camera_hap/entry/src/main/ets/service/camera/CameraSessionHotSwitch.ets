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

import { camera } from '@kit.CameraKit';
import { image } from '@kit.ImageKit';
import { media } from '@kit.MediaKit';
import { fileIo } from '@kit.CoreFileKit';
import { display } from '@kit.ArkUI';
import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { AppConstants } from '../../common/constants/AppConstants';
import { RemoteCameraTrace } from '../../common/constants/RemoteCameraTrace';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { AppContext } from '../../common/context/AppContext';
import {
  CameraFrameRateOption,
  CameraProfileSizeHolder,
  CameraResolutionOption,
  CameraSourceType,
  CameraTarget,
  CameraFocusCapability,
  CameraZoomCapability,
  RemoteCameraDiscoverOptions,
  DeviceFormFactor,
  StreamPipelineType,
  StudioMode
} from '../../common/types/CameraTypes';
import { isLocalDeviceId, TrustDeviceInfo } from '../../common/types/DeviceTypes';
import { DeviceManagerService } from '../DeviceManagerService';
import { DistributedHardwareService } from '../DistributedHardwareService';
import { PermissionHelper } from '../../common/util/PermissionHelper';
import { DhDiag } from '../../common/util/DhDiag';
import { CameraStreamSpecSnapshot } from './CameraSessionHost';
import { CameraSessionUtils } from './CameraSessionUtils';
import { CameraSessionState } from './CameraSessionState';
import { CameraSessionService } from './CameraSessionService';
import { CameraSessionProfileHelpers } from './CameraSessionProfileHelpers';
import { CameraModeOrchestrator } from './CameraModeOrchestrator';
import { PreviewFrameWaiter } from './PreviewFrameWaiter';

const TAG = AppConstants.TAG;

interface BusinessError { code: number; message: string; }

interface CameraManagerRemoteProbe extends camera.CameraManager {
  getCameraDevices(position: camera.CameraPosition, types: Array<camera.CameraType>,
    connectType: camera.ConnectionType): Array<camera.CameraDevice>;
}

export class CameraSessionHotSwitch {
  private readonly facade: CameraSessionService;

  constructor(facade: CameraSessionService) {
    this.facade = facade;
  }

  private get st(): CameraSessionState { return this.facade.state; }

  public async tryHotSwitchCamera(
    target: CameraTarget,
    mode: StudioMode,
    pipeline: StreamPipelineType
  ): Promise<boolean> {
    if (!this.st.session || !this.st.cameraInput || !this.st.previewOutput || !this.st.cameraManager) {
      return false;
    }
    if (this.st.isRecordingVideo || this.st.recordingPrepareActive) {
      return false;
    }
    if (mode !== StudioMode.PREVIEW || pipeline !== StreamPipelineType.CONTINUOUS) {
      return false;
    }
    // 同镜头切换分辨率/帧率时须重建 PreviewOutput，不可短路为 true
    if (target.cameraId === this.st.currentTarget?.cameraId) {
      return false;
    }
    const bothLocal = target.sourceType === CameraSourceType.LOCAL &&
      this.st.currentTarget?.sourceType === CameraSourceType.LOCAL;
    const sameRemoteNet = target.sourceType === CameraSourceType.DISTRIBUTED &&
      this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED &&
      CameraSessionUtils.networkIdMatches(this.st.currentTarget!.networkId, target.networkId);
    if (!bothLocal && !sameRemoteNet) {
      return false;
    }
    const probeRemote = target.sourceType === CameraSourceType.DISTRIBUTED;
    let device = this.facade.findCameraDevice(target.cameraId, probeRemote);
    if (!device) {
      device = this.facade.resolveDeviceForTarget(target);
    }
    if (!device) {
      return false;
    }
    const session = this.st.session;
    const isDistributed = target.sourceType === CameraSourceType.DISTRIBUTED;
    DhDiag.info('CF', `tryHotSwitchCamera id=${target.cameraId} local=${bothLocal ? 1 : 0}`);
    try {
      await session.stop();
      session.beginConfig();
      session.removeInput(this.st.cameraInput);
      session.removeOutput(this.st.previewOutput);
      if (this.st.photoOutput) {
        try {
          session.removeOutput(this.st.photoOutput);
        } catch (err) {
          DhDiag.logCatchWarn('CF', 'tryHotSwitchCamera removeOutput photo', err);
        }
        try {
          this.st.photoOutput.off('photoAvailable');
        } catch (err) {
          DhDiag.logCatchWarn('CF', 'tryHotSwitchCamera photoOutput.off photoAvailable', err);
        }
        await this.st.photoOutput.release();
        this.st.photoOutput = null;
      }
      await this.st.cameraInput.close();
      await this.st.previewOutput.release();

      this.st.cameraInput = this.st.cameraManager.createCameraInput(device);
      await this.st.cameraInput.open();

      const sceneMode = this.st.sessionSceneMode;
      const capability: camera.CameraOutputCapability | undefined = this.st.cameraManager.getSupportedOutputCapability(device, sceneMode);
      let previewProfiles = capability?.previewProfiles ?? [];
      if (!capability || previewProfiles.length === 0) {
        throw new Error('hot switch: no preview profile');
      }
      const videoCap: camera.CameraOutputCapability | null =
        CameraSessionProfileHelpers.tryGetVideoCapability(this.st.cameraManager, device, sceneMode);
      this.facade.rebuildResolutionOptions(capability, isDistributed, videoCap);
      const previewProfile = this.facade.pickPreviewProfile(previewProfiles);
      this.st.previewOutput = this.st.cameraManager.createPreviewOutput(previewProfile, this.st.surfaceId);
      this.st.activePreviewWidth = previewProfile.size.width;
      this.st.activePreviewHeight = previewProfile.size.height;
      this.st.activePreviewFormat = previewProfile.format;
      if ((capability.videoProfiles?.length ?? 0) > 0 || this.st.recordableVideoProfiles.length > 0) {
        this.facade.refreshVideoPlanFromSelection(isDistributed);
      }

      session.addInput(this.st.cameraInput);
      if (!CameraSessionUtils.canAddOutput(session, this.st.previewOutput)) {
        throw new Error('hot switch: canAddOutput preview false');
      }
      session.addOutput(this.st.previewOutput);

      await session.commitConfig();
      await session.start();
      const frameBudget = isDistributed ? AppConstants.PREVIEW_FIRST_FRAME_REMOTE_MS :
        AppConstants.PREVIEW_FIRST_FRAME_MS;
      const gotFrame = await PreviewFrameWaiter.waitFirstPreviewFrame(this.st.previewOutput, frameBudget);
      if (!gotFrame) {
        DhDiag.warn('CF',
          `tryHotSwitchCamera no frame dist=${isDistributed ? 1 : 0} id=${target.cameraId} — full reopen`);
        await this.facade.forceReleaseAll();
        return false;
      }

      this.st.activeCameraDevice = device;
      CameraSessionUtils.applyPreviewOrientation(this.st.previewOutput, device);
      this.facade.applyPreviewOutputFrameRate(this.st.selectedFps > 0 ? this.st.selectedFps : 30);
      this.st.currentTarget = target;
      this.st.currentMode = mode;
      this.st.currentPipeline = pipeline;
      this.st.isPreviewActive = true;
      this.st.previewFrameReceived = true;
      this.facade.scheduleSessionCameraControls();
      DhDiag.info('CF', `tryHotSwitchCamera OK frame=1`);
      return true;
    } catch (err) {
      DhDiag.warn('CF', `tryHotSwitchCamera failed ${JSON.stringify(err)} — fallback full release`);
      await this.facade.forceReleaseAll();
      return false;
    }
  }

  /**
   * 前后台切换或 XComponent 新 Surface：仅重建 PreviewOutput 绑定，不 forceReleaseAll。
   */
  public async tryRebindPreviewSurface(): Promise<boolean> {
    if (this.st.isRecordingVideo || this.st.recordingPrepareActive) {
      DhDiag.info('CF', 'tryRebindPreviewSurface skip — recording');
      return false;
    }
    if (!this.st.session || !this.st.previewOutput || !this.st.cameraManager ||
      !this.st.surfaceId || !this.st.isPreviewActive || !this.st.currentTarget) {
      return false;
    }
    const isDistributed = this.st.currentTarget.sourceType === CameraSourceType.DISTRIBUTED;
    const probeRemote = isDistributed;
    let device = this.st.activeCameraDevice ?? this.facade.findCameraDevice(this.st.currentTarget.cameraId, probeRemote);
    if (!device) {
      device = this.facade.resolveDeviceForTarget(this.st.currentTarget);
    }
    if (!device) {
      DhDiag.warn('CF', 'tryRebindPreviewSurface skip — device not found');
      return false;
    }
    const session = this.st.session;
    const oldPreview = this.st.previewOutput;
    const t0 = Date.now();
    DhDiag.info('CF',
      `tryRebindPreviewSurface dist=${isDistributed ? 1 : 0} surface=${this.st.surfaceId.length > 0 ? 1 : 0} ` +
      `preview=${this.st.activePreviewWidth}x${this.st.activePreviewHeight}`);
    try {
      await session.stop();
      session.beginConfig();
      session.removeOutput(oldPreview);
      await oldPreview.release();
      this.st.previewOutput = null;
      this.st.activePreviewFps = 0;
      this.st.previewFrameReceived = false;

      const capability: camera.CameraOutputCapability | undefined =
        this.st.cameraManager.getSupportedOutputCapability(device, this.st.sessionSceneMode);
      const previewProfiles = capability?.previewProfiles ?? [];
      if (previewProfiles.length === 0) {
        throw new Error('rebind: no preview profiles');
      }
      let previewProfile = previewProfiles[0];
      if (this.st.activePreviewWidth > 0 && this.st.activePreviewHeight > 0) {
        const exact = previewProfiles.find(p =>
          p.size.width === this.st.activePreviewWidth && p.size.height === this.st.activePreviewHeight);
        if (exact) {
          previewProfile = exact;
        } else {
          previewProfile = this.facade.pickPreviewProfile(previewProfiles);
        }
      } else {
        previewProfile = this.facade.pickPreviewProfile(previewProfiles);
      }
      this.st.previewOutput = this.st.cameraManager.createPreviewOutput(previewProfile, this.st.surfaceId);
      this.st.activePreviewWidth = previewProfile.size.width;
      this.st.activePreviewHeight = previewProfile.size.height;
      this.st.activePreviewFormat = previewProfile.format;

      if (!CameraSessionUtils.canAddOutput(session, this.st.previewOutput)) {
        throw new Error('rebind: canAddOutput preview false');
      }
      session.addOutput(this.st.previewOutput);
      await session.commitConfig();
      await session.start();

      this.st.activeCameraDevice = device;
      CameraSessionUtils.applyPreviewOrientation(this.st.previewOutput, device);
      this.facade.applyPreviewOutputFrameRate(this.st.selectedFps > 0 ? this.st.selectedFps : 30);
      this.st.isPreviewActive = true;
      this.facade.scheduleSessionCameraControls();

      const frameBudget = isDistributed ? AppConstants.PREVIEW_FIRST_FRAME_REMOTE_MS :
        AppConstants.RESUME_SURFACE_FRAME_MS;
      const gotFrame = await PreviewFrameWaiter.waitFirstPreviewFrame(this.st.previewOutput, frameBudget);
      if (gotFrame) {
        this.st.previewFrameReceived = true;
      }
      DhDiag.info('CF',
        `tryRebindPreviewSurface OK frame=${gotFrame ? 1 : 0} elapsed=${Date.now() - t0}ms`);
      return gotFrame;
    } catch (err) {
      DhDiag.warn('CF', `tryRebindPreviewSurface failed ${JSON.stringify(err)}`);
      this.abandonSessionAfterHotFail();
      return false;
    }
  }

  /**
   * 同镜头 Session 内热换 PreviewOutput 分辨率（不关 CameraInput、不 release Session）。
   * 部分机型 stop 仍可能 7400101，失败时由 hardReset 兜底。
   */
  public async tryHotReconfigurePreviewSpec(): Promise<boolean> {
    if (!this.st.session || !this.st.cameraInput || !this.st.previewOutput || !this.st.cameraManager || !this.st.currentTarget) {
      DhDiag.warn('CF',
        `tryHotReconfigurePreviewSpec skip — session=${this.st.session ? 1 : 0} ` +
        `input=${this.st.cameraInput ? 1 : 0} preview=${this.st.previewOutput ? 1 : 0} ` +
        `mgr=${this.st.cameraManager ? 1 : 0} target=${this.st.currentTarget ? 1 : 0}`);
      return false;
    }
    if (this.st.isRecordingVideo || this.st.recordingPrepareActive) {
      DhDiag.warn('CF', 'tryHotReconfigurePreviewSpec skip — recording');
      return false;
    }
    if (this.st.sessionSceneMode === camera.SceneMode.NORMAL_VIDEO) {
      DhDiag.warn('CF', 'tryHotReconfigurePreviewSpec skip — video session');
      return false;
    }
    if (this.st.selectedWidth <= 0 || this.st.selectedHeight <= 0) {
      DhDiag.warn('CF', 'tryHotReconfigurePreviewSpec skip — invalid selected size');
      return false;
    }
    if (this.st.activePreviewWidth === this.st.selectedWidth &&
      this.st.activePreviewHeight === this.st.selectedHeight) {
      return true;
    }
    const isDistributed = this.st.currentTarget.sourceType === CameraSourceType.DISTRIBUTED;
    if (isDistributed) {
      DhDiag.info('CF', 'tryHotReconfigurePreviewSpec skip — distributed requires cold path');
      return false;
    }
    const probeRemote = false;
    let device = this.st.activeCameraDevice ?? this.facade.findCameraDevice(this.st.currentTarget.cameraId, probeRemote);
    if (!device) {
      device = this.facade.resolveDeviceForTarget(this.st.currentTarget);
    }
    if (!device) {
      DhDiag.warn('CF', 'tryHotReconfigurePreviewSpec skip — device not found');
      return false;
    }
    DhDiag.info('CF',
      `tryHotReconfigurePreviewSpec ${this.st.selectedWidth}x${this.st.selectedHeight} ` +
      `from ${this.st.activePreviewWidth}x${this.st.activePreviewHeight}`);
    await this.facade.prepareSessionForSpecChange();
    await CameraSessionUtils.delay(AppConstants.PREVIEW_HOT_RECONFIG_SETTLE_MS);

    const session = this.st.session;
    const oldPreview = this.st.previewOutput;
    try {
      await session.stop();
      session.beginConfig();
      session.removeOutput(oldPreview);
      await oldPreview.release();
      this.st.previewOutput = null;
      this.st.activePreviewFps = 0;

      const capability: camera.CameraOutputCapability | undefined = this.st.cameraManager.getSupportedOutputCapability(device, this.st.sessionSceneMode);
      const previewProfiles = capability?.previewProfiles ?? [];
      if (previewProfiles.length === 0) {
        throw new Error('hot reconfig: no preview profiles');
      }
      const videoCap: camera.CameraOutputCapability | null =
        CameraSessionProfileHelpers.tryGetVideoCapability(this.st.cameraManager, device, this.st.sessionSceneMode);
      this.facade.rebuildResolutionOptions(capability, isDistributed, videoCap);
      const previewProfile = this.facade.pickPreviewProfile(previewProfiles);
      this.st.previewOutput = this.st.cameraManager.createPreviewOutput(previewProfile, this.st.surfaceId);
      this.st.activePreviewWidth = previewProfile.size.width;
      this.st.activePreviewHeight = previewProfile.size.height;
      this.st.activePreviewFormat = previewProfile.format;
      if ((capability.videoProfiles?.length ?? 0) > 0 || this.st.recordableVideoProfiles.length > 0) {
        this.facade.refreshVideoPlanFromSelection(isDistributed);
      }

      if (!CameraSessionUtils.canAddOutput(session, this.st.previewOutput)) {
        throw new Error('hot reconfig: canAddOutput preview false');
      }
      session.addOutput(this.st.previewOutput);
      await session.commitConfig();
      await session.start();

      this.st.activeCameraDevice = device;
      CameraSessionUtils.applyPreviewOrientation(this.st.previewOutput, device);
      this.facade.applyPreviewOutputFrameRate(this.st.selectedFps > 0 ? this.st.selectedFps : 30);
      this.st.isPreviewActive = true;
      this.facade.scheduleSessionCameraControls();

      const gotFrame = await PreviewFrameWaiter.waitFirstPreviewFrame(this.st.previewOutput,
        AppConstants.PREVIEW_FIRST_FRAME_MS);
      DhDiag.info('CF',
        `tryHotReconfigurePreviewSpec OK preview=${this.st.activePreviewWidth}x${this.st.activePreviewHeight} ` +
        `@${this.st.activePreviewFps}fps frame=${gotFrame ? 1 : 0}`);
      if (!gotFrame) {
        DhDiag.warn('CF', 'tryHotReconfigurePreviewSpec no first frame — treat as fail');
        this.abandonSessionAfterHotFail();
        return false;
      }
      this.facade.commitUserStreamSpec('tryHotReconfigurePreviewSpec');
      return true;
    } catch (err) {
      DhDiag.warn('CF', `tryHotReconfigurePreviewSpec failed ${JSON.stringify(err)}`);
      this.abandonSessionAfterHotFail();
      return false;
    }
  }

  /** 热换失败后同步丢弃 broken Session 句柄，避免阻塞后续 hardReset */
  public abandonSessionAfterHotFail(): void {
    this.facade.cancelScheduledCameraControls();
    const stalePreview = this.st.previewOutput;
    const stalePhoto = this.st.photoOutput;
    const staleSession = this.st.session;
    const staleInput = this.st.cameraInput;
    this.st.session = null;
    this.st.previewOutput = null;
    this.st.photoOutput = null;
    this.st.cameraInput = null;
    this.st.activeCameraDevice = null;
    this.st.isPreviewActive = false;
    this.st.activePreviewFps = 0;
    this.st.activePreviewWidth = 0;
    this.st.activePreviewHeight = 0;
    if (stalePhoto) {
      try {
        stalePhoto.off('photoAvailable');
      } catch (err) {
        DhDiag.logCatchWarn('CF', 'hotSwitchReset stalePhoto.off photoAvailable', err);
      }
    }
    void stalePreview?.release().catch((err: Object) => {
      DhDiag.logCatchWarn('CF', 'hotSwitchReset stalePreview.release', err);
    });
    void stalePhoto?.release().catch((err: Object) => {
      DhDiag.logCatchWarn('CF', 'hotSwitchReset stalePhoto.release', err);
    });
    void staleSession?.release().catch((err: Object) => {
      DhDiag.logCatchWarn('CF', 'hotSwitchReset staleSession.release', err);
    });
    void staleInput?.close().catch((err: Object) => {
      DhDiag.logCatchWarn('CF', 'hotSwitchReset staleInput.close', err);
    });
  }

  /**
   * 预览分辨率/帧率切换：本机可试 L1 热换；远端禁热换，分辨率变化时 cold + DH stop/resume 重协商。
   */
  public async reopenPreviewWithUserSpec(): Promise<boolean> {
    const target = this.st.currentTarget;
    if (!target || !this.st.cameraManager) {
      DhDiag.warn('CF', 'reopenPreviewWithUserSpec abort — no target/manager');
      return false;
    }
    const mode = this.st.currentMode;
    const pipeline = this.st.currentPipeline;
    const saved = this.facade.snapshotStreamSpec();
    const priorPreviewW = this.st.activePreviewWidth;
    const priorPreviewH = this.st.activePreviewHeight;
    const distributedResChanging = target.sourceType === CameraSourceType.DISTRIBUTED &&
      (priorPreviewW !== saved.width || priorPreviewH !== saved.height);
    this.facade.commitUserStreamSpec('reopenPreviewWithUserSpec.begin');
    const firstChange = !this.st.previewSpecChangeSucceeded;
    DhDiag.info('CF',
      `reopenPreviewWithUserSpec ${saved.width}x${saved.height}@${saved.fps}fps mode=${mode} ` +
      `first=${firstChange ? 1 : 0} distResChange=${distributedResChanging ? 1 : 0}`);

    const isDistributed = target.sourceType === CameraSourceType.DISTRIBUTED;
    this.facade.setDeferSessionCameraControls(true);
    try {
      const skipHot = this.facade.shouldSkipHotReconfigureForStability();
      const allowHot = !skipHot && !isDistributed;
      if (allowHot) {
        const hotOk = await this.tryHotReconfigurePreviewSpec();
        if (hotOk) {
          this.facade.commitUserStreamSpec('reopenPreviewWithUserSpec.hot');
          this.st.previewSpecChangeSucceeded = true;
          DhDiag.info('CF',
            `reopenPreviewWithUserSpec OK hot ${this.st.activePreviewWidth}x${this.st.activePreviewHeight} ` +
            `@${this.st.activePreviewFps}fps dist=${isDistributed ? 1 : 0}`);
          return true;
        }
        DhDiag.warn('CF',
          `reopenPreviewWithUserSpec hot failed — cold path dist=${isDistributed ? 1 : 0}`);
      } else if (isDistributed) {
        DhDiag.info('CF', 'reopenPreviewWithUserSpec skip hot — distributed cold+DH path');
      } else {
        DhDiag.info('CF', 'reopenPreviewWithUserSpec skip hot — bootstrap guard');
      }

      await this.facade.prepareSessionForSpecChange();
      const released = await this.facade.forceReleaseAll();
      if (!released) {
        DhDiag.warn('CF', 'reopenPreviewWithUserSpec forceRelease stopOk=0 — hardReset');
      }

      const useFastReset = !skipHot && !firstChange && released && !isDistributed;
      DhDiag.info('CF',
        `reopenPreviewWithUserSpec cold hardReset fast=${useFastReset ? 1 : 0} released=${released ? 1 : 0} ` +
        `distResChange=${distributedResChanging ? 1 : 0}`);
      await this.facade.hardResetCameraPipeline(useFastReset);

      const readyBudgetMs = isDistributed && distributedResChanging ?
        AppConstants.REMOTE_SPEC_CHANGE_READY_WAIT_MS : AppConstants.PREVIEW_SPEC_READY_WAIT_MS;
      const deadline = Date.now() + readyBudgetMs;
      let attempt = 0;
      while (Date.now() < deadline) {
        attempt++;
        const remain = deadline - Date.now();
        if (remain <= 0) {
          break;
        }
        const initialSettle = (skipHot || firstChange) ? AppConstants.PREVIEW_RELEASE_SETTLE_MS : 0;
        const ready = await this.waitForCameraServiceReady(Math.min(2500, remain), initialSettle);
        if (!ready) {
          DhDiag.warn('CF', `reopenPreviewWithUserSpec cold attempt=${attempt} camera not ready`);
          await CameraSessionUtils.delay(AppConstants.PREVIEW_SPEC_OPEN_RETRY_MS);
          continue;
        }
        this.facade.restoreStreamSpecSnapshot(saved);
        this.facade.commitUserStreamSpec('reopenPreviewWithUserSpec.cold');
        let openTarget = target;
        if (isDistributed && target.networkId && distributedResChanging) {
          const dhOk = await DistributedHardwareService.getInstance()
            .renegotiateCameraForSpecChange(target.networkId);
          if (!dhOk) {
            this.st.lastPhotoHint = '远端 DH 重协商失败，请确认对端在线后重试';
            DhDiag.warn('CF', `reopenPreviewWithUserSpec cold attempt=${attempt} DH renegotiate failed`);
            await CameraSessionUtils.delay(AppConstants.PREVIEW_SPEC_OPEN_RETRY_MS);
            continue;
          }
          await this.facade.discoverRemoteCameras(target.networkId, undefined,
            RemoteCameraTrace.discoverOptions(RemoteCameraTrace.SPEC));
          const fresh = this.facade.findRemoteCameraTarget(target.networkId, target.position);
          if (fresh) {
            openTarget = fresh;
          }
        }
        // 与 WORKING 版 applySelectedResolutionAt 一致：完整 release 后 openAndPreview（远端不可 skipRelease）
        const skipReleaseOnOpen = isDistributed ? false : released;
        const ok = await this.facade.openAndPreview(openTarget, mode, pipeline, skipReleaseOnOpen);
        if (ok) {
          this.st.previewSpecChangeSucceeded = true;
          this.facade.commitUserStreamSpec('reopenPreviewWithUserSpec.cold.ok');
          DhDiag.info('CF',
            `reopenPreviewWithUserSpec OK cold ${this.st.activePreviewWidth}x${this.st.activePreviewHeight} ` +
            `@${this.st.activePreviewFps}fps attempt=${attempt}`);
          return true;
        }
        DhDiag.warn('CF',
          `reopenPreviewWithUserSpec cold attempt=${attempt} open failed ${this.st.lastOpenErrorMessage}`);
        await this.facade.hardResetCameraPipeline(true);
        await CameraSessionUtils.delay(AppConstants.PREVIEW_SPEC_OPEN_RETRY_MS);
      }
      DhDiag.warn('CF', `reopenPreviewWithUserSpec timeout after ${attempt} attempt(s)`);
      this.st.lastPhotoHint = isDistributed ?
        '远端分辨率切换超时（含 DH 重建），请稍后重试' : '分辨率切换超时，请稍后重试';
      return false;
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'reopenPreviewWithUserSpec', err);
      this.st.lastPhotoHint = isDistributed ?
        '远端分辨率切换异常，请稍后重试' : '分辨率切换异常，请稍后重试';
      return false;
    } finally {
      this.facade.setDeferSessionCameraControls(false);
      if (this.st.isPreviewActive) {
        this.facade.scheduleSessionCameraControls();
      }
    }
  }

  public async waitForCameraServiceReady(maxMs: number = 2500,
    initialSettleMs: number = 0): Promise<boolean> {
    if (!this.st.cameraManager) {
      return false;
    }
    if (initialSettleMs > 0) {
      await CameraSessionUtils.delay(initialSettleMs);
    }
    const startMs = Date.now();
    const deadline = startMs + maxMs;
    while (Date.now() < deadline) {
      try {
        const list = this.st.cameraManager.getSupportedCameras();
        if (list.length > 0) {
          DhDiag.info('CF',
            `waitForCameraServiceReady ok count=${list.length} elapsed=${Date.now() - startMs}ms`);
          return true;
        }
      } catch (err) {
        const code = (err as BusinessError)?.code ?? 0;
        DhDiag.warn('CF', `waitForCameraServiceReady busy code=${code}`);
      }
      await CameraSessionUtils.delay(200);
    }
    DhDiag.warn('CF', `waitForCameraServiceReady timeout elapsed=${Date.now() - startMs}ms`);
    return false;
  }

  private static canAddOutput(session: camera.Session, output: camera.CameraOutput): boolean {
    try {
      return session.canAddOutput(output);
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'canAddOutput fallback true', err);
      return true;
    }
  }
  /** 同对端前/后摄切换 — 短释放，避免黑屏过久 */
}
