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

export class CameraSessionPreview {
  private readonly facade: CameraSessionService;

  constructor(facade: CameraSessionService) {
    this.facade = facade;
  }

  private get st(): CameraSessionState { return this.facade.state; }

  public async openAndPreview(
    target: CameraTarget,
    mode: StudioMode = StudioMode.PREVIEW,
    pipeline: StreamPipelineType = StreamPipelineType.CONTINUOUS,
    skipPreRelease: boolean = false
  ): Promise<boolean> {
    const startMs = Date.now();
    const isDistributed = target.sourceType === CameraSourceType.DISTRIBUTED;
    DhDiag.flowBegin('CF', 'openAndPreview',
      `id=${target.cameraId.substring(0, 24)} dist=${isDistributed ? 1 : 0} ` +
      `mode=${mode} skipRelease=${skipPreRelease ? 1 : 0}`);
    const savedSpec = this.facade.snapshotStreamSpec();
    const switchingCamera = this.st.currentTarget !== null && this.st.currentTarget.cameraId !== target.cameraId;
    if (switchingCamera) {
      this.st.selectedWidth = 0;
      this.st.selectedHeight = 0;
      this.st.resolutionIndex = 0;
      this.st.selectedFps = 0;
      this.st.fpsIndex = 0;
      this.facade.clearPinnedUserStreamSpec('openAndPreview.switchCamera');
      this.facade.clearUserChosenStreamSpec('openAndPreview.switchCamera');
      this.st.lastCommittedUserStreamSpec = null;
    } else if (savedSpec.width > 0 && savedSpec.height > 0) {
      this.facade.restoreStreamSpecSnapshot(savedSpec);
    }
    if (!switchingCamera && this.st.recordingPrepareActive &&
      this.st.pinnedUserStreamSpec !== null && this.st.pinnedUserStreamSpec.width > 0) {
      this.facade.restoreStreamSpecSnapshot(this.st.pinnedUserStreamSpec);
    }
    this.st.lastOpenErrorCode = 0;
    this.st.lastOpenErrorMessage = '';
    DhDiag.flowStep('CF', 'openAndPreview', 'enter',
      `surface=${this.st.surfaceId.length > 0 ? 1 : 0} skipRelease=${skipPreRelease ? 1 : 0}`);
    if (!this.st.surfaceId) {
      DhDiag.flowEnd('CF', 'openAndPreview', false, 'surfaceId empty');
      return false;
    }
    if (!this.st.cameraManager) {
      DhDiag.flowEnd('CF', 'openAndPreview', false, 'CameraManager null');
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'openAndPreview: CameraManager 未就绪');
      return false;
    }
    if (mode === StudioMode.VIDEO && this.facade.getModeOrchestrator().video.tryReuseForModeSwitch(target)) {
      this.st.currentMode = mode;
      DhDiag.flowEnd('CF', 'openAndPreview', true, 'reuse active VideoSession');
      return true;
    }
    try {
      if (this.st.isRecordingVideo) {
        await this.facade.stopRecordingInternal();
      }
      const distAwaitingFrame = target.sourceType === CameraSourceType.DISTRIBUTED && !this.st.previewFrameReceived;
      if (skipPreRelease && this.st.currentTarget?.cameraId === target.cameraId && this.st.isPreviewActive &&
        !distAwaitingFrame) {
        const needVideoUpgrade = mode === StudioMode.VIDEO &&
          this.st.sessionSceneMode !== camera.SceneMode.NORMAL_VIDEO;
        if (!needVideoUpgrade) {
          DhDiag.flowEnd('CF', 'openAndPreview', true, 'reuse active session skipRelease');
          return true;
        }
        DhDiag.info('CF', 'openAndPreview Photo→Video — bypass skipRelease reuse for session upgrade');
      }
      if (!skipPreRelease) {
        const hot = await this.facade.tryHotSwitchCamera(target, mode, pipeline);
        if (hot) {
          return true;
        }
        await this.facade.forceReleaseAll();
        await this.facade.waitForCameraServiceReady(2500, AppConstants.PREVIEW_RELEASE_SETTLE_MS);
      }
      let openTarget = target;
      if (target.sourceType === CameraSourceType.DISTRIBUTED && target.networkId) {
        const fresh = this.findRemoteCameraTarget(target.networkId, target.position);
        if (fresh) {
          openTarget = fresh;
        }
      }
      // 调用方已完成 discoverRemoteCameras，此处不再重复 discover（避免 tryTriggerRemoteAuth 与 open 冲突）
      const probeRemote = openTarget.sourceType === CameraSourceType.DISTRIBUTED;
      let device = this.findCameraDevice(openTarget.cameraId, probeRemote);
      if (!device) {
        device = this.resolveDeviceForTarget(openTarget);
        if (device) {
          DhDiag.warn('CF', `findCameraDevice miss id=${openTarget.cameraId}, resolved by position/source`);
        }
      }
      if (device && openTarget.sourceType === CameraSourceType.DISTRIBUTED &&
        device.connectionType !== camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
        openTarget.networkId) {
        const remoteNet: string = openTarget.networkId;
        const remoteDev = this.findRemoteCameraDevice(remoteNet, openTarget.position);
        if (remoteDev) {
          DhDiag.warn('CF',
            `openAndPreview reject local device id=${device.cameraId} — re-resolve REMOTE`);
          device = remoteDev;
          const rebuilt = this.findRemoteCameraTarget(remoteNet, openTarget.position);
          if (rebuilt) {
            openTarget = rebuilt;
          }
        }
      }
      if (!device) {
        DhDiag.warn('CF', `findCameraDevice miss id=${openTarget.cameraId}, retry collect`);
        const retry = this.facade.collectCameraDevices(probeRemote)
          .find(c => c.cameraId === openTarget.cameraId);
        if (!retry) {
          throw new Error(`camera not found: ${openTarget.cameraId}`);
        }
        return this.openAndPreviewWithDevice(retry, openTarget, mode, pipeline);
      }
      const ok = await this.openAndPreviewWithDevice(device, openTarget, mode, pipeline);
      if (ok) {
        DhDiag.flowEnd('CF', 'openAndPreview', true,
          `id=${openTarget.cameraId.substring(0, 24)} elapsed=${Date.now() - startMs}ms`);
        return true;
      }
      if (openTarget.sourceType === CameraSourceType.LOCAL) {
        const fb = await this.tryOpenLocalPreviewFallback(openTarget, mode, pipeline, openTarget.cameraId);
        DhDiag.flowEnd('CF', 'openAndPreview', fb, fb ? 'local fallback' : 'fallback failed');
        return fb;
      }
      DhDiag.flowEnd('CF', 'openAndPreview', false,
        `${this.st.lastOpenErrorMessage} elapsed=${Date.now() - startMs}ms`);
      return false;
    } catch (err) {
      const biz = err as BusinessError;
      this.st.lastOpenErrorCode = biz?.code ?? 0;
      this.st.lastOpenErrorMessage = biz?.message ?? (err instanceof Error ? err.message : JSON.stringify(err));
      const detail = this.st.lastOpenErrorMessage;
      DhDiag.flowEnd('CF', 'openAndPreview', false,
        `code=${this.st.lastOpenErrorCode} ${detail} elapsed=${Date.now() - startMs}ms`);
      await this.facade.forceReleaseAll();
      if (target.sourceType === CameraSourceType.LOCAL) {
        const fb = await this.tryOpenLocalPreviewFallback(target, mode, pipeline, target.cameraId);
        DhDiag.flowEnd('CF', 'openAndPreview', fb, fb ? 'local fallback after err' : detail);
        return fb;
      }
      DhDiag.flowEnd('CF', 'openAndPreview', false, detail);
      return false;
    }
  }

  public async tryOpenLocalPreviewFallback(
    target: CameraTarget,
    mode: StudioMode,
    pipeline: StreamPipelineType,
    skipCameraId: string
  ): Promise<boolean> {
    const candidates = this.facade.collectCameraDevices(false).filter(d =>
      d.connectionType !== camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
      d.cameraId !== skipCameraId &&
      (d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_BACK ||
        d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_FRONT));
    if (candidates.length === 0) {
      return false;
    }
    DhDiag.warn('CF', `openAndPreview local fallback try ${candidates.length} alt device(s)`);
    await this.facade.forceReleaseAll();
    await CameraSessionUtils.delay(200);
    for (const dev of candidates) {
      const altTargets = this.facade.buildCameraTargets([dev]);
      const alt = altTargets.find(t => t.position === target.position) ?? altTargets[0];
      if (!alt) {
        continue;
      }
      DhDiag.info('CF', `openAndPreview fallback device=${dev.cameraId}`);
      const ok = await this.openAndPreviewWithDevice(dev, alt, mode, pipeline);
      if (ok) {
        return true;
      }
      await this.facade.forceReleaseAll();
      await CameraSessionUtils.delay(150);
    }
    return false;
  }

  public async openAndPreviewWithDevice(
    device: camera.CameraDevice,
    target: CameraTarget,
    mode: StudioMode,
    pipeline: StreamPipelineType
  ): Promise<boolean> {
    const startMs = Date.now();
    if (!this.st.cameraManager || !this.st.surfaceId) {
      return false;
    }
    try {
      const isDistributed = target.sourceType === CameraSourceType.DISTRIBUTED;
      let sceneMode = this.resolveSceneMode(device, mode, pipeline);
      if (!isDistributed && mode === StudioMode.PREVIEW) {
        sceneMode = this.resolveLocalPreviewSceneMode(device);
      }
      DhDiag.flowBegin('CF', 'openAndPreviewWithDevice',
        `sceneMode=${sceneMode} conn=${device.connectionType} id=${device.cameraId.substring(0, 24)} ` +
        `dist=${isDistributed ? 1 : 0}`);
      this.st.cameraInput = this.st.cameraManager.createCameraInput(device);
      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'createInput',
        `elapsed=${Date.now() - startMs}ms`);
      const inputOpened = await CameraSessionUtils.waitWithTimeout<void>(
        this.st.cameraInput.open(), AppConstants.CAMERA_OPEN_STEP_TIMEOUT_MS);
      if (inputOpened === false) {
        throw new Error('cameraInput.open timeout');
      }
      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'input-opened',
        `elapsed=${Date.now() - startMs}ms`);

      let capability: camera.CameraOutputCapability | undefined = this.st.cameraManager.getSupportedOutputCapability(device, sceneMode);
      let previewProfiles = capability?.previewProfiles ?? [];
      let photoCap: camera.CameraOutputCapability | null = null;
      if (previewProfiles.length === 0 && sceneMode === camera.SceneMode.NORMAL_VIDEO) {
        photoCap = this.st.cameraManager.getSupportedOutputCapability(device, camera.SceneMode.NORMAL_PHOTO);
        if (photoCap && photoCap.previewProfiles.length > 0) {
          previewProfiles = photoCap.previewProfiles;
          DhDiag.warn('CF',
            `sceneMode=VIDEO 无 previewProfiles，回退 PHOTO 预览规格（远端${isDistributed ? 1 : 0} pos=${target.position}）`);
        }
      }
      if (!capability || previewProfiles.length === 0) {
        throw new Error(`no preview profile sceneMode=${sceneMode} pos=${target.position}`);
      }
      const capForOptions = capability ?? photoCap!;
      const videoCap: camera.CameraOutputCapability | null =
        CameraSessionProfileHelpers.tryGetVideoCapability(this.st.cameraManager, device, sceneMode);
      this.facade.restoreEffectiveUserStreamSpec('openAndPreviewWithDevice rebuild');
      this.facade.rebuildResolutionOptions(capForOptions, isDistributed, videoCap);
      const previewProfile = this.facade.pickPreviewProfile(previewProfiles);
      this.st.previewOutput = this.st.cameraManager.createPreviewOutput(previewProfile, this.st.surfaceId);
      this.st.activePreviewWidth = previewProfile.size.width;
      this.st.activePreviewHeight = previewProfile.size.height;
      this.st.activePreviewFormat = previewProfile.format;
      const videoProfiles = capability.videoProfiles ?? [];
      if (videoProfiles.length > 0) {
        this.facade.refreshVideoPlanFromSelection(isDistributed);
      } else if (this.st.recordableVideoProfiles.length > 0) {
        this.facade.refreshVideoPlanFromSelection(isDistributed);
      } else {
        this.st.activeVideoWidth = 0;
        this.st.activeVideoHeight = 0;
        this.st.activeRecordFps = this.st.selectedFps > 0 ? this.st.selectedFps : 30;
      }

      if (sceneMode === camera.SceneMode.NORMAL_VIDEO) {
        this.st.session = this.st.cameraManager.createSession(sceneMode) as camera.VideoSession;
      } else {
        this.st.session = this.st.cameraManager.createSession(sceneMode) as camera.PhotoSession;
      }

      this.st.session.beginConfig();
      this.st.session.addInput(this.st.cameraInput);
      if (!CameraSessionUtils.canAddOutput(this.st.session, this.st.previewOutput)) {
        throw new Error('canAddOutput preview false');
      }
      this.st.session.addOutput(this.st.previewOutput);

      const needPhoto = pipeline === StreamPipelineType.SNAPSHOT || mode === StudioMode.PHOTO;
      if (needPhoto) {
        let photoProfiles = capability?.photoProfiles ?? [];
        const supportedModes = this.st.cameraManager.getSupportedSceneModes(device);
        if (photoProfiles.length === 0 && sceneMode !== camera.SceneMode.NORMAL_PHOTO &&
          supportedModes.includes(camera.SceneMode.NORMAL_PHOTO)) {
          const photoCap = this.st.cameraManager.getSupportedOutputCapability(device, camera.SceneMode.NORMAL_PHOTO);
          photoProfiles = photoCap?.photoProfiles ?? [];
          if (photoProfiles.length > 0) {
            DhDiag.info('CF', `photoProfiles from NORMAL_PHOTO fallback count=${photoProfiles.length}`);
          }
        }
        if (photoProfiles.length > 0) {
          const attached = await this.attachPhotoOutput(photoProfiles[0], isDistributed);
          if (!attached) {
            DhDiag.warn('CF', `attachPhotoOutput failed distributed=${isDistributed ? 1 : 0}`);
          }
        } else {
          DhDiag.warn('CF', `no photoProfiles distributed=${isDistributed ? 1 : 0} sceneMode=${sceneMode}`);
        }
      }
      // VideoOutput 在 startRecording 时通过 AVRecorder surface 动态挂载，预览阶段不写文件

      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'commitConfig',
        `elapsed=${Date.now() - startMs}ms`);
      const committed = await CameraSessionUtils.waitWithTimeout<void>(
        this.st.session.commitConfig(), AppConstants.CAMERA_OPEN_STEP_TIMEOUT_MS);
      if (committed === false) {
        throw new Error('session.commitConfig timeout');
      }
      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'commitDone',
        `elapsed=${Date.now() - startMs}ms`);
      this.st.previewFrameReceived = false;
      const frameWait = PreviewFrameWaiter.waitFirstPreviewFrame(this.st.previewOutput,
        AppConstants.PREVIEW_FIRST_FRAME_MS);
      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'sessionStart',
        `elapsed=${Date.now() - startMs}ms`);
      const started = await CameraSessionUtils.waitWithTimeout<void>(
        this.st.session.start(), AppConstants.CAMERA_OPEN_STEP_TIMEOUT_MS);
      if (started === false) {
        throw new Error('session.start timeout');
      }
      DhDiag.flowStep('CF', 'openAndPreviewWithDevice', 'started',
        `elapsed=${Date.now() - startMs}ms`);
      // 对齐 AI_Dev：frameStart 异步，不阻塞 switchStream ACTIVE
      void frameWait.then((gotFrame: boolean) => {
        this.st.previewFrameReceived = gotFrame;
        if (gotFrame) {
          if (isDistributed) {
            DhDiag.info('CF', 'preview frameStart OK distributed=1');
            this.notifyDistributedPreviewFrame(true);
          }
          return;
        }
        DhDiag.warn('CF', 'preview frameStart timeout after session.start');
        if (isDistributed) {
          this.scheduleDistributedPreviewHeal('frameStart-timeout');
        }
      }).catch((err: Error) => {
        DhDiag.warn('CF', `preview frameWait failed ${JSON.stringify(err)}`);
        if (isDistributed) {
          this.scheduleDistributedPreviewHeal('frameWait-error');
        }
      });

      this.st.activeCameraDevice = device;
      this.st.currentTarget = target;
      CameraSessionUtils.applyPreviewOrientation(this.st.previewOutput, device);

      const previewFpsTarget = this.st.selectedFps > 0 ? this.st.selectedFps : 30;
      this.facade.applyPreviewOutputFrameRate(previewFpsTarget);
      this.st.currentMode = mode;
      this.st.currentPipeline = pipeline;
      this.st.sessionSceneMode = sceneMode;
      this.st.isPreviewActive = true;
      this.facade.scheduleSessionCameraControls();
      this.st.lastPreviewOpenOkMs = Date.now();
      if (this.st.recordingPrepareActive && this.st.pinnedUserStreamSpec !== null &&
        this.st.pinnedUserStreamSpec.width > 0) {
        this.facade.restoreStreamSpecSnapshot(this.st.pinnedUserStreamSpec);
      }
      DhDiag.flowEnd('CF', 'openAndPreviewWithDevice', true,
        `id=${target.cameraId.substring(0, 24)} preview=${this.st.activePreviewWidth}x${this.st.activePreviewHeight} ` +
        `dist=${isDistributed ? 1 : 0} elapsed=${Date.now() - startMs}ms`);
      return true;
    } catch (err) {
      const biz = err as BusinessError;
      this.st.lastOpenErrorCode = biz?.code ?? 0;
      this.st.lastOpenErrorMessage = biz?.message ?? (err instanceof Error ? err.message : JSON.stringify(err));
      const detail = this.st.lastOpenErrorMessage;
      DhDiag.flowEnd('CF', 'openAndPreviewWithDevice', false,
        `code=${this.st.lastOpenErrorCode} ${detail} elapsed=${Date.now() - startMs}ms`);
      await this.facade.forceReleaseAll();
      return false;
    }
  }

  public async releasePhotoImageReceiver(): Promise<void> {
    this.st.useDistributedPhotoReceiver = false;
    if (this.st.distributedPhotoCapturePending) {
      this.st.distributedPhotoCapturePending = false;
      this.completeDistributedPhotoCapture(false);
    }
    if (this.st.photoImageReceiver) {
      try {
        this.st.photoImageReceiver.off('imageArrival');
      } catch (_e) {
        DhDiag.logCatchWarn('CF', 'releasePhotoImageReceiver off imageArrival', _e);
      }
      this.st.photoImageReceiver = null;
    }
    this.st.photoReceiverSurfaceId = '';
  }

  /**
   * 远端：ImageReceiver + createPhotoOutput(profile, surfaceId)；本机：createPhotoOutput + photoAvailable。
   */
  public async attachPhotoOutput(profile: camera.Profile, isDistributed: boolean): Promise<boolean> {
    if (!this.st.cameraManager || !this.st.session) {
      DhDiag.warn('CF', 'attachPhotoOutput skip — manager/session null');
      return false;
    }
    await this.releasePhotoImageReceiver();
    if (this.st.photoOutput) {
      try {
        this.st.photoOutput.off('photoAvailable');
        await this.st.photoOutput.release();
      } catch (_e) {
        DhDiag.logCatchWarn('CF', 'attachPhotoOutput stale photoOutput release', _e);
      }
      this.st.photoOutput = null;
    }
    if (isDistributed) {
      try {
        const w = profile.size.width;
        const h = profile.size.height;
        this.st.photoImageReceiver = image.createImageReceiver(w, h, profile.format, 8);
        this.st.photoImageReceiver.on('imageArrival', () => {
          void this.onDistributedPhotoImageArrival();
        });
        this.st.photoReceiverSurfaceId = await this.st.photoImageReceiver.getReceivingSurfaceId();
        this.st.photoOutput = this.st.cameraManager.createPhotoOutput(profile, this.st.photoReceiverSurfaceId);
        this.st.useDistributedPhotoReceiver = this.st.photoOutput !== null;
        DhDiag.info('CF',
          `attachPhotoOutput ImageReceiver dist=1 ${w}x${h} fmt=${profile.format} surface=${this.st.photoReceiverSurfaceId.length > 0 ? 1 : 0}`);
      } catch (err) {
        DhDiag.warn('CF', `attachPhotoOutput ImageReceiver failed ${JSON.stringify(err)} — fallback photoAvailable`);
        this.st.useDistributedPhotoReceiver = false;
        this.st.photoOutput = this.st.cameraManager.createPhotoOutput(profile);
      }
    } else {
      this.st.useDistributedPhotoReceiver = false;
      this.st.photoOutput = this.st.cameraManager.createPhotoOutput(profile);
    }
    if (!this.st.photoOutput) {
      DhDiag.warn('CF', 'attachPhotoOutput failed — createPhotoOutput null');
      this.st.lastPhotoHint = '无法创建 PhotoOutput';
      return false;
    }
    if (!this.st.useDistributedPhotoReceiver) {
      this.registerPhotoCallback();
    }
    try {
      this.st.session.addOutput(this.st.photoOutput);
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'attachPhotoOutput addOutput', err);
      this.st.lastPhotoHint = '拍照输出挂载失败';
      await this.releasePhotoImageReceiver();
      this.st.photoOutput = null;
      return false;
    }
    return true;
  }

  public onDistributedPhotoImageArrival(): void {
    if (!this.st.photoImageReceiver || !this.st.distributedPhotoCapturePending) {
      return;
    }
    this.st.photoImageReceiver.readNextImage((err: BusinessError, img: image.Image) => {
      if (err || !img) {
        DhDiag.error('CF', `dist photo readNextImage err=${JSON.stringify(err)}`);
        this.finishDistributedPhotoHint('远端收帧失败', false);
        return;
      }
      img.getComponent(image.ComponentType.JPEG, (compErr: BusinessError, component: image.Component) => {
        const jpegBuffer = (compErr || !component?.byteBuffer) ? null : component.byteBuffer;
        void this.saveDistributedPhotoComponent(img, compErr, jpegBuffer);
      });
    });
  }

  public async saveDistributedPhotoComponent(img: image.Image, compErr: BusinessError,
    jpegBuffer: ArrayBuffer | null): Promise<void> {
    try {
      if (compErr || !jpegBuffer) {
        DhDiag.error('CF', `dist photo getComponent err=${JSON.stringify(compErr)}`);
        this.finishDistributedPhotoHint('远端 JPEG 分量无效', false);
        return;
      }
      const ctx: common.UIAbilityContext | null = AppContext.tryGet();
      if (!ctx) {
        this.finishDistributedPhotoHint('无应用上下文', false);
        return;
      }
      const rotatedJpeg = await CameraSessionProfileHelpers.applyPhotoRotationToJpegBuffer(
        jpegBuffer, this.st.distributedPhotoSaveDegrees);
      const tempPath = `${ctx.filesDir}/harmonycast_dist_photo_${Date.now()}.jpg`;
      const file = fileIo.openSync(tempPath,
        fileIo.OpenMode.CREATE | fileIo.OpenMode.READ_WRITE | fileIo.OpenMode.TRUNC);
      fileIo.writeSync(file.fd, rotatedJpeg);
      fileIo.closeSync(file.fd);
      await img.release();
      DhDiag.info('CF', `dist photo sandbox bytes=${fileIo.statSync(tempPath).size}`);
      const result = await this.st.galleryService.savePhotoFromSandbox(tempPath);
      DhDiag.info('GALLERY', `dist photo save success=${result.success ? 1 : 0} msg=${result.message}`);
      this.st.lastPhotoHint = result.message;
      if (this.st.photoSaveListener) {
        this.st.photoSaveListener(result.message, result.success);
      }
      this.completeDistributedPhotoCapture(result.success);
    } catch (err) {
      try {
        await img.release();
      } catch (_e) {
        DhDiag.logCatchWarn('CF', 'saveDistributedPhotoComponent img.release', _e);
      }
      const msg = err instanceof Error ? err.message : JSON.stringify(err);
      DhDiag.error('CF', `dist photo save err ${msg}`);
      this.finishDistributedPhotoHint(`远端照片保存失败: ${msg}`, false);
    }
  }

  public finishDistributedPhotoHint(message: string, success: boolean): void {
    this.st.lastPhotoHint = message;
    if (this.st.photoSaveListener) {
      this.st.photoSaveListener(message, success);
    }
    this.completeDistributedPhotoCapture(success);
  }

  public completeDistributedPhotoCapture(success: boolean): void {
    this.st.distributedPhotoCapturePending = false;
    if (this.st.distributedPhotoCaptureDone) {
      const done = this.st.distributedPhotoCaptureDone;
      this.st.distributedPhotoCaptureDone = null;
      done(success);
    }
  }

  /**
   * 远端 ImageReceiver 落盘角度：JPEG 常与预览同向，勿套用 getPhotoRotation（该值给 capture 管线）。
   * capture 固定 ROTATION_0。前后摄 API 返回 90°/270° 时再转会偏 90°（270° 顺 = 90° 逆）。
   */
  public resolveDistributedPhotoSaveDegrees(settings: camera.PhotoCaptureSetting): number {
    const device = this.st.activeCameraDevice;
    const deviceDegree = CameraSessionUtils.resolveDisplayRotationDegrees();
    const settingsDeg = CameraSessionProfileHelpers.rotationToDegrees(settings.rotation);
    let previewDeg = 0;
    let photoDeg = settingsDeg;

    if (this.st.previewOutput) {
      try {
        previewDeg = CameraSessionProfileHelpers.rotationToDegrees(
          this.st.previewOutput.getPreviewRotation(deviceDegree));
      } catch (err) {
        DhDiag.logCatchWarn('CF', 'resolveDistributedPhotoSaveDegrees getPreviewRotation', err);
      }
    }
    if (this.st.photoOutput) {
      try {
        photoDeg = CameraSessionProfileHelpers.rotationToDegrees(
          this.st.photoOutput.getPhotoRotation(deviceDegree));
      } catch (err) {
        DhDiag.logCatchWarn('CF', 'resolveDistributedPhotoSaveDegrees getPhotoRotation', err);
      }
    }

    const isRemote = device?.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE;
    const isFront = device !== null && CameraSessionUtils.isFrontCamera(device);

    if (!isRemote) {
      return settingsDeg;
    }

    let saveDeg = settingsDeg > 0 ? settingsDeg :
      (photoDeg > 0 ? photoDeg : previewDeg);

    // 前摄横屏：OHDocs 双路预览 +180；再与下方 90/270→0 合并（横屏 90→270 亦归零）
    if (isFront && (deviceDegree === 90 || deviceDegree === 270)) {
      saveDeg = (saveDeg + 180) % 360;
    }

    // 远端前后摄：ImageReceiver JPEG 已对齐预览，忽略 getPhotoRotation 的 90°/270°
    if (saveDeg === 90 || saveDeg === 270) {
      saveDeg = 0;
    }

    DhDiag.info('CF',
      `dist photo resolve saveDeg=${saveDeg} settings=${settingsDeg} preview=${previewDeg} photo=${photoDeg} ` +
      `devDeg=${deviceDegree} remote=${isRemote ? 1 : 0} front=${isFront ? 1 : 0}`);
    return saveDeg;
  }

  public async captureDistributedPhoto(settings: camera.PhotoCaptureSetting): Promise<boolean> {
    if (!this.st.photoOutput) {
      return false;
    }
    if (this.st.distributedPhotoCapturePending) {
      this.st.lastPhotoHint = '上一张远端照片仍在处理…';
      return false;
    }
    this.st.distributedPhotoCapturePending = true;
    this.st.distributedPhotoSaveDegrees = this.resolveDistributedPhotoSaveDegrees(settings);
    const captureSettings: camera.PhotoCaptureSetting = {
      quality: settings.quality,
      rotation: camera.ImageRotation.ROTATION_0,
      mirror: settings.mirror ?? false
    };
    const waitDone = new Promise<boolean>((resolve) => {
      this.st.distributedPhotoCaptureDone = resolve;
    });
    const timer = setTimeout(() => {
      if (this.st.distributedPhotoCapturePending) {
        DhDiag.warn('CF', 'dist photo capture timeout');
        this.finishDistributedPhotoHint('远端拍照超时（未收到图像帧）', false);
      }
    }, AppConstants.REMOTE_PHOTO_CAPTURE_TIMEOUT_MS) as number;
    try {
      DhDiag.info('CF', `dist photo capture saveDeg=${this.st.distributedPhotoSaveDegrees} captureRotation=0`);
      this.st.photoOutput.capture(captureSettings);
      DhDiag.info('CF', 'takePhoto capture ImageReceiver path');
      return await waitDone;
    } catch (err) {
      this.finishDistributedPhotoHint(`拍照异常: ${JSON.stringify(err)}`, false);
      return false;
    } finally {
      clearTimeout(timer);
    }
  }

  public registerPhotoCallback(): void {
    if (!this.st.photoOutput) {
      return;
    }
    try {
      this.st.photoOutput.off('photoAvailable');
    } catch (_e) {
      DhDiag.logCatchWarn('CF', 'registerPhotoCallback off photoAvailable', _e);
    }
    this.st.photoOutput.on('photoAvailable', (err: BusinessError, photo: camera.Photo) => {
      if (err) {
        this.st.lastPhotoHint = `拍照失败: ${err.message}`;
        hilog.error(AppConstants.LOG_DOMAIN, TAG, 'photoAvailable err: %{public}s', JSON.stringify(err));
        if (this.st.photoSaveListener) {
          this.st.photoSaveListener(this.st.lastPhotoHint, false);
        }
        return;
      }
      this.st.galleryService.savePhotoFromCamera(photo).then((result) => {
        this.st.lastPhotoHint = result.message;
        DhDiag.info('GALLERY', `photo save result success=${result.success ? 1 : 0} msg=${result.message}`);
        if (this.st.photoSaveListener) {
          this.st.photoSaveListener(result.message, result.success);
        }
      }).catch((err: Error) => {
        this.st.lastPhotoHint = `相册保存失败: ${err.message}`;
        DhDiag.warn('GALLERY', `photo save failed ${JSON.stringify(err)}`);
        if (this.st.photoSaveListener) {
          this.st.photoSaveListener(this.st.lastPhotoHint, false);
        }
      });
    });
  }

  public resolveSceneMode(device: camera.CameraDevice, mode: StudioMode,
    _pipeline: StreamPipelineType = StreamPipelineType.CONTINUOUS): camera.SceneMode {
    const modes = this.st.cameraManager!.getSupportedSceneModes(device);
    if (mode === StudioMode.VIDEO && modes.includes(camera.SceneMode.NORMAL_VIDEO)) {
      const videoCap: camera.CameraOutputCapability | undefined = this.st.cameraManager!.getSupportedOutputCapability(device, camera.SceneMode.NORMAL_VIDEO);
      const hasVideo = (videoCap?.videoProfiles?.length ?? 0) > 0;
      const hasPreview = (videoCap?.previewProfiles?.length ?? 0) > 0;
      if (hasVideo || hasPreview) {
        return camera.SceneMode.NORMAL_VIDEO;
      }
      DhDiag.warn('CF', 'NORMAL_VIDEO 无 video/preview capability，降级 NORMAL_PHOTO');
    }
    if (modes.includes(camera.SceneMode.NORMAL_PHOTO)) {
      return camera.SceneMode.NORMAL_PHOTO;
    }
    return modes[0];
  }

  /** 本机预览用 PhotoSession（部分机型 VideoSession 仅预览时 stop 易 7400101）；开录时再升 VideoSession */
  public resolveLocalPreviewSceneMode(device: camera.CameraDevice): camera.SceneMode {
    const modes = this.st.cameraManager!.getSupportedSceneModes(device);
    if (modes.includes(camera.SceneMode.NORMAL_PHOTO)) {
      return camera.SceneMode.NORMAL_PHOTO;
    }
    return modes[0];
  }

  /** 预览(Photo) → 录像(Video) 升级（委托 VideoController） */
  public async prepareVideoSessionForRecording(target: CameraTarget): Promise<boolean> {
    return this.facade.getModeOrchestrator().video.prepareForRecording(target);
  }

  public findCameraDevice(cameraId: string, includeRemoteProbe: boolean = false): camera.CameraDevice | undefined {
    return this.facade.collectCameraDevices(includeRemoteProbe).find(c => c.cameraId === cameraId);
  }

  /** 辅画面（PIP）解析物理相机 — 与主 Session 独立 */
  public resolveDeviceForTarget(target: CameraTarget): camera.CameraDevice | undefined {
    const probeRemote = target.sourceType === CameraSourceType.DISTRIBUTED;
    const byId = this.findCameraDevice(target.cameraId, probeRemote);
    if (byId) {
      return byId;
    }
    if (target.sourceType === CameraSourceType.LOCAL) {
      return this.findLocalCameraDevice(target.position);
    }
    if (target.networkId) {
      return this.findRemoteCameraDevice(target.networkId, target.position);
    }
    return undefined;
  }

  /** 本机多后摄时优先 device/N 序号更小者（部分机型超广角为 device/0，避免误开 device/11 主摄观感偏「放大」） */
  private static localBackCameraSortKey(cameraId: string): number {
    const m = cameraId.match(/device\/(\d+)/);
    if (m) {
      return parseInt(m[1], 10);
    }
    return 9999;
  }

  public findLocalCameraDevice(position: string = 'back'): camera.CameraDevice | undefined {
    const locals = this.facade.collectCameraDevices(false).filter(d =>
      d.connectionType !== camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
      (d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_BACK ||
        d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_FRONT));
    const posEnum = position === 'front' ?
      camera.CameraPosition.CAMERA_POSITION_FRONT : camera.CameraPosition.CAMERA_POSITION_BACK;
    const samePos = locals.filter(d => d.cameraPosition === posEnum);
    if (samePos.length === 0) {
      DhDiag.warn('CF', `findLocalCameraDevice miss position=${position} locals=${locals.length}`);
      return undefined;
    }
    if (position === 'back' && samePos.length > 1) {
      samePos.sort((a, b) =>
        CameraSessionPreview.localBackCameraSortKey(a.cameraId) -
        CameraSessionPreview.localBackCameraSortKey(b.cameraId));
      DhDiag.info('CF',
        `findLocalCameraDevice back pick ${samePos[0].cameraId} among ${samePos.length} ` +
        `candidates=${samePos.map(d => d.cameraId).join(',')}`);
    }
    return samePos[0];
  }

  public findRemoteCameraDevice(networkId: string, position: string = 'back'): camera.CameraDevice | undefined {
    const posEnum = position === 'front' ?
      camera.CameraPosition.CAMERA_POSITION_FRONT : camera.CameraPosition.CAMERA_POSITION_BACK;
    return this.facade.collectCameraDevices(true).find(d =>
      d.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
      d.cameraPosition === posEnum &&
      (CameraSessionUtils.networkIdMatches(d.cameraId, networkId) ||
        CameraSessionUtils.networkIdMatches(d.hostDeviceName ?? '', networkId))) ??
      this.facade.collectCameraDevices(true).find(d =>
        d.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
        d.cameraPosition === posEnum);
  }

  public findRemoteCameraTarget(networkId: string, position: string = 'back'): CameraTarget | null {
    const device = this.findRemoteCameraDevice(networkId, position);
    if (!device) {
      return null;
    }
    return this.facade.buildCameraTargets([device]).find(c => c.position === position) ??
      this.facade.buildCameraTargets([device])[0] ?? null;
  }

  public async closeSession(): Promise<void> {
    DhDiag.flowBegin('CF', 'closeSession',
      `recording=${this.st.isRecordingVideo ? 1 : 0} active=${this.st.isPreviewActive ? 1 : 0}`);
    if (this.st.isRecordingVideo) {
      await this.facade.stopRecordingInternal();
    }
    await this.facade.forceReleaseAll();
    DhDiag.flowEnd('CF', 'closeSession', true, '');
  }

  public async closeSessionInternal(): Promise<void> {
    try {
      if (this.st.session) {
        await this.st.session.stop();
        await this.st.session.release();
        this.st.session = null;
      }
      if (this.st.videoOutput) {
        await this.st.videoOutput.release();
        this.st.videoOutput = null;
      }
      await this.releasePhotoImageReceiver();
      if (this.st.photoOutput) {
        this.st.photoOutput.off('photoAvailable');
        await this.st.photoOutput.release();
        this.st.photoOutput = null;
      }
      if (this.st.previewOutput) {
        await this.st.previewOutput.release();
        this.st.previewOutput = null;
        this.st.activePreviewFps = 0;
      }
      if (this.st.cameraInput) {
        await this.st.cameraInput.close();
        this.st.cameraInput = null;
      }
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'closeSessionInternal err: %{public}s', JSON.stringify(err));
    }
  }

  public async takePhoto(): Promise<boolean> {
    if (!this.st.cameraManager) {
      this.st.lastPhotoHint = '相机未就绪';
      return false;
    }
    const isDistributed = this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED;
    DhDiag.info('CF',
      `takePhoto enter distributed=${isDistributed ? 1 : 0} photoOut=${this.st.photoOutput ? 1 : 0} ` +
      `sceneMode=${this.st.sessionSceneMode}`);
    if (this.st.recordingPrepareActive || this.st.isRecordingVideo) {
      this.st.lastPhotoHint = '录像准备/进行中，请先停止录像再拍照';
      return false;
    }
    if (this.st.currentTarget) {
      const sessionOk = await this.facade.getModeOrchestrator().photo.ensurePhotoSession(this.st.currentTarget);
      if (!sessionOk) {
        if (this.st.lastPhotoHint.length === 0) {
          const detail = this.facade.getLastOpenErrorMessage();
          this.st.lastPhotoHint = detail.length > 0 ? `无法打开拍照会话: ${detail}` : '拍照会话未就绪';
        }
        return false;
      }
    }
    if (!this.st.photoOutput) {
      this.st.lastPhotoHint = 'PhotoOutput 未就绪，请先切换到拍照模式';
      return false;
    }
    const captureDevice = this.st.activeCameraDevice;
    const settings = captureDevice ?
      CameraSessionUtils.buildPhotoCaptureSetting(this.st.photoOutput, captureDevice) :
      CameraSessionUtils.defaultPhotoCaptureSetting();
    DhDiag.info('CF',
      `takePhoto settings rotation=${settings.rotation} mirror=${settings.mirror ? 1 : 0}`);
    if (isDistributed && this.st.useDistributedPhotoReceiver) {
      await CameraSessionUtils.delay(400);
      DhDiag.info('CF', `takePhoto capture distributed=1 receiver=1`);
      const ok = await this.captureDistributedPhoto(settings);
      if (!ok && this.st.lastPhotoHint.length === 0) {
        this.st.lastPhotoHint = '远端拍照失败';
      }
      return ok;
    }
    this.registerPhotoCallback();
    if (isDistributed) {
      await CameraSessionUtils.delay(300);
    }
    try {
      DhDiag.info('CF', `takePhoto capture distributed=${isDistributed ? 1 : 0} receiver=0`);
      this.st.photoOutput.capture(settings);
      this.st.lastPhotoHint = '拍照指令已发送，等待保存相册…';
      return true;
    } catch (err) {
      this.st.lastPhotoHint = `拍照异常: ${JSON.stringify(err)}`;
      DhDiag.error('CF', this.st.lastPhotoHint);
      return false;
    }
  }

  public setRecordingFileStamp(stamp: number): void {
    this.st.recordingFileStamp = stamp;
  }

  /** ViewModel 点录像前调用，锁定当前预览选定分辨率/帧率 */
  public pinUserStreamSpecForRecording(): void {
    this.facade.captureUserStreamSpecForRecording();
  }

  /** 开录前将 selected* 与 lastCommitted/pinned 对齐（beforeRecordPin 诊断前调用） */
  public ensureUserStreamSpecForRecording(): void {
    const effective = this.facade.pickAuthoritativeForRecording();
    if (effective === null) {
      return;
    }
    this.facade.restoreStreamSpecSnapshot(effective);
    this.facade.syncSelectedFromIndices();
  }

  public async toggleRecording(start: boolean): Promise<boolean> {
    if (!this.st.cameraManager) {
      return false;
    }
    if (start) {
      DhDiag.info('CF', `toggleRecording start target=${this.st.currentTarget?.sourceType ?? 'n/a'} ` +
        `pos=${this.st.currentTarget?.position ?? 'n/a'}`);
      return this.facade.startRecordingInternal();
    }
    return this.facade.stopRecording();
  }


  /** 无首帧自愈（单飞）：仅 frameStart 超时后 CF 轻量重拉；禁止在 ACTIVE 瞬间打断首轮 session.start */
  public scheduleDistributedPreviewHeal(reason: string): void {
    if (reason === 'switch-active-no-frame') {
      return;
    }
    if (this.st.distributedPreviewHealInFlight || !this.st.isPreviewActive) {
      return;
    }
    const target = this.st.currentTarget;
    if (!target || target.sourceType !== CameraSourceType.DISTRIBUTED || !target.networkId) {
      return;
    }
    this.st.distributedPreviewHealInFlight = true;
    const networkId = target.networkId;
    const captureMs = Math.floor(AppConstants.PREVIEW_FIRST_FRAME_REMOTE_MS / 2);
    DhDiag.warn('CF', `distributedPreviewHeal schedule reason=${reason} net=${networkId.substring(0, 12)}...`);
    void (async () => {
      try {
        await CameraSessionUtils.delay(500);
        let gotFrame = await this.restartDistributedPreviewCapture(captureMs);
        if (!gotFrame && !this.st.previewFrameReceived) {
          // 预览 Session 已 ACTIVE 时禁止 DH stop→resume，避免把已显示的首帧冻死在 Surface 上
          DhDiag.warn('CF', 'distributedPreviewHeal CF retry + background DH resume (no stop/renegotiate)');
          void DistributedHardwareService.getInstance().resumeCamera(networkId);
          await CameraSessionUtils.delay(AppConstants.DH_RESUME_RETRY_GAP_MS);
          gotFrame = await this.restartDistributedPreviewCapture(captureMs);
        }
        this.notifyDistributedPreviewFrame(gotFrame || this.st.previewFrameReceived);
        DhDiag.info('CF',
          `distributedPreviewHeal done reason=${reason} gotFrame=${gotFrame || this.st.previewFrameReceived ? 1 : 0}`);
      } catch (err) {
        DhDiag.logCatchWarn('CF', 'distributedPreviewHeal', err);
      } finally {
        this.st.distributedPreviewHealInFlight = false;
      }
    })();
  }

  private notifyDistributedPreviewFrame(ready: boolean): void {
    const hint = ready ? '' : AppConstants.REMOTE_PREVIEW_NO_FRAME_HINT;
    this.st.distributedPreviewFrameListeners.forEach((fn) => {
      try {
        fn(ready, hint);
      } catch (err) {
        DhDiag.logCatchWarn('CF', 'notifyDistributedPreviewFrame listener', err);
      }
    });
  }

  /** 远端无首帧：纯 CF stop→start 重拉预览并等待 frameStart */
  public async restartDistributedPreviewCapture(timeoutMs: number): Promise<boolean> {
    if (!this.st.session || !this.st.previewOutput) {
      DhDiag.warn('CF', 'restartDistributedPreviewCapture: no active session');
      return false;
    }
    this.st.previewFrameReceived = false;
    const onLateFrame = (): void => {
      this.st.previewFrameReceived = true;
      this.st.distributedPreviewFrameListeners.forEach((fn) => {
        try {
          fn(true, '');
        } catch (err) {
          DhDiag.logCatchWarn('CF', 'restartDistributedPreviewCapture lateFrame listener', err);
        }
      });
    };
    const got = await PreviewFrameWaiter.restartCaptureAndWaitFrame(
      this.st.session, this.st.previewOutput, timeoutMs, onLateFrame);
    if (got) {
      this.st.previewFrameReceived = true;
      this.notifyDistributedPreviewFrame(true);
      return true;
    }
    this.st.lastOpenErrorMessage = AppConstants.REMOTE_PREVIEW_NO_FRAME_HINT;
    return false;
  }

  public healDistributedPreviewNoFrame(reason: string): void {
    this.scheduleDistributedPreviewHeal(reason);
  }
}
