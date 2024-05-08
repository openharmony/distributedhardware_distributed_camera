/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DECODE_DATA_PROCESS_H
#define OHOS_DECODE_DATA_PROCESS_H

#include <chrono>
#include <cstdint>
#include <queue>
#include <deque>
#include <thread>
#include <vector>

#include "avcodec_common.h"
#include "avcodec_video_decoder.h"
#include "buffer/avsharedmemory.h"
#include "event_handler.h"
#include "meta/format.h"
#include "ibuffer_consumer_listener.h"
#include "iconsumer_surface.h"
#include "avcodec_errors.h"
#include "securec.h"
#include "surface.h"

#include "abstract_data_process.h"
#include "data_buffer.h"
#include "dcamera_codec_event.h"
#include "dcamera_pipeline_source.h"
#include "distributed_camera_errno.h"
#include "image_common_type.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraPipelineSource;
class DecodeVideoCallback;
const uint32_t EVENT_NO_ACTION = 0;
const uint32_t EVENT_ACTION_ONCE_AGAIN = 1;
const uint32_t EVENT_ACTION_GET_DECODER_OUTPUT_BUFFER = 2;

class DecodeDataProcess : public AbstractDataProcess, public std::enable_shared_from_this<DecodeDataProcess> {
public:
    DecodeDataProcess(const std::shared_ptr<DCameraPipelineSource::DCameraPipelineSrcEventHandler>& pipeEventHandler,
        const std::weak_ptr<DCameraPipelineSource>& callbackPipSource)
        : pipeSrcEventHandler_(pipeEventHandler), callbackPipelineSource_(callbackPipSource) {}
    ~DecodeDataProcess() override;

    int32_t InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
        VideoConfigParams& processedConfig) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers) override;
    void ReleaseProcessNode() override;

    void OnError();
    void OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer);
    void OnOutputFormatChanged(const Media::Format &format);
    void OnOutputBufferAvailable(uint32_t index, const MediaAVCodec::AVCodecBufferInfo& info,
        const MediaAVCodec::AVCodecBufferFlag& flag, std::shared_ptr<Media::AVSharedMemory> buffer);
    void GetDecoderOutputBuffer(const sptr<IConsumerSurface>& surface);
    VideoConfigParams GetSourceConfig() const;
    VideoConfigParams GetTargetConfig() const;
    void OnSurfaceOutputBufferAvailable(const sptr<IConsumerSurface>& surface);
    void AlignFirstFrameTime();

    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier) override;

    class DecodeDataProcessEventHandler : public AppExecFwk::EventHandler {
        public:
            DecodeDataProcessEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
                std::shared_ptr<DecodeDataProcess> decPtr);
            ~DecodeDataProcessEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
        private:
            std::weak_ptr<DecodeDataProcess> decPtrWPtr_;
    };

private:
    bool IsInDecoderRange(const VideoConfigParams& curConfig);
    bool IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig);
    void InitCodecEvent();
    int32_t InitDecoder();
    int32_t ConfigureVideoDecoder();
    int32_t InitDecoderMetadataFormat();
    int32_t SetDecoderOutputSurface();
    int32_t StartVideoDecoder();
    int32_t StopVideoDecoder();
    void ReleaseVideoDecoder();
    void ReleaseDecoderSurface();
    void ReleaseCodecEvent();
    void BeforeDecodeDump(uint8_t *buffer, size_t bufSize);
    int32_t FeedDecoderInputBuffer();
    int64_t GetDecoderTimeStamp();
    void IncreaseWaitDecodeCnt();
    void ReduceWaitDecodeCnt();
    void CopyDecodedImage(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth, int32_t alignedHeight);
    bool IsCorrectSurfaceBuffer(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth, int32_t alignedHeight);
    void PostOutputDataBuffers(std::shared_ptr<DataBuffer>& outputBuffer);
    int32_t DecodeDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers);
    void ProcessFeedDecoderInputBuffer();
    void ProcessGetDecoderOutputBuffer(const AppExecFwk::InnerEvent::Pointer &event);
    void ProcessDecodeDone(const AppExecFwk::InnerEvent::Pointer &event);

private:
    constexpr static int32_t VIDEO_DECODER_QUEUE_MAX = 1000;
    constexpr static int32_t MAX_YUV420_BUFFER_SIZE = 1920 * 1080 * 3 / 2 * 2;
    constexpr static int32_t MAX_RGB32_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    constexpr static int32_t MAX_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    constexpr static int32_t MIN_FRAME_RATE = 0;
    constexpr static double MAX_FRAME_RATE = 30;
    constexpr static int32_t MIN_VIDEO_WIDTH = 320;
    constexpr static int32_t MIN_VIDEO_HEIGHT = 240;
    constexpr static int32_t MAX_VIDEO_WIDTH = 1920;
    constexpr static int32_t MAX_VIDEO_HEIGHT = 1080;
    constexpr static int32_t FIRST_FRAME_INPUT_NUM = 2;
    constexpr static int32_t RGB32_MEMORY_COEFFICIENT = 4;
    constexpr static int32_t YUV_BYTES_PER_PIXEL = 3;
    constexpr static int32_t Y2UV_RATIO = 2;
    constexpr static int32_t BUFFER_MAX_SIZE = 50 * 1024 * 1024;
    constexpr static int32_t ALIGNED_WIDTH_MAX_SIZE = 10000;
    constexpr static uint32_t MEMORY_RATIO_UV = 1;

    std::shared_ptr<DCameraPipelineSource::DCameraPipelineSrcEventHandler> pipeSrcEventHandler_;
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    std::mutex mtxDecoderLock_;
    std::mutex mtxDecoderState_;
    std::mutex mtxHoldCount_;
    std::mutex mtxDequeLock_;
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
    std::shared_ptr<DecodeDataProcessEventHandler> decEventHandler_;
    std::shared_ptr<MediaAVCodec::AVCodecVideoDecoder> videoDecoder_ = nullptr;
    std::shared_ptr<MediaAVCodec::AVCodecCallback> decodeVideoCallback_ = nullptr;
    sptr<IConsumerSurface> decodeConsumerSurface_ = nullptr;
    sptr<Surface> decodeProducerSurface_ = nullptr;
    sptr<IBufferConsumerListener> decodeSurfaceListener_ = nullptr;

    std::atomic<bool> isDecoderProcess_ = false;
    int32_t waitDecoderOutputCount_ = 0;
    int32_t alignedHeight_ = 0;
    int64_t lastFeedDecoderInputBufferTimeUs_ = 0;
    int64_t outputTimeStampUs_ = 0;
    std::string processType_;
    Media::Format metadataFormat_;
    Media::Format decodeOutputFormat_;
    MediaAVCodec::AVCodecBufferInfo outputInfo_;
    std::queue<std::shared_ptr<DataBuffer>> inputBuffersQueue_;
    std::queue<std::shared_ptr<Media::AVSharedMemory>> availableInputBufferQueue_;
    std::queue<uint32_t> availableInputIndexsQueue_;
    std::deque<DCameraFrameInfo> frameInfoDeque_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DECODE_DATA_PROCESS_H
