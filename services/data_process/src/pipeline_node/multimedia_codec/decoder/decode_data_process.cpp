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
#include "dcamera_radar.h"
#include "decode_surface_listener.h"
#include "decode_video_callback.h"
#include "graphic_common_c.h"
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
const std::string ENUM_VIDEOFORMAT_STRINGS[] = {
    "YUVI420", "NV12", "NV21", "RGBA_8888"
};

const static int32_t ROTATION_0 = 0;
const static int32_t ROTATION_90 = 90;
const static int32_t ROTATION_180 = 180;
const static int32_t ROTATION_270 = 270;
const static int32_t ROTATION_360 = 360;

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
    DcameraRadar::GetInstance().ReportDcameraOpenProgress("InitDecoder", CameraOpen::INIT_DECODE, err);
    if (err != DCAMERA_OK) {
        DHLOGE("Init video decoder failed.");
        ReleaseProcessNode();
        return err;
    }
    alignedHeight_ = GetAlignedHeight(sourceConfig_.GetHeight());
    processedConfig = processedConfig_;
    if (targetConfig_.GetIsSystemSwitch()) {
        int32_t rotate = targetConfig_.GetRotation();
        DHLOGI("DecodeDataProcess::InitNode current stream is system switch, orientation: %{public}d", rotate);
        if (rotate > 0) {
            rotate_ = ROTATION_360 - rotate;
        }
    }
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
        auto videoFormat = sourceConfig_.GetVideoformat();
        auto formatIndex = static_cast<int32_t>(videoFormat);
        if (formatIndex >= 0 && static_cast<size_t>(formatIndex) < std::size(ENUM_VIDEOFORMAT_STRINGS)) {
            ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_DECODE_ERROR,
                CreateMsg("start video decoder failed, width: %d, height: %d, format: %s",
                sourceConfig_.GetWidth(), sourceConfig_.GetHeight(),
                ENUM_VIDEOFORMAT_STRINGS[formatIndex].c_str()));
        } else {
            DHLOGE("Invalid video format received from source: %d", formatIndex);
            ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_DECODE_ERROR,
                CreateMsg("start video decoder failed with INVALID format index %d, width: %d, height: %d",
                formatIndex, sourceConfig_.GetWidth(), sourceConfig_.GetHeight()));
        }
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
    CHECK_AND_RETURN_RET_LOG(videoDecoder_ == nullptr, DCAMERA_INIT_ERR, "%{public}s",
        "Create video decoder failed.");
    decodeVideoCallback_ = std::make_shared<DecodeVideoCallback>(shared_from_this());
    ret = videoDecoder_->SetCallback(decodeVideoCallback_);
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Set video decoder callback failed.  ret %{public}d.", ret);
    ret = videoDecoder_->Configure(metadataFormat_);
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Set video decoder metadata format failed. ret %{public}d.", ret);
    ret = SetDecoderOutputSurface();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
        "Set decoder output surface failed. ret %d.", ret);
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::InitDecoderMetadataFormat()
{
    DHLOGI("Init video decoder metadata format. codecType: %{public}d", sourceConfig_.GetVideoCodecType());
    processedConfig_ = sourceConfig_;
    processedConfig_.SetVideoCodecType(VideoCodecType::NO_CODEC);
    processedConfig_.SetVideoformat(Videoformat::YUVI420);
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

    metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::NV12));
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
    CHECK_AND_LOG(ret != GSERROR_OK || decodeConsumerSurface_ == nullptr, "%{public}s", "Set Usage failed.");

    decodeSurfaceListener_ =
        OHOS::sptr<IBufferConsumerListener>(new DecodeSurfaceListener(decodeConsumerSurface_, shared_from_this()));
    if (decodeConsumerSurface_->RegisterConsumerListener(decodeSurfaceListener_) !=
        SURFACE_ERROR_OK) {
        DHLOGE("Register consumer listener failed.");
        return DCAMERA_INIT_ERR;
    }

    sptr<IBufferProducer> surfaceProducer = decodeConsumerSurface_->GetProducer();
    CHECK_AND_RETURN_RET_LOG(surfaceProducer == nullptr, DCAMERA_INIT_ERR, "%{public}s",
        "Get the surface producer of the decode consumer surface failed.");
    decodeProducerSurface_ = Surface::CreateSurfaceAsProducer(surfaceProducer);
    if (decodeProducerSurface_ == nullptr) {
        DHLOGE("Create the decode producer surface of the decode consumer surface failed.");
        return DCAMERA_INIT_ERR;
    }

    DHLOGD("Set the producer surface to video decoder output surface.");
    int32_t err = videoDecoder_->SetOutputSurface(decodeProducerSurface_);
    CHECK_AND_RETURN_RET_LOG(err != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR, "%{public}s",
        "Set decoder output surface failed.");
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::StartVideoDecoder()
{
    if (videoDecoder_ == nullptr) {
        DHLOGE("The video decoder does not exist before StartVideoDecoder.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ret = videoDecoder_->Prepare();
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Video decoder prepare failed. ret %{public}d.", ret);
    ret = videoDecoder_->Start();
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Video decoder start failed. ret %{public}d.", ret);
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
    CHECK_AND_LOG(ret != DCAMERA_OK, "%{public}s", "StopVideoDecoder failed.");
    ret = videoDecoder_->Release();
    CHECK_AND_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK,
        "VideoDecoder release failed. ret %{public}d.", ret);
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
    CHECK_AND_LOG(ret != SURFACE_ERROR_OK, "VideoDecoder release failed. ret %d.", ret);
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
    if (inputBuffers[0]->Size() > MAX_YUV420_BUFFER_SIZE) {
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
        int32_t ret = ProcessSingleInputBuffer();
        if (ret != DCAMERA_OK) {
            DHLOGD("ProcessSingleInputBuffer failed with %{public}d, stopping feed.", ret);
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::ProcessSingleInputBuffer()
{
    std::shared_ptr<DataBuffer> buffer = inputBuffersQueue_.front();
    if (buffer == nullptr) {
        DHLOGE("Input buffer is null, skipping this frame.");
        inputBuffersQueue_.pop();
        return DCAMERA_OK;
    }
    uint32_t index;
    std::shared_ptr<Media::AVSharedMemory> sharedMemoryInput;
    int32_t ret = GetAvailableDecoderBuffer(index, sharedMemoryInput);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
        "Get available decoder buffer failed. ret %{public}d.", ret);
    buffer->frameInfo_.timePonit.startDecode = GetNowTimeStampUs();
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        frameInfoDeque_.push_back(buffer->frameInfo_);
    }
    ret = QueueBufferToDecoder(buffer, index, sharedMemoryInput);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
        "Queue buffer to decoder failed. ret %{public}d.", ret);
    inputBuffersQueue_.pop();
    DHLOGD("Push inputBuffer sucess. inputBuffersQueue size is %{public}zu.", inputBuffersQueue_.size());

    IncreaseWaitDecodeCnt();
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::GetAvailableDecoderBuffer(uint32_t& index,
    std::shared_ptr<Media::AVSharedMemory>& sharedMemoryInput)
{
    std::lock_guard<std::mutex> lock(mtxHoldCount_);
    if (availableInputIndexsQueue_.empty() || availableInputBufferQueue_.empty()) {
        DHLOGD("No available decoder buffers, wait for callback.");
        return DCAMERA_BAD_VALUE;
    }
    index = availableInputIndexsQueue_.front();
    sharedMemoryInput = availableInputBufferQueue_.front();
    return DCAMERA_OK;
}

int32_t DecodeDataProcess::QueueBufferToDecoder(std::shared_ptr<DataBuffer>& buffer, uint32_t index,
    std::shared_ptr<Media::AVSharedMemory>& sharedMemoryInput)
{
    int64_t timeStamp = buffer->frameInfo_.pts;
    std::lock_guard<std::mutex> inputLock(mtxDecoderLock_);

    CHECK_AND_RETURN_RET_LOG(
        videoDecoder_ == nullptr, DCAMERA_OK, "The video decoder does not exist before GetInputBuffer.");

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
    int32_t ret = videoDecoder_->QueueInputBuffer(index, bufferInfo, MediaAVCodec::AVCODEC_BUFFER_FLAG_NONE);
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, ret,
        "Queue input buffer to decoder failed. ret %{public}d.", ret);
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
    ret = surfaceBuffer->InvalidateCache();
    CHECK_AND_LOG(ret != GSERROR_OK, "Invalidate cache failed.");
    int32_t alignedWidth = surfaceBuffer->GetStride();
    if (surfaceBuffer->GetSize() > BUFFER_MAX_SIZE || alignedWidth > ALIGNED_WIDTH_MAX_SIZE) {
        DHLOGE("surface buffer size or alignedWidth too long");
        return;
    }
    int32_t alignedHeight = alignedHeight_;
    DHLOGD("OutputBuffer alignedWidth %{public}d, alignedHeight %{public}d, timeStamp %{public}ld ns.",
        alignedWidth, alignedHeight, timeStamp);
    CopyDecodedImage(surfaceBuffer, alignedWidth, alignedHeight);
    surface->ReleaseBuffer(surfaceBuffer, -1);
    outputTimeStampUs_ = timeStamp;
    ReduceWaitDecodeCnt();
}

OpenSourceLibyuv::RotationMode DecodeDataProcess::ParseAngle(int normalizedAngle)
{
    switch (normalizedAngle) {
        case ROTATION_0:   return OpenSourceLibyuv::RotationMode::kRotate0;
        case ROTATION_90:  return OpenSourceLibyuv::RotationMode::kRotate90;
        case ROTATION_180: return OpenSourceLibyuv::RotationMode::kRotate180;
        case ROTATION_270: return OpenSourceLibyuv::RotationMode::kRotate270;
        default: return OpenSourceLibyuv::RotationMode::kRotate0;
    }
}

bool DecodeDataProcess::I420CopyBySystemSwitch(ImageDataInfo srcInfo, ImageDataInfo dstInfo, int srcWidth,
    int srcHeight, int32_t normalizedAngle)
{
    int32_t cropWidth = 0;
    int32_t cropHeight = 0;
    int32_t pasteX = 0;
    int32_t pasteY = 0;
    if (normalizedAngle == ROTATION_90 || normalizedAngle == ROTATION_270) {
        const int32_t min_dimension = std::min(srcWidth, srcHeight);
        cropWidth = min_dimension;
        cropHeight = min_dimension;
        pasteX = (srcWidth - min_dimension) / Y2UV_RATIO;
        pasteY = (srcHeight - min_dimension) / Y2UV_RATIO;
    } else {
        cropWidth = srcWidth;
        cropHeight = srcHeight;
        pasteX = 0;
        pasteY = 0;
    }
    int32_t cropX = 0;
    int32_t cropY = 0;
    if (normalizedAngle == ROTATION_90 || normalizedAngle == ROTATION_270) {
        cropX = (srcInfo.width - cropWidth) / Y2UV_RATIO;
        cropY = (srcInfo.height - cropHeight) / Y2UV_RATIO;
    } else {
        cropX = 0;
        cropY = 0;
    }
    cropX = static_cast<uint32_t>(cropX) & ~1;
    cropY = static_cast<uint32_t>(cropY) & ~1;
    pasteX = static_cast<uint32_t>(pasteX) & ~1;
    pasteY = static_cast<uint32_t>(pasteY) & ~1;
    uint8_t* srcDataY = srcInfo.dataY + cropY * srcInfo.strideY + cropX;
    uint8_t* srcDataU = srcInfo.dataU + (cropY / Y2UV_RATIO) * srcInfo.strideU + (cropX / Y2UV_RATIO);
    uint8_t* srcDataV = srcInfo.dataV + (cropY / Y2UV_RATIO) * srcInfo.strideV + (cropX / Y2UV_RATIO);

    uint8_t* dstDataY = dstInfo.dataY + pasteY * dstInfo.strideY + pasteX;
    uint8_t* dstDataU = dstInfo.dataU + (pasteY / Y2UV_RATIO) * dstInfo.strideU + (pasteX / Y2UV_RATIO);
    uint8_t* dstDataV = dstInfo.dataV + (pasteY / Y2UV_RATIO) * dstInfo.strideV + (pasteX / Y2UV_RATIO);
    auto converter = ConverterHandle::GetInstance().GetHandle();
    return converter.I420Copy(srcDataY, srcInfo.strideY, srcDataU, srcInfo.strideU, srcDataV,
        srcInfo.strideV, dstDataY, dstInfo.strideY, dstDataU, dstInfo.strideU, dstDataV, dstInfo.strideV,
        cropWidth, cropHeight) == 0;
}

bool DecodeDataProcess::CheckParameters(ImageDataInfo srcInfo, ImageDataInfo dstInfo)
{
    if (!srcInfo.dataY || !srcInfo.dataU || !dstInfo.dataY || !dstInfo.dataU || !dstInfo.dataV) {
        return false;
    }
    if (srcInfo.width <= 0 || srcInfo.height <= 0) {
        return false;
    }
    if ((srcInfo.width % Y2UV_RATIO != 0) || (srcInfo.height % Y2UV_RATIO != 0)) {
        return false;
    }
    return true;
}

bool DecodeDataProcess::UniversalRotateCropAndPadNv12ToI420(ImageDataInfo srcInfo, ImageDataInfo dstInfo,
    int angleDegrees)
{
    if (!CheckParameters(srcInfo, dstInfo)) {
        return false;
    }
    const int normalizedAngle = (angleDegrees % ROTATION_360 + ROTATION_360) % ROTATION_360;
    OpenSourceLibyuv::RotationMode rotationMode = ParseAngle(normalizedAngle);
    int rotatedNaturalWidth = srcInfo.width;
    int rotatedNaturalHeight = srcInfo.height;
    if (normalizedAngle == ROTATION_90 || normalizedAngle == ROTATION_270) {
        rotatedNaturalWidth = srcInfo.height;
        rotatedNaturalHeight = srcInfo.width;
    }
    bool needs_crop_and_pad = (rotatedNaturalWidth != srcInfo.width) ||
        (rotatedNaturalHeight != srcInfo.height);
    auto converter = ConverterHandle::GetInstance().GetHandle();
    if (!needs_crop_and_pad) {
        return converter.NV12ToI420Rotate(srcInfo.dataY, srcInfo.strideY, srcInfo.dataU, srcInfo.strideU,
            dstInfo.dataY, dstInfo.strideY, dstInfo.dataU, dstInfo.strideU, dstInfo.dataV, dstInfo.strideV,
            srcInfo.width, srcInfo.height, rotationMode) == 0;
    }
    int ret = converter.I420Rect(dstInfo.dataY,  dstInfo.strideY, dstInfo.dataU, dstInfo.strideU, dstInfo.dataV,
        dstInfo.strideV, OFFSET_X_0, OFFSET_Y_0, srcInfo.width, srcInfo.height, BLACK_COLOR_PEXEL,
        WHITE_COLOR_PEXEL, WHITE_COLOR_PEXEL);
    if (ret != 0) {
        DHLOGE("I420Rect fail ret = %{public}d", ret);
        return false;
    }
    std::vector<uint8_t> rotatedBuffer(
        rotatedNaturalWidth * rotatedNaturalHeight * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    uint8_t* rotatedY = rotatedBuffer.data();
    uint8_t* rotatedU = rotatedY + rotatedNaturalWidth * rotatedNaturalHeight;
    uint8_t* rotatedV = rotatedU + (rotatedNaturalWidth * rotatedNaturalHeight) / RGB32_MEMORY_COEFFICIENT;
    int rotatedStrideY = rotatedNaturalWidth;
    int rotatedStrideU = (rotatedNaturalWidth + 1) / Y2UV_RATIO;
    int rotatedStrideV = (rotatedNaturalWidth + 1) / Y2UV_RATIO;
    int rotateResult = converter.NV12ToI420Rotate(
        srcInfo.dataY, srcInfo.strideY, srcInfo.dataU, srcInfo.strideU,
        rotatedY, rotatedStrideY, rotatedU, rotatedStrideU, rotatedV, rotatedStrideV,
        srcInfo.width, srcInfo.height, rotationMode);
    if (rotateResult != 0) {
        DHLOGE("NV12ToI420RotateBySystemSwitch fail");
        return false;
    }
    ImageDataInfo rotateInfo = { .width = rotatedNaturalWidth, .height = rotatedNaturalHeight, .dataY = rotatedY,
        .strideY = rotatedStrideY, .dataU = rotatedU, .strideU = rotatedStrideU, .dataV = rotatedV,
        .strideV = rotatedStrideV };
    return I420CopyBySystemSwitch(rotateInfo, dstInfo, srcInfo.width, srcInfo.height, normalizedAngle);
}

bool DecodeDataProcess::FreeYUVBuffer(uint8_t*& dataY, uint8_t*& dataU, uint8_t*& dataV)
{
    if (dataY != nullptr) {
        free(dataY);
        dataY = nullptr;
    }
    if (dataU != nullptr) {
        free(dataU);
        dataU = nullptr;
    }
    if (dataV != nullptr) {
        free(dataV);
        dataV = nullptr;
    }
    return true;
}

bool DecodeDataProcess::ConvertToI420BySystemSwitch(uint8_t *srcDataY, uint8_t *srcDataUV, int32_t alignedWidth,
    int32_t alignedHeight, std::shared_ptr<DataBuffer> bufferOutput)
{
    int dstSizeY = sourceConfig_.GetWidth() * sourceConfig_.GetHeight();
    int dstSizeUV = (static_cast<uint32_t>(sourceConfig_.GetWidth()) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(sourceConfig_.GetHeight()) >> MEMORY_RATIO_UV);
    uint8_t *dstDataY = static_cast<uint8_t*>(aligned_alloc(16, dstSizeY));
    uint8_t *dstDataU = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));
    uint8_t *dstDataV = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));

    if (dstDataY == nullptr || dstDataU == nullptr || dstDataV == nullptr) {
        DHLOGE("aligned_alloc buffer failed.");
        FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
        return false;
    }

    int width = sourceConfig_.GetWidth();
    int dstStrideY = width;
    int dstStrideUV = width / Y2UV_RATIO;
    ImageDataInfo srcInfo = { .width = sourceConfig_.GetWidth(), .height = sourceConfig_.GetHeight(),
        .dataY = srcDataY, .strideY = alignedWidth, .dataU = srcDataUV, .strideU = alignedWidth };
    ImageDataInfo dstInfo = { .dataY = dstDataY, .strideY = dstStrideY, .dataU = dstDataU, .strideU = dstStrideUV,
        .dataV = dstDataV, .strideV = dstStrideUV };
    bool result = UniversalRotateCropAndPadNv12ToI420(srcInfo, dstInfo, rotate_);
    if (!result) {
        DHLOGE("Convert NV12 to I420 failed.");
        FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
        return false;
    }
    if (memcpy_s(bufferOutput->Data(), dstSizeY, dstDataY, dstSizeY) != EOK) {
        DHLOGE("memcpy_s buffer failed.");
        FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
        return false;
    }
    if (memcpy_s(bufferOutput->Data() + dstSizeY, dstSizeUV, dstDataU, dstSizeUV) != EOK) {
        DHLOGE("memcpy_s buffer failed.");
        FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
        return false;
    }
    if (memcpy_s(bufferOutput->Data() + dstSizeY + dstSizeUV, dstSizeUV, dstDataV, dstSizeUV) != EOK) {
        DHLOGE("memcpy_s buffer failed.");
        FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
        return false;
    }
    FreeYUVBuffer(dstDataY, dstDataU, dstDataV);
    return true;
}

bool DecodeDataProcess::ConvertToI420(uint8_t *srcDataY, uint8_t *srcDataUV, int32_t alignedWidth,
    int32_t alignedHeight, std::shared_ptr<DataBuffer> bufferOutput)
{
    int dstSizeY = sourceConfig_.GetWidth() * sourceConfig_.GetHeight();
    int dstSizeUV = (static_cast<uint32_t>(sourceConfig_.GetWidth()) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(sourceConfig_.GetHeight()) >> MEMORY_RATIO_UV);
    uint8_t *dstDataY = bufferOutput->Data();
    uint8_t *dstDataU = bufferOutput->Data() + dstSizeY;
    uint8_t *dstDataV = bufferOutput->Data() + dstSizeY + dstSizeUV;

    auto converter = ConverterHandle::GetInstance().GetHandle();
    CHECK_AND_RETURN_RET_LOG(converter.NV12ToI420 == nullptr, false, "converter is null.");
    int32_t ret = converter.NV12ToI420(srcDataY, alignedWidth, srcDataUV, alignedWidth, dstDataY,
        sourceConfig_.GetWidth(), dstDataU, static_cast<uint32_t>(sourceConfig_.GetWidth()) >> MEMORY_RATIO_UV,
        dstDataV, static_cast<uint32_t>(sourceConfig_.GetWidth()) >> MEMORY_RATIO_UV,
        processedConfig_.GetWidth(), processedConfig_.GetHeight());
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false, "Convert NV12 to I420 failed.");
    return true;
}

void DecodeDataProcess::CopyDecodedImage(const sptr<SurfaceBuffer>& surBuf, int32_t alignedWidth,
    int32_t alignedHeight)
{
    if (!IsCorrectSurfaceBuffer(surBuf, alignedWidth, alignedHeight)) {
        DHLOGE("Surface output buffer error.");
        return;
    }

    DHLOGD("Convert NV12 to I420, format=%{public}d, width=[%{public}d, %{public}d], height=[%{public}d, %{public}d]",
        sourceConfig_.GetVideoformat(), sourceConfig_.GetWidth(), alignedWidth, sourceConfig_.GetHeight(),
        alignedHeight);
    int srcSizeY = alignedWidth * alignedHeight;
    uint8_t *srcDataY = static_cast<uint8_t *>(surBuf->GetVirAddr());
    uint8_t *srcDataUV = static_cast<uint8_t *>(surBuf->GetVirAddr()) + srcSizeY;

    int dstSizeY = sourceConfig_.GetWidth() * sourceConfig_.GetHeight();
    std::shared_ptr<DataBuffer> bufferOutput =
        std::make_shared<DataBuffer>(dstSizeY * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    if (targetConfig_.GetIsSystemSwitch()) {
        if (!ConvertToI420BySystemSwitch(srcDataY, srcDataUV, alignedWidth, alignedHeight, bufferOutput)) {
            DHLOGE("Convert NV12 to I420 by systemSwitch failed.");
            return;
        }
    } else {
        if (!ConvertToI420(srcDataY, srcDataUV, alignedWidth, alignedHeight, bufferOutput)) {
            return;
        }
    }
    {
        std::lock_guard<std::mutex> lock(mtxDequeLock_);
        bufferOutput->frameInfo_ = frameInfoDeque_.front();
        frameInfoDeque_.pop_front();
    }
    DHLOGD("get videoPts=%{public}" PRId64 " from decoder", bufferOutput->frameInfo_.rawTime);
    bufferOutput->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::YUVI420));
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

    size_t yuvImageAlignedSize = static_cast<size_t>(alignedWidth * alignedHeight *
                                                              YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t yuvImageSize = static_cast<size_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight() *
                                                       YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t surfaceBufSize = static_cast<size_t>(surBuf->GetSize());
    if (yuvImageAlignedSize > surfaceBufSize || yuvImageAlignedSize < yuvImageSize) {
        DHLOGE("Buffer size error, yuvImageSize %{public}zu, yuvImageAlignedSize %{public}zu, surBufSize %{public}"
            PRIu32, yuvImageSize, yuvImageAlignedSize, surBuf->GetSize());
        return false;
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
    DHLOGD("Video decode buffer info: presentation TimeUs %{public}" PRId64", size %{public}d, offset %{public}d, "
        "flag %{public}d", info.presentationTimeUs, info.size, info.offset, flag);
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
    if (frameInfoDeque_.empty()) {
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
