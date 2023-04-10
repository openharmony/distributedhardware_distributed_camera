/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "decode_data_process.h"

#include "libyuv.h"
#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_utils_tools.h"
#include "decode_surface_listener.h"
#include "decode_video_callback.h"
#include "graphic_common_c.h"

namespace OHOS {
namespace DistributedHardware {
const std::string ENUM_VIDEOFORMAT_STRINGS[] = {
    "YUVI420", "NV12", "NV21", "RGBA_8888"
};

DecodeDataProcess::~DecodeDataProcess()
{
    if (isDecoderProcess_.load()) {
        DHLOGD("~DecodeDataProcess : ReleaseProcessNode.");
        ReleaseProcessNode();
    }
}

int32_t DecodeDataProcess::InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
    VideoConfigParams& processedConfig)
{
    DHLOGD("Init DCamera DecodeNode start.");
    if (!(IsInDecoderRange(sourceConfig) && IsInDecoderRange(targetConfig))) {
        DHLOGE("Source config or target config are invalid.");
        return DCAMERA_BAD_VALUE;
    }
    if (!IsConvertible(sourceConfig, targetConfig)) {
        DHLOGE("The DecodeNode can't convert %d to %d.", sourceConfig.GetVideoCodecType(),
            targetConfig_.GetVideoCodecType());
        return DCAMERA_BAD_TYPE;
    }

    sourceConfig_ = sourceConfig;
    targetConfig_ = targetConfig;
    if (sourceConfig_.GetVideoCodecType() == targetConfig_.GetVideoCodecType()) {
        DHLOGD("Disable DecodeNode. The target video codec type %d is the same as the source video codec type %d.",
            targetConfig_.GetVideoCodecType(), sourceConfig_.GetVideoCodecType());
        processedConfig_ = sourceConfig;
        processedConfig = processedConfig_;
        isDecoderProcess_.store(true);
        return DCAMERA_OK;
    }

    InitCodecEvent();
    int32_t err = InitDecoder();
    if (err != DCAMERA_OK) {
        DHLOGE("Init video decoder failed.");
        ReleaseProcessNode();
        return err;
    }
    alignedHeight_ = GetAlignedHeight(sourceConfig_.GetHeight());
    processedConfig = processedConfig_;
    isDecoderProcess_.store(true);
    return DCAMERA_OK;
}

bool DecodeDataProcess::IsInDecoderRange(const VideoConfigParams& curConfig)
{
    return (curConfig.GetWidth() >= MIN_VIDEO_WIDTH || curConfig.GetWidth() <= MAX_VIDEO_WIDTH ||
        curConfig.GetHeight() >= MIN_VIDEO_HEIGHT || curConfig.GetHeight() <= MAX_VIDEO_HEIGHT ||
        curConfig.GetFrameRate() >= MIN_FRAME_RATE || curConfig.GetFrameRate() <= MAX_FRAME_RATE);
}

bool DecodeDataProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return (sourceConfig.GetVideoCodecType() == targetConfig.GetVideoCodecType() ||
        targetConfig.GetVideoCodecType() == VideoCodecType::NO_CODEC);
}

void DecodeDataProcess::InitCodecEvent()
{
    DHLOGD("Init DecodeNode eventBus, and add handler for it.");
    eventBusDecode_ = std::make_shared<EventBus>("DeDtProcHandler");
    DCameraCodecEvent codecEvent(*this, std::make_shared<CodecPacket>());
    eventBusRegHandleDecode_ = eventBusDecode_->AddHandler<DCameraCodecEvent>(codecEvent.GetType(), *this);

    DHLOGD("Add handler for DCamera pipeline eventBus.");
    eventBusRegHandlePipeline2Decode_ = eventBusPipeline_->AddHandler<DCameraCodecEvent>(codecEvent.GetType(), *this);
}

int32_t DecodeDataProcess::InitDecoder()
{
    DHLOGD("Init video decoder.");
    int32_t ret = ConfigureVideoDecoder();
    if (ret != DCAMERA_OK) {
        DHLOGE("Init video decoder metadata format failed.");
        return ret;
    }

    ret = StartVideoDecoder();
    if (ret != DCAMERA_OK) {
        DHLOGE("Start Video decoder failed.");
        ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_DECODE_ERROR,
            CreateMsg("start video decoder failed, width: %d, height: %d, format: %s", sourceConfig_.GetWidth(),
            sourceConfig_.GetHeight(),
            ENUM_VIDEOFORMAT_STRINGS[static_cast<int32_t>(sourceConfig_.GetVideoformat())].c_str()));
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::ConfigureVideoDecoder()
{
    int32_t ret = InitDecoderMetadataFormat();
    if (ret != DCAMERA_OK) {
        DHLOGE("Init video decoder metadata format failed. Error code %d.", ret);
        return ret;
    }

    videoDecoder_ = Media::VideoDecoderFactory::CreateByMime(processType_);
    if (videoDecoder_ == nullptr) {
        DHLOGE("Create video decoder failed.");
        return DCAMERA_INIT_ERR;
    }
    decodeVideoCallback_ = std::make_shared<DecodeVideoCallback>(shared_from_this());
    ret = videoDecoder_->SetCallback(decodeVideoCallback_);
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Set video decoder callback failed. Error code %d.", ret);
        return DCAMERA_INIT_ERR;
    }

    ret = videoDecoder_->Configure(metadataFormat_);
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Set video decoder metadata format failed. Error code %d.", ret);
        return DCAMERA_INIT_ERR;
    }

    ret = SetDecoderOutputSurface();
    if (ret != DCAMERA_OK) {
        DHLOGE("Set decoder output surface failed. Error code %d.", ret);
        return ret;
    }

    return DCAMERA_OK;
}

int32_t DecodeDataProcess::InitDecoderMetadataFormat()
{
    DHLOGI("Init video decoder metadata format. codecType: %d", sourceConfig_.GetVideoCodecType());
    processedConfig_ = sourceConfig_;
    processedConfig_.SetVideoCodecType(VideoCodecType::NO_CODEC);
    switch (sourceConfig_.GetVideoCodecType()) {
        case VideoCodecType::CODEC_H264:
            processType_ = "video/avc";
            processedConfig_.SetVideoformat(Videoformat::NV12);
            break;
        case VideoCodecType::CODEC_H265:
            processType_ = "video/hevc";
            processedConfig_.SetVideoformat(Videoformat::NV12);
            break;
        case VideoCodecType::CODEC_MPEG4_ES:
            processType_ = "video/mp4v-es";
            break;
        default:
            DHLOGE("The current codec type does not support decoding.");
            return DCAMERA_NOT_FOUND;
    }

    metadataFormat_.PutIntValue("pixel_format", Media::VideoPixelFormat::NV12);
    metadataFormat_.PutStringValue("codec_mime", processType_);
    metadataFormat_.PutIntValue("width", sourceConfig_.GetWidth());
    metadataFormat_.PutIntValue("height", sourceConfig_.GetHeight());
    metadataFormat_.PutIntValue("frame_rate", MAX_FRAME_RATE);

    return DCAMERA_OK;
}

