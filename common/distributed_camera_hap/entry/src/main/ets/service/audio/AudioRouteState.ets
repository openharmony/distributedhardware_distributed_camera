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

import { audio } from '@kit.AudioKit';
import { AudioRouteConfig } from '../../common/types/AudioTypes';

/** 音频路由可变状态 */
export class AudioRouteState {
  public audioManager: audio.AudioManager | null = null;
  public routingManager: audio.AudioRoutingManager | null = null;
  public audioRenderer: audio.AudioRenderer | null = null;
  public audioCapturer: audio.AudioCapturer | null = null;
  public routeConfig: AudioRouteConfig = { outputDevice: null, inputDevice: null, volume: 80 };
  public initError: string = '';
  public lastRouteError: string = '';
  public isPlayingTone: boolean = false;
  public lastPlaybackTimestampMs: number = -1;
  public deviceDescriptorCache: Map<string, audio.AudioDeviceDescriptor> = new Map();
  /** 由 DISTRIBUTED_INPUT/OUTPUT_DEVICES_FLAG 枚举到的设备 id */
  public distributedNativeIds: Set<number> = new Set();
  /** 本次录像会话已拉起对端麦 — 停录时关闭 */
  public recordingRemoteMicNetId: string = '';
  /** 递增后进行中的 beginRecordingRemoteMic 须中止（与停录竞态） */
  public recordingMicEpoch: number = 0;
  /** ensureRemoteRecordingAudio(micOnly:false) 时开过对端 Speaker */
  public recordingAlsoEnabledSpeaker: boolean = false;
  /** 预览返听：Capturer → Renderer */
  public previewMonitorDesired: boolean = false;
  public previewMonitorRunning: boolean = false;
  public previewMonitorLoopId: number = 0;
  public previewMonitorSuppressed: boolean = false;
  public previewMonitorHoldUntilMs: number = 0;
  /** 合并并发 ensureRemoteRecordingAudio，避免双路 enableSink 互锁 */
  public ensureRemoteRecordingInflight: Map<string, Promise<boolean>> = new Map();
  /** 与 beginRecordingRemoteMic 共享，供开录等待（勿再二次 ensure） */
  public recordingMicSetupPromise: Promise<boolean> | null = null;
  public recordingMicSetupNetId: string = '';
  /** 远端录像写入 AVRecorder 期间禁止预览返听抢 Mic */
  public recordMicLockedForAvRecorder: boolean = false;
}
