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

import { DesignTokens } from '../common/theme/DesignTokens';
import { UiTestIds } from '../common/constants/UiTestIds';
import { AppContext } from '../common/context/AppContext';
import { DeviceCenterViewModel } from '../viewmodel/DeviceCenterViewModel';
import { StudioViewModel } from '../viewmodel/studio/StudioViewModel';
import { AudioRouteViewModel } from '../viewmodel/AudioRouteViewModel';
import { DeviceCenterPage } from './DeviceCenterPage';
import { StudioPage } from './StudioPage';
import { AudioRoutePage } from './AudioRoutePage';
import { SettingsPage } from './SettingsPage';
import { PeerAuthOverlay } from '../components/PeerAuthOverlay';
import { PeerSignalService, PeerSignalAction, PeerSignalPayload } from '../service/PeerSignalService';
import { AppConstants } from '../common/constants/AppConstants';
import { common } from '@kit.AbilityKit';
import { DhDiag } from '../common/util/DhDiag';

@Entry
@Component
struct Index {
  @State currentTabIndex: number = 1;
  @State deviceCenterVM: DeviceCenterViewModel = new DeviceCenterViewModel();
  @State studioVM: StudioViewModel = new StudioViewModel();
  @State audioRouteVM: AudioRouteViewModel = new AudioRouteViewModel();
  @State peerAuthVisible: boolean = false;
  @State peerAuthSourceName: string = '';
  @State peerAuthAction: PeerSignalAction = PeerSignalAction.REQUEST_CAMERA;
  private initialized: boolean = false;
  /** 设备中心「设为镜头」切 Tab 时已自带连接，跳过并发 refresh */
  private skipNextStudioTabRefresh: boolean = false;
  private peerSignalListener = (payload: PeerSignalPayload): void => {
    this.peerAuthVisible = true;
    this.peerAuthSourceName = payload.sourceDeviceName;
    this.peerAuthAction = payload.action;
  };

  aboutToAppear(): void {
    if (this.initialized) {
      return;
    }
    const ctx = AppContext.tryGet() as common.UIAbilityContext | null;
    void this.initViewModels(ctx ?? undefined);
    PeerSignalService.getInstance().onSignal(this.peerSignalListener);
    DhDiag.smoke('Index.aboutToAppear');
    this.initialized = true;
  }

  private async initViewModels(context?: common.UIAbilityContext): Promise<void> {
    DhDiag.flowBegin('APP', 'initViewModels', '');
    PeerSignalService.getInstance().init(context);
    DhDiag.flowStep('APP', 'initViewModels', 'deviceCenter', '');
    await this.deviceCenterVM.init(context);
    DhDiag.flowStep('APP', 'initViewModels', 'studio', '');
    await this.studioVM.init(context);
    AppContext.setLifecycleListeners(() => {
      this.studioVM.onAppForeground();
    }, () => {
      this.studioVM.onAppBackground();
    });
    this.studioVM.markAppBootstrapReady();
    DhDiag.flowStep('APP', 'initViewModels', 'audio', 'background');
    void this.audioRouteVM.init(context).then(() => {
      DhDiag.flowEnd('APP', 'initViewModels', true, 'audio background done');
    });
    DhDiag.flowEnd('APP', 'initViewModels', true, 'studio ready, audio async');
    DhDiag.startHeartbeat(2000);
  }

  aboutToDisappear(): void {
    DhDiag.stopHeartbeat();
    PeerSignalService.getInstance().offSignal(this.peerSignalListener);
    this.deviceCenterVM.release();
    this.studioVM.release();
    this.audioRouteVM.release();
  }

  build() {
    Stack() {
      Column() {
      // 品牌栏
      Row() {
        Text('DistributedHardWareCast')
          .fontSize(16)
          .fontWeight(FontWeight.Bold)
          .fontColor(DesignTokens.ACCENT_PRIMARY)
          .id(UiTestIds.APP_TXT_BRAND)
          .accessibilityText('DistributedHardWareCast')
        Text(' Studio')
          .fontSize(16)
          .fontColor(DesignTokens.TEXT_SECONDARY)
          .id(UiTestIds.APP_TXT_SUBTITLE)
          .accessibilityText('Studio')
        Blank()
        Text('分布式硬件')
          .fontSize(10)
          .fontColor(DesignTokens.TEXT_MUTED)
      }
      .width('100%')
      .padding({ left: 16, right: 16, top: 8, bottom: 4 })

      Tabs({ barPosition: BarPosition.End, index: this.currentTabIndex }) {
        TabContent() {
          DeviceCenterPage({
            viewModel: $deviceCenterVM,
            onNavigateStudio: () => { this.currentTabIndex = 1; },
            onSetDeviceAsLens: (networkId: string) => {
              this.skipNextStudioTabRefresh = true;
              this.currentTabIndex = 1;
              void this.studioVM.setLensFromDeviceCenter(networkId, 'back');
            }
          })
        }.tabBar(this.tabBarItem('设备', '◉', 0))

        TabContent() {
          StudioPage({ viewModel: $studioVM })
        }.tabBar(this.tabBarItem('创作台', '▶', 1))

        TabContent() {
          AudioRoutePage({ viewModel: $audioRouteVM })
        }.tabBar(this.tabBarItem('声场', '♫', 2))

        TabContent() {
          SettingsPage()
        }.tabBar(this.tabBarItem('设置', '⚙', 3))
      }
      .barMode(BarMode.Fixed)
      .barBackgroundColor(DesignTokens.BG_ELEVATED)
      .barHeight(56)
      .onChange((index: number) => {
        this.currentTabIndex = index;
        if (index === 1 && this.initialized) {
          if (this.skipNextStudioTabRefresh) {
            this.skipNextStudioTabRefresh = false;
            DhDiag.info('APP', 'tab→创作台 skip refresh (setAsLens in flight)');
          } else {
            DhDiag.info('APP', 'tab→创作台 refreshFromDeviceCenter');
            void this.studioVM.refreshFromDeviceCenter();
          }
        }
      })
      .layoutWeight(1)
    }
    .width('100%')
    .height('100%')
    .backgroundColor(DesignTokens.BG_PRIMARY)

      PeerAuthOverlay({
        visible: this.peerAuthVisible,
        sourceDeviceName: this.peerAuthSourceName,
        action: this.peerAuthAction,
        onAllow: () => {
          this.peerAuthVisible = false;
          PeerSignalService.getInstance().clearSignal();
        },
        onDeny: () => {
          this.peerAuthVisible = false;
          PeerSignalService.getInstance().clearSignal();
        },
        onDismiss: () => {
          this.peerAuthVisible = false;
        }
      })
    }
    .width('100%')
    .height('100%')
  }

  @Builder
  tabBarItem(title: string, icon: string, index: number) {
    Column({ space: 2 }) {
      Text(icon)
        .fontSize(16)
        .fontColor(this.currentTabIndex === index ?
          DesignTokens.ACCENT_PRIMARY : DesignTokens.TEXT_MUTED)
        .id(UiTestIds.tabText(index, 'icon'))
        .accessibilityText(title)
      Text(title)
        .fontSize(10)
        .fontColor(this.currentTabIndex === index ?
          DesignTokens.ACCENT_PRIMARY : DesignTokens.TEXT_MUTED)
        .id(UiTestIds.tabText(index, 'title'))
        .accessibilityText(title)
    }
    .padding({ top: 6, bottom: 6 })
    .id(UiTestIds.tab(index))
    .accessibilityText(title)
  }
}
