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
 
#ifndef OHOS_EIS_DATA_PROCESS_H
#define OHOS_EIS_DATA_PROCESS_H
 
#include "abstract_data_process.h"
#include "dcamera_pipeline_source.h"
#include "image_common_type.h"
#include <mutex>
#include <queue>
 
namespace OHOS {
namespace DistributedHardware {
 
 
class EISDataProcess : public AbstractDataProcess {
public:
    explicit EISDataProcess(const std::weak_ptr<DCameraPipelineSource>& callbackPipeSource)
        : callbackPipelineSource_(callbackPipeSource) {}
    ~EISDataProcess() override;
 
    int32_t InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
        VideoConfigParams& processedConfig) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers) override;
    void ReleaseProcessNode() override;
    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier) override;
    int32_t UpdateSettings(const std::shared_ptr<Camera::CameraMetadata> settings) override;
 
private:
    int32_t EISDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers);
 
private:
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    
    std::atomic<bool> isEISProcess_ = false;
};
 
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_EIS_DATA_PROCESS_H