int32_t DecodeDataProcess::SetDecoderOutputSurface()
{
    DHLOGD("Set the video decoder output surface.");
    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder is null.");
        return DCAMERA_BAD_VALUE;
    }

    decodeConsumerSurface_ = IConsumerSurface::Create();
    if (decodeConsumerSurface_ == nullptr) {
        DHLOGE("Create the decode consumer surface failed.");
        return DCAMERA_INIT_ERR;
    }
    decodeConsumerSurface_->SetDefaultWidthAndHeight(static_cast<int32_t>(sourceConfig_.GetWidth()),
        static_cast<int32_t>(sourceConfig_.GetHeight()));
    decodeSurfaceListener_ = new DecodeSurfaceListener(decodeConsumerSurface_, shared_from_this());
    if (decodeConsumerSurface_->RegisterConsumerListener(decodeSurfaceListener_) !=
        SURFACE_ERROR_OK) {
        DHLOGE("Register consumer listener failed.");
        return DCAMERA_INIT_ERR;
    }

    sptr<IBufferProducer> surfaceProducer = decodeConsumerSurface_->GetProducer();
    if (surfaceProducer == nullptr) {
        DHLOGE("Get the surface producer of the decode consumer surface failed.");
        return DCAMERA_INIT_ERR;
    }
    decodeProducerSurface_ = Surface::CreateSurfaceAsProducer(surfaceProducer);
    if (decodeProducerSurface_ == nullptr) {
        DHLOGE("Create the decode producer surface of the decode consumer surface failed.");
        return DCAMERA_INIT_ERR;
    }

    DHLOGD("Set the producer surface to video decoder output surface.");
    int32_t err = videoDecoder_->SetOutputSurface(decodeProducerSurface_);
    if (err != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Set decoder output surface failed.");
        return DCAMERA_INIT_ERR;
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::StartVideoDecoder()
{
    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder does not exist before StartVideoDecoder.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ret = videoDecoder_->Prepare();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Video decoder prepare failed. Error code %d.", ret);
        return DCAMERA_INIT_ERR;
    }
    ret = videoDecoder_->Start();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("Video decoder start failed. Error code %d.", ret);
        return DCAMERA_INIT_ERR;
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::StopVideoDecoder()
{
    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder does not exist before StopVideoDecoder.");
        return DCAMERA_BAD_VALUE;
    }

    bool isSuccess = true;
    int32_t ret = videoDecoder_->Flush();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("VideoDecoder flush failed. Error type: %d.", ret);
        isSuccess = isSuccess && false;
    }
    ret = videoDecoder_->Stop();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("VideoDecoder stop failed. Error type: %d.", ret);
        isSuccess = isSuccess && false;
    }
    if (!isSuccess) {
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

void DecodeDataProcess::ReleaseVideoDecoder()
{
    DHLOGD("Start release videoDecoder.");
    std::lock_guard<std::mutex> inputLock(mtxDecoderLock_);
    std::lock_guard<std::mutex> outputLock(mtxDecoderState_);
    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder does not exist before ReleaseVideoDecoder.");
        decodeVideoCallback_ = nullptr;
        return;
    }
    int32_t ret = StopVideoDecoder();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopVideoDecoder failed.");
    }
    ret = videoDecoder_->Release();
    if (ret != Media::MediaServiceErrCode::MSERR_OK) {
        DHLOGE("VideoDecoder release failed. Error type: %d.", ret);
    }
    videoDecoder_ = nullptr;
    decodeVideoCallback_ = nullptr;
}

void DecodeDataProcess::ReleaseDecoderSurface()
{
    if (decodeConsumerSurface_ == nullptr) {
        decodeProducerSurface_ = nullptr;
        DHLOGE("The decode consumer surface does not exist before UnregisterConsumerListener.");
        return;
    }
    int32_t ret = decodeConsumerSurface_->UnregisterConsumerListener();
    if (ret != SURFACE_ERROR_OK) {
        DHLOGE("Unregister consumer listener failed. Error type: %d.", ret);
    }
    decodeConsumerSurface_ = nullptr;
    decodeProducerSurface_ = nullptr;
}

void DecodeDataProcess::ReleaseCodecEvent()
{
    DCameraCodecEvent codecEvent(*this, std::make_shared<CodecPacket>());
    if (eventBusDecode_ != nullptr) {
        eventBusDecode_->RemoveHandler<DCameraCodecEvent>(codecEvent.GetType(), eventBusRegHandleDecode_);
        eventBusRegHandleDecode_ = nullptr;
        eventBusDecode_ = nullptr;
    }
    if (eventBusPipeline_ != nullptr) {
        eventBusPipeline_->RemoveHandler<DCameraCodecEvent>(codecEvent.GetType(), eventBusRegHandlePipeline2Decode_);
        eventBusRegHandlePipeline2Decode_ = nullptr;
        eventBusPipeline_ = nullptr;
    }
    DHLOGD("Release DecodeNode eventBusDecode and eventBusPipeline end.");
}

void DecodeDataProcess::ReleaseProcessNode()
{
    DHLOGD("Start release [%d] node : DecodeNode.", nodeRank_);
    isDecoderProcess_.store(false);
    ReleaseVideoDecoder();
    ReleaseDecoderSurface();
    ReleaseCodecEvent();

    processType_ = "";
    std::queue<std::shared_ptr<DataBuffer>>().swap(inputBuffersQueue_);
    std::queue<uint32_t>().swap(availableInputIndexsQueue_);
    std::deque<DCameraFrameInfo>().swap(frameInfoDeque_);
    waitDecoderOutputCount_ = 0;
    lastFeedDecoderInputBufferTimeUs_ = 0;
    outputTimeStampUs_ = 0;
    alignedHeight_ = 0;

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGD("Release [%d] node : DecodeNode end.", nodeRank_);
}

int32_t DecodeDataProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    DHLOGD("Process data in DecodeDataProcess.");
    if (inputBuffers.empty()) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }
    if (sourceConfig_.GetVideoCodecType() == processedConfig_.GetVideoCodecType()) {
        DHLOGD("The target VideoCodecType : %d is the same as the source VideoCodecType : %d.",
            sourceConfig_.GetVideoCodecType(), processedConfig_.GetVideoCodecType());
        return DecodeDone(inputBuffers);
    }

    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder does not exist before decoding data.");
        return DCAMERA_INIT_ERR;
    }
    if (inputBuffersQueue_.size() > VIDEO_DECODER_QUEUE_MAX) {
        DHLOGE("video decoder input buffers queue over flow.");
        return DCAMERA_INDEX_OVERFLOW;
    }
    if (inputBuffers[0]->Size() > MAX_YUV420_BUFFER_SIZE) {
        DHLOGE("DecodeNode input buffer size %zu error.", inputBuffers[0]->Size());
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    if (!isDecoderProcess_.load()) {
        DHLOGE("Decoder node occurred error or start release.");
        return DCAMERA_DISABLE_PROCESS;
    }
    inputBuffersQueue_.push(inputBuffers[0]);
    DHLOGD("Push inputBuf sucess. BufSize %zu, QueueSize %zu.", inputBuffers[0]->Size(), inputBuffersQueue_.size());
    int32_t err = FeedDecoderInputBuffer();
    if (err != DCAMERA_OK) {
        int32_t sleepTimeUs = 5000;
        std::this_thread::sleep_for(std::chrono::microseconds(sleepTimeUs));
        DHLOGD("Feed decoder input buffer failed. Try FeedDecoderInputBuffer again.");
        std::shared_ptr<CodecPacket> reFeedInputPacket = std::make_shared<CodecPacket>();
        reFeedInputPacket->SetVideoCodecType(sourceConfig_.GetVideoCodecType());
        DCameraCodecEvent dCamCodecEv(*this, reFeedInputPacket, VideoCodecAction::ACTION_ONCE_AGAIN);
        if (eventBusPipeline_ == nullptr) {
            DHLOGE("eventBusPipeline_ is nullptr.");
            return DCAMERA_BAD_VALUE;
        }
        eventBusPipeline_->PostEvent<DCameraCodecEvent>(dCamCodecEv, POSTMODE::POST_ASYNC);
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::FeedDecoderInputBuffer()
{
    DHLOGD("Feed decoder input buffer.");
    while ((!inputBuffersQueue_.empty()) && (isDecoderProcess_.load())) {
        std::shared_ptr<DataBuffer> buffer = inputBuffersQueue_.front();
        if (buffer == nullptr || availableInputIndexsQueue_.empty()) {
            DHLOGE("inputBuffersQueue size %zu, availableInputIndexsQueue size %zu.",
                inputBuffersQueue_.size(), availableInputIndexsQueue_.size());
            return DCAMERA_BAD_VALUE;
        }
        buffer->frameInfo_.timePonit.startDecode = GetNowTimeStampUs();
        {
            std::lock_guard<std::mutex> lock(mtxDequeLock_);
            frameInfoDeque_.push_back(buffer->frameInfo_);
        }
        int64_t timeStamp = buffer->frameInfo_.pts;
        {
            std::lock_guard<std::mutex> inputLock(mtxDecoderLock_);
            if (videoDecoder_ == nullptr) {
                DHLOGE("The video decoder does not exist before GetInputBuffer.");
                return DCAMERA_OK;
            }
            uint32_t index = availableInputIndexsQueue_.front();
            std::shared_ptr<Media::AVSharedMemory> sharedMemoryInput = videoDecoder_->GetInputBuffer(index);
            if (sharedMemoryInput == nullptr) {
                DHLOGE("Failed to obtain the input shared memory corresponding to the [%u] index.", index);
                return DCAMERA_BAD_VALUE;
            }
            size_t inputMemoDataSize = static_cast<size_t>(sharedMemoryInput->GetSize());
            errno_t err = memcpy_s(sharedMemoryInput->GetBase(), inputMemoDataSize, buffer->Data(), buffer->Size());
            if (err != EOK) {
                DHLOGE("memcpy_s buffer failed.");
                return DCAMERA_MEMORY_OPT_ERROR;
            }
            DHLOGD("Decoder input buffer size %zu, timeStamp %ld us.", buffer->Size(), timeStamp);
            Media::AVCodecBufferInfo bufferInfo {timeStamp, static_cast<int32_t>(buffer->Size()), 0};
            int32_t ret = videoDecoder_->QueueInputBuffer(index, bufferInfo,
                Media::AVCODEC_BUFFER_FLAG_NONE);
            if (ret != Media::MediaServiceErrCode::MSERR_OK) {
                DHLOGE("queue Input buffer failed.");
                return DCAMERA_BAD_OPERATE;
            }
        }

        inputBuffersQueue_.pop();
        DHLOGD("Push inputBuffer sucess. inputBuffersQueue size is %d.", inputBuffersQueue_.size());

        IncreaseWaitDecodeCnt();
    }
    return DCAMERA_OK;
}

int64_t DecodeDataProcess::GetDecoderTimeStamp()
{
    int64_t TimeIntervalStampUs = 0;
    int64_t nowTimeUs = GetNowTimeStampUs();
    if (lastFeedDecoderInputBufferTimeUs_ == 0) {
        lastFeedDecoderInputBufferTimeUs_ = nowTimeUs;
        return TimeIntervalStampUs;
    }
    TimeIntervalStampUs = nowTimeUs - lastFeedDecoderInputBufferTimeUs_;
    lastFeedDecoderInputBufferTimeUs_ = nowTimeUs;
    return TimeIntervalStampUs;
}

void DecodeDataProcess::IncreaseWaitDecodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    availableInputIndexsQueue_.pop();
    waitDecoderOutputCount_++;
    DHLOGD("Wait decoder output frames number is %d.", waitDecoderOutputCount_);
}

void DecodeDataProcess::ReduceWaitDecodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (waitDecoderOutputCount_ <= 0) {
        DHLOGE("The waitDecoderOutputCount_ = %d.", waitDecoderOutputCount_);
    }
    if (outputTimeStampUs_ == 0) {
        waitDecoderOutputCount_ -= FIRST_FRAME_INPUT_NUM;
    } else {
        waitDecoderOutputCount_--;
    }
    DHLOGD("Wait decoder output frames number is %d.", waitDecoderOutputCount_);
}

void DecodeDataProcess::OnSurfaceOutputBufferAvailable(const sptr<IConsumerSurface>& surface)
{
    std::shared_ptr<CodecPacket> bufferPkt = std::make_shared<CodecPacket>(surface);
    DCameraCodecEvent dCamCodecEv(*this, bufferPkt, VideoCodecAction::ACTION_GET_DECODER_OUTPUT_BUFFER);
    eventBusDecode_->PostEvent<DCameraCodecEvent>(dCamCodecEv, POSTMODE::POST_ASYNC);
}

void DecodeDataProcess::GetDecoderOutputBuffer(const sptr<IConsumerSurface>& surface)
{
    DHLOGD("Get decoder output buffer.");
    if (surface == nullptr) {
        DHLOGE("Get decode consumer surface failed.");
        return;
    }
    Rect damage = {0, 0, 0, 0};
    int32_t acquireFence = 0;
    int64_t timeStamp = 0;
    sptr<SurfaceBuffer> surfaceBuffer = nullptr;
    GSError ret = surface->AcquireBuffer(surfaceBuffer, acquireFence, timeStamp, damage);
    if (ret != GSERROR_OK || surfaceBuffer == nullptr) {
        DHLOGE("Acquire surface buffer failed!");
        return;
    }
    int32_t alignedWidth = surfaceBuffer->GetStride();
    if (surfaceBuffer->GetSize() > BUFFER_MAX_SIZE || alignedWidth > ALIGNED_WIDTH_MAX_SIZE) {
        DHLOGE("surface buffer size or alignedWidth too long");
        return;
    }
    int32_t alignedHeight = alignedHeight_;
    DHLOGD("OutputBuffer alignedWidth %d, alignedHeight %d, timeStamp %ld ns.",
        alignedWidth, alignedHeight, timeStamp);
    CopyDecodedImage(surfaceBuffer, alignedWidth, alignedHeight);
    surface->ReleaseBuffer(surfaceBuffer, -1);
    outputTimeStampUs_ = timeStamp;
    ReduceWaitDecodeCnt();
}

void DecodeDataProcess::CopyDecodedImage(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth,
    int32_t alignedHeight)
{
    if (!IsCorrectSurfaceBuffer(surBuf, alignedWidth, alignedHeight)) {
        DHLOGE("Surface output buffer error.");
        return;
    }

    DHLOGD("Convert NV12 to I420, format=%d, width=[%d, %d], height=[%d, %d]", sourceConfig_.GetVideoformat(),
        sourceConfig_.GetWidth(), alignedWidth, sourceConfig_.GetHeight(), alignedHeight);
    int srcSizeY = alignedWidth * alignedHeight;
    uint8_t *srcDataY = static_cast<uint8_t *>(surBuf->GetVirAddr());
    uint8_t *srcDataUV = static_cast<uint8_t *>(surBuf->GetVirAddr()) + srcSizeY;

    int dstSizeY = sourceConfig_.GetWidth() * sourceConfig_.GetHeight();
    int dstSizeUV = (sourceConfig_.GetWidth() >> 1) * (sourceConfig_.GetHeight() >> 1);
    std::shared_ptr<DataBuffer> bufferOutput =
        std::make_shared<DataBuffer>(dstSizeY * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    uint8_t *dstDataY = bufferOutput->Data();
    uint8_t *dstDataU = bufferOutput->Data() + dstSizeY;
    uint8_t *dstDataV = bufferOutput->Data() + dstSizeY + dstSizeUV;

    int32_t ret = libyuv::NV12ToI420(
        srcDataY, alignedWidth,
        srcDataUV, alignedWidth,
        dstDataY, sourceConfig_.GetWidth(),
        dstDataU, sourceConfig_.GetWidth() >> 1,
        dstDataV, sourceConfig_.GetWidth() >> 1,
        processedConfig_.GetWidth(), processedConfig_.GetHeight());
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert NV12 to I420 failed.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        bufferOutput->frameInfo_ = frameInfoDeque_.front();
        frameInfoDeque_.pop_front();
    }
    bufferOutput->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::YUVI420));
    bufferOutput->SetInt32("alignedWidth", processedConfig_.GetWidth());
    bufferOutput->SetInt32("alignedHeight", processedConfig_.GetHeight());
    bufferOutput->SetInt32("width", processedConfig_.GetWidth());
    bufferOutput->SetInt32("height", processedConfig_.GetHeight());

    PostOutputDataBuffers(bufferOutput);
}

