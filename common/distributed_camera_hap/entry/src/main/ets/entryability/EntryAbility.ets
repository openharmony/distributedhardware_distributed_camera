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

import { AbilityConstant, ConfigurationConstant, UIAbility, Want } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { window } from '@kit.ArkUI';
import { AppConstants } from '../common/constants/AppConstants';
import { AppContext } from '../common/context/AppContext';
import { CameraSessionService } from '../service/camera/CameraSessionService';
import { AudioRouteService } from '../service/AudioRouteService';
import { DeviceManagerService } from '../service/DeviceManagerService';
import { PeerSignalService } from '../service/PeerSignalService';
import { HardwareManagerAdapter } from '../common/adapters/HardwareManagerAdapter';
import { DhDiag, DH_BUILD_STAMP } from '../common/util/DhDiag';
import { PermissionHelper } from '../common/util/PermissionHelper';
const DOMAIN = AppConstants.LOG_DOMAIN;
const TAG = AppConstants.TAG;

export default class EntryAbility extends UIAbility {
  onCreate(want: Want, launchParam: AbilityConstant.LaunchParam): void {
    AppContext.set(this.context);
    try {
      this.context.getApplicationContext().setColorMode(ConfigurationConstant.ColorMode.COLOR_MODE_NOT_SET);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'Failed to set colorMode: %{public}s', JSON.stringify(err));
    }
    hilog.info(DOMAIN, TAG, 'Ability onCreate');
    DhDiag.smoke('EntryAbility.onCreate');
    DhDiag.info('APP', `bundle=${AppConstants.BUNDLE_NAME} build=${DH_BUILD_STAMP} dhKit=${HardwareManagerAdapter.isKitAvailable() ? 1 : 0}`);
    void this.bootstrapPermissions();
    PeerSignalService.getInstance().init(this.context);
    CameraSessionService.getInstance().init(this.context);
    AudioRouteService.getInstance().init(this.context);
  }

  private bootstrapPermissions(): void {
    void this.bootstrapRequiredPermissions();
    void this.bootstrapAudioPermissions();
  }

  private async bootstrapRequiredPermissions(): Promise<void> {
    try {
      const ok = await PermissionHelper.requestRequiredPermissions(this.context);
      DhDiag.info('APP', `permissions granted=${ok ? 1 : 0}`);
      if (ok) {
        await DeviceManagerService.getInstance().init(this.context);
      }
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'bootstrapRequiredPermissions failed: %{public}s', JSON.stringify(err));
    }
  }

  private async bootstrapAudioPermissions(): Promise<void> {
    try {
      const ok = await PermissionHelper.requestAudioPermissions(this.context);
      DhDiag.info('APP', `audio permissions granted=${ok ? 1 : 0}`);
    } catch (err) {
      hilog.error(DOMAIN, TAG, 'bootstrapAudioPermissions failed: %{public}s', JSON.stringify(err));
    }
  }

  onDestroy(): void {
    DhDiag.flowBegin('APP', 'onDestroy', '');
    CameraSessionService.getInstance().closeSession();
    AudioRouteService.getInstance().release();
    DeviceManagerService.getInstance().release();
    PeerSignalService.getInstance().release();
    DhDiag.flowEnd('APP', 'onDestroy', true, '');
    hilog.info(DOMAIN, TAG, 'Ability onDestroy');
  }

  onWindowStageCreate(windowStage: window.WindowStage): void {
    // Main window is created, set main page for this ability
    hilog.info(DOMAIN, TAG, 'Ability onWindowStageCreate');

    windowStage.loadContent('pages/Index', (err) => {
      if (err.code) {
        DhDiag.flowEnd('APP', 'loadContent', false, JSON.stringify(err));
        hilog.error(DOMAIN, TAG, 'Failed to load content: %{public}s', JSON.stringify(err));
        return;
      }
      DhDiag.info('APP', 'loadContent Index ok');
      hilog.info(DOMAIN, TAG, 'Succeeded in loading content');
      windowStage.getMainWindow().then((win: window.Window) => {
        win.setKeepScreenOn(true);
        DhDiag.info('APP', 'setKeepScreenOn=true');
      }).catch((e: Error) => {
        hilog.warn(DOMAIN, TAG, 'setKeepScreenOn failed: %{public}s', JSON.stringify(e));
      });
    });
  }

  onWindowStageDestroy(): void {
    hilog.info(DOMAIN, TAG, 'Ability onWindowStageDestroy');
  }

  onForeground(): void {
    DhDiag.info('APP', 'EntryAbility onForeground — B端应在此态等待授权');
    hilog.info(DOMAIN, TAG, 'Ability onForeground');
    AppContext.notifyForeground();
  }

  onBackground(): void {
    DhDiag.warn('APP', 'EntryAbility onBackground — 对端授权可能中断');
    hilog.info(DOMAIN, TAG, 'Ability onBackground');
    AppContext.notifyBackground();
  }
}