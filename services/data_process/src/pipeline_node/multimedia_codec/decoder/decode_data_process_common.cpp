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

#include "decode_data_process.h"

#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_hidumper.h"
#include "decode_surface_listener.h"
#include "decode_video_callback.h"
#include "graphic_common_c.h"
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
const std::string ENUM_VIDEOFORMAT_STRINGS[] = {
    "YUVI420", "NV12", "NV21", "RGBA_8888"
};

DecodeDataProcess::~DecodeDataProcess()
{
    DumpFileUtil::CloseDumpFile(&dumpDecBeforeFile_);
    DumpFileUtil::CloseDumpFile(&dumpDecAfterFile_);
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
        DHLOGE("The DecodeNode can't convert %{public}d to %{public}d.", sourceConfig.GetVideoCodecType(),
            targetConfig_.GetVideoCodecType());
        return DCAMERA_BAD_TYPE;
    }

    sourceConfig_ = sourceConfig;
    targetConfig_ = targetConfig;
    if (sourceConfig_.GetVideoCodecType() == targetConfig_.GetVideoCodecType()) {
        DHLOGD("Disable DecodeNode. The target video codec type %{public}d is the same as the source video codec "
            "type %{public}d.", targetConfig_.GetVideoCodecType(), sourceConfig_.GetVideoCodecType());
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
    bool isWidthValid = (curConfig.GetWidth() >= MIN_VIDEO_WIDTH && curConfig.GetWidth() <= MAX_VIDEO_WIDTH);
    bool isHeightValid = (curConfig.GetHeight() >= MIN_VIDEO_HEIGHT && curConfig.GetHeight() <= MAX_VIDEO_HEIGHT);
    bool isFrameRateValid = (curConfig.GetFrameRate() >= MIN_FRAME_RATE && curConfig.GetFrameRate() <= MAX_FRAME_RATE);
    return isWidthValid && isHeightValid && isFrameRateValid;
}

bool DecodeDataProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return (sourceConfig.GetVideoCodecType() == targetConfig.GetVideoCodecType() ||
        targetConfig.GetVideoCodecType() == VideoCodecType::NO_CODEC);
}

void DecodeDataProcess::InitCodecEvent()
{
    DHLOGD("Init DecodeNode eventBus, and add handler for it.");
    eventThread_ = std::thread([this]() { this->StartEventHandler(); });
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return decEventHandler_ != nullptr;
    });
}

