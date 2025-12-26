/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dcamera_utils_tools.h"
#include "image_converter.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraPipelineSource;
class DecodeVideoCallback;

typedef struct {
    int32_t width;
    int32_t height;
    uint8_t* dataY;
    int32_t strideY;
    uint8_t* dataU;
    int32_t strideU;
    uint8_t* dataV;
    int32_t strideV;
} ImageDataInfo;

class DecodeDataProcess : public AbstractDataProcess, public std::enable_shared_from_this<DecodeDataProcess> {
public:
    DecodeDataProcess(const std::shared_ptr<AppExecFwk::EventHandler>& pipeEventHandler,
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

    int32_t UpdateSettings(const std::shared_ptr<Camera::CameraMetadata> settings) override;

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
    void StartEventHandler();
    bool UniversalRotateCropAndPadNv12ToI420(ImageDataInfo srcInfo, ImageDataInfo dstInfo, int angleDegrees);
    bool ConvertToI420BySystemSwitch(uint8_t *srcDataY, uint8_t *srcDataUV, int32_t alignedWidth,
        int32_t alignedHeight, std::shared_ptr<DataBuffer> bufferOutput);
    bool ConvertToI420(uint8_t *srcDataY, uint8_t *srcDataUV, int32_t alignedWidth,
        int32_t alignedHeight, std::shared_ptr<DataBuffer> bufferOutput);
    bool NV12ToI420RotateBySystemSwitch(ImageDataInfo dataInfo, std::vector<uint8_t> rotatedBuffer,
        int srcWidth, int srcHeight, OpenSourceLibyuv::RotationMode mode);
    bool I420CopyBySystemSwitch(ImageDataInfo srcInfo, ImageDataInfo dstInfo,
        int srcWidth, int srcHeight, int32_t normalizedAngle);
    bool CheckParameters(ImageDataInfo srcInfo, ImageDataInfo dstInfo);
    OpenSourceLibyuv::RotationMode ParseAngle(int angleDegrees);
    bool FreeYUVBuffer(uint8_t*& dataY, uint8_t*& dataU, uint8_t*& dataV);

private:
    constexpr static int32_t VIDEO_DECODER_QUEUE_MAX = 1000;
    constexpr static int32_t MAX_YUV420_BUFFER_SIZE = 1920 * 1080 * 3 / 2 * 2;
    constexpr static int32_t MAX_RGB32_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    constexpr static int32_t MAX_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    constexpr static int32_t MIN_FRAME_RATE = 0;
    constexpr static double MAX_FRAME_RATE = 30;
    constexpr static int32_t MIN_VIDEO_WIDTH = 320;
    constexpr static int32_t MIN_VIDEO_HEIGHT = 240;
    constexpr static int32_t MAX_VIDEO_WIDTH = 4160;
    constexpr static int32_t MAX_VIDEO_HEIGHT = 3120;
    constexpr static int32_t FIRST_FRAME_INPUT_NUM = 2;
    constexpr static int32_t RGB32_MEMORY_COEFFICIENT = 4;
    constexpr static int32_t YUV_BYTES_PER_PIXEL = 3;
    constexpr static int32_t Y2UV_RATIO = 2;
    constexpr static int32_t BUFFER_MAX_SIZE = 50 * 1024 * 1024;
    constexpr static int32_t ALIGNED_WIDTH_MAX_SIZE = 10000;
    constexpr static uint32_t MEMORY_RATIO_UV = 1;
    constexpr static uint32_t OFFSET_X_0 = 0;
    constexpr static uint32_t OFFSET_Y_0 = 0;
    constexpr static uint32_t BLACK_COLOR_PEXEL = 0;
    constexpr static uint32_t WHITE_COLOR_PEXEL = 128;
    std::shared_ptr<AppExecFwk::EventHandler> pipeSrcEventHandler_;
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    std::mutex mtxDecoderLock_;
    std::mutex mtxDecoderState_;
    std::mutex mtxHoldCount_;
    std::mutex mtxDequeLock_;
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
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
    FILE *dumpDecBeforeFile_ = nullptr;
    FILE *dumpDecAfterFile_ = nullptr;
    int32_t rotate_ = 0;

    std::mutex eventMutex_;
    std::thread eventThread_;
    std::condition_variable eventCon_;
    std::shared_ptr<AppExecFwk::EventHandler> decEventHandler_ = nullptr;
    int32_t ProcessSingleInputBuffer();
    int32_t GetAvailableDecoderBuffer(uint32_t& index, std::shared_ptr<Media::AVSharedMemory>& sharedMemoryInput);
    int32_t QueueBufferToDecoder(std::shared_ptr<DataBuffer>& buffer, uint32_t index,
        std::shared_ptr<Media::AVSharedMemory>& sharedMemoryInput);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DECODE_DATA_PROCESS_H
