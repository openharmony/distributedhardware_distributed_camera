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
import { CameraStreamSpecSnapshot, RemoteCameraProgressListener } from './CameraSessionHost';
import { CameraSessionUtils } from './CameraSessionUtils';
import { CameraSessionState } from './CameraSessionState';
import { CameraSessionService } from './CameraSessionService';
import { CameraModeOrchestrator } from './CameraModeOrchestrator';
import { PreviewFrameWaiter } from './PreviewFrameWaiter';

const TAG = AppConstants.TAG;

interface BusinessError { code: number; message: string; }

interface DiscoverEvaluateResult {
  targets: CameraTarget[] | null;
  remoteCnt: number;
  reason: string;
}

interface CameraManagerRemoteProbe extends camera.CameraManager {
  getCameraDevices(position: camera.CameraPosition, types: Array<camera.CameraType>,
    connectType: camera.ConnectionType): Array<camera.CameraDevice>;
}

export class CameraSessionDiscover {
  private readonly facade: CameraSessionService;

  constructor(facade: CameraSessionService) {
    this.facade = facade;
  }

  private get st(): CameraSessionState { return this.facade.state; }

  public async enumerateCameras(includeRemoteProbe: boolean = false): Promise<CameraTarget[]> {
    return this.buildCameraTargets(this.collectCameraDevices(includeRemoteProbe));
  }

  /** 仅枚举本机相机（前/后摄，排除远端 connectionType） */
  public enumerateLocalCameras(): CameraTarget[] {
    if (!this.st.cameraManager) {
      return [];
    }
    try {
      const locals = this.st.cameraManager.getSupportedCameras().filter(d =>
        d.connectionType !== camera.ConnectionType.CAMERA_CONNECTION_REMOTE &&
        (d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_BACK ||
          d.cameraPosition === camera.CameraPosition.CAMERA_POSITION_FRONT));
      DhDiag.info('CF', `enumerateLocalCameras count=${locals.length}`);
      return this.buildCameraTargets(locals);
    } catch (err) {
      DhDiag.warn('CF', `enumerateLocalCameras failed ${JSON.stringify(err)}`);
      return [];
    }
  }

  /**
   * 两阶段远端相机发现：
   * Phase 1 — 快速轮询 (3×500ms)，捕捉 DH resume 后快速出现的 REMOTE
   * Phase 2 — 事件驱动 (cameraStatus 回调 + 回退轮询)，等待对端授权 + dcamera 注册
   */
  public async discoverRemoteCameras(preferredNetworkId?: string,
    onProgress?: RemoteCameraProgressListener,
    options?: RemoteCameraDiscoverOptions): Promise<CameraTarget[]> {
    const trace = options?.traceId ?? 'LEGACY';
    const logTag = CameraSessionUtils.cfTraceTag(trace);
    const bound = DeviceManagerService.getInstance().getBoundRemoteDevices();
    if (bound.length === 0 || !this.st.cameraManager) {
      DhDiag.warn(logTag, `discover skip bound=${bound.length} mgr=${this.st.cameraManager !== null}`);
      return this.enumerateCameras(true);
    }

    if (options?.skipPrepareProbe !== true) {
      await this.prepareForRemoteProbe();
    }

    const quickRetries = options?.quickRetries ?? AppConstants.REMOTE_CAMERA_QUICK_RETRIES;
    const quickIntervalMs = options?.quickIntervalMs ?? AppConstants.REMOTE_CAMERA_QUICK_INTERVAL_MS;
    const eventTimeoutMs = options?.eventTimeoutMs ?? AppConstants.REMOTE_CAMERA_EVENT_TIMEOUT_MS;
    const fallbackProbeMs = options?.fallbackProbeIntervalMs ?? AppConstants.REMOTE_CAMERA_FALLBACK_PROBE_MS;
    const skipAuthProbe = options?.skipAuthProbe ?? false;
    const peerName = bound[0]?.deviceName ?? '对端手机';
    const peerNet = bound[0]?.networkId ?? '';
    const progressPrefix = CameraSessionUtils.discoverProgressPrefix(trace);
    const startMs = Date.now();

    DhDiag.flowBegin(logTag, 'discoverRemoteCameras',
      `peer=${peerName} net=${preferredNetworkId?.substring(0, 12) ?? 'any'}... ` +
      `quick=${quickRetries}×${quickIntervalMs}ms eventTimeout=${eventTimeoutMs}ms`);

    const targetMatchesPeerId = (c: CameraTarget, peerId: string): boolean => {
      if (peerId.length === 0) {
        return false;
      }
      return CameraSessionUtils.networkIdMatches(c.networkId, peerId) ||
        CameraSessionUtils.networkIdMatches(c.cameraId, peerId) ||
        CameraSessionUtils.networkIdMatches(c.deviceId, peerId);
    };

    const evaluateDiscover = (): DiscoverEvaluateResult => {
      const devices = this.collectCameraDevices();
      const targets = this.buildCameraTargets(devices);
      const remoteTargets = targets.filter(c => c.sourceType === CameraSourceType.DISTRIBUTED);
      if (remoteTargets.length === 0) {
        const out: DiscoverEvaluateResult = { targets: null, remoteCnt: 0, reason: 'no-remote' };
        return out;
      }
      if (!preferredNetworkId) {
        const out: DiscoverEvaluateResult =
          { targets: targets, remoteCnt: remoteTargets.length, reason: 'any-peer' };
        return out;
      }
      const matchedPreferred = remoteTargets.some(c => targetMatchesPeerId(c, preferredNetworkId));
      const singleBoundPeer = bound.length === 1 ? bound[0] : undefined;
      const matchedBoundPeer = singleBoundPeer !== undefined && remoteTargets.some(c =>
        targetMatchesPeerId(c, singleBoundPeer.networkId) ||
        targetMatchesPeerId(c, singleBoundPeer.deviceId) ||
        (singleBoundPeer.deviceName.length > 0 && c.deviceName.length > 0 &&
          c.deviceName === singleBoundPeer.deviceName));
      if (matchedPreferred) {
        const out: DiscoverEvaluateResult =
          { targets: targets, remoteCnt: remoteTargets.length, reason: 'preferred-match' };
        return out;
      }
      if (matchedBoundPeer) {
        const out: DiscoverEvaluateResult =
          { targets: targets, remoteCnt: remoteTargets.length, reason: 'single-bound-peer' };
        return out;
      }
      const mismatch: DiscoverEvaluateResult =
        { targets: null, remoteCnt: remoteTargets.length, reason: 'peer-id-mismatch' };
      return mismatch;
    };

    const tryFinishDiscover = (): CameraTarget[] | null => evaluateDiscover().targets;

    // —— Phase 1: 快速轮询 ——
    DhDiag.flowStep(logTag, 'discoverRemoteCameras', 'phase1',
      `${quickRetries}×${quickIntervalMs}ms elapsed=${Date.now() - startMs}ms`);
    for (let attempt = 0; attempt < quickRetries; attempt++) {
      onProgress?.(attempt + 1, quickRetries + 1,
        `${progressPrefix}等待「${peerName}」授权相机 (${attempt + 1}/${quickRetries}) · 请看对端系统弹窗点允许`);
      this.probeRemoteCameraDevices();
      if (attempt === 0 && !skipAuthProbe) {
        await this.tryTriggerRemoteAuth(preferredNetworkId ?? peerNet, 'back', false);
      }
      const evalResult = evaluateDiscover();
      if (evalResult.targets) {
        const remoteCnt = evalResult.targets.filter(c => c.sourceType === CameraSourceType.DISTRIBUTED).length;
        DhDiag.flowEnd(logTag, 'discoverRemoteCameras', true,
          `phase1 OK attempt=${attempt + 1} remote=${remoteCnt} reason=${evalResult.reason} ` +
          `elapsed=${Date.now() - startMs}ms`);
        return evalResult.targets;
      }
      DhDiag.info(logTag,
        `phase1 attempt=${attempt + 1}/${quickRetries} remoteEnum=${evalResult.remoteCnt} ` +
        `reason=${evalResult.reason} elapsed=${Date.now() - startMs}ms`);
      if (attempt < quickRetries - 1) {
        await CameraSessionUtils.delay(quickIntervalMs);
      }
    }

    // —— Phase 2: 事件驱动 (cameraStatus + 回退轮询) ——
    DhDiag.flowStep(logTag, 'discoverRemoteCameras', 'phase2',
      `cameraStatus event timeout=${eventTimeoutMs}ms elapsed=${Date.now() - startMs}ms`);
    onProgress?.(quickRetries + 1, quickRetries + 1,
      `${progressPrefix}等待「${peerName}」相机上线… · 请保持对端亮屏解锁`);

    let eventResolve: ((result: CameraTarget[]) => void) | null = null;
    let eventDone = false;
    const onRemoteAppear = (_cameraId: string): void => {
      if (eventDone) {
        return;
      }
      const done = tryFinishDiscover();
      if (done && eventResolve) {
        eventResolve(done);
        eventResolve = null;
      }
    };
    this.facade.onRemoteCameraAppeared(onRemoteAppear);

    const eventPromise = new Promise<CameraTarget[] | null>((resolve) => {
      eventResolve = resolve as ((result: CameraTarget[]) => void) | null;
    });

    const fallbackProbe = async (): Promise<CameraTarget[] | null> => {
      while (!eventDone) {
        await CameraSessionUtils.delay(fallbackProbeMs);
        if (eventDone) {
          break;
        }
        this.probeRemoteCameraDevices();
        const done = tryFinishDiscover();
        if (done) {
          return done;
        }
      }
      return null;
    };

    const timeoutPromise = CameraSessionUtils.delay(eventTimeoutMs).then(() => null as CameraTarget[] | null);

    try {
      const result = await Promise.race([eventPromise, fallbackProbe(), timeoutPromise]);
      eventDone = true;

      if (result && result.length > 0) {
        const remoteCnt = result.filter(c => c.sourceType === CameraSourceType.DISTRIBUTED).length;
        DhDiag.flowEnd(logTag, 'discoverRemoteCameras', true,
          `phase2 OK remote=${remoteCnt} elapsed=${Date.now() - startMs}ms`);
        return result;
      }

      const finalTargets = tryFinishDiscover();
      if (finalTargets) {
        DhDiag.flowEnd(logTag, 'discoverRemoteCameras', true,
          `phase2 late-found elapsed=${Date.now() - startMs}ms`);
        return finalTargets;
      }
    } finally {
      eventDone = true;
      eventResolve = null;
      this.facade.offRemoteCameraAppeared(onRemoteAppear);
    }

    DhDiag.flowEnd(logTag, 'discoverRemoteCameras', false,
      `TIMEOUT elapsed=${Date.now() - startMs}ms — 请看对端系统相机弹窗点允许`);
    hilog.warn(AppConstants.LOG_DOMAIN, TAG, 'discoverRemoteCameras [%{public}s] timeout preferred=%{public}s',
      trace, preferredNetworkId ?? 'any');
    return this.enumerateCameras(true);
  }

  public countSupportedRemoteCameras(): number {
    if (!this.st.cameraManager) {
      return 0;
    }
    try {
      return this.st.cameraManager.getSupportedCameras()
        .filter(d => d.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE).length;
    } catch (_e) {
      return 0;
    }
  }

  public static cfTraceTag(traceId: string): string {
    return `CF-${traceId}`;
  }

  private static discoverProgressPrefix(traceId: string): string {
    switch (traceId) {
      case 'SCENE':
        return '[场景] ';
      case 'CONNECT':
        return '[连接] ';
      case 'LENS':
        return '[切镜] ';
      case 'RECORD':
        return '[录像] ';
      default:
        return '';
    }
  }

  public static networkIdMatches(a: string | undefined, b: string | undefined): boolean {
    if (!a || !b) {
      return false;
    }
    if (a === b) {
      return true;
    }
    const minLen = 8;
    if (a.length >= minLen && b.length >= minLen) {
      return a.startsWith(b.substring(0, minLen)) || b.startsWith(a.substring(0, minLen));
    }
    return a.includes(b) || b.includes(a);
  }

  /** 探测 REMOTE 前释放本机 Session，避免 7400201；勿关远端主 Session */
  public async prepareForRemoteProbe(): Promise<void> {
    DhDiag.flowBegin('CF', 'prepareForRemoteProbe', '');
    const remoteMainActive = this.st.isPreviewActive &&
      this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED;
    if (remoteMainActive) {
      DhDiag.flowEnd('CF', 'prepareForRemoteProbe', true, 'skip — remote main preview active');
      return;
    }
    const hadLocal = (this.st.isPreviewActive || this.st.cameraInput !== null) &&
      this.st.currentTarget?.sourceType !== CameraSourceType.DISTRIBUTED;
    if (hadLocal) {
      DhDiag.flowStep('CF', 'prepareForRemoteProbe', 'close-local', '');
      await this.facade.closeSession();
      await CameraSessionUtils.delay(200);
    } else {
      DhDiag.flowStep('CF', 'prepareForRemoteProbe', 'no-local', '');
    }
    DhDiag.flowEnd('CF', 'prepareForRemoteProbe', true, '');
  }

  public collectCameraDevices(includeRemoteProbe: boolean = false): camera.CameraDevice[] {
    if (!this.st.cameraManager) {
      return [];
    }
    const merged: camera.CameraDevice[] = [];
    const seen: Set<string> = new Set();
    const append = (list: camera.CameraDevice[]): void => {
      for (const dev of list) {
        if (!seen.has(dev.cameraId)) {
          seen.add(dev.cameraId);
          merged.push(dev);
        }
      }
    };
    try {
      const supported = this.st.cameraManager.getSupportedCameras();
      append(supported);
      const remoteCnt = supported.filter(d =>
        d.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE).length;
      DhDiag.info('CF',
        `getSupportedCameras total=${supported.length} remote=${remoteCnt} probe=${includeRemoteProbe ? 1 : 0}`);
    } catch (err) {
      DhDiag.error('CF', `getSupportedCameras failed ${JSON.stringify(err)}`);
    }
    if (includeRemoteProbe) {
      this.appendRemoteViaGetCameraDevices(append);
    }
    return merged;
  }

  /** getCameraDevices(REMOTE) 在部分 ROM 上恒 7400201；官方样例仅用 getSupportedCameras */
  public appendRemoteViaGetCameraDevices(append: (list: camera.CameraDevice[]) => void): void {
    if (this.st.skipGetCameraDevicesRemote || !this.st.cameraManager) {
      return;
    }
    const probeMgr = this.st.cameraManager as CameraManagerRemoteProbe;
    let saw7400201 = false;
    for (const pos of [camera.CameraPosition.CAMERA_POSITION_BACK, camera.CameraPosition.CAMERA_POSITION_FRONT]) {
      try {
        const remoteList = probeMgr.getCameraDevices(
          pos, [camera.CameraType.CAMERA_TYPE_DEFAULT], camera.ConnectionType.CAMERA_CONNECTION_REMOTE);
        append(remoteList);
        DhDiag.info('CF', `getCameraDevices(REMOTE) pos=${pos} count=${remoteList.length}`);
      } catch (err) {
        const code = (err as BusinessError)?.code ?? 0;
        if (code === 7400201) {
          saw7400201 = true;
        }
        DhDiag.warn('CF',
          `getCameraDevices(REMOTE) pos=${pos} code=${code} — 非致命，改靠 getSupportedCameras 等 REMOTE 出现`);
      }
    }
    if (saw7400201) {
      this.st.skipGetCameraDevicesRemote = true;
      DhDiag.warn('CF', 'ROM 上 getCameraDevices(REMOTE) 不可用(7400201)，后续仅用 getSupportedCameras');
    }
  }

  /** 尝试 open 远端 CameraInput 以触发 CF→DHFWK→对端授权链（带防抖，避免 B 端 session churn） */
  public async tryTriggerRemoteAuth(networkId: string, position: string = 'back',
    skipWhenMainActive: boolean = true, forceProbe: boolean = false): Promise<boolean> {
    if (!this.st.cameraManager) {
      return false;
    }
    if (skipWhenMainActive && this.st.isPreviewActive &&
      this.st.currentTarget?.sourceType === CameraSourceType.DISTRIBUTED) {
      DhDiag.info('CF', 'tryTriggerRemoteAuth skip — remote main session active');
      return true;
    }
    const now = Date.now();
    const lastProbe = this.st.lastRemoteAuthProbeMs.get(networkId) ?? 0;
    if (!forceProbe && now - lastProbe < AppConstants.REMOTE_AUTH_PROBE_DEBOUNCE_MS) {
      DhDiag.info('CF',
        `tryTriggerRemoteAuth skip debounce net=${networkId.substring(0, 12)}... age=${now - lastProbe}ms`);
      return true;
    }
    let remote = this.facade.findRemoteCameraDevice(networkId, position);
    if (!remote) {
      const altPos = position === 'front' ? 'back' : 'front';
      remote = this.facade.findRemoteCameraDevice(networkId, altPos);
    }
    if (!remote) {
      DhDiag.info('CF', `tryTriggerRemoteAuth no REMOTE device yet net=${networkId.substring(0, 12)}...`);
      return false;
    }
    let probeInput: camera.CameraInput | null = null;
    let probed = false;
    try {
      DhDiag.info('CF', `tryTriggerRemoteAuth open cameraId=${remote.cameraId} host=${remote.hostDeviceName ?? ''}`);
      probeInput = this.st.cameraManager.createCameraInput(remote);
      await probeInput.open();
      probed = true;
      DhDiag.info('CF', 'tryTriggerRemoteAuth open OK — 对端应出现系统授权');
      return true;
    } catch (err) {
      probed = true;
      DhDiag.warn('CF', `tryTriggerRemoteAuth open failed (may need auth) ${JSON.stringify(err)}`);
      return false;
    } finally {
      if (probeInput) {
        try {
          await probeInput.close();
        } catch (_e) {
          // ignore
        }
      }
      if (probed) {
        this.st.lastRemoteAuthProbeMs.set(networkId, Date.now());
      }
    }
  }

