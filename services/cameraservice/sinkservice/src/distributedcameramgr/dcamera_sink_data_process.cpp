/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "dcamera_sink_data_process.h"

#include "anonymous_string.h"
#include "dcamera_channel_sink_impl.h"
#include "dcamera_pipeline_sink.h"
#include "dcamera_sink_data_process_listener.h"
#include "dcamera_hidumper.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
DCameraSinkDataProcess::DCameraSinkDataProcess(const std::string& dhId, std::shared_ptr<ICameraChannel>& channel)
    : dhId_(dhId), channel_(channel), eventHandler_(nullptr)
{
    DHLOGI("DCameraSinkDataProcess Constructor dhId: %{public}s", GetAnonyString(dhId_).c_str());
}

DCameraSinkDataProcess::~DCameraSinkDataProcess()
{
    DHLOGI("DCameraSinkDataProcess delete dhId: %{public}s", GetAnonyString(dhId_).c_str());
    DumpFileUtil::CloseDumpFile(&dumpFile_);
    if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
        eventHandler_->GetEventRunner()->Stop();
    }
    if (eventThread_.joinable()) {
        eventThread_.join();
    }
    eventHandler_ = nullptr;
}

void DCameraSinkDataProcess::Init()
{
    DHLOGI("DCameraSinkDataProcess Init dhId: %{public}s", GetAnonyString(dhId_).c_str());
    eventThread_ = std::thread([this]() { this->StartEventHandler(); });
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
}

void DCameraSinkDataProcess::StartEventHandler()
{
    prctl(PR_SET_NAME, SINK_START_EVENT.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

int32_t DCameraSinkDataProcess::StartCapture(std::shared_ptr<DCameraCaptureInfo>& captureInfo)
{
    CHECK_AND_RETURN_RET_LOG(captureInfo == nullptr, DCAMERA_BAD_VALUE, "StartCapture captureInfo is null");
    DHLOGI("StartCapture dhId: %{public}s, width: %{public}d, height: %{public}d, format: %{public}d, stream: "
        "%{public}d, encode: %{public}d", GetAnonyString(dhId_).c_str(), captureInfo->width_, captureInfo->height_,
        captureInfo->format_, captureInfo->streamType_, captureInfo->encodeType_);
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_DCAMERA_AFTER_ENC_FILENAME, &dumpFile_);
    captureInfo_ = captureInfo;
    if (pipeline_ != nullptr) {
        DHLOGI("StartCapture %{public}s pipeline already exits", GetAnonyString(dhId_).c_str());
        return DCAMERA_OK;
    }

    if (captureInfo->streamType_ == CONTINUOUS_FRAME) {
        DHLOGI("StartCapture %{public}s create data process pipeline", GetAnonyString(dhId_).c_str());
        pipeline_ = std::make_shared<DCameraPipelineSink>();
        auto dataProcess = std::shared_ptr<DCameraSinkDataProcess>(shared_from_this());
        std::shared_ptr<DataProcessListener> listener = std::make_shared<DCameraSinkDataProcessListener>(dataProcess);
        int32_t maxFps = GetMaxFrameRate(captureInfo);
        VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                    GetPipelineFormat(captureInfo->format_),
                                    maxFps,
                                    captureInfo->width_,
                                    captureInfo->height_);
        VideoConfigParams destParams(GetPipelineCodecType(captureInfo->encodeType_),
                                     GetPipelineFormat(captureInfo->format_),
                                     maxFps,
                                     captureInfo->width_,
                                     captureInfo->height_);
        int32_t ret = pipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
        if (ret != DCAMERA_OK) {
            DHLOGE("create data process pipeline failed, dhId: %{public}s, ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
    }
    DHLOGI("StartCapture %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkDataProcess::StopCapture()
{
    DHLOGI("StopCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (pipeline_ != nullptr) {
        pipeline_->DestroyDataProcessPipeline();
        pipeline_ = nullptr;
    }
    if (eventHandler_ != nullptr) {
        DHLOGI("StopCapture dhId: %{public}s, remove all events", GetAnonyString(dhId_).c_str());
        eventHandler_->RemoveAllEvents();
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkDataProcess::FeedStream(std::shared_ptr<DataBuffer>& dataBuffer)
{
    DCStreamType type = captureInfo_->streamType_;
    DHLOGD("FeedStream dhId: %{public}s, stream type: %{public}d", GetAnonyString(dhId_).c_str(), type);
    switch (type) {
        case CONTINUOUS_FRAME: {
            int32_t ret = FeedStreamInner(dataBuffer);
            if (ret != DCAMERA_OK) {
                DHLOGE("FeedStream continuous frame failed, dhId: %{public}s, ret: %{public}d",
                    GetAnonyString(dhId_).c_str(), ret);
                return ret;
            }
            break;
        }
        case SNAPSHOT_FRAME: {
            SendDataAsync(dataBuffer);
            break;
        }
        default: {
            DHLOGE("FeedStream %{public}s unknown stream type: %{public}d", GetAnonyString(dhId_).c_str(), type);
            break;
        }
    }
    return DCAMERA_OK;
}

void DCameraSinkDataProcess::SendDataAsync(const std::shared_ptr<DataBuffer>& buffer)
{
    auto sendFunc = [this, buffer]() mutable {
        std::shared_ptr<DataBuffer> sendBuffer = buffer;
        int32_t ret = channel_->SendData(sendBuffer);
        uint64_t buffersSize = static_cast<uint64_t>(buffer->Size());
        DHLOGD("SendData type: %{public}d output data ret: %{public}d, dhId: %{public}s, bufferSize: %{public}" PRIu64,
            captureInfo_->streamType_, ret, GetAnonyString(dhId_).c_str(), buffersSize);
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(sendFunc);
    }
}

int32_t DCameraSinkDataProcess::OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult)
{
#ifdef DUMP_DCAMERA_FILE
    if (DcameraHidumper::GetInstance().GetDumpFlag() && (IsUnderDumpMaxSize(DUMP_PATH, AFTER_ENCODE) == DCAMERA_OK)) {
        DumpBufferToFile(DUMP_PATH, AFTER_ENCODE, videoResult->Data(), videoResult->Size());
    }
#endif
    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(videoResult->Data()), videoResult->Size());
    if (eventHandler_ == nullptr) {
        DHLOGE("eventHandler_ is uninit");
        return DCAMERA_TRANS_BUSY;
    }
    if (eventHandler_->IsIdle()) {
        SendDataAsync(videoResult);
        return DCAMERA_OK;
    } else {
        return DCAMERA_TRANS_BUSY;
    }
}

void DCameraSinkDataProcess::OnError(DataProcessErrorType errorType)
{
    DHLOGE("OnError %{public}s data process pipeline error, errorType: %{public}d",
           GetAnonyString(dhId_).c_str(), errorType);
}

int32_t DCameraSinkDataProcess::FeedStreamInner(std::shared_ptr<DataBuffer>& dataBuffer)
{
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    buffers.push_back(dataBuffer);
    CHECK_AND_RETURN_RET_LOG(pipeline_ == nullptr, DCAMERA_BAD_VALUE, "pipeline_ is null.");
    int32_t ret = pipeline_->ProcessData(buffers);
    if (ret != DCAMERA_OK) {
        DHLOGE("process data failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    return DCAMERA_OK;
}

VideoCodecType DCameraSinkDataProcess::GetPipelineCodecType(DCEncodeType encodeType)
{
    VideoCodecType codecType;
    switch (encodeType) {
        case ENCODE_TYPE_H264:
            codecType = VideoCodecType::CODEC_H264;
            break;
        case ENCODE_TYPE_H265:
            codecType = VideoCodecType::CODEC_H265;
            break;
        case ENCODE_TYPE_MPEG4_ES:
            codecType = VideoCodecType::CODEC_MPEG4_ES;
            break;
        default:
            codecType = VideoCodecType::NO_CODEC;
            break;
    }
    return codecType;
}

Videoformat DCameraSinkDataProcess::GetPipelineFormat(int32_t format)
{
    Videoformat videoFormat;
    switch (format) {
        case OHOS_CAMERA_FORMAT_RGBA_8888:
            videoFormat = Videoformat::RGBA_8888;
            break;
        default:
            videoFormat = Videoformat::NV21;
            break;
    }
    return videoFormat;
}

int32_t DCameraSinkDataProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    if (pipeline_ == nullptr) {
        DHLOGD("GetProperty: pipeline is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    return pipeline_->GetProperty(propertyName, propertyCarrier);
}

int32_t DCameraSinkDataProcess::GetMaxFrameRate(std::shared_ptr<DCameraCaptureInfo>& captureInfo)
{
    int32_t maxFps = 0;
    if (captureInfo == nullptr || captureInfo->streamType_ != CONTINUOUS_FRAME) {
        return DCAMERA_PRODUCER_FPS_DEFAULT;
    }
    std::vector<std::shared_ptr<DCameraSettings>> captureSettings = captureInfo->captureSettings_;
    std::string metadataSetting;
    for (const auto& setting : captureSettings) {
        if (setting != nullptr && setting->type_ == UPDATE_METADATA) {
            DHLOGI("update metadata settings");
            metadataSetting = setting->value_;
        }
    }

    if (metadataSetting.empty()) {
        DHLOGE("no metadata settings to update");
        return DCAMERA_PRODUCER_FPS_DEFAULT;
    }

    std::string metadataStr = Base64Decode(metadataSetting);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = Camera::MetadataUtils::DecodeFromString(metadataStr);
    if (cameraMetadata == nullptr) {
        DHLOGE("invalid cameraMetadata");
        return DCAMERA_PRODUCER_FPS_DEFAULT;
    }
    camera_metadata_item_t fpsItem;
    int32_t val = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_FPS_RANGES, &fpsItem);
    if (val == CAM_META_SUCCESS) {
        uint32_t fpscount = fpsItem.count;
        DHLOGI("find fps ranges fpscount %{public}d", fpscount);
        if (fpscount != DCAMERA_FPS_SIZE) {
            DHLOGI("find fps ranges fpscount: %{public}u is not DCAMERA_FPS_SIZE.", fpscount);
            return DCAMERA_PRODUCER_FPS_DEFAULT;
        }
        for (uint32_t i = 0; i < fpscount; i++) {
            maxFps = std::max(maxFps, fpsItem.data.i32[i]);
        }
        DHLOGI("get maxFps success, maxFps: %{public}d", maxFps);
        if (maxFps == 0) {
            return DCAMERA_PRODUCER_FPS_DEFAULT;
        }
        return maxFps;
    } else {
        DHLOGE("get maxFps failed, ret: %{public}d", val);
    }
    return DCAMERA_PRODUCER_FPS_DEFAULT;
}
} // namespace DistributedHardware
} // namespace OHOS