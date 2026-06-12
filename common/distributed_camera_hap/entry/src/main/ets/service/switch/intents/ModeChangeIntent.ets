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

import { StudioMode } from '../../../common/types/CameraTypes';
import { DhDiag } from '../../../common/util/DhDiag';
import { CreationIntentResult, ModeChangeHost } from '../CreationIntentKind';

/** 显式切换预览 / 拍照 / 录像流模式 */
export class ModeChangeIntent {
  static async execute(host: ModeChangeHost, mode: StudioMode): Promise<CreationIntentResult> {
    if (host.isRecordingBlocked()) {
      const msg = '录像中请先点「停止录像」再切换流模式';
      host.setFallbackHint(msg);
      return { success: false, message: msg };
    }
    if (host.getStudioMode() === mode && host.hasActiveCamera() &&
      !(mode === StudioMode.PHOTO && !host.hasActivePhotoSession())) {
      return { success: true, message: '' };
    }
    const prevMode = host.getStudioMode();
    host.setStudioMode(mode);
    const result = await host.switchMode(mode);
    if (!result.success) {
      host.restoreStudioMode(prevMode);
    }
    await host.applySwitchResult(result);
    if (!result.success && result.errorMessage) {
      host.setFallbackHint(result.errorMessage);
      return { success: false, message: result.errorMessage };
    }
    if (result.success && mode === StudioMode.VIDEO && !host.isRecording()) {
      host.setMediaHint('已切换录像模式，点击下方 ⏺ 开始录制');
    }
    if (result.success) {
      host.closeStudioTools();
    }
    DhDiag.info('INTENT', `modeChange ok=${result.success ? 1 : 0} mode=${mode}`);
    return { success: result.success, message: result.errorMessage ?? '' };
  }
}
