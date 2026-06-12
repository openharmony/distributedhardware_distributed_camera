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

import { CameraTarget, StreamPipelineType, StudioMode } from '../../common/types/CameraTypes';
import { DhDiag } from '../../common/util/DhDiag';
import { CameraPhotoController } from './CameraPhotoController';
import { CameraPreviewController } from './CameraPreviewController';
import { ICameraSessionHost } from './CameraSessionHost';
import { CameraVideoController } from './CameraVideoController';

/**
 * 预览 / 拍照 / 录像 模式编排 — 统一「停旧 Session → 开新 Session」入口，各 Controller 低耦合。
 */
export class CameraModeOrchestrator {
  public readonly preview: CameraPreviewController;
  public readonly photo: CameraPhotoController;
  public readonly video: CameraVideoController;

  constructor(host: ICameraSessionHost) {
    this.preview = new CameraPreviewController(host);
    this.photo = new CameraPhotoController(host);
    this.video = new CameraVideoController(host);
  }

  /**
   * 按 Studio 模式切换 Session 类型（供 StreamSwitch / openAndPreview 前置路由）。
   */
  async switchToMode(
    target: CameraTarget,
    mode: StudioMode,
    pipeline: StreamPipelineType,
    skipPreRelease: boolean
  ): Promise<boolean | null> {
    DhDiag.info('CF',
      `modeOrchestrator.switch mode=${mode} pipeline=${pipeline} skipRelease=${skipPreRelease ? 1 : 0}`);
    if (mode === StudioMode.VIDEO && this.video.tryReuseForModeSwitch(target)) {
      return true;
    }
    if (mode === StudioMode.PREVIEW && pipeline === StreamPipelineType.CONTINUOUS) {
      return null;
    }
    if (mode === StudioMode.PHOTO) {
      return this.photo.openPhoto(target, skipPreRelease);
    }
    if (mode === StudioMode.VIDEO) {
      return this.video.openVideoPreview(target, skipPreRelease);
    }
    return null;
  }
}
