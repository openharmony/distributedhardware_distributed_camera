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
import { SessionReportService } from '../SessionReportService';
import { CameraStreamSpecSnapshot } from './CameraSessionHost';
import { CameraSessionProfileHelpers } from './CameraSessionProfileHelpers';
import { CameraSessionUtils } from './CameraSessionUtils';
import { CameraSessionState } from './CameraSessionState';
import { CameraSessionService } from './CameraSessionService';
import { CameraModeOrchestrator } from './CameraModeOrchestrator';
import { PreviewFrameWaiter } from './PreviewFrameWaiter';

const TAG = AppConstants.TAG;

interface BusinessError { code: number; message: string; }

interface CameraManagerRemoteProbe extends camera.CameraManager {
  getCameraDevices(position: camera.CameraPosition, types: Array<camera.CameraType>,
    connectType: camera.ConnectionType): Array<camera.CameraDevice>;
}

export class CameraSessionControls {
  private readonly facade: CameraSessionService;

  constructor(facade: CameraSessionService) {
    this.facade = facade;
  }

  private get st(): CameraSessionState { return this.facade.state; }

  public async applySelectedFrameRateAt(index: number): Promise<boolean> {
    return this.facade.getModeOrchestrator().preview.applyFrameRateAt(index);
  }

  public async cycleSelectedFrameRate(): Promise<boolean> {
    if (this.st.isRecordingVideo || !this.st.currentTarget || !this.st.cameraManager) {
      return false;
    }
    if (this.st.fpsOptions.length <= 1) {
      this.st.lastPhotoHint = '当前分辨率下仅有一种帧率';
      return false;
    }
    const next = (this.st.fpsIndex + 1) % this.st.fpsOptions.length;
    return this.applySelectedFrameRateAt(next);
  }

  /**
   * 远端录像设置弹层：仅 pin 用户规格，不重建预览 Session（避免 DH 重协商卡死在「应用规格」）。
   * Photo→Video 开录时由 video.prepare 统一做 stop/resume + open。
   */
  public applyDistributedRecordingStreamSpec(resIdx: number, targetFps: number): boolean {
    if (resIdx < 0 || resIdx >= this.st.resolutionOptions.length) {
      return false;
    }
    this.facade.applyResolutionSelection(resIdx);
    const supported = this.facade.getSupportedFpsForResolutionIndex(resIdx);
    if (targetFps > 0 && supported.length > 0 && !supported.includes(targetFps)) {
      this.st.lastPhotoHint = `当前分辨率不支持 ${targetFps}fps 录像`;
      return false;
    }
    if (targetFps > 0) {
      this.st.selectedFps = targetFps;
      const fidx = this.st.fpsOptions.findIndex(o => o.fps === targetFps);
      if (fidx >= 0) {
        this.st.fpsIndex = fidx;
      }
    } else if (supported.length > 0) {
      this.st.selectedFps = CameraSessionProfileHelpers.pickDefaultFps(supported);
      this.st.fpsIndex = this.st.fpsOptions.findIndex(o => o.fps === this.st.selectedFps);
      if (this.st.fpsIndex < 0) {
        this.st.fpsIndex = 0;
      }
    }
    this.facade.stageUserStreamSpec(resIdx, this.st.fpsIndex);
    this.facade.refreshVideoPlanFromSelection(true);
    const snap = this.facade.snapshotStreamSpec();
    this.facade.pinUserStreamSpec(snap);
    this.facade.commitUserStreamSpec('recordingSetup.dist.pin');
    DhDiag.info('CF',
      `applyDistributedRecordingStreamSpec pin ${snap.width}x${snap.height}@${snap.fps}fps (deferred Video open)`);
    return true;
  }

  /** 按目标 fps 应用（1–60；0 表示自动选默认档） */
  public async applyFrameRateFps(fps: number): Promise<boolean> {
    if (this.st.isRecordingVideo || !this.st.currentTarget || !this.st.cameraManager) {
      return false;
    }
    if (fps <= 0) {
      const opts = this.st.fpsOptions.map(o => o.fps);
      fps = CameraSessionProfileHelpers.pickDefaultFps(opts.length > 0 ? opts : [30]);
    }
    const idx = this.st.fpsOptions.findIndex(o => o.fps === fps);
    if (idx >= 0) {
      return this.applySelectedFrameRateAt(idx);
    }
    if (!this.facade.isFpsSupportedByCapability(fps)) {
      this.st.lastPhotoHint = `当前分辨率不支持 ${fps}fps`;
      return false;
    }
    this.st.selectedFps = fps;
    const fidx = this.st.fpsOptions.findIndex(o => o.fps === fps);
    this.st.fpsIndex = fidx >= 0 ? fidx : 0;
    DhDiag.info('CF', `applyFrameRateFps direct -> ${fps}fps`);
    return this.applySelectedFrameRateAt(this.st.fpsIndex);
  }

  public isFpsOptionSupported(fps: number): boolean {
    if (fps <= 0) {
      return true;
    }
    return this.st.fpsOptions.some(o => o.fps === fps);
  }

  public getZoomCapability(): CameraZoomCapability {
    return {
      supported: this.st.zoomSupported,
      minRatio: this.st.zoomMinRatio,
      maxRatio: this.st.zoomMaxRatio,
      currentRatio: this.st.currentZoomRatio
    };
  }

  public getFocusCapability(): CameraFocusCapability {
    return {
      supported: this.st.focusSupported,
      tapToFocusSupported: this.st.tapFocusSupported,
      modeLabel: this.st.focusModeLabel
    };
  }

