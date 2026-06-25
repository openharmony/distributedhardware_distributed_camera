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
import { image } from '@kit.ImageKit';
import { media } from '@kit.MediaKit';
import { fileIo } from '@kit.CoreFileKit';
import { display } from '@kit.ArkUI';
import { common } from '@kit.AbilityKit';
import { hilog } from '@kit.PerformanceAnalysisKit';
import { AppConstants } from '../../common/constants/AppConstants';
import { RemoteCameraTrace } from '../../common/constants/RemoteCameraTrace';
import { HardwareManagerAdapter } from '../../common/adapters/HardwareManagerAdapter';
import { AppContext } from '../../common/context/AppContext';
import {
  CameraFrameRateOption,
  CameraProfileSizeHolder,
  CameraResolutionOption,
  CameraSourceType,
  CameraTarget,
  CameraFocusCapability,
  CameraZoomCapability,
  RemoteCameraDiscoverOptions,
  DeviceFormFactor,
  StreamPipelineType,
  StudioMode
} from '../../common/types/CameraTypes';
import { isLocalDeviceId, TrustDeviceInfo } from '../../common/types/DeviceTypes';
import { DeviceManagerService } from '../DeviceManagerService';
import { DistributedHardwareService } from '../DistributedHardwareService';
import { PermissionHelper } from '../../common/util/PermissionHelper';
import { DhDiag } from '../../common/util/DhDiag';
import { CameraStreamSpecSnapshot } from './CameraSessionHost';
import { CameraSessionUtils } from './CameraSessionUtils';
import { CameraSessionState } from './CameraSessionState';
import { CameraSessionService } from './CameraSessionService';
import { CameraModeOrchestrator } from './CameraModeOrchestrator';
import { PreviewFrameWaiter } from './PreviewFrameWaiter';

const TAG = AppConstants.TAG;

interface BusinessError { code: number; message: string; }

interface CameraManagerRemoteProbe extends camera.CameraManager {
  getCameraDevices(position: camera.CameraPosition, types: Array<camera.CameraType>,
    connectType: camera.ConnectionType): Array<camera.CameraDevice>;
}

export class CameraSessionPipeline {
  private readonly facade: CameraSessionService;

  constructor(facade: CameraSessionService) {
    this.facade = facade;
  }

  private get st(): CameraSessionState { return this.facade.state; }

  public init(context?: common.UIAbilityContext): void {
    const ctx = context ?? AppContext.tryGet();
    if (!ctx) {
      this.st.initError = '无 UIAbilityContext';
      this.st.cameraManager = null;
      return;
    }
    try {
      this.st.cameraManager = camera.getCameraManager(ctx);
      this.facade.registerCameraStatusListener();
      this.st.initError = '';
      DhDiag.info('CF', 'CameraManager init ok');
    } catch (err) {
      this.st.initError = JSON.stringify(err);
      this.st.cameraManager = null;
      DhDiag.error('CF', `CameraManager init failed ${this.st.initError}`);
    }
  }

  public isReady(): boolean {
    return this.st.cameraManager !== null;
  }

  public getLastOpenErrorCode(): number {
    return this.st.lastOpenErrorCode;
  }

  public getLastOpenErrorMessage(): string {
    if (this.st.lastOpenErrorCode === AppConstants.CAMERA_ERR_ILLEGAL_OP) {
      return '相机非法操作 (7400102)：请检查 Session 是否已释放，或勿同时开主路+画中画';
    }
    if (this.st.lastOpenErrorCode === AppConstants.CAMERA_ERR_CONFLICT) {
      return '相机冲突 (7400107)：已有会话占用相机，请关闭画中画后重试';
    }
    const msg = this.st.lastOpenErrorMessage;
    if (msg.toLowerCase().includes('fatal')) {
      return `${msg} · 远端切录像易触发，请稍后重试或先切远端后摄再录`;
    }
    return msg;
  }

  /** 切流前彻底释放主 Session（不做 reconfigure，避免远端 7400201/7400102） */
  public async releaseAllForSwitch(): Promise<void> {
    DhDiag.flowBegin('CF', 'releaseAllForSwitch', '');
    await this.facade.forceReleaseAll();
    await CameraSessionUtils.delay(250);
    DhDiag.flowEnd('CF', 'releaseAllForSwitch', true, '');
  }

  public offRemoteCameraAppeared(listener: (cameraId: string) => void): void {
    const idx = this.st.remoteCameraAppearedListeners.indexOf(listener);
    if (idx >= 0) {
      this.st.remoteCameraAppearedListeners.splice(idx, 1);
    }
  }

  /**
   * Session 内热切换（官方 stop → removeOutput/removeInput → 再 commitConfig/start）
   * 本机前后摄 / 同对端网络换镜头，避免整 Session 拆除。
   */

}
