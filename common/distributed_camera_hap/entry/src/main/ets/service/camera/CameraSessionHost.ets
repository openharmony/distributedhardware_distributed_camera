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
import {
  CameraResolutionOption,
  CameraTarget,
  StreamPipelineType,
  StudioMode
} from '../../common/types/CameraTypes';

/** 远端相机发现进度回调 */
export type RemoteCameraProgressListener = (attempt: number, total: number, message: string) => void;

/** 用户选中的分辨率/帧率快照（模式切换时保留） */
export interface CameraStreamSpecSnapshot {
  width: number;
  height: number;
  fps: number;
  resolutionIndex: number;
  fpsIndex: number;
}

/**
 * 相机管线宿主 — Preview / Photo / Video 控制器仅依赖此接口，与 CameraSessionService 解耦。
 */
export interface ICameraSessionHost {
  forceReleaseAll(): Promise<boolean>;

  hardResetCameraPipeline(fastCooldown?: boolean): Promise<void>;

  /** 同镜头 Session 内更换 PreviewOutput 分辨率（失败时由 hardReset 兜底） */
  tryHotReconfigurePreviewSpec(): Promise<boolean>;

  waitForCameraServiceReady(maxMs: number, initialSettleMs: number): Promise<boolean>;

  releaseForRemoteLensSwitch(): Promise<void>;

  delayMs(ms: number): Promise<void>;

  openAndPreview(
    target: CameraTarget,
    mode: StudioMode,
    pipeline: StreamPipelineType,
    skipPreRelease: boolean
  ): Promise<boolean>;

  getCurrentTarget(): CameraTarget | null;

  getCurrentMode(): StudioMode;

  getCurrentPipeline(): StreamPipelineType;

  isRecordingActive(): boolean;

  hasCameraManager(): boolean;

  getResolutionOptionCount(): number;

  getResolutionOptionAt(index: number): CameraResolutionOption | null;

  getResolutionIndex(): number;

  getFpsOptionCount(): number;

  getFpsIndex(): number;

  getFpsAtIndex(index: number): number;

  snapshotStreamSpec(): CameraStreamSpecSnapshot;

  /** 预览规格切换失败回滚：lastCommitted / 当前预览，而非 UI 已 stage 的 selected* */
  capturePreviewRollbackSpec(): CameraStreamSpecSnapshot;

  restoreStreamSpecSnapshot(snapshot: CameraStreamSpecSnapshot): void;

  pinUserStreamSpec(snapshot: CameraStreamSpecSnapshot): void;

  stageUserStreamSpec(resolutionIndex: number, fpsIndex: number): void;

  commitUserStreamSpec(source: string): void;

  restoreUserStreamSpec(snapshot: CameraStreamSpecSnapshot): void;

  syncSelectedFromIndices(): void;

  /** 开录前锁定 UI 选定分辨率/帧率 */
  pinUserStreamSpecForRecording(): void;

  /** 开录前将内存 selected* 与 lastCommitted 对齐 */
  ensureUserStreamSpecForRecording(): void;

  shouldSkipHotReconfigureForStability(): boolean;

  /** 预览分辨率/帧率切换（布尔锁，避免 Promise 链死锁） */
  runPreviewSpecChange(fn: () => Promise<boolean>): Promise<boolean>;

  /** L1 热换（仅本机）→ L3 cold；远端分辨率变化时 cold + DH stop/resume 重协商 */
  reopenPreviewWithUserSpec(): Promise<boolean>;

  resetSpecChangeGate(): void;

  runExclusiveSpecChange<T>(label: string, fn: () => Promise<T>): Promise<T>;

  restoreStreamSpecSelection(resIdx: number, width: number, height: number, fpsIdx: number, fps: number): void;

  applyResolutionSelection(index: number): void;

  applyFpsSelection(index: number): void;

  setLastPhotoHint(hint: string): void;

  tryApplyPreviewFps(fps: number): boolean;

  syncVideoPlanFromSelection(): void;

  getActivePreviewFps(): number;

  getActiveRecordFps(): number;

  getActivePreviewWidth(): number;

  getActivePreviewHeight(): number;

  getActiveVideoWidth(): number;

  getActiveVideoHeight(): number;

  getSelectedResolution(): CameraResolutionOption | null;

  canReuseLocalVideoSession(target: CameraTarget): boolean;

  refreshVideoPlanFromSelection(isDistributed: boolean): void;

  discoverRemoteCamerasForRecord(networkId: string): Promise<void>;

  hasPhotoOutput(): boolean;

  isPhotoSceneMode(): boolean;

  getSessionSceneMode(): camera.SceneMode;

  getLastOpenErrorMessage(): string;
}
