/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dcamera_pipeline_source.h"

#include "dcamera_hitrace_adapter.h"
#include "distributed_hardware_log.h"

#include "decode_data_process.h"
#include "fps_controller_process.h"
#include "scale_convert_process.h"

namespace OHOS {
namespace DistributedHardware {
const std::string DCameraPipelineSource::PIPELINE_OWNER = "Source";

DCameraPipelineSource::~DCameraPipelineSource()
{
    if (isProcess_) {
        DHLOGD("~DCameraPipelineSource : Destroy source data process pipeline.");
        DestroyDataProcessPipeline();
    }
}

int32_t DCameraPipelineSource::CreateDataProcessPipeline(PipelineType piplineType,
    const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
    const std::shared_ptr<DataProcessListener>& listener)
{
    DCAMERA_SYNC_TRACE(DCAMERA_SOURCE_CREATE_PIPELINE);
    DHLOGD("Create source data process pipeline.");
    switch (piplineType) {
        case PipelineType::VIDEO:
            if (!(IsInRange(sourceConfig) && IsInRange(targetConfig))) {
                DHLOGE("Source config or target config of source pipeline are invalid.");
                return DCAMERA_BAD_VALUE;
            }
            break;
        default:
            DHLOGE("JPEG or other pipeline type are not supported in source pipeline.");
            return DCAMERA_NOT_FOUND;
    }
    if (listener == nullptr) {
        DHLOGE("The process listener of source pipeline is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (pipelineHead_ != nullptr) {
        DHLOGD("The source pipeline already exists.");
        return DCAMERA_OK;
    }

    InitDCameraPipEvent();
    int32_t err = InitDCameraPipNodes(sourceConfig, targetConfig);
    if (err != DCAMERA_OK) {
        DestroyDataProcessPipeline();
        return err;
    }
    piplineType_ = piplineType;
    processListener_ = listener;
    isProcess_ = true;
    return DCAMERA_OK;
}

bool DCameraPipelineSource::IsInRange(const VideoConfigParams& curConfig)
{
    return (curConfig.GetFrameRate() >= MIN_FRAME_RATE || curConfig.GetFrameRate() <= MAX_FRAME_RATE ||
        curConfig.GetWidth() >= MIN_VIDEO_WIDTH || curConfig.GetWidth() <= MAX_VIDEO_WIDTH ||
        curConfig.GetHeight() >= MIN_VIDEO_HEIGHT || curConfig.GetHeight() <= MAX_VIDEO_HEIGHT);
}

void DCameraPipelineSource::InitDCameraPipEvent()
{
    DHLOGD("Init source DCamera pipeline event to asynchronously process data.");
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    pipeEventHandler_ = std::make_shared<DCameraPipelineSource::DCameraPipelineSrcEventHandler>(
        runner, shared_from_this());
}

int32_t DCameraPipelineSource::InitDCameraPipNodes(const VideoConfigParams& sourceConfig,
    const VideoConfigParams& targetConfig)
{
    DHLOGD("Init source DCamera pipeline Nodes.");
    if (piplineType_ == PipelineType::PHOTO_JPEG) {
        DHLOGE("JPEG data process is not supported.");
        return DCAMERA_NOT_FOUND;
    }
    if (pipeEventHandler_ == nullptr) {
        DHLOGE("eventBusSource is nullptr.");
        return DCAMERA_BAD_VALUE;
    }

    pipNodeRanks_.push_back(std::make_shared<DecodeDataProcess>(pipeEventHandler_, shared_from_this()));
    pipNodeRanks_.push_back(std::make_shared<ScaleConvertProcess>(shared_from_this()));
    if (pipNodeRanks_.size() == 0) {
        DHLOGD("Creating an empty source pipeline.");
        pipelineHead_ = nullptr;
        return DCAMERA_BAD_VALUE;
    }

    VideoConfigParams curNodeSourceCfg = sourceConfig;
    for (size_t i = 0; i < pipNodeRanks_.size(); i++) {
        pipNodeRanks_[i]->SetNodeRank(i);
        DHLOGI("DCameraPipelineSource::InitDCameraPipNodes Node %{public}zu Source Config: width %{public}d height "
            "%{public}d format %{public}d codecType %{public}d frameRate %{public}d", i, curNodeSourceCfg.GetWidth(),
            curNodeSourceCfg.GetHeight(), curNodeSourceCfg.GetVideoformat(), curNodeSourceCfg.GetVideoCodecType(),
            curNodeSourceCfg.GetFrameRate());

        VideoConfigParams curNodeProcessedCfg;
        int32_t err = pipNodeRanks_[i]->InitNode(curNodeSourceCfg, targetConfig, curNodeProcessedCfg);
        if (err != DCAMERA_OK) {
            DHLOGE("Init source DCamera pipeline Node [%{public}zu] failed.", i);
            return DCAMERA_INIT_ERR;
        }
        curNodeSourceCfg = curNodeProcessedCfg;

        if (i == 0) {
            continue;
        }

        err = pipNodeRanks_[i - 1]->SetNextNode(pipNodeRanks_[i]);
        if (err != DCAMERA_OK) {
            DHLOGE("Set the next node of Node [%{public}zu] failed in source pipeline.", i - 1);
            return DCAMERA_INIT_ERR;
        }
    }
    DHLOGD("All nodes have been linked in source pipeline, Target Config: "
        "width %{public}d height %{public}d format %{public}d codecType %{public}d frameRate %{public}d",
        targetConfig.GetWidth(), targetConfig.GetHeight(),
        targetConfig.GetVideoformat(), targetConfig.GetVideoCodecType(), targetConfig.GetFrameRate());
    pipelineHead_ = pipNodeRanks_[0];
    return DCAMERA_OK;
}

int32_t DCameraPipelineSource::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& dataBuffers)
{
    DHLOGD("Process data buffers in source pipeline.");
    if (piplineType_ == PipelineType::PHOTO_JPEG) {
        DHLOGE("JPEG data process is not supported in source pipeline.");
        return DCAMERA_NOT_FOUND;
    }
    if (pipelineHead_ == nullptr) {
        DHLOGE("The current source pipeline node is empty. Processing failed.");
        return DCAMERA_INIT_ERR;
    }
    if (dataBuffers.empty()) {
        DHLOGE("Source Pipeline Input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }
    if (!isProcess_) {
        DHLOGE("Source Pipeline node occurred error or start destroy.");
        return DCAMERA_DISABLE_PROCESS;
    }

    DHLOGD("Send asynchronous event to process data in source pipeline.");
    std::shared_ptr<PipelineConfig> pipConfigSource = std::make_shared<PipelineConfig>(piplineType_,
        PIPELINE_OWNER, dataBuffers);
    CHECK_AND_RETURN_RET_LOG(pipeEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "pipeEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_PIPELINE_PROCESS_DATA, pipConfigSource, 0);
    pipeEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

void DCameraPipelineSource::DestroyDataProcessPipeline()
{
    DCAMERA_SYNC_TRACE(DCAMERA_SOURCE_DESTORY_PIPELINE);
    DHLOGD("Destroy source data process pipeline start.");
    isProcess_ = false;
    if (pipelineHead_ != nullptr) {
        pipelineHead_->ReleaseProcessNode();
        pipelineHead_ = nullptr;
    }
    pipeEventHandler_ = nullptr;
    processListener_ = nullptr;
    pipNodeRanks_.clear();
    piplineType_ = PipelineType::VIDEO;
    DHLOGD("Destroy source data process pipeline end.");
}

void DCameraPipelineSource::DoProcessData(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGD("Receive asynchronous event then start process data in source pipeline.");
    std::shared_ptr<PipelineConfig> pipelineConfig = event->GetSharedObject<PipelineConfig>();
    if (pipelineConfig == nullptr) {
        DHLOGE("pipeline config is nullptr.");
        OnError(ERROR_PIPELINE_EVENTBUS);
        return;
    }
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers = pipelineConfig->GetDataBuffers();
    if (inputBuffers.empty()) {
        DHLOGE("Receiving process data buffers is empty in source pipeline.");
        OnError(ERROR_PIPELINE_EVENTBUS);
        return;
    }
    pipelineHead_->ProcessData(inputBuffers);
}

void DCameraPipelineSource::OnError(DataProcessErrorType errorType)
{
    DHLOGE("A runtime error occurred in the source pipeline.");
    isProcess_ = false;
    if (processListener_ == nullptr) {
        DHLOGE("The process listener of source pipeline is empty.");
        return;
    }
    processListener_->OnError(errorType);
}

void DCameraPipelineSource::OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult)
{
    DHLOGD("Source pipeline output the processed video buffer.");
    if (processListener_ == nullptr) {
        DHLOGE("The process listener of source pipeline is empty.");
        return;
    }
    processListener_->OnProcessedVideoBuffer(videoResult);
}

int32_t DCameraPipelineSource::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    return DCAMERA_OK;
}

DCameraPipelineSource::DCameraPipelineSrcEventHandler::DCameraPipelineSrcEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, std::shared_ptr<DCameraPipelineSource> pipeSourcePtr)
    : AppExecFwk::EventHandler(runner), pipeSourceWPtr_(pipeSourcePtr)
{
    DHLOGI("Ctor DCameraPipelineSrcEventHandler.");
}

void DCameraPipelineSource::DCameraPipelineSrcEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    CHECK_AND_RETURN_LOG(event == nullptr, "event is nullptr.");
    uint32_t eventId = event->GetInnerEventId();
    auto pipeSrc = pipeSourceWPtr_.lock();
    if (pipeSrc == nullptr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_PIPELINE_PROCESS_DATA:
            pipeSrc->DoProcessData(event);
            break;
        default:
            DHLOGE("event is undefined, id is %{public}d", eventId);
            break;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
