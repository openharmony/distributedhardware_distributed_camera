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
