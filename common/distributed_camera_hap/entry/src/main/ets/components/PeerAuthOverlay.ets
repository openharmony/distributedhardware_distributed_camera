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
import { PeerSignalAction, PeerSignalPayload } from '../service/PeerSignalService';

@Component
export struct PeerAuthOverlay {
  @Prop visible: boolean = false;
  @Prop sourceDeviceName: string = '导播手机';
  @Prop action: PeerSignalAction = PeerSignalAction.REQUEST_CAMERA;
  onAllow?: () => void;
  onDeny?: () => void;
  onDismiss?: () => void;

  build() {
    if (!this.visible) {
      Column()
    } else {
      Column() {
        Column({ space: 16 }) {
          Text('📷 相机共享请求')
            .fontSize(20)
            .fontWeight(FontWeight.Bold)
            .fontColor(DesignTokens.ACCENT_WARN)
            .width('100%')
            .textAlign(TextAlign.Center)
            .id(UiTestIds.PEER_TXT_TITLE)
            .accessibilityText('相机共享请求')

          Text(`「${this.sourceDeviceName}」正在请求使用本机相机`)
            .fontSize(15)
            .fontColor(DesignTokens.TEXT_PRIMARY)
            .width('100%')
            .textAlign(TextAlign.Center)
            .lineHeight(22)
            .id(UiTestIds.PEER_TXT_BODY)
            .accessibilityText(`${this.sourceDeviceName} 正在请求使用本机相机`)

          Text(this.hintText())
            .fontSize(12)
            .fontColor(DesignTokens.TEXT_MUTED)
            .width('100%')
            .textAlign(TextAlign.Center)
            .lineHeight(18)
            .id(UiTestIds.PEER_TXT_HINT)
            .accessibilityText(this.hintText())

          Row({ space: 12 }) {
            Button('拒绝')
              .layoutWeight(1)
              .height(44)
              .fontSize(15)
              .backgroundColor(DesignTokens.BG_INPUT)
              .fontColor(DesignTokens.TEXT_SECONDARY)
              .id(UiTestIds.PEER_BTN_DENY)
              .accessibilityText('拒绝相机共享')
              .onClick(() => {
                this.onDeny?.();
              })
            Button('允许共享相机')
              .layoutWeight(1)
              .height(44)
              .fontSize(15)
              .backgroundColor(DesignTokens.ACCENT_PRIMARY)
              .fontColor('#FFFFFF')
              .id(UiTestIds.PEER_BTN_ALLOW_CAMERA)
              .accessibilityText('允许共享相机')
              .onClick(() => {
                this.onAllow?.();
              })
          }
          .width('100%')

          Text('稍后')
            .fontSize(12)
            .fontColor(DesignTokens.TEXT_MUTED)
            .width('100%')
            .textAlign(TextAlign.Center)
            .id(UiTestIds.PEER_BTN_DISMISS)
            .accessibilityText('稍后处理')
            .onClick(() => {
              this.onDismiss?.();
            })
        }
        .width('88%')
        .padding(20)
        .backgroundColor('#1E2838')
        .borderRadius(16)
        .border({ width: 2, color: DesignTokens.ACCENT_WARN })
      }
      .width('100%')
      .height('100%')
      .backgroundColor('#CC000000')
      .justifyContent(FlexAlign.Center)
      .alignItems(HorizontalAlign.Center)
      .position({ x: 0, y: 0 })
      .zIndex(999)
    }
  }

  private hintText(): string {
    if (this.action === PeerSignalAction.REQUEST_SCENE) {
      return '双机场景即将启动 · 请保持本应用在前台并允许相机';
    }
    return '导播端已发起连接 · 请关注系统「允许使用相机」弹窗并点允许';
  }
}

/** 供 Index 绑定 PeerSignal 的统一状态 */
export class PeerAuthUiState {
  public visible: boolean = false;
  public sourceDeviceName: string = '';
  public action: PeerSignalAction = PeerSignalAction.REQUEST_CAMERA;

  public applySignal(payload: PeerSignalPayload): void {
    this.visible = true;
    this.sourceDeviceName = payload.sourceDeviceName;
    this.action = payload.action;
  }

  public reset(): void {
    this.visible = false;
  }
}
