/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_PIPELINE_SINK_H
#define OHOS_DCAMERA_PIPELINE_SINK_H

#include <memory>
#include <vector>

#include "data_buffer.h"
#include "image_common_type.h"
#include "distributed_camera_errno.h"
#include "idata_process_pipeline.h"
#include "abstract_data_process.h"
#include "data_process_listener.h"

namespace OHOS {
namespace DistributedHardware {
class EncodeDataProcess;

class DCameraPipelineSink : public IDataProcessPipeline, public std::enable_shared_from_this<DCameraPipelineSink> {
public:
    ~DCameraPipelineSink() override;

    int32_t CreateDataProcessPipeline(PipelineType piplineType, const VideoConfigParams& sourceConfig,
        const VideoConfigParams& targetConfig, const std::shared_ptr<DataProcessListener>& listener) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& dataBuffers) override;
    void DestroyDataProcessPipeline() override;

    void OnError(DataProcessErrorType errorType);
    void OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult);

    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier) override;

private:
    bool IsInRange(const VideoConfigParams& curConfig);
    int32_t InitDCameraPipNodes(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig);

private:
    const static std::string PIPELINE_OWNER;
    constexpr static int32_t MIN_FRAME_RATE = 0;
    constexpr static int32_t MAX_FRAME_RATE = 30;
    constexpr static int32_t MIN_VIDEO_WIDTH = 320;
    constexpr static int32_t MIN_VIDEO_HEIGHT = 240;
    constexpr static int32_t MAX_VIDEO_WIDTH = 1920;
    constexpr static int32_t MAX_VIDEO_HEIGHT = 1080;

    std::shared_ptr<DataProcessListener> processListener_ = nullptr;
    std::shared_ptr<AbstractDataProcess> pipelineHead_ = nullptr;

    std::atomic<bool> isProcess_ = false;
    PipelineType piplineType_ = PipelineType::VIDEO;
    std::vector<std::shared_ptr<AbstractDataProcess>> pipNodeRanks_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_PIPELINE_SINK_H
