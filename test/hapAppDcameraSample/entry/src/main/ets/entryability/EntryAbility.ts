/*
 * Copyright (C) 2023-2024 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the 'License');
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an 'AS IS' BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

import { UIAbility } from '@kit.AbilityKit';
import { window } from '@kit.ArkUI';
import { abilityAccessCtrl } from '@kit.AbilityKit';
import { Permissions } from '@kit.AbilityKit';
import Logger from '../utils/Logger';
import { BusinessError } from '@ohos.base';

/**
 * Lift cycle management of Ability.
 */
export default class EntryAbility extends UIAbility {
  onCreate(want, launchParam) {
    Logger.info('Sample_VideoRecorder', 'Ability onCreate,requestPermissionsFromUser');
    let permissionNames: Array<Permissions> = ['ohos.permission.MEDIA_LOCATION', 'ohos.permission.READ_MEDIA',
      'ohos.permission.WRITE_MEDIA', 'ohos.permission.CAMERA', 'ohos.permission.MICROPHONE',
      'ohos.permission.DISTRIBUTED_DATASYNC'];
    abilityAccessCtrl.createAtManager().requestPermissionsFromUser(this.context, permissionNames).then((data) => {
      console.log("Sample_VideoRecorder", data);
    }).catch((err: BusinessError) => {
      console.log("Sample_VideoRecorder", err.message);
    });
  }

  onDestroy() {
    Logger.info('Sample_VideoRecorder', 'Ability onDestroy');
  }

  async onWindowStageCreate(windowStage: window.WindowStage) {
    // Main window is created, set main page for this ability
    Logger.info('Sample_VideoRecorder', 'Ability onWindowStageCreate');

    windowStage.loadContent('pages/ListPage', (err, data) => {
      if (err.code) {
        Logger.error('Sample_VideoRecorder', 'Failed to load the content. Cause: ' + JSON.stringify(err));
        return;
      }
      Logger.info('Sample_VideoRecorder', 'Succeeded in loading the content. Data: ' + JSON.stringify(data));
      windowStage.getMainWindow().then((win: window.Window) => {
        win.setKeepScreenOn(true);
      })
    });
  }

  onWindowStageDestroy() {
    // Main window is destroyed, release UI related resources
    Logger.info('Sample_VideoRecorder', 'Ability onWindowStageDestroy');
  }

  onForeground() {
    // Ability has brought to foreground
    Logger.info('Sample_VideoRecorder', 'Ability onForeground');
  }

  onBackground() {
    // Ability has back to background
    Logger.info('Sample_VideoRecorder', 'Ability onBackground');
  }
}