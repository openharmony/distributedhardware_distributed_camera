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
import { AudioRoutePreset } from '../../common/types/CreationSessionTypes';

export interface AudioDescriptorExt extends audio.AudioDeviceDescriptor {
  networkId?: string;
}

export interface AudioRoutingManagerExt extends audio.AudioRoutingManager {
  selectInputDevice?(devices: audio.AudioDeviceDescriptors): Promise<void>;
  selectOutputDevice?(devices: audio.AudioDeviceDescriptors): Promise<void>;
}

export interface BusinessError {
  code: number;
  message: string;
}

export interface RouteSpeakerForMusicOptions {
  /** 远端 Mic DH 仍占用时，播歌/回放前先 release Mic 再 warm Speaker */
  forceReleaseMic?: boolean;
}

export interface AudioRouteFallbackOptions {
  allowInputFallback?: boolean;
  allowOutputFallback?: boolean;
  /** prepareScene 已 enableSink 时跳过重复 resume */
  skipDhEnable?: boolean;
  discoverMaxRetries?: number;
  discoverIntervalMs?: number;
}

export interface RemoteRecordingAudioOptions {
  /** 默认 true：只开 Mic，不动 Speaker，避免预览/切镜头卡顿 */
  micOnly?: boolean;
  probeRetries?: number;
}

/** 停录后恢复预览声场（非写死 LOCAL/LOCAL） */
export interface EndRecordingRemoteMicOptions {
  restorePreset?: AudioRoutePreset;
  remoteNetworkId?: string;
}
