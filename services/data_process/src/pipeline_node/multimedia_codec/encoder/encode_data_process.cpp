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

#include <cmath>
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_radar.h"
#include "dcamera_utils_tools.h"
#include "distributed_hardware_log.h"
#include "encode_data_process.h"
#include "encode_video_callback.h"
#include "graphic_common_c.h"
#include <ctime>

#ifndef DH_LOG_TAG
#define DH_LOG_TAG "DCDP_NODE_ENCODEC"
#endif

namespace OHOS {
namespace DistributedHardware {
const std::map<int64_t, int64_t> EncodeDataProcess::ENCODER_BITRATE_TABLE = {
    std::map<int64_t, int64_t>::value_type(WIDTH_320_HEIGHT_240, BITRATE_500000),
    std::map<int64_t, int64_t>::value_type(WIDTH_480_HEIGHT_360, BITRATE_1110000),
    std::map<int64_t, int64_t>::value_type(WIDTH_640_HEIGHT_360, BITRATE_1500000),
    std::map<int64_t, int64_t>::value_type(WIDTH_640_HEIGHT_480, BITRATE_1800000),
    std::map<int64_t, int64_t>::value_type(WIDTH_720_HEIGHT_540, BITRATE_2100000),
    std::map<int64_t, int64_t>::value_type(WIDTH_960_HEIGHT_540, BITRATE_2300000),
    std::map<int64_t, int64_t>::value_type(WIDTH_960_HEIGHT_720, BITRATE_2800000),
    std::map<int64_t, int64_t>::value_type(WIDTH_1280_HEIGHT_720, BITRATE_3400000),
    std::map<int64_t, int64_t>::value_type(WIDTH_1440_HEIGHT_1080, BITRATE_5000000),
    std::map<int64_t, int64_t>::value_type(WIDTH_1920_HEIGHT_1080, BITRATE_6000000),
};
const std::string ENUM_VIDEOFORMAT_STRINGS[] = {
    "YUVI420", "NV12", "NV21", "RGBA_8888"
};

EncodeDataProcess::~EncodeDataProcess()
{
    if (isEncoderProcess_.load()) {
        DHLOGD("~EncodeDataProcess : ReleaseProcessNode.");
        ReleaseProcessNode();
    }
}

int32_t EncodeDataProcess::InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
    VideoConfigParams& processedConfig)
{
    DHLOGD("Init DCamera EncodeNode start.");
    if (!(IsInEncoderRange(sourceConfig) && IsInEncoderRange(targetConfig))) {
        DHLOGE("Source config or target config are invalid.");
        return DCAMERA_BAD_VALUE;
    }
    if (!IsConvertible(sourceConfig, targetConfig)) {
        DHLOGE("The EncodeNode cannot convert source VideoCodecType %{public}d to target VideoCodecType %{public}d.",
            sourceConfig.GetVideoCodecType(), targetConfig.GetVideoCodecType());
        return DCAMERA_BAD_TYPE;
    }

    sourceConfig_ = sourceConfig;
    targetConfig_ = targetConfig;
    if (sourceConfig_.GetVideoCodecType() == targetConfig_.GetVideoCodecType()) {
        DHLOGD("Disable EncodeNode. The target VideoCodecType %{public}d is the same as the source VideoCodecType "
            "%{public}d.", sourceConfig_.GetVideoCodecType(), targetConfig_.GetVideoCodecType());
        processedConfig_ = sourceConfig;
        processedConfig = processedConfig_;
        {
            std::unique_lock<std::mutex> lock(isEncoderProcessMtx_);
            isEncoderProcess_.store(true);
        }
        isEncoderProcessCond_.notify_one();
        return DCAMERA_OK;
    }

    int32_t err = InitEncoder();
    if (err != DCAMERA_OK) {
        DHLOGE("Init video encoder failed.");
        ReleaseProcessNode();
        return err;
    }
    processedConfig = processedConfig_;
    {
        std::unique_lock<std::mutex> lock(isEncoderProcessMtx_);
        isEncoderProcess_.store(true);
    }
    isEncoderProcessCond_.notify_one();
    return DCAMERA_OK;
}

bool EncodeDataProcess::IsInEncoderRange(const VideoConfigParams& curConfig)
{
    bool isWidthValid = (curConfig.GetWidth() >= MIN_VIDEO_WIDTH && curConfig.GetWidth() <= MAX_VIDEO_WIDTH);
    bool isHeightValid = (curConfig.GetHeight() >= MIN_VIDEO_HEIGHT && curConfig.GetHeight() <= MAX_VIDEO_HEIGHT);
    bool isFrameRateValid = (curConfig.GetFrameRate() >= MIN_FRAME_RATE && curConfig.GetFrameRate() <= MAX_FRAME_RATE);
    return isWidthValid && isHeightValid && isFrameRateValid;
}

bool EncodeDataProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return (sourceConfig.GetVideoCodecType() == targetConfig.GetVideoCodecType() ||
        sourceConfig.GetVideoCodecType() == VideoCodecType::NO_CODEC);
}

int32_t EncodeDataProcess::InitEncoder()
{
    DHLOGD("Init video encoder.");
    int32_t ret = ConfigureVideoEncoder();
    if (ret != DCAMERA_OK) {
        DHLOGE("Init video encoder metadata format failed. ret %{public}d.", ret);
        return ret;
    }

    ret = StartVideoEncoder();
    if (ret != DCAMERA_OK) {
        DHLOGE("Start Video encoder failed.");
        ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_ENCODE_ERROR,
            CreateMsg("start video encoder failed, width: %d, height: %d, format: %s",
            sourceConfig_.GetWidth(), sourceConfig_.GetHeight(),
            ENUM_VIDEOFORMAT_STRINGS[static_cast<int32_t>(sourceConfig_.GetVideoformat())].c_str()));
        return ret;
    }

    return DCAMERA_OK;
}

int32_t EncodeDataProcess::ConfigureVideoEncoder()
{
    int32_t ret = InitEncoderMetadataFormat();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
        "Init video encoder metadata format failed. ret %{public}d.", ret);
    ret = InitEncoderBitrateFormat();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
        "Init video encoder bitrate format failed. ret %{public}d.", ret);
    videoEncoder_ = MediaAVCodec::VideoEncoderFactory::CreateByMime(processType_);
    if (videoEncoder_ == nullptr) {
        DHLOGE("Create video encoder failed.");
        return DCAMERA_INIT_ERR;
    }
    encodeVideoCallback_ = std::make_shared<EncodeVideoCallback>(shared_from_this());
    ret = videoEncoder_->SetCallback(encodeVideoCallback_);
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("Set video encoder callback failed. ret %{public}d.", ret);
        return DCAMERA_INIT_ERR;
    }

    ret = videoEncoder_->Configure(metadataFormat_);
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Set video encoder metadata format failed. ret %{public}d.", ret);

    encodeProducerSurface_ = videoEncoder_->CreateInputSurface();
    CHECK_AND_RETURN_RET_LOG(encodeProducerSurface_ == nullptr, DCAMERA_INIT_ERR,
        "%s", "Get video encoder producer surface failed.");

    return DCAMERA_OK;
}

int32_t EncodeDataProcess::InitEncoderMetadataFormat()
{
    processedConfig_ = sourceConfig_;
    switch (targetConfig_.GetVideoCodecType()) {
        case VideoCodecType::CODEC_H264:
            processType_ = "video/avc";
            metadataFormat_.PutIntValue("codec_profile", MediaAVCodec::AVCProfile::AVC_PROFILE_BASELINE);
            processedConfig_.SetVideoCodecType(VideoCodecType::CODEC_H264);
            break;
        case VideoCodecType::CODEC_H265:
            processType_ = "video/hevc";
            metadataFormat_.PutIntValue("codec_profile", MediaAVCodec::HEVCProfile::HEVC_PROFILE_MAIN);
            processedConfig_.SetVideoCodecType(VideoCodecType::CODEC_H265);
            break;
        case VideoCodecType::CODEC_MPEG4_ES:
            processType_ = "video/mp4v-es";
            metadataFormat_.PutIntValue("codec_profile",
                MediaAVCodec::MPEG4Profile::MPEG4_PROFILE_ADVANCED_CODING_EFFICIENCY);
            processedConfig_.SetVideoCodecType(VideoCodecType::CODEC_MPEG4_ES);
            break;
        default:
            DHLOGE("The current codec type does not support encoding.");
            return DCAMERA_NOT_FOUND;
    }
    switch (sourceConfig_.GetVideoformat()) {
        case Videoformat::YUVI420:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::YUVI420));
            metadataFormat_.PutLongValue("max_input_size", NORM_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::NV12:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::NV12));
            metadataFormat_.PutLongValue("max_input_size", NORM_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::NV21:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::NV21));
            metadataFormat_.PutLongValue("max_input_size", NORM_YUV420_BUFFER_SIZE);
            break;
        case Videoformat::RGBA_8888:
            metadataFormat_.PutIntValue("pixel_format", static_cast<int32_t>(MediaAVCodec::VideoPixelFormat::RGBA));
            metadataFormat_.PutLongValue("max_input_size", NORM_RGB32_BUFFER_SIZE);
            break;
        default:
            DHLOGE("The current pixel format does not support encoding.");
            return DCAMERA_NOT_FOUND;
    }
    metadataFormat_.PutStringValue("codec_mime", processType_);
    metadataFormat_.PutIntValue("width", static_cast<int32_t>(sourceConfig_.GetWidth()));
    metadataFormat_.PutIntValue("height", static_cast<int32_t>(sourceConfig_.GetHeight()));
    metadataFormat_.PutDoubleValue("frame_rate", MAX_FRAME_RATE);
    return DCAMERA_OK;
}

int32_t EncodeDataProcess::InitEncoderBitrateFormat()
{
    DHLOGD("Init video encoder bitrate format.");
    CHECK_AND_RETURN_RET_LOG(!(IsInEncoderRange(sourceConfig_) && IsInEncoderRange(targetConfig_)), DCAMERA_BAD_VALUE,
        "%{public}s", "Source config or target config are invalid.");
    metadataFormat_.PutIntValue("i_frame_interval", IDR_FRAME_INTERVAL_MS);
    metadataFormat_.PutIntValue("video_encode_bitrate_mode", MediaAVCodec::VideoEncodeBitrateMode::VBR);

    CHECK_AND_RETURN_RET_LOG(ENCODER_BITRATE_TABLE.empty(), DCAMERA_OK, "%{public}s",
        "ENCODER_BITRATE_TABLE is null, use the default bitrate of the encoder.");
    int64_t pixelformat = static_cast<int64_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight());
    int64_t matchedBitrate = BITRATE_6000000;
    int64_t minPixelformatDiff = WIDTH_1920_HEIGHT_1080 - pixelformat;
    for (auto it = ENCODER_BITRATE_TABLE.begin(); it != ENCODER_BITRATE_TABLE.end(); it++) {
        int64_t pixelformatDiff = abs(pixelformat - it->first);
        if (pixelformatDiff == 0) {
            matchedBitrate = it->second;
            break;
        }
        if (minPixelformatDiff >= pixelformatDiff) {
            minPixelformatDiff = pixelformatDiff;
            matchedBitrate = it->second;
        }
    }
    DHLOGD("Source config: width : %{public}d, height : %{public}d, matched bitrate %{public}" PRId64,
        sourceConfig_.GetWidth(), sourceConfig_.GetHeight(), matchedBitrate);
    metadataFormat_.PutLongValue("bitrate", matchedBitrate);
    return DCAMERA_OK;
}

int32_t EncodeDataProcess::StartVideoEncoder()
{
    if (videoEncoder_ == nullptr) {
        DHLOGE("The video encoder does not exist before StopVideoEncoder.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ret = videoEncoder_->Prepare();
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Video encoder prepare failed. ret %{public}d.", ret);
    ret = videoEncoder_->Start();
    CHECK_AND_RETURN_RET_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK, DCAMERA_INIT_ERR,
        "Video encoder start failed. ret %{public}d.", ret);
    return DCAMERA_OK;
}

int32_t EncodeDataProcess::StopVideoEncoder()
{
    if (videoEncoder_ == nullptr) {
        DHLOGE("The video encoder does not exist before StopVideoEncoder.");
        return DCAMERA_BAD_VALUE;
    }

    bool isSuccess = true;
    int32_t ret = videoEncoder_->Flush();
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("VideoEncoder flush failed. ret %{public}d.", ret);
        isSuccess = isSuccess && false;
    }
    ret = videoEncoder_->Stop();
    if (ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK) {
        DHLOGE("VideoEncoder stop failed. ret %{public}d.", ret);
        isSuccess = isSuccess && false;
    }

    if (!isSuccess) {
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

void EncodeDataProcess::ReleaseVideoEncoder()
{
    std::lock_guard<std::mutex> lck(mtxEncoderState_);
    DHLOGD("Start release videoEncoder.");
    if (videoEncoder_ == nullptr) {
        DHLOGE("The video encoder does not exist before ReleaseVideoEncoder.");
        encodeProducerSurface_ = nullptr;
        encodeVideoCallback_ = nullptr;
        return;
    }
    int32_t ret = StopVideoEncoder();
    CHECK_AND_LOG(ret != DCAMERA_OK, "%{public}s", "StopVideoEncoder failed.");
    ret = videoEncoder_->Release();
    CHECK_AND_LOG(ret != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK,
        "VideoEncoder release failed. ret %{public}d.", ret);
    encodeProducerSurface_ = nullptr;
    videoEncoder_ = nullptr;
    encodeVideoCallback_ = nullptr;
    DHLOGD("Start release videoEncoder success.");
}

void EncodeDataProcess::ReleaseProcessNode()
{
    DHLOGD("Start release [%{public}zu] node : EncodeNode.", nodeRank_);
    isEncoderProcess_.store(false);
    ReleaseVideoEncoder();

    {
        std::lock_guard<std::mutex> lck(mtxHoldCount_);
        waitEncoderOutputCount_ = 0;
    }
    lastFeedEncoderInputBufferTimeUs_ = 0;
    inputTimeStampUs_ = 0;
    processType_ = "";

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGD("Release [%{public}zu] node : EncodeNode end.", nodeRank_);
}

int32_t EncodeDataProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    DHLOGD("Process data in EncodeDataProcess.");
    if (inputBuffers.empty() || inputBuffers[0] == nullptr) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }
    if (sourceConfig_.GetVideoCodecType() == processedConfig_.GetVideoCodecType()) {
        DHLOGD("The target VideoCodecType : %{public}d is the same as the source VideoCodecType : %{public}d.",
            sourceConfig_.GetVideoCodecType(), processedConfig_.GetVideoCodecType());
        return EncodeDone(inputBuffers);
    }
    if (videoEncoder_ == nullptr) {
        DHLOGE("The video encoder does not exist before encoding data.");
        return DCAMERA_INIT_ERR;
    }
    if (inputBuffers[0]->Size() > NORM_YUV420_BUFFER_SIZE) {
        DHLOGE("EncodeNode input buffer size %{public}zu error.", inputBuffers[0]->Size());
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    CHECK_AND_RETURN_RET_LOG(!isEncoderProcess_.load(), DCAMERA_DISABLE_PROCESS, "%{public}s",
        "EncodeNode occurred error or start release.");
    int32_t err = FeedEncoderInputBuffer(inputBuffers[0]);
    CHECK_AND_RETURN_RET_LOG(err != DCAMERA_OK, err, "%{public}s", "Feed encoder input Buffer failed.");
    return DCAMERA_OK;
}

int32_t EncodeDataProcess::FeedEncoderInputBuffer(std::shared_ptr<DataBuffer>& inputBuffer)
{
    std::lock_guard<std::mutex> lck(mtxEncoderState_);
    DHLOGD("Feed encoder input buffer, buffer size %{public}zu.", inputBuffer->Size());
    CHECK_AND_RETURN_RET_LOG(encodeProducerSurface_ == nullptr, DCAMERA_INIT_ERR, "%{public}s",
        "Get encoder input producer surface failed.");
    sptr<SurfaceBuffer> surfacebuffer = GetEncoderInputSurfaceBuffer();
    CHECK_AND_RETURN_RET_LOG(surfacebuffer == nullptr, DCAMERA_BAD_OPERATE, "%{public}s",
        "Get encoder input producer surface buffer failed.");
    uint8_t *addr = static_cast<uint8_t *>(surfacebuffer->GetVirAddr());
    if (addr == nullptr) {
        DHLOGE("SurfaceBuffer address is nullptr");
        encodeProducerSurface_->CancelBuffer(surfacebuffer);
        return DCAMERA_BAD_OPERATE;
    }
    size_t size = static_cast<size_t>(surfacebuffer->GetSize());
    errno_t err = memcpy_s(addr, size, inputBuffer->Data(), inputBuffer->Size());
    CHECK_AND_RETURN_RET_LOG(err != EOK, DCAMERA_MEMORY_OPT_ERROR,
        "memcpy_s encoder input producer surfacebuffer failed, surBufSize %{public}zu.", size);

    inputTimeStampUs_ = GetEncoderTimeStamp();
    DHLOGD("Encoder input buffer size %{public}zu, timeStamp %{public}lld.", inputBuffer->Size(),
        (long long)inputTimeStampUs_);
    if (surfacebuffer->GetExtraData() == nullptr) {
        DHLOGE("Surface buffer exist null extra data.");
        return DCAMERA_BAD_OPERATE;
    }
    surfacebuffer->GetExtraData()->ExtraSet("timeStamp", inputTimeStampUs_);

    BufferFlushConfig flushConfig = { {0, 0, sourceConfig_.GetWidth(), sourceConfig_.GetHeight()}, 0};
    SurfaceError ret = encodeProducerSurface_->FlushBuffer(surfacebuffer, -1, flushConfig);
    CHECK_AND_RETURN_RET_LOG(ret != SURFACE_ERROR_OK, DCAMERA_BAD_OPERATE, "%s",
        "Flush encoder input producer surface buffer failed.");
    return DCAMERA_OK;
}

sptr<SurfaceBuffer> EncodeDataProcess::GetEncoderInputSurfaceBuffer()
{
    BufferRequestConfig requestConfig;
    requestConfig.width = sourceConfig_.GetWidth();
    requestConfig.height = sourceConfig_.GetHeight();
    requestConfig.usage = BUFFER_USAGE_CPU_READ | BUFFER_USAGE_CPU_WRITE | BUFFER_USAGE_MEM_DMA;
    requestConfig.timeout = 0;
    requestConfig.strideAlignment = ENCODER_STRIDE_ALIGNMENT;
    switch (sourceConfig_.GetVideoformat()) {
        case Videoformat::YUVI420:
            requestConfig.format = PixelFormat::PIXEL_FMT_YCBCR_420_P;
            break;
        case Videoformat::NV12:
            requestConfig.format = PixelFormat::PIXEL_FMT_YCBCR_420_SP;
            break;
        case Videoformat::NV21:
            requestConfig.format = PixelFormat::PIXEL_FMT_YCRCB_420_SP;
            break;
        case Videoformat::RGBA_8888:
            requestConfig.format = PixelFormat::PIXEL_FMT_RGBA_8888;
            break;
        default:
            DHLOGE("The current pixel format does not support encoding.");
            return nullptr;
    }
    sptr<SurfaceBuffer> surfacebuffer = nullptr;
    int32_t flushFence = -1;
    if (encodeProducerSurface_ == nullptr) {
        DHLOGE("Encode producer surface is null.");
        return nullptr;
    }
    GSError err = encodeProducerSurface_->RequestBuffer(surfacebuffer, flushFence, requestConfig);
    if (err != GSERROR_OK || surfacebuffer == nullptr) {
        DHLOGE("Request encoder input producer surface buffer failed, error code: %d.", err);
    }
    return surfacebuffer;
}

int64_t EncodeDataProcess::GetEncoderTimeStamp()
{
    if (inputTimeStampUs_ != 0) {
        lastFeedEncoderInputBufferTimeUs_ = inputTimeStampUs_;
    }
    const int64_t nsPerUs = 1000L;
    int64_t nowTimeUs = GetNowTimeStampUs() * nsPerUs;
    return nowTimeUs;
}

void EncodeDataProcess::IncreaseWaitEncodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (lastFeedEncoderInputBufferTimeUs_ == 0) {
        waitEncoderOutputCount_ += FIRST_FRAME_OUTPUT_NUM;
    } else {
        waitEncoderOutputCount_++;
    }
    DHLOGD("Wait encoder output frames number is %{public}d.", waitEncoderOutputCount_);
}

void EncodeDataProcess::ReduceWaitEncodeCnt()
{
    std::lock_guard<std::mutex> lck(mtxHoldCount_);
    if (waitEncoderOutputCount_ <= 0) {
        DHLOGE("The waitEncoderOutputCount_ = %{public}d.", waitEncoderOutputCount_);
    }
    waitEncoderOutputCount_--;
    DHLOGD("Wait encoder output frames number is %{public}d.", waitEncoderOutputCount_);
}

int32_t EncodeDataProcess::GetEncoderOutputBuffer(uint32_t index, MediaAVCodec::AVCodecBufferInfo info,
    MediaAVCodec::AVCodecBufferFlag flag, std::shared_ptr<Media::AVSharedMemory>& buffer)
{
    DHLOGD("Get encoder output buffer.");
    if (videoEncoder_ == nullptr) {
        DHLOGE("The video encoder does not exist before output encoded data.");
        return DCAMERA_BAD_VALUE;
    }
    if (buffer == nullptr) {
        DHLOGE("Failed to get the output shared memory, index : %{public}u", index);
        return DCAMERA_BAD_OPERATE;
    }

    CHECK_AND_RETURN_RET_LOG(info.size <= 0 || info.size > DATABUFF_MAX_SIZE, DCAMERA_BAD_VALUE,
        "AVCodecBufferInfo error, buffer size : %{public}d", info.size);
    size_t outputMemoDataSize = static_cast<size_t>(info.size);
    CHECK_AND_RETURN_RET_LOG(buffer->GetBase() == nullptr, DCAMERA_BAD_OPERATE,
        "Sink point check failed: Source buffer base is null.");
    int64_t sourceAllocatedSize = buffer->GetSize();
    if (sourceAllocatedSize < 0) {
        DHLOGE("Sink point check failed: buffer->GetSize() returned a negative error code.");
        return DCAMERA_BAD_VALUE;
    }
    if (outputMemoDataSize > static_cast<size_t>(sourceAllocatedSize)) {
        DHLOGE("Sink point check failed: outputMemoDataSize exceeds source allocated size.");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGD("Encoder output buffer size : %{public}zu", outputMemoDataSize);
    std::shared_ptr<DataBuffer> bufferOutput = std::make_shared<DataBuffer>(outputMemoDataSize);
    CHECK_AND_RETURN_RET_LOG(bufferOutput->Data() == nullptr, DCAMERA_MEMORY_OPT_ERROR,
        "Sink point check failed: Failed to allocate output buffer.");
    errno_t err = memcpy_s(bufferOutput->Data(), bufferOutput->Size(),
        buffer->GetBase(), outputMemoDataSize);
    CHECK_AND_RETURN_RET_LOG(err != EOK, DCAMERA_MEMORY_OPT_ERROR, "%{public}s", "memcpy_s buffer failed.");
    int64_t timeStamp = info.presentationTimeUs;
    DHLOGI("get videoPts=%{public}" PRId64" from encoder", timeStamp);
    struct timespec time = {0, 0};
    clock_gettime(CLOCK_REALTIME, &time);
    int64_t timeNs = static_cast<int64_t>(time.tv_sec) * S2NS + static_cast<int64_t>(time.tv_nsec);
    int64_t encodeT = timeNs / static_cast<int64_t>(US2NS) - timeStamp;
    int64_t finishEncodeT = GetNowTimeStampUs();
    int64_t startEncodeT = finishEncodeT - encodeT;
    bufferOutput->SetInt64(START_ENCODE_TIME_US, startEncodeT);
    bufferOutput->SetInt64(FINISH_ENCODE_TIME_US, finishEncodeT);
    bufferOutput->SetInt64(TIME_STAMP_US, timeStamp);
    bufferOutput->SetInt32(FRAME_TYPE, flag);
    bufferOutput->SetInt32(INDEX, index_);
    index_++;
    std::vector<std::shared_ptr<DataBuffer>> nextInputBuffers;
    nextInputBuffers.push_back(bufferOutput);
    return EncodeDone(nextInputBuffers);
}

int32_t EncodeDataProcess::EncodeDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    DHLOGD("Encoder done.");
    if (outputBuffers.empty()) {
        DHLOGE("The received data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (nextDataProcess_ != nullptr) {
        DHLOGD("Send to the next node of the encoder for processing.");
        int32_t err = nextDataProcess_->ProcessData(outputBuffers);
        CHECK_AND_LOG(err != DCAMERA_OK, "%{public}s", "Someone node after the encoder processes failed.");
        return err;
    }
    DHLOGD("The current node is the last node, and Output the processed video buffer");
    std::shared_ptr<DCameraPipelineSink> targetPipelineSink = callbackPipelineSink_.lock();
    if (targetPipelineSink == nullptr) {
        DHLOGE("callbackPipelineSink_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    targetPipelineSink->OnProcessedVideoBuffer(outputBuffers[0]);
    return DCAMERA_OK;
}

void EncodeDataProcess::OnError()
{
    DHLOGD("EncodeDataProcess : OnError.");
    isEncoderProcess_.store(false);
    if (videoEncoder_ != nullptr) {
        videoEncoder_->Flush();
        videoEncoder_->Stop();
    }
    std::shared_ptr<DCameraPipelineSink> targetPipelineSink = callbackPipelineSink_.lock();
    CHECK_AND_RETURN_LOG(targetPipelineSink == nullptr, "%{public}s", "callbackPipelineSink_ is nullptr.");
    targetPipelineSink->OnError(DataProcessErrorType::ERROR_PIPELINE_ENCODER);
}

void EncodeDataProcess::OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("The available input buffer index : %{public}u. No operation when using input.", index);
}

void EncodeDataProcess::OnOutputFormatChanged(const Media::Format &format)
{
    if (encodeOutputFormat_.GetFormatMap().empty()) {
        DHLOGE("The first changed video encoder output format is null.");
        return;
    }
    encodeOutputFormat_ = format;
}

void EncodeDataProcess::OnOutputBufferAvailable(uint32_t index, MediaAVCodec::AVCodecBufferInfo info,
    MediaAVCodec::AVCodecBufferFlag flag, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGI("Waiting for encoder process to become available...");
    std::unique_lock<std::mutex> lock(isEncoderProcessMtx_);
    bool timeOut = !isEncoderProcessCond_.wait_for(lock, TIMEOUT_3_SEC, [this] {
        return isEncoderProcess_.load();
    });
    if (timeOut) {
        DHLOGE("Timed out waiting for encoder process after 3 second.");
        return;
    }
    DHLOGD("Video encode buffer info: presentation TimeUs %{public}" PRId64", size %{public}d, offset %{public}d, "
        "flag %{public}d", info.presentationTimeUs, info.size, info.offset, flag);
    int32_t err = GetEncoderOutputBuffer(index, info, flag, buffer);
    if (err != DCAMERA_OK) {
        DHLOGE("Get encode output Buffer failed.");
        return;
    }
    CHECK_AND_RETURN_LOG(videoEncoder_ == nullptr, "%{public}s",
        "The video encoder does not exist before release output buffer index.");
    int32_t errRelease = videoEncoder_->ReleaseOutputBuffer(index);
    CHECK_AND_LOG(errRelease != MediaAVCodec::AVCodecServiceErrCode::AVCS_ERR_OK,
        "The video encoder release output buffer failed, index : [%{public}u].", index);
}

VideoConfigParams EncodeDataProcess::GetSourceConfig() const
{
    return sourceConfig_;
}

VideoConfigParams EncodeDataProcess::GetTargetConfig() const
{
    return targetConfig_;
}

int32_t EncodeDataProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    if (propertyName != surfaceStr_) {
        return DCAMERA_OK;
    }
    CHECK_AND_RETURN_RET_LOG(encodeProducerSurface_ == nullptr, DCAMERA_BAD_VALUE, "%{public}s",
        "EncodeDataProcess::GetProperty: encode dataProcess get property fail, encode surface is nullptr.");
    encodeProducerSurface_->SetDefaultUsage(encodeProducerSurface_->GetDefaultUsage() & (~BUFFER_USAGE_VIDEO_ENCODER));
    return propertyCarrier.CarrySurfaceProperty(encodeProducerSurface_);
}

int32_t EncodeDataProcess::UpdateSettings(const std::shared_ptr<Camera::CameraMetadata> settings)
{
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
