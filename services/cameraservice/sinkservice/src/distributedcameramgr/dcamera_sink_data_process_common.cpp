/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkDataProcess::DCameraSinkDataProcess(const std::string& dhId, std::shared_ptr<ICameraChannel>& channel)
    : dhId_(dhId), channel_(channel), eventHandler_(nullptr)
{
    DHLOGI("DCameraSinkDataProcess Constructor dhId: %s", GetAnonyString(dhId_).c_str());
}

DCameraSinkDataProcess::~DCameraSinkDataProcess()
{
    DHLOGI("DCameraSinkDataProcess delete dhId: %s", GetAnonyString(dhId_).c_str());
    if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
        eventHandler_->GetEventRunner()->Stop();
    }
    eventThread_.join();
    eventHandler_ = nullptr;
}

void DCameraSinkDataProcess::Init()
{
    DHLOGI("DCameraSinkDataProcess Init dhId: %s", GetAnonyString(dhId_).c_str());
    eventThread_ = std::thread(&DCameraSinkDataProcess::StartEventHandler, this);
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return eventHandler_ != nullptr;
    });
}

void DCameraSinkDataProcess::StartEventHandler()
{
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
    DHLOGI("DCameraSinkDataProcess::StartCapture dhId: %s, width: %d, height: %d, format: %d, stream: %d, encode: %d",
           GetAnonyString(dhId_).c_str(), captureInfo->width_, captureInfo->height_, captureInfo->format_,
           captureInfo->streamType_, captureInfo->encodeType_);
    captureInfo_ = captureInfo;
    if (pipeline_ != nullptr) {
        DHLOGI("DCameraSinkDataProcess::StartCapture %s pipeline already exits", GetAnonyString(dhId_).c_str());
        return DCAMERA_OK;
    }

    if (captureInfo->streamType_ == CONTINUOUS_FRAME) {
        DHLOGI("DCameraSinkDataProcess::StartCapture %s create data process pipeline", GetAnonyString(dhId_).c_str());
        pipeline_ = std::make_shared<DCameraPipelineSink>();
        auto dataProcess = std::shared_ptr<DCameraSinkDataProcess>(shared_from_this());
        std::shared_ptr<DataProcessListener> listener = std::make_shared<DCameraSinkDataProcessListener>(dataProcess);
        VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                    GetPipelineFormat(captureInfo->format_),
                                    DCAMERA_PRODUCER_FPS_DEFAULT,
                                    captureInfo->width_,
                                    captureInfo->height_);
        VideoConfigParams destParams(GetPipelineCodecType(captureInfo->encodeType_),
                                     GetPipelineFormat(captureInfo->format_),
                                     DCAMERA_PRODUCER_FPS_DEFAULT,
                                     captureInfo->width_,
                                     captureInfo->height_);
        int32_t ret = pipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSinkDataProcess::StartCapture create data process pipeline failed, dhId: %s, ret: %d",
                   GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
    }
    DHLOGI("DCameraSinkDataProcess::StartCapture %s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkDataProcess::StopCapture()
{
    DHLOGI("DCameraSinkDataProcess::StopCapture dhId: %s", GetAnonyString(dhId_).c_str());
    if (pipeline_ != nullptr) {
        pipeline_->DestroyDataProcessPipeline();
        pipeline_ = nullptr;
    }
    if (eventHandler_ != nullptr) {
        DHLOGI("DCameraSinkDataProcess::StopCapture dhId: %s, remove all events", GetAnonyString(dhId_).c_str());
        eventHandler_->RemoveAllEvents();
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkDataProcess::FeedStream(std::shared_ptr<DataBuffer>& dataBuffer)
{
    DCStreamType type = captureInfo_->streamType_;
    DHLOGD("DCameraSinkDataProcess::FeedStream dhId: %s, stream type: %d", GetAnonyString(dhId_).c_str(), type);
    switch (type) {
        case CONTINUOUS_FRAME: {
            int32_t ret = FeedStreamInner(dataBuffer);
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraSinkDataProcess::FeedStream continuous frame failed, dhId: %s, ret: %d",
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
            DHLOGE("DCameraSinkDataProcess::FeedStream %s unknown stream type: %d",
                GetAnonyString(dhId_).c_str(), type);
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
        DHLOGD("SendData type: %d output data ret: %d, dhId: %s, bufferSize: %d", captureInfo_->streamType_, ret,
            GetAnonyString(dhId_).c_str(), buffer->Size());
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(sendFunc);
    }
}

void DCameraSinkDataProcess::OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult)
{
    SendDataAsync(videoResult);
}

void DCameraSinkDataProcess::OnError(DataProcessErrorType errorType)
{
    DHLOGE("DCameraSinkDataProcess::OnError %s data process pipeline error, errorType: %d",
           GetAnonyString(dhId_).c_str(), errorType);
}

int32_t DCameraSinkDataProcess::FeedStreamInner(std::shared_ptr<DataBuffer>& dataBuffer)
{
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    buffers.push_back(dataBuffer);
    int32_t ret = pipeline_->ProcessData(buffers);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSinkDataProcess::FeedStreamInner process data failed, dhId: %s, ret: %d",
               GetAnonyString(dhId_).c_str(), ret);
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
        default:
            codecType = VideoCodecType::NO_CODEC;
            break;
    }
    return codecType;
}

Videoformat DCameraSinkDataProcess::GetPipelineFormat(int32_t format)
{
    return Videoformat::NV21;
}
} // namespace DistributedHardware
} // namespace OHOS