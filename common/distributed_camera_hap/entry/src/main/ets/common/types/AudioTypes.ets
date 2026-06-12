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

/** 音频设备类型 */
export enum AudioDeviceKind {
  SPEAKER = 'speaker',
  MIC = 'mic'
}

/** 音频设备来源 */
export enum AudioSourceType {
  LOCAL = 'local',
  DISTRIBUTED = 'distributed'
}

/** 音频设备模型 */
export interface AudioDeviceInfo {
  deviceId: string;
  deviceName: string;
  kind: AudioDeviceKind;
  sourceType: AudioSourceType;
  networkId?: string;
  isAvailable: boolean;
  /** AudioKit 原生设备 id，用于 selectOutputDevice */
  nativeDeviceId?: number;
  address?: string;
}

/** 音频路由配置 */
export interface AudioRouteConfig {
  outputDevice: AudioDeviceInfo | null;
  inputDevice: AudioDeviceInfo | null;
  volume: number;
}

/** 音量查询结果（系统 / 播放链路 / 应用配置） */
export interface VolumeQueryResult {
  appVolume: number;
  /** 系统媒体流音量等级，见 AudioVolumeManager.getVolumeByStream */
  streamVolume: number;
  streamVolumeMin: number;
  streamVolumeMax: number;
  streamVolumePercent: number;
  /** AudioRenderer.getVolume × 100，无 Renderer 时为 -1 */
  rendererVolumePercent: number;
  detail: string;
}