void DecodeDataProcess::StartEventHandler()
{
    prctl(PR_SET_NAME, DECODE_DATA_EVENT.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    if (runner == nullptr) {
        DHLOGE("Creat runner failed.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        decEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
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
            CreateMsg("start video decoder failed, width: %d, height: %d, format: %s",
            sourceConfig_.GetWidth(), sourceConfig_.GetHeight(),
            ENUM_VIDEOFORMAT_STRINGS[static_cast<int32_t>(sourceConfig_.GetVideoformat())].c_str()));
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::ConfigureVideoDecoder()
{
    int32_t ret = InitDecoderMetadataFormat();
    if (ret != DCAMERA_OK) {
        DHLOGE("Init video decoder metadata format failed. ret %{public}d.", ret);
        return ret;
    }

    videoDecoder_ = MediaAVCodec::VideoDecoderFactory::CreateByMime(processType_);
    if (videoDecoder_ == nullptr) {
        DHLOGE("Create video decoder failed.");
        return DCAMERA_INIT_ERR;
    }
    decodeVideoCallback_ = std::make_shared<DecodeVideoCallback>(shared_from_this());
    ret = videoDecoder_->SetCallback(decodeVideoCallback_);
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("Set video decoder callback failed. ret %{public}d.", ret);
        return DCAMERA_INIT_ERR;
    }

    ret = videoDecoder_->Configure(metadataFormat_);
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("Set video decoder metadata format failed. ret %{public}d.", ret);
        return DCAMERA_INIT_ERR;
    }

    ret = SetDecoderOutputSurface();
    if (ret != DCAMERA_OK) {
        DHLOGE("Set decoder outputsurface failed. ret %{public}d.", ret);
        return ret;
    }

    return DCAMERA_OK;
}

int32_t DecodeDataProcess::InitDecoderMetadataFormat()
{
    DHLOGI("Init video decoder metadata format. codecType: %{public}d", sourceConfig_.GetVideoCodecType());
    processedConfig_ = sourceConfig_;
    processedConfig_.SetVideoCodecType(VideoCodecType::NO_CODEC);
    switch (sourceConfig_.GetVideoCodecType()) {
        case VideoCodecType::CODEC_H264:
            processType_ = "video/avc";
            break;
        case VideoCodecType::CODEC_H265:
            processType_ = "video/hevc";
            break;
        default:
            DHLOGE("The current codec type does not support decoding.");
            return DCAMERA_NOT_FOUND;
    }

    DHLOGI("Init video decoder metadata format. videoformat: %{public}d", processedConfig_.GetVideoformat());
    switch (processedConfig_.GetVideoformat()) {
        case Videoformat::YUVI420:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::YUVI420));
            metadataFormat_.PutIntValue("max_input_size", MAX_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::NV12:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::NV12));
            metadataFormat_.PutIntValue("max_input_size", MAX_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::NV21:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::NV21));
            metadataFormat_.PutIntValue("max_input_size", MAX_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::RGBA_8888:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::RGBA));
            metadataFormat_.PutIntValue("max_input_size", MAX_RGB32_BUFFER_SIZE);
            break;
        default:
            DHLOGE("The current pixel format does not support encoding.");
            return DCAMERA_NOT_FOUND;
    }

    metadataFormat_.PutStringValue("codec_mime", processType_);
    metadataFormat_.PutIntValue("width", sourceConfig_.GetWidth());
    metadataFormat_.PutIntValue("height", sourceConfig_.GetHeight());
    metadataFormat_.PutDoubleValue("frame_rate", MAX_FRAME_RATE);

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
    GSError ret = decodeConsumerSurface_->SetDefaultUsage(SurfaceBufferUsage::BUFFER_USAGE_MEM_MMZ_CACHE |
        SurfaceBufferUsage::BUFFER_USAGE_CPU_READ);
    if (ret != GSERROR_OK || decodeConsumerSurface_ == nullptr) {
        DHLOGE("Set Usage failed.");
    }

    decodeSurfaceListener_ =
        OHOS::sptr<IBufferConsumerListener>(new DecodeSurfaceListener(decodeConsumerSurface_, shared_from_this()));
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
    if (err != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
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
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("Video decoder prepare failed. ret %{public}d.", ret);
        return DCAMERA_INIT_ERR;
    }
    ret = videoDecoder_->Start();
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("Video decoder start failed. ret %{public}d.", ret);
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
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("VideoDecoder flush failed. ret %{public}d.", ret);
        isSuccess = isSuccess && false;
    }
    ret = videoDecoder_->Stop();
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("VideoDecoder stop failed. ret %{public}d.", ret);
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
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("VideoDecoder release failed. ret %{public}d.", ret);
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
        DHLOGE("Unregister consumer listener failed. ret %{public}d.", ret);
    }
    decodeConsumerSurface_ = nullptr;
    decodeProducerSurface_ = nullptr;
}

void DecodeDataProcess::ReleaseCodecEvent()
{
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        if ((decEventHandler_ != nullptr) && (decEventHandler_->GetEventRunner() != nullptr)) {
            decEventHandler_->GetEventRunner()->Stop();
            eventThread_.join();
        }
        decEventHandler_ = nullptr;
    }
    pipeSrcEventHandler_ = nullptr;
    DHLOGD("Release DecodeNode eventBusDecode and eventBusPipeline end.");
}

void DecodeDataProcess::ReleaseProcessNode()
{
    DHLOGD("Start release [%{public}zu] node : DecodeNode.", nodeRank_);
    isDecoderProcess_.store(false);
    ReleaseVideoDecoder();
    ReleaseDecoderSurface();
    ReleaseCodecEvent();

    processType_ = "";
    std::queue<std::shared_ptr<DataBuffer>>().swap(inputBuffersQueue_);
    std::queue<uint32_t>().swap(availableInputIndexsQueue_);
    std::queue<std::shared_ptr<Media::AVSharedMemory>>().swap(availableInputBufferQueue_);
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        std::deque<DCameraFrameInfo>().swap(frameInfoDeque_);
    }
    waitDecoderOutputCount_ = 0;
    lastFeedDecoderInputBufferTimeUs_ = 0;
    outputTimeStampUs_ = 0;
    alignedHeight_ = 0;

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGD("Release [%{public}zu] node : DecodeNode end.", nodeRank_);
}

