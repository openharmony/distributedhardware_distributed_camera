/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <thread>
#include <vector>

#include "avcodec_common.h"
#include "avcodec_video_decoder.h"
#include "avsharedmemory.h"
#include "event.h"
#include "event_bus.h"
#include "event_sender.h"
#include "event_registration.h"
#include "eventbus_handler.h"
#include "format.h"
#include "ibuffer_consumer_listener.h"
#include "media_errors.h"
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

class DecodeDataProcess : public EventSender, public EventBusHandler<DCameraCodecEvent>, public AbstractDataProcess,
    public std::enable_shared_from_this<DecodeDataProcess> {
public:
    DecodeDataProcess(const std::shared_ptr<EventBus>& eventBusPipeline,
        const std::weak_ptr<DCameraPipelineSource>& callbackPipSource)
        : eventBusPipeline_(eventBusPipeline), callbackPipelineSource_(callbackPipSource) {}
    ~DecodeDataProcess();

    int32_t InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
        VideoConfigParams& processedConfig) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers) override;
    void ReleaseProcessNode() override;
    void OnEvent(DCameraCodecEvent& ev) override;

    void OnError();
    void OnInputBufferAvailable(uint32_t index);
    void OnOutputFormatChanged(const Media::Format &format);
    void OnOutputBufferAvailable(uint32_t index, const Media::AVCodecBufferInfo& info,
        const Media::AVCodecBufferFlag& flag);
    void GetDecoderOutputBuffer(const sptr<Surface>& surface);
    VideoConfigParams GetSourceConfig() const;
    VideoConfigParams GetTargetConfig() const;

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
    int32_t FeedDecoderInputBuffer();
    int64_t GetDecoderTimeStamp();
    void IncreaseWaitDecodeCnt();
    void ReduceWaitDecodeCnt();
    void CopyDecodedImage(const sptr<SurfaceBuffer>& surBuf, int64_t timeStampUs, int32_t alignedWidth,
        int32_t alignedHeight);
    int32_t CopyYUVPlaneByRow(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t CheckCopyImageInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    bool IsCorrectImageUnitInfo(const ImageUnitInfo& imgInfo);
    bool IsCorrectSurfaceBuffer(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth, int32_t alignedHeight);
    void PostOutputDataBuffers(std::shared_ptr<DataBuffer>& outputBuffer);
    int32_t DecodeDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers);

private:
    constexpr static int32_t VIDEO_DECODER_QUEUE_MAX = 1000;
    constexpr static int32_t MAX_YUV420_BUFFER_SIZE = 1920 * 1080 * 3 / 2 * 2;
    constexpr static int32_t MAX_RGB32_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    constexpr static int32_t MIN_FRAME_RATE = 0;
    constexpr static int32_t MAX_FRAME_RATE = 30;
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

    std::shared_ptr<EventBus> eventBusPipeline_;
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    std::mutex mtxDecoderState_;
    std::mutex mtxHoldCount_;
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
    std::shared_ptr<EventBus> eventBusDecode_ = nullptr;
    std::shared_ptr<EventRegistration> eventBusRegHandleDecode_ = nullptr;
    std::shared_ptr<EventRegistration> eventBusRegHandlePipeline2Decode_ = nullptr;
    std::shared_ptr<Media::AVCodecVideoDecoder> videoDecoder_ = nullptr;
    std::shared_ptr<Media::AVCodecCallback> decodeVideoCallback_ = nullptr;
    sptr<Surface> decodeConsumerSurface_ = nullptr;
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
    Media::AVCodecBufferInfo outputInfo_;
    std::queue<std::shared_ptr<DataBuffer>> inputBuffersQueue_;
    std::queue<uint32_t> availableInputIndexsQueue_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DECODE_DATA_PROCESS_H
