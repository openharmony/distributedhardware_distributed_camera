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

import { abilityAccessCtrl, common, Permissions } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { AppConstants } from '../constants/AppConstants';

const TAG = AppConstants.TAG;

const REQUIRED_PERMISSIONS: Permissions[] = [
  'ohos.permission.DISTRIBUTED_DATASYNC',
  'ohos.permission.CAMERA'
];

const AUDIO_PERMISSIONS: Permissions[] = [
  'ohos.permission.MICROPHONE'
];

export class PermissionHelper {
  static async requestRequiredPermissions(context: common.UIAbilityContext): Promise<boolean> {
    return PermissionHelper.requestPermissions(context, REQUIRED_PERMISSIONS);
  }

  static async requestAudioPermissions(context: common.UIAbilityContext): Promise<boolean> {
    return PermissionHelper.requestPermissions(context, [...REQUIRED_PERMISSIONS, ...AUDIO_PERMISSIONS]);
  }

  private static async requestPermissions(context: common.UIAbilityContext,
    permissions: Permissions[]): Promise<boolean> {
    const atManager = abilityAccessCtrl.createAtManager();
    try {
      const result = await atManager.requestPermissionsFromUser(context, permissions);
      const allGranted = result.authResults.every(r => r === abilityAccessCtrl.GrantStatus.PERMISSION_GRANTED);
      if (!allGranted) {
        hilog.warn(AppConstants.LOG_DOMAIN, TAG, 'permissions not all granted');
      }
      return allGranted;
    } catch (err) {
      hilog.error(AppConstants.LOG_DOMAIN, TAG, 'requestPermissions failed: %{public}s', JSON.stringify(err));
      return false;
    }
  }
}
