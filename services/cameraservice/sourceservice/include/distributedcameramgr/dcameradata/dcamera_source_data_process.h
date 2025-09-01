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

#ifndef OHOS_DCAMERA_SOURCE_DATRA_PROCESS_H
#define OHOS_DCAMERA_SOURCE_DATRA_PROCESS_H

#include <mutex>
#include <set>
#include <string>

#include "dcamera_stream_data_process.h"
#include "icamera_source_data_process.h"

#include "v1_1/dcamera_types.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class DCameraSourceDataProcess : public ICameraSourceDataProcess {
public:
    DCameraSourceDataProcess(std::string devId, std::string dhId, DCStreamType streamType);
    ~DCameraSourceDataProcess() override;

    int32_t FeedStream(std::vector<std::shared_ptr<DataBuffer>>& buffers) override;
    int32_t ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos) override;
    int32_t ReleaseStreams(std::vector<int32_t>& streamIds) override;
    int32_t StartCapture(std::shared_ptr<DCCaptureInfo>& captureInfo) override;
    int32_t StopCapture(std::vector<int32_t>& streamIds) override;
    int32_t GetProducerSize() override;
    void GetAllStreamIds(std::vector<int32_t>& streamIds) override;
    int32_t UpdateProducerWorkMode(std::vector<int32_t>& streamIds, const WorkModeParam& param) override;

private:
    void DestroyPipeline();

private:
    std::mutex streamMutex_;
    std::vector<std::shared_ptr<DCameraStreamDataProcess>> streamProcess_;
    std::set<int32_t> streamIds_;
    std::string devId_;
    std::string dhId_;
    DCStreamType streamType_;
    bool isFirstContStream_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOURCE_DATRA_PROCESS_H