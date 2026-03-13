/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "eis_data_process.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {

EISDataProcess::~EISDataProcess()
{
    if (isEISProcess_.load()) {
        ReleaseProcessNode();
    }
}

int32_t EISDataProcess::InitNode(const VideoConfigParams& sourceConfig,
    const VideoConfigParams& targetConfig, VideoConfigParams& processedConfig)
{
    DHLOGI("EISDataProcess::InitNode start");
 
    isEISProcess_.store(true);
    DHLOGI("EISDataProcess::InitNode success");
    return DCAMERA_OK;
}

int32_t EISDataProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    if (!isEISProcess_.load()) {
        DHLOGE("EISData node occurred error or start release.");
        return DCAMERA_DISABLE_PROCESS;
    }
    
    if (inputBuffers.empty()) {
        DHLOGE("Input buffers is empty");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("EIS already started");
    return EISDone(inputBuffers);
}

void EISDataProcess::ReleaseProcessNode()
{
    DHLOGI("ReleaseProcessNode start");
    isEISProcess_.store(false);
    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGI("ReleaseProcessNode end");
}

int32_t EISDataProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    return DCAMERA_OK;
}

int32_t EISDataProcess::EISDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    if (outputBuffers.empty() || outputBuffers[0] == nullptr) {
        DHLOGE("Output buffers is empty");
        return DCAMERA_BAD_VALUE;
    }

    if (nextDataProcess_ != nullptr) {
        DHLOGD("Send to the next node of your process for processing.");
        int32_t err = nextDataProcess_->ProcessData(outputBuffers);
        if (err != DCAMERA_OK) {
            DHLOGE("Some node after EISData process failed.");
        }
        return err;
    }

    DHLOGI("EIS deal success!, frameId: %{public}d, frameTimeStamp: %{public}" PRId64 ", initParams: %{public}s,"
       "imuData: %{public}s", outputBuffers[0]->eisInfo_.frameId, outputBuffers[0]->eisInfo_.frameTimeStamp,
       outputBuffers[0]->eisInfo_.initParams.c_str(), outputBuffers[0]->eisInfo_.imuData.c_str());
    DHLOGD("The current node is the last node, and output the processed video buffer.");
    auto pipelineSource = callbackPipelineSource_.lock();
    if (pipelineSource == nullptr) {
        DHLOGE("Pipeline source is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    pipelineSource->OnProcessedVideoBuffer(outputBuffers[0]);

    return DCAMERA_OK;
}

int32_t EISDataProcess::UpdateSettings(const std::shared_ptr<Camera::CameraMetadata> settings)
{
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
