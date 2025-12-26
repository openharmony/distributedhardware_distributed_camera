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

#include "dcamera_pipeline_source.h"

#include "dcamera_hitrace_adapter.h"
#include "distributed_hardware_log.h"
#include "distributed_camera_constants.h"
#include "decode_data_process.h"
#include "fps_controller_process.h"
#include "scale_convert_process.h"
#include <sys/prctl.h>

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
    {
        std::unique_lock<std::mutex> lock(listenerMutex_);
        processListener_ = listener;
    }
    isProcess_ = true;
    return DCAMERA_OK;
}

bool DCameraPipelineSource::IsInRange(const VideoConfigParams& curConfig)
{
    bool isWidthValid = (curConfig.GetWidth() >= MIN_VIDEO_WIDTH && curConfig.GetWidth() <= MAX_VIDEO_WIDTH);
    bool isHeightValid = (curConfig.GetHeight() >= MIN_VIDEO_HEIGHT && curConfig.GetHeight() <= MAX_VIDEO_HEIGHT);
    bool isFrameRateValid = (curConfig.GetFrameRate() >= MIN_FRAME_RATE && curConfig.GetFrameRate() <= MAX_FRAME_RATE);
    return isWidthValid && isHeightValid && isFrameRateValid;
}

void DCameraPipelineSource::InitDCameraPipEvent()
{
    DHLOGD("Init source DCamera pipeline event to asynchronously process data.");
    eventThread_ = std::thread([this]() { this->StartEventHandler(); });
    std::unique_lock<std::mutex> lock(eventMutex_);
    eventCon_.wait(lock, [this] {
        return pipeEventHandler_ != nullptr;
    });
}

void DCameraPipelineSource::StartEventHandler()
{
    prctl(PR_SET_NAME, PIPELINE_SRC_EVENT.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    if (runner == nullptr) {
        DHLOGE("Creat runner failed.");
        return;
    }
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        pipeEventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
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
        CHECK_AND_RETURN_RET_LOG((pipNodeRanks_[i] == nullptr), DCAMERA_BAD_VALUE, "Node is null.");
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
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers = pipConfigSource->GetDataBuffers();
    if (inputBuffers.empty()) {
        DHLOGE("Receiving process data buffers is empty in source pipeline.");
        OnError(ERROR_PIPELINE_EVENTBUS);
        return DCAMERA_BAD_VALUE;
    }
    auto sendFunc = [this, inputBuffers]() mutable {
        int32_t ret = pipelineHead_->ProcessData(inputBuffers);
        DHLOGD("excute ProcessData ret %{public}d.", ret);
    };
    std::unique_lock<std::mutex> lock(eventMutex_);
    CHECK_AND_RETURN_RET_LOG(pipeEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "pipeEventHandler_ is nullptr.");
    pipeEventHandler_->PostTask(sendFunc);
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
    {
        std::unique_lock<std::mutex> lock(eventMutex_);
        if ((pipeEventHandler_ != nullptr) && (pipeEventHandler_->GetEventRunner() != nullptr)) {
            pipeEventHandler_->GetEventRunner()->Stop();
            eventThread_.join();
        }
        pipeEventHandler_ = nullptr;
    }
    {
        std::unique_lock<std::mutex> lock(listenerMutex_);
        processListener_ = nullptr;
    }
    pipNodeRanks_.clear();
    piplineType_ = PipelineType::VIDEO;
    DHLOGD("Destroy source data process pipeline end.");
}

void DCameraPipelineSource::OnError(DataProcessErrorType errorType)
{
    DHLOGE("A runtime error occurred in the source pipeline.");
    isProcess_ = false;
    std::unique_lock<std::mutex> lock(listenerMutex_);
    if (processListener_ == nullptr) {
        DHLOGE("The process listener of source pipeline is empty.");
        return;
    }
    processListener_->OnError(errorType);
}

void DCameraPipelineSource::OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult)
{
    DHLOGD("Source pipeline output the processed video buffer.");
    std::unique_lock<std::mutex> lock(listenerMutex_);
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

int32_t DCameraPipelineSource::UpdateSettings(const std::shared_ptr<Camera::CameraMetadata> settings)
{
    DHLOGI("DCameraPipelineSource::UpdateSettings");
    for (size_t i = 0; i < pipNodeRanks_.size(); i++) {
        CHECK_AND_RETURN_RET_LOG((pipNodeRanks_[i] == nullptr), DCAMERA_BAD_VALUE, "Node is null.");
        pipNodeRanks_[i]->UpdateSettings(settings);
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