  /** 触发 CF 枚举 — 官方路径 getSupportedCameras，辅以 getCameraDevices(REMOTE) */
  public probeRemoteCameraDevices(): void {
    if (!this.st.cameraManager) {
      return;
    }
    try {
      const supported = this.st.cameraManager.getSupportedCameras();
      const remoteCnt = supported.filter(d =>
        d.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE).length;
      DhDiag.info('CF', `probe getSupportedCameras total=${supported.length} remote=${remoteCnt}`);
    } catch (err) {
      DhDiag.warn('CF', `probe getSupportedCameras failed ${JSON.stringify(err)}`);
    }
    const merged: camera.CameraDevice[] = [];
    this.appendRemoteViaGetCameraDevices((list) => {
      for (const d of list) {
        merged.push(d);
      }
    });
  }

  public buildCameraTargets(devices: camera.CameraDevice[]): CameraTarget[] {
    const trustDevices = DeviceManagerService.getInstance().getBoundRemoteDevices()
      .concat(DeviceManagerService.getInstance().getTrustDevices().filter(d => isLocalDeviceId(d.deviceId)));
    const cameras: CameraTarget[] = [];
    for (const dev of devices) {
      const isRemote = dev.connectionType === camera.ConnectionType.CAMERA_CONNECTION_REMOTE;
      const isDistributed = isRemote;
      const position = dev.cameraPosition === camera.CameraPosition.CAMERA_POSITION_FRONT ? 'front' : 'back';
      const matchedTrust = this.matchTrustDevice(trustDevices, dev, isDistributed);
      cameras.push({
        cameraId: dev.cameraId,
        deviceId: isDistributed ? (matchedTrust?.deviceId ?? dev.cameraId) : 'local',
        deviceName: isDistributed ?
          (matchedTrust?.deviceName ?? dev.hostDeviceName ?? '远端设备 (Sink)') : '本机',
        sourceType: isDistributed ? CameraSourceType.DISTRIBUTED : CameraSourceType.LOCAL,
        position: position,
        networkId: matchedTrust?.networkId ?? (isDistributed ? dev.cameraId : 'local'),
        dhId: isDistributed ? dev.cameraId : undefined,
        deviceForm: matchedTrust ? this.toFormFactor(matchedTrust) :
          (isDistributed ? DeviceFormFactor.UNKNOWN : DeviceFormFactor.PHONE)
      });
    }
    if (cameras.length === 0) {
      hilog.warn(AppConstants.LOG_DOMAIN, TAG, 'buildCameraTargets: 无可用相机');
    }
    return cameras;
  }

  public matchTrustDevice(trustDevices: TrustDeviceInfo[], dev: camera.CameraDevice,
    isDistributed: boolean): TrustDeviceInfo | undefined {
    if (!isDistributed) {
      return trustDevices.find(d => isLocalDeviceId(d.deviceId));
    }
    const cameraId = dev.cameraId;
    const hostName = dev.hostDeviceName ?? '';
    return trustDevices.find(d => !isLocalDeviceId(d.deviceId) &&
      (CameraSessionUtils.networkIdMatches(d.networkId, cameraId) ||
        CameraSessionUtils.networkIdMatches(d.deviceId, cameraId) ||
        d.deviceName === hostName));
  }

  public async isRemoteReadyForPeer(networkId: string): Promise<boolean> {
    if (this.facade.findRemoteCameraDevice(networkId, 'back') ||
      this.facade.findRemoteCameraDevice(networkId, 'front')) {
      return true;
    }
    const cameras = await this.enumerateCameras(true);
    for (const c of cameras) {
      if (c.sourceType === CameraSourceType.DISTRIBUTED &&
        (CameraSessionUtils.networkIdMatches(c.networkId, networkId) ||
          CameraSessionUtils.networkIdMatches(c.cameraId, networkId) ||
          CameraSessionUtils.networkIdMatches(c.deviceId, networkId))) {
        return true;
      }
    }
    return false;
  }

  public toFormFactor(device: TrustDeviceInfo): DeviceFormFactor {
    if (AppConstants.WEARABLE_DEVICE_TYPES.includes(device.deviceType)) {
      return DeviceFormFactor.WEARABLE;
    }
    if (device.deviceType === 0x11 || device.deviceType === 17) {
      return DeviceFormFactor.TABLET;
    }
    if (device.deviceType === 0x0A || device.deviceType === 10) {
      return DeviceFormFactor.PHONE;
    }
    return DeviceFormFactor.UNKNOWN;
  }
}