  /** Zoom.setZoomRatio — 文档建议设置后 1–2 帧再 getZoomRatio 读回 */
  public ensureZoomCapabilityReady(): boolean {
    if (this.st.zoomSupported) {
      return true;
    }
    const session = this.st.session;
    if (!session || !this.st.isPreviewActive) {
      return false;
    }
    try {
      this.facade.refreshZoomFromSession(session);
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'ensureZoomCapabilityReady refreshZoomFromSession', err);
      return false;
    }
    return this.st.zoomSupported;
  }

  public async setZoomRatio(ratio: number): Promise<boolean> {
    const session = this.st.session;
    if (!session) {
      this.st.lastPhotoHint = '预览未就绪';
      return false;
    }
    if (!this.st.zoomSupported && !this.ensureZoomCapabilityReady()) {
      this.st.lastPhotoHint = '当前镜头不支持变焦';
      return false;
    }
    const clamped = CameraSessionProfileHelpers.clampZoom(ratio, this.st.zoomMinRatio, this.st.zoomMaxRatio);
    this.st.currentZoomRatio = clamped;
    try {
      session.setZoomRatio(clamped);
      DhDiag.info('CF', `setZoomRatio target=${clamped.toFixed(2)}`);
      return true;
    } catch (err) {
      DhDiag.warn('CF', `setZoomRatio failed ${JSON.stringify(err)}`);
      this.st.lastPhotoHint = '变焦设置失败';
      return false;
    }
  }

  /** Zoom.setSmoothZoom — 滑条/捏合等连续调节 */
  public async setSmoothZoomRatio(ratio: number): Promise<boolean> {
    const session = this.st.session;
    if (!session) {
      this.st.lastPhotoHint = '预览未就绪';
      return false;
    }
    if (!this.st.zoomSupported && !this.ensureZoomCapabilityReady()) {
      this.st.lastPhotoHint = '当前镜头不支持变焦';
      return false;
    }
    const clamped = CameraSessionProfileHelpers.clampZoom(ratio, this.st.zoomMinRatio, this.st.zoomMaxRatio);
    try {
      session.setSmoothZoom(clamped, camera.SmoothZoomMode.NORMAL);
      this.st.currentZoomRatio = clamped;
      DhDiag.info('CF', `setSmoothZoom ${clamped.toFixed(2)}`);
      return true;
    } catch (err) {
      DhDiag.warn('CF', `setSmoothZoom fallback setZoomRatio ${JSON.stringify(err)}`);
      return this.setZoomRatio(clamped);
    }
  }

  /**
   * Focus.setFocusPoint — 竖屏预览触点 (localX,localY)，区域 areaW×areaH
   * 坐标换算见 Focus.setFocusPoint 文档：{y/h, 1-x/w}
   */
  public async focusAtPreviewTap(localX: number, localY: number, areaW: number, areaH: number): Promise<boolean> {
    const session = this.st.session;
    if (!session) {
      DhDiag.warn('CF',
        `focusAtPreviewTap skip: session=null tap=(${localX},${localY}) area=${areaW}x${areaH}`);
      this.st.lastPhotoHint = '预览未就绪';
      return false;
    }
    if (areaW <= 0 || areaH <= 0) {
      DhDiag.warn('CF',
        `focusAtPreviewTap skip: invalid area ${areaW}x${areaH} tap=(${localX},${localY})`);
      return false;
    }
    if (!this.st.tapFocusSupported) {
      this.st.lastPhotoHint = '当前镜头不支持点击对焦';
      return false;
    }
    const point: camera.Point = {
      x: Math.min(1, Math.max(0, localY / areaH)),
      y: Math.min(1, Math.max(0, 1 - localX / areaW))
    };
    try {
      if (!CameraSessionProfileHelpers.applyTapFocusMode(session)) {
        this.st.lastPhotoHint = '当前镜头不支持点击对焦';
        return false;
      }
      session.setFocusPoint(point);
      DhDiag.info('CF', `focusAtPreviewTap norm=(${point.x.toFixed(2)},${point.y.toFixed(2)})`);
      this.st.lastPhotoHint = '已对焦';
      return true;
    } catch (err) {
      DhDiag.warn('CF', `focusAtPreviewTap failed ${JSON.stringify(err)}`);
      this.st.lastPhotoHint = '对焦失败';
      return false;
    }
  }

  public getCurrentMode(): StudioMode {
    return this.st.currentMode;
  }

  public getCurrentPipeline(): StreamPipelineType {
    return this.st.currentPipeline;
  }

  public getLastPhotoHint(): string {
    return this.st.lastPhotoHint;
  }

  public getVideoFilePath(): string {
    return this.st.videoFilePath;
  }

  public isActive(): boolean {
    return this.st.isPreviewActive || this.isMainSessionRunning();
  }

  public isMainSessionRunning(): boolean {
    return this.st.session !== null && this.st.cameraInput !== null;
  }

  public getRecordingSegments(): string[] {
    return this.st.recordingSegmentPaths.slice();
  }

  public onPhotoSaveResult(listener: (message: string, success: boolean) => void): void {
    this.st.photoSaveListener = listener;
  }

  public offPhotoSaveResult(): void {
    this.st.photoSaveListener = null;
  }

  public onRecordingStateChanged(listener: (recording: boolean, hint: string) => void): void {
    this.st.recordingStateListeners.push(listener);
  }

  public offRecordingStateChanged(listener: (recording: boolean, hint: string) => void): void {
    const idx = this.st.recordingStateListeners.indexOf(listener);
    if (idx >= 0) {
      this.st.recordingStateListeners.splice(idx, 1);
    }
  }

  public onRecordingProgress(listener: (hint: string) => void): void {
    this.st.recordingProgressListeners.push(listener);
  }

  public offRecordingProgress(listener: (hint: string) => void): void {
    const idx = this.st.recordingProgressListeners.indexOf(listener);
    if (idx >= 0) {
      this.st.recordingProgressListeners.splice(idx, 1);
    }
  }

  public onCameraControlsApplied(listener: () => void): void {
    this.st.cameraControlsAppliedListeners.push(listener);
  }

  public offCameraControlsApplied(listener: () => void): void {
    const idx = this.st.cameraControlsAppliedListeners.indexOf(listener);
    if (idx >= 0) {
      this.st.cameraControlsAppliedListeners.splice(idx, 1);
    }
  }

  /** 预览稳定后延迟探测变焦/对焦（冷启动可 defer，结束后再调本方法） */
  public scheduleSessionCameraControlsRefresh(): void {
    this.facade.scheduleSessionCameraControls();
  }

  public notifyCameraControlsApplied(): void {
    for (const listener of this.st.cameraControlsAppliedListeners) {
      listener();
    }
  }

  public reportRecordingProgress(hint: string): void {
    for (const listener of this.st.recordingProgressListeners) {
      listener(hint);
    }
  }

  public emitRecordingState(): void {
    const hint = this.st.lastPhotoHint;
    const recording = this.st.isRecordingVideo;
    for (const listener of this.st.recordingStateListeners) {
      listener(recording, hint);
    }
  }

  /** 录制中切流前：结束当前片段（仅落沙箱，不弹相册） */
  public async finalizeRecordingSegment(): Promise<string> {
    if (!this.st.isRecordingVideo) {
      return '';
    }
    const path = this.st.videoFilePath;
    const ok = await this.facade.stopRecordingInternal(false);
    if (ok && path) {
      SessionReportService.getInstance().recordRecordingSegment();
      return path;
    }
    return '';
  }

  public isRecording(): boolean {
    return this.st.isRecordingVideo;
  }

  public hasActivePhotoSession(): boolean {
    return this.st.photoOutput !== null && this.st.sessionSceneMode === camera.SceneMode.NORMAL_PHOTO;
  }

  /** 已开始写片（含 isRecordingVideo 刚被清掉的停止窗口） */
  public hasRecordingSession(): boolean {
    return this.st.isRecordingVideo || this.st.recordingStartedAtMs > 0 || this.st.recordingPrepareActive;
  }

  public getRecordingStartedAtMs(): number {
    return this.st.recordingStartedAtMs;
  }

  public getRecordingElapsedMs(): number {
    if (this.st.recordingStartedAtMs <= 0) {
      return 0;
    }
    return Date.now() - this.st.recordingStartedAtMs;
  }

  public requestAbortRecordingStart(): void {
    this.st.recordingStartAborted = true;
    DhDiag.info('CF', 'recording start abort requested');
  }

  /** 用户点停且开录未完成：尽快让 stopRecordingForUser 脱离 prep 轮询 */
  public forceEndRecordingPrepare(): void {
    this.st.recordingStartAborted = true;
    this.st.recordingPrepareActive = false;
    this.facade.resetSpecChangeGate();
  }

  /**
   * 启动尚未进入 isRecordingVideo 时中止（如用户很快点停）；saveToGallery 时尝试保存已有片段。
   */
  public async cancelRecordingInFlight(saveToGallery: boolean = true): Promise<boolean> {
    this.st.recordingStartAborted = true;
    if (this.st.isRecordingVideo || this.st.recordingStartedAtMs > 0 ||
      this.st.avRecorder || this.st.videoFilePath || this.st.recordFile) {
      return this.facade.stopRecordingInternal(saveToGallery);
    }
    const savedPath = this.st.videoFilePath;
    try {
      if (this.st.videoOutput) {
        try {
          await this.st.videoOutput.stop();
        } catch (err) {
          DhDiag.logCatchWarn('CF', 'cancelRecordingInFlight videoOutput.stop', err);
        }
      }
      const isDistributed = this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED;
      const flushMs = isDistributed ? AppConstants.RECORD_FILE_FLUSH_MS_REMOTE : AppConstants.RECORD_FILE_FLUSH_MS_LOCAL;
      await CameraSessionUtils.delay(flushMs);
      await this.facade.releaseAvRecorderOnly();
      if (this.st.session && this.st.sessionSceneMode === camera.SceneMode.NORMAL_VIDEO) {
        await this.facade.detachVideoOutputAfterRecord();
      }
    } catch (err) {
      DhDiag.warn('CF', `cancelRecordingInFlight cleanup ${JSON.stringify(err)}`);
      await this.facade.releaseAvRecorderOnly();
    }
    this.st.isRecordingVideo = false;
    this.st.recordingStartedAtMs = 0;
    this.emitRecordingState();

    const isDistributed = this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED;
    let size = savedPath.length > 0 ? await this.facade.awaitRecordingFileReady(isDistributed, savedPath) : 0;
    if (saveToGallery && savedPath && size >= 256) {
      DhDiag.info('CF', `cancelRecordingInFlight save partial size=${size}`);
      return await this.facade.saveRecordingToGallery(savedPath, true);
    }
    if (saveToGallery && await this.facade.trySaveAnyRecordingToGallery()) {
      return true;
    }
    this.st.lastPhotoHint = saveToGallery ?
      '录像未生成有效文件，未写入相册' : '录像已取消';
    DhDiag.info('CF', `cancelRecordingInFlight: ${this.st.lastPhotoHint} path=${savedPath} size=${size}`);
    this.emitRecordingState();
    return false;
  }
}