bool DecodeDataProcess::IsCorrectSurfaceBuffer(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth,
    int32_t alignedHeight)
{
    if (surBuf == nullptr) {
        DHLOGE("surface buffer is null!");
        return false;
    }

    size_t yuvImageAlignedSize = static_cast<size_t>(alignedWidth * alignedHeight *
                                                              YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t yuvImageSize = static_cast<size_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight() *
                                                       YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t surfaceBufSize = static_cast<size_t>(surBuf->GetSize());
    if (yuvImageAlignedSize > surfaceBufSize || yuvImageAlignedSize < yuvImageSize) {
        DHLOGE("Buffer size error, yuvImageSize %zu, yuvImageAlignedSize %zu, surBufSize %zu.",
            yuvImageSize, yuvImageAlignedSize, surBuf->GetSize());
        return false;
    }
    return true;
}

void DecodeDataProcess::PostOutputDataBuffers(std::shared_ptr<DataBuffer>& outputBuffer)
{
    if (eventBusDecode_ == nullptr || outputBuffer == nullptr) {
        DHLOGE("eventBusDecode_ or outputBuffer is null.");
        return;
    }
    std::vector<std::shared_ptr<DataBuffer>> multiDataBuffers;
    multiDataBuffers.push_back(outputBuffer);
    std::shared_ptr<CodecPacket> transNextNodePacket = std::make_shared<CodecPacket>(VideoCodecType::NO_CODEC,
        multiDataBuffers);
    DCameraCodecEvent dCamCodecEv(*this, transNextNodePacket, VideoCodecAction::NO_ACTION);
    eventBusDecode_->PostEvent<DCameraCodecEvent>(dCamCodecEv, POSTMODE::POST_ASYNC);
    DHLOGD("Send video decoder output asynchronous DCameraCodecEvents success.");
}

int32_t DecodeDataProcess::DecodeDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    DHLOGD("Decoder Done.");
    if (outputBuffers.empty()) {
        DHLOGE("The received data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (nextDataProcess_ != nullptr) {
        DHLOGD("Send to the next node of the decoder for processing.");
        int32_t err = nextDataProcess_->ProcessData(outputBuffers);
        if (err != DCAMERA_OK) {
            DHLOGE("Someone node after the decoder processes failed.");
        }
        return err;
    }
    DHLOGD("The current node is the last node, and Output the processed video buffer");
    std::shared_ptr<DCameraPipelineSource> targetPipelineSource = callbackPipelineSource_.lock();
    if (targetPipelineSource == nullptr) {
        DHLOGE("callbackPipelineSource_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    targetPipelineSource->OnProcessedVideoBuffer(outputBuffers[0]);
    return DCAMERA_OK;
}

void DecodeDataProcess::OnEvent(DCameraCodecEvent& ev)
{
    DHLOGD("Receiving asynchronous DCameraCodecEvents.");
    std::shared_ptr<CodecPacket> receivedCodecPacket = ev.GetCodecPacket();
    VideoCodecAction action = ev.GetAction();
    switch (action) {
        case VideoCodecAction::NO_ACTION: {
            if (receivedCodecPacket == nullptr) {
                DHLOGE("the received codecPacket of action [%d] is null.", action);
                OnError();
                return;
            }
            std::vector<std::shared_ptr<DataBuffer>> dataBuffers = receivedCodecPacket->GetDataBuffers();
            DecodeDone(dataBuffers);
            break;
        }
        case VideoCodecAction::ACTION_ONCE_AGAIN:
            DHLOGD("Try FeedDecoderInputBuffer again.");
            FeedDecoderInputBuffer();
            return;
        case VideoCodecAction::ACTION_GET_DECODER_OUTPUT_BUFFER:
            if (receivedCodecPacket == nullptr) {
                DHLOGE("the received codecPacket of action [%d] is null.", action);
                OnError();
                return;
            }
            GetDecoderOutputBuffer(receivedCodecPacket->GetSurface());
            break;
        default:
            DHLOGD("The action : %d is not supported.", action);
            return;
    }
}

void DecodeDataProcess::OnError()
{
    DHLOGD("DecodeDataProcess : OnError.");
    isDecoderProcess_.store(false);
    if (videoDecoder_ != nullptr) {
        videoDecoder_->Stop();
    }
    std::shared_ptr<DCameraPipelineSource> targetPipelineSource = callbackPipelineSource_.lock();
    if (targetPipelineSource == nullptr) {
        DHLOGE("callbackPipelineSource_ is nullptr.");
        return;
    }
    targetPipelineSource->OnError(DataProcessErrorType::ERROR_PIPELINE_DECODER);
}

void DecodeDataProcess::OnInputBufferAvailable(uint32_t index)
{
    DHLOGD("DecodeDataProcess::OnInputBufferAvailable");
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (availableInputIndexsQueue_.size() > VIDEO_DECODER_QUEUE_MAX) {
        DHLOGE("Video decoder available indexs queue overflow.");
        return;
    }
    DHLOGD("Video decoder available indexs queue push index [%u].", index);
    availableInputIndexsQueue_.push(index);
}

void DecodeDataProcess::OnOutputFormatChanged(const Media::Format &format)
{
    if (decodeOutputFormat_.GetFormatMap().empty()) {
        DHLOGE("The first changed video decoder output format is null.");
        return;
    }
    decodeOutputFormat_ = format;
}

void DecodeDataProcess::OnOutputBufferAvailable(uint32_t index, const Media::AVCodecBufferInfo& info,
    const Media::AVCodecBufferFlag& flag)
{
    int64_t finishDecodeT = GetNowTimeStampUs();
    if (!isDecoderProcess_.load()) {
        DHLOGE("Decoder node occurred error or start release.");
        return;
    }
    DHLOGD("Video decode buffer info: presentation TimeUs %lld, size %d, offset %d, flag %d",
        info.presentationTimeUs, info.size, info.offset, flag);
    outputInfo_ = info;
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        AlignFirstFrameTime();
        for (auto it = frameInfoDeque_.begin(); it != frameInfoDeque_.end(); it++) {
            DCameraFrameInfo frameInfo = *it;
            if (frameInfo.timePonit.finishDecode != 0) {
                continue;
            }
            frameInfo.timePonit.finishDecode = finishDecodeT;
            frameInfoDeque_.emplace(frameInfoDeque_.erase(it), frameInfo);
            break;
        }
    }
    {
        std::lock_guard<std::mutex> outputLock(mtxDecoderState_);
        if (videoDecoder_ == nullptr) {
            DHLOGE("The video decoder does not exist before decoding data.");
            return;
        }
        int32_t errRelease = videoDecoder_->ReleaseOutputBuffer(index, true);
        if (errRelease != Media::MediaServiceErrCode::MSERR_OK) {
            DHLOGE("The video decoder output decoded data to surface failed, index : [%u].", index);
        }
    }
}

VideoConfigParams DecodeDataProcess::GetSourceConfig() const
{
    return sourceConfig_;
}

VideoConfigParams DecodeDataProcess::GetTargetConfig() const
{
    return targetConfig_;
}

int32_t DecodeDataProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    return DCAMERA_OK;
}

void DecodeDataProcess::AlignFirstFrameTime()
{
    if (frameInfoDeque_.empty()) {
        return;
    }
    DCameraFrameInfo frameInfo = frameInfoDeque_.front();
    if (frameInfo.index != FRAME_HEAD || frameInfo.type != Media::AVCODEC_BUFFER_FLAG_CODEC_DATA) {
        return;
    }
    frameInfoDeque_.pop_front();
    DCameraFrameInfo front = frameInfoDeque_.front();
    frameInfo.index = front.index;
    frameInfo.pts = front.pts;
    frameInfo.offset = front.offset;
    frameInfo.type = front.type;
    frameInfo.ver = front.ver;
    frameInfo.timePonit.finishEncode = front.timePonit.finishEncode;
    frameInfoDeque_.emplace(frameInfoDeque_.erase(frameInfoDeque_.begin()), frameInfo);
}
} // namespace DistributedHardware
} // namespace OHOS
