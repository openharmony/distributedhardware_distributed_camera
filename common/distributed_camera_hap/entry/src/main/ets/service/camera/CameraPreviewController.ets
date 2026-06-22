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

import { CameraSourceType, CameraTarget } from '../../common/types/CameraTypes';
import { DhDiag } from '../../common/util/DhDiag';
import { ICameraSessionHost } from './CameraSessionHost';

/**
 * 预览模式：分辨率/帧率 — L0 setFrameRate → L1 Session 内热换 → L3 hardReset；
 * 远端换分辨率走 cold + DH stop/resume 重协商，跳过 setFrameRate。
 */
export class CameraPreviewController {
  private readonly host: ICameraSessionHost;

  constructor(host: ICameraSessionHost) {
    this.host = host;
  }

  public async applyResolutionAt(index: number): Promise<boolean> {
    if (this.host.isRecordingActive() || !this.host.getCurrentTarget() || !this.host.hasCameraManager()) {
      return false;
    }
    if (this.host.getResolutionOptionCount() === 0) {
      this.host.setLastPhotoHint('当前镜头无可用分辨率');
      return false;
    }
    if (index < 0 || index >= this.host.getResolutionOptionCount()) {
      return false;
    }
    if (index === this.host.getResolutionIndex()) {
      return true;
    }
    const prevSnap = this.host.capturePreviewRollbackSpec();
    const prevIndex = prevSnap.resolutionIndex;
    this.host.applyResolutionSelection(index);
    this.host.stageUserStreamSpec(index, this.host.getFpsIndex());
    const opt = this.host.getResolutionOptionAt(index);
    DhDiag.info('CF', `preview.applyResolution[${index}] -> ${opt?.label ?? index}`);

    DhDiag.info('CF', 'preview.applyResolution — enter reopen');
    let ok = false;
    try {
      ok = await this.reopenWithSpecChange();
    } catch (err) {
      DhDiag.warn('CF', `preview.applyResolution reopen threw ${JSON.stringify(err)}`);
      ok = false;
    }
    if (ok) {
      this.host.commitUserStreamSpec('preview.applyResolution.ok');
      const sel = this.host.getSelectedResolution();
      if (sel && (sel.width !== this.host.getActivePreviewWidth() ||
        sel.height !== this.host.getActivePreviewHeight())) {
        this.host.setLastPhotoHint(
          `预览就近 ${this.host.getActivePreviewWidth()}x${this.host.getActivePreviewHeight()}，` +
          `录像计划 ${this.host.getActiveVideoWidth()}x${this.host.getActiveVideoHeight()}`);
      } else {
        this.host.setLastPhotoHint('');
      }
    } else {
      this.host.restoreStreamSpecSelection(prevIndex, prevSnap.width, prevSnap.height,
        prevSnap.fpsIndex, prevSnap.fps);
      this.host.restoreUserStreamSpec(prevSnap);
      const dist = this.host.getCurrentTarget()?.sourceType === CameraSourceType.DISTRIBUTED;
      this.host.setLastPhotoHint(dist ?
        '远端分辨率切换失败：DH 重协商或开预览超时，请重试' : '分辨率切换失败，请稍后重试');
    }
    return ok;
  }

  public async applyFrameRateAt(index: number): Promise<boolean> {
    if (this.host.isRecordingActive() || !this.host.getCurrentTarget() || !this.host.hasCameraManager()) {
      return false;
    }
    if (this.host.getFpsOptionCount() === 0) {
      this.host.setLastPhotoHint('当前分辨率下无可用帧率');
      return false;
    }
    if (index < 0 || index >= this.host.getFpsOptionCount()) {
      return false;
    }
    if (index === this.host.getFpsIndex()) {
      return true;
    }
    const prevSnap = this.host.capturePreviewRollbackSpec();
    const prevIndex = prevSnap.fpsIndex;
    this.host.applyFpsSelection(index);
    this.host.stageUserStreamSpec(this.host.getResolutionIndex(), index);
    const fps = this.host.getFpsAtIndex(index);
    DhDiag.info('CF', `preview.applyFrameRate[${index}] -> ${fps}fps`);

    if (this.host.getCurrentTarget()?.sourceType === CameraSourceType.DISTRIBUTED) {
      this.host.restoreStreamSpecSelection(prevSnap.resolutionIndex, prevSnap.width, prevSnap.height,
        prevIndex, prevSnap.fps);
      this.host.restoreUserStreamSpec(prevSnap);
      this.host.setLastPhotoHint('远端预览帧率由开流协商（约 30fps），暂不支持切换');
      return false;
    }

    if (this.host.tryApplyPreviewFps(fps)) {
      this.host.commitUserStreamSpec('preview.applyFrameRate.hot');
      this.host.syncVideoPlanFromSelection();
      this.host.setLastPhotoHint(
        `预览 ${this.host.getActivePreviewFps()}fps · 录像计划 ${this.host.getActiveRecordFps()}fps`);
      return true;
    }
    DhDiag.warn('CF', 'preview.applyFrameRate setFrameRate failed — reopen session');
    const ok = await this.reopenWithSpecChange();
    if (ok) {
      this.host.commitUserStreamSpec('preview.applyFrameRate.reopen');
      this.host.setLastPhotoHint(
        `预览 ${this.host.getActivePreviewFps()}fps · 录像计划 ${this.host.getActiveRecordFps()}fps`);
      return true;
    }
    this.host.restoreStreamSpecSelection(prevSnap.resolutionIndex, prevSnap.width, prevSnap.height,
      prevIndex, prevSnap.fps);
    this.host.restoreUserStreamSpec(prevSnap);
    this.host.setLastPhotoHint('帧率切换失败，请稍后重试');
    return false;
  }

  public async reopenWithSpecChange(): Promise<boolean> {
    return this.host.runPreviewSpecChange(async () => this.host.reopenPreviewWithUserSpec());
  }

  public async openPreview(target: CameraTarget, skipPreRelease: boolean = false): Promise<boolean> {
    DhDiag.info('CF', `preview.open target=${target.cameraId} skipRelease=${skipPreRelease ? 1 : 0}`);
    return this.host.openAndPreview(target, this.host.getCurrentMode(), this.host.getCurrentPipeline(),
      skipPreRelease);
  }
}