int32_t DecodeDataProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    DHLOGD("Process data in DecodeDataProcess.");
    if (inputBuffers.empty() || inputBuffers[0] == nullptr) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_DCAMERA_BEFORE_DEC_FILENAME, &dumpDecBeforeFile_);
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_DCAMERA_AFTER_DEC_FILENAME, &dumpDecAfterFile_);
    if (sourceConfig_.GetVideoCodecType() == processedConfig_.GetVideoCodecType()) {
        DHLOGD("The target VideoCodecType : %{public}d is the same as the source VideoCodecType : %{public}d.",
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
    if (inputBuffers[0]->Size() > MAX_BUFFER_SIZE) {
        DHLOGE("DecodeNode input buffer size %{public}zu error.", inputBuffers[0]->Size());
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    if (!isDecoderProcess_.load()) {
        DHLOGE("Decoder node occurred error or start release.");
        return DCAMERA_DISABLE_PROCESS;
    }
    inputBuffersQueue_.push(inputBuffers[0]);
    DHLOGD("Push inputBuf sucess. BufSize %{public}zu, QueueSize %{public}zu.", inputBuffers[0]->Size(),
        inputBuffersQueue_.size());
    int32_t err = FeedDecoderInputBuffer();
    if (err != DCAMERA_OK) {
        int32_t sleepTimeUs = 5000;
        std::this_thread::sleep_for(std::chrono::microseconds(sleepTimeUs));
        DHLOGD("Feed decoder input buffer failed. Try FeedDecoderInputBuffer again.");
        auto sendFunc = [this]() mutable {
            int32_t ret = FeedDecoderInputBuffer();
            DHLOGD("excute FeedDecoderInputBuffer ret %{public}d.", ret);
        };
        CHECK_AND_RETURN_RET_LOG(pipeSrcEventHandler_ == nullptr, DCAMERA_BAD_VALUE,
            "%{public}s", "pipeSrcEventHandler_ is nullptr.");
        pipeSrcEventHandler_->PostTask(sendFunc);
    }
    return DCAMERA_OK;
}

void DecodeDataProcess::BeforeDecodeDump(uint8_t *buffer, size_t bufSize)
{
#ifdef DUMP_DCAMERA_FILE
    if (buffer == nullptr) {
        DHLOGE("dumpsaving : input param nullptr.");
        return;
    }
    if (DcameraHidumper::GetInstance().GetDumpFlag() && (IsUnderDumpMaxSize(DUMP_PATH, BEFORE_DECODE) == DCAMERA_OK)) {
        DumpBufferToFile(DUMP_PATH, BEFORE_DECODE, buffer, bufSize);
    }
#endif
    return;
}

int32_t DecodeDataProcess::FeedDecoderInputBuffer()
{
    DHLOGD("Feed decoder input buffer.");
    while ((!inputBuffersQueue_.empty()) && (isDecoderProcess_.load())) {
        std::shared_ptr<DataBuffer> buffer = inputBuffersQueue_.front();
        if (buffer == nullptr || availableInputIndexsQueue_.empty() || availableInputBufferQueue_.empty()) {
            DHLOGE("inputBuffersQueue size %{public}zu, availableInputIndexsQueue size %{public}zu, "
                "availableInputBufferQueue size %{public}zu",
                inputBuffersQueue_.size(), availableInputIndexsQueue_.size(), availableInputBufferQueue_.size());
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
            CHECK_AND_RETURN_RET_LOG(
                videoDecoder_ == nullptr, DCAMERA_OK, "The video decoder does not exist before GetInputBuffer.");
            uint32_t index = availableInputIndexsQueue_.front();
            std::shared_ptr<Media::AVSharedMemory> sharedMemoryInput = availableInputBufferQueue_.front();
            if (sharedMemoryInput == nullptr) {
                DHLOGE("Failed to obtain the input shared memory corresponding to the [%{public}u] index.", index);
                return DCAMERA_BAD_VALUE;
            }
            BeforeDecodeDump(buffer->Data(), buffer->Size());
            DumpFileUtil::WriteDumpFile(dumpDecBeforeFile_, static_cast<void *>(buffer->Data()), buffer->Size());
            size_t inputMemoDataSize = static_cast<size_t>(sharedMemoryInput->GetSize());
            errno_t err = memcpy_s(sharedMemoryInput->GetBase(), inputMemoDataSize, buffer->Data(), buffer->Size());
            CHECK_AND_RETURN_RET_LOG(err != EOK, DCAMERA_MEMORY_OPT_ERROR, "memcpy_s buffer failed.");
            DHLOGD("Decoder input buffer size %{public}zu, timeStamp %{public}" PRId64"us.", buffer->Size(), timeStamp);
            MediaAVCodec::AVCodecBufferInfo bufferInfo {timeStamp, static_cast<int32_t>(buffer->Size()), 0};
            int32_t ret = videoDecoder_->QueueInputBuffer(index, bufferInfo,
                MediaAVCodec::AVCODEC_BUFFER_FLAG_NONE);
            if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
                DHLOGE("queue Input buffer failed.");
                return DCAMERA_BAD_OPERATE;
            }
        }

        inputBuffersQueue_.pop();
        DHLOGD("Push inputBuffer sucess. inputBuffersQueue size is %{public}zu.", inputBuffersQueue_.size());

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
    availableInputBufferQueue_.pop();
    waitDecoderOutputCount_++;
    DHLOGD("Wait decoder output frames number is %{public}d.", waitDecoderOutputCount_);
}

void DecodeDataProcess::ReduceWaitDecodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (waitDecoderOutputCount_ <= 0) {
        DHLOGE("The waitDecoderOutputCount_ = %{public}d.", waitDecoderOutputCount_);
    }
    if (outputTimeStampUs_ == 0) {
        waitDecoderOutputCount_ -= FIRST_FRAME_INPUT_NUM;
    } else {
        waitDecoderOutputCount_--;
    }
    DHLOGD("Wait decoder output frames number is %{public}d.", waitDecoderOutputCount_);
}

void DecodeDataProcess::OnSurfaceOutputBufferAvailable(const sptr<IConsumerSurface>& surface)
{
    auto sendFunc = [this, surface]() mutable {
        GetDecoderOutputBuffer(surface);
        DHLOGD("excute GetDecoderOutputBuffer.");
    };
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (decEventHandler_ != nullptr) {
        decEventHandler_->PostTask(sendFunc);
    }
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
    DHLOGD("OutputBuffer alignedWidth %{public}d, alignedHeight %{public}d, timeStamp %{public}" PRId64" ns.",
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

    size_t imageSize = 0;
    if (processedConfig_.GetVideoformat() == Videoformat::RGBA_8888) {
        imageSize = static_cast<size_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight() *
            RGB32_MEMORY_COEFFICIENT);
    } else {
        imageSize = static_cast<size_t>(
            sourceConfig_.GetWidth() * sourceConfig_.GetHeight() * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    }
    std::shared_ptr<DataBuffer> bufferOutput = std::make_shared<DataBuffer>(imageSize);
    uint8_t *addr = static_cast<uint8_t *>(surBuf->GetVirAddr());
    errno_t err = memcpy_s(bufferOutput->Data(), bufferOutput->Size(), addr, imageSize);
    if (err != EOK) {
        DHLOGE("memcpy_s surface buffer failed.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        bufferOutput->frameInfo_ = frameInfoDeque_.front();
        frameInfoDeque_.pop_front();
    }
    bufferOutput->SetInt32("Videoformat", static_cast<int32_t>(processedConfig_.GetVideoformat()));
    bufferOutput->SetInt32("alignedWidth", processedConfig_.GetWidth());
    bufferOutput->SetInt32("alignedHeight", processedConfig_.GetHeight());
    bufferOutput->SetInt32("width", processedConfig_.GetWidth());
    bufferOutput->SetInt32("height", processedConfig_.GetHeight());
#ifdef DUMP_DCAMERA_FILE
    std::string fileName = "SourceAfterDecode_width(" + std::to_string(processedConfig_.GetWidth())
        + ")height(" + std::to_string(processedConfig_.GetHeight()) + ").yuv";
    if (DcameraHidumper::GetInstance().GetDumpFlag() && (IsUnderDumpMaxSize(DUMP_PATH, fileName) == DCAMERA_OK)) {
        DumpBufferToFile(DUMP_PATH, fileName, bufferOutput->Data(), bufferOutput->Size());
    }
#endif
    DumpFileUtil::WriteDumpFile(dumpDecAfterFile_, static_cast<void *>(bufferOutput->Data()), bufferOutput->Size());
    PostOutputDataBuffers(bufferOutput);
}

bool DecodeDataProcess::IsCorrectSurfaceBuffer(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth,
    int32_t alignedHeight)
{
    if (surBuf == nullptr) {
        DHLOGE("surface buffer is null!");
        return false;
    }

    if (processedConfig_.GetVideoformat() == Videoformat::RGBA_8888) {
        size_t rgbImageSize = static_cast<size_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight() *
            RGB32_MEMORY_COEFFICIENT);
        size_t surfaceBufSize = static_cast<size_t>(surBuf->GetSize());
        if (rgbImageSize > surfaceBufSize) {
            DHLOGE("Buffer size error, rgbImageSize %{public}zu, surBufSize %{public}" PRIu32, rgbImageSize,
                surBuf->GetSize());
            return false;
        }
    } else {
        size_t surfaceBufSize = static_cast<size_t>(surBuf->GetSize());
        size_t yuvImageAlignedSize = static_cast<size_t>(
            alignedWidth * alignedHeight * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
        size_t yuvImageSize = static_cast<size_t>(
            sourceConfig_.GetWidth() * sourceConfig_.GetHeight() * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
        if (yuvImageAlignedSize > surfaceBufSize || yuvImageAlignedSize < yuvImageSize) {
            DHLOGE("Buffer size error, yuvImageSize %{public}zu, yuvImageAlignedSize %{public}zu, surBufSize "
                "%{public}" PRIu32, yuvImageSize, yuvImageAlignedSize, surBuf->GetSize());
            return false;
        }
    }
    return true;
}

void DecodeDataProcess::PostOutputDataBuffers(std::shared_ptr<DataBuffer>& outputBuffer)
{
    if (decEventHandler_ == nullptr || outputBuffer == nullptr) {
        DHLOGE("decEventHandler_ or outputBuffer is null.");
        return;
    }
    auto sendFunc = [this, outputBuffer]() mutable {
        std::vector<std::shared_ptr<DataBuffer>> multiDataBuffers;
        multiDataBuffers.push_back(outputBuffer);
        int32_t ret = DecodeDone(multiDataBuffers);
        DHLOGD("excute DecodeDone ret %{public}d.", ret);
    };
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (decEventHandler_ != nullptr) {
        decEventHandler_->PostTask(sendFunc);
    }
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

void DecodeDataProcess::OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("DecodeDataProcess::OnInputBufferAvailable");
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (availableInputIndexsQueue_.size() > VIDEO_DECODER_QUEUE_MAX) {
        DHLOGE("Video decoder available indexs queue overflow.");
        return;
    }
    DHLOGD("Video decoder available indexs queue push index [%{public}u].", index);
    availableInputIndexsQueue_.push(index);
    availableInputBufferQueue_.push(buffer);
}

void DecodeDataProcess::OnOutputFormatChanged(const Media::Format &format)
{
    if (decodeOutputFormat_.GetFormatMap().empty()) {
        DHLOGE("The first changed video decoder output format is null.");
        return;
    }
    decodeOutputFormat_ = format;
}

void DecodeDataProcess::OnOutputBufferAvailable(uint32_t index, const MediaAVCodec::AVCodecBufferInfo& info,
    const MediaAVCodec::AVCodecBufferFlag& flag, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    int64_t finishDecodeT = GetNowTimeStampUs();
    if (!isDecoderProcess_.load()) {
        DHLOGE("Decoder node occurred error or start release.");
        return;
    }
    DHLOGD("Video decode buffer info: presentation TimeUs %{public}" PRId64", size %{public}d, offset %{public}d, flag "
        "%{public}" PRIu32, info.presentationTimeUs, info.size, info.offset, flag);
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
        if (errRelease != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
            DHLOGE("The video decoder output decoded data to surfacebuffer failed, index : [%{public}u].", index);
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
    if (frameInfoDeque_.size() < FIRST_FRAME_INPUT_NUM) {
        return;
    }
    DCameraFrameInfo frameInfo = frameInfoDeque_.front();
    if (frameInfo.index != FRAME_HEAD || frameInfo.type != MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA) {
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
