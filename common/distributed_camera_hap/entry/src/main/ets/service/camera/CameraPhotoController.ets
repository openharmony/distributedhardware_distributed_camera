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

import { CameraSourceType, CameraTarget, StreamPipelineType, StudioMode } from '../../common/types/CameraTypes';
import { DhDiag } from '../../common/util/DhDiag';
import { ICameraSessionHost } from './CameraSessionHost';

/**
 * 拍照模式：确保 PhotoSession + PhotoOutput，与预览/录像 Session 互斥切换。
 */
export class CameraPhotoController {
  private readonly host: ICameraSessionHost;

  constructor(host: ICameraSessionHost) {
    this.host = host;
  }

  public needsPhotoSession(): boolean {
    return !this.host.hasPhotoOutput() || !this.host.isPhotoSceneMode();
  }

  /** 进入拍照 Session（PreviewOutput + PhotoOutput） */
  public async ensurePhotoSession(target: CameraTarget): Promise<boolean> {
    if (!this.host.hasCameraManager()) {
      this.host.setLastPhotoHint('相机未就绪');
      return false;
    }
    const isDistributed = target.sourceType === CameraSourceType.DISTRIBUTED;
    DhDiag.info('CF',
      `photo.ensureSession dist=${isDistributed ? 1 : 0} photoOut=${this.host.hasPhotoOutput() ? 1 : 0} ` +
      `scene=${this.host.getSessionSceneMode()}`);
    if (!this.needsPhotoSession()) {
      return true;
    }
    let ok = false;
    try {
      if (isDistributed) {
        await this.host.releaseForRemoteLensSwitch();
        await this.host.delayMs(250);
      }
      ok = await this.host.openAndPreview(target, StudioMode.PHOTO, StreamPipelineType.SNAPSHOT,
        isDistributed);
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'photo.ensureSession', err);
      ok = false;
    }
    if (!ok || !this.host.hasPhotoOutput()) {
      const msg = this.host.getLastOpenErrorMessage().length > 0 ?
        `无法打开拍照会话: ${this.host.getLastOpenErrorMessage()}` : '无法创建 PhotoOutput';
      this.host.setLastPhotoHint(msg);
      DhDiag.warn('CF', msg);
      return false;
    }
    if (isDistributed) {
      await this.host.delayMs(500);
    }
    return true;
  }

  public async openPhoto(target: CameraTarget, skipPreRelease: boolean = false): Promise<boolean> {
    DhDiag.info('CF', `photo.open target=${target.cameraId}`);
    try {
      return await this.host.openAndPreview(target, StudioMode.PHOTO, StreamPipelineType.SNAPSHOT, skipPreRelease);
    } catch (err) {
      DhDiag.logCatchWarn('CF', 'photo.open', err);
      return false;
    }
  }
}

