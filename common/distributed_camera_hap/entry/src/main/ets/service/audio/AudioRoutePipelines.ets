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
import { DhDiag } from '../../common/util/DhDiag';
import { AudioRouteHelpers } from './AudioRouteHelpers';
import { AudioRouteState } from './AudioRouteState';
import { AudioRouteService } from '../AudioRouteService';

/** Capturer / Renderer 管道创建与释放 */
export class AudioRoutePipelines {
  private readonly host: AudioRouteService;

  constructor(host: AudioRouteService) {
    this.host = host;
  }

  private get st(): AudioRouteState { return this.host.state; }

  public async resetOutputPipeline(): Promise<void> {
    if (this.st.audioRenderer) {
      try {
        await this.st.audioRenderer.stop();
        await this.st.audioRenderer.release();
      } catch (_e) {
        DhDiag.logCatchWarn('AUDIO', 'resetOutputPipeline stop/release', _e);
      }
      this.st.audioRenderer = null;
    }
  }

  public async resetInputPipeline(): Promise<void> {
    if (this.st.audioCapturer) {
      try {
        await this.st.audioCapturer.stop();
        await this.st.audioCapturer.release();
      } catch (_e) {
        DhDiag.logCatchWarn('AUDIO', 'resetInputPipeline stop/release', _e);
      }
      this.st.audioCapturer = null;
    }
  }

  public async ensureRenderer(): Promise<void> {
    if (this.st.audioRenderer) {
      return;
    }
    const streamInfo: audio.AudioStreamInfo = {
      samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
      channels: audio.AudioChannel.CHANNEL_2,
      sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
      encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    };
    const rendererInfo: audio.AudioRendererInfo = {
      usage: audio.StreamUsage.STREAM_USAGE_VOICE_COMMUNICATION,
      rendererFlags: 0
    };
    this.st.audioRenderer = await audio.createAudioRenderer({ streamInfo, rendererInfo });
    this.st.audioRenderer.setVolume(this.st.routeConfig.volume / 100);
  }

  public async ensureCapturer(): Promise<void> {
    if (this.st.audioCapturer) {
      return;
    }
    const streamInfo: audio.AudioStreamInfo = {
      samplingRate: audio.AudioSamplingRate.SAMPLE_RATE_48000,
      channels: audio.AudioChannel.CHANNEL_1,
      sampleFormat: audio.AudioSampleFormat.SAMPLE_FORMAT_S16LE,
      encodingType: audio.AudioEncodingType.ENCODING_TYPE_RAW
    };
    const capturerInfo: audio.AudioCapturerInfo = {
      source: audio.SourceType.SOURCE_TYPE_MIC,
      capturerFlags: 0
    };
    this.st.audioCapturer = await audio.createAudioCapturer({ streamInfo, capturerInfo });
  }

  public generateToneBuffer(durationSec: number = AudioRouteHelpers.TONE_SEC,
    freq: number = AudioRouteHelpers.TONE_HZ): ArrayBuffer {
    const samples = Math.floor(durationSec * AudioRouteHelpers.SAMPLE_RATE);
    const buffer = new ArrayBuffer(samples * 4);
    const view = new DataView(buffer);
    for (let i = 0; i < samples; i++) {
      const sample = Math.floor(Math.sin(2 * Math.PI * freq * i / AudioRouteHelpers.SAMPLE_RATE) * 0.25 * 32767);
      view.setInt16(i * 4, sample, true);
      view.setInt16(i * 4 + 2, sample, true);
    }
    return buffer;
  }
}
