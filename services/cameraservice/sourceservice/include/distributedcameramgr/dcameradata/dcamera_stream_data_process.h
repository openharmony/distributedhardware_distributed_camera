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

#ifndef OHOS_DCAMERA_STREAM_DATA_PROCESS_H
#define OHOS_DCAMERA_STREAM_DATA_PROCESS_H

#include <mutex>
#include <set>

#include "data_buffer.h"
#include "icamera_source_data_process.h"
#include "idata_process_pipeline.h"
#include "image_common_type.h"
#include "v1_1/dcamera_types.h"

#include "dcamera_stream_data_process_producer.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class DCameraStreamDataProcess : public std::enable_shared_from_this<DCameraStreamDataProcess> {
public:
    DCameraStreamDataProcess(std::string devId, std::string dhId, DCStreamType streamType);
    ~DCameraStreamDataProcess();
    void FeedStream(std::shared_ptr<DataBuffer>& buffer);
    void ConfigStreams(std::shared_ptr<DCameraStreamConfig>& dstConfig, std::set<int32_t>& streamIds);
    void ReleaseStreams(std::set<int32_t>& streamIds);
    void StartCapture(std::shared_ptr<DCameraStreamConfig>& srcConfig, std::set<int32_t>& streamIds);
    void StopCapture(std::set<int32_t>& streamIds);
    void GetAllStreamIds(std::set<int32_t>& streamIds);
    int32_t GetProducerSize();

    void OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult);
    void OnError(const DataProcessErrorType errorType);
    void DestroyPipeline();
    int32_t UpdateProducerWorkMode(std::vector<int32_t>& streamIds, const WorkModeParam& param);
    int32_t UpdateSettings(const std::vector<std::shared_ptr<DCameraSettings>>& settings);

private:
    void FeedStreamToSnapShot(const std::shared_ptr<DataBuffer>& buffer);
    void FeedStreamToContinue(const std::shared_ptr<DataBuffer>& buffer);
    void CreatePipeline();
    VideoCodecType GetPipelineCodecType(DCEncodeType encodeType);
    Videoformat GetPipelineFormat(int32_t format);

private:
    std::string devId_;
    std::string dhId_;
    DCStreamType streamType_;

    std::mutex pipelineMutex_;
    std::mutex producerMutex_;

    std::set<int32_t> streamIds_;
    std::shared_ptr<DCameraStreamConfig> srcConfig_;
    std::shared_ptr<DCameraStreamConfig> dstConfig_;
    std::shared_ptr<IDataProcessPipeline> pipeline_;
    std::shared_ptr<DataProcessListener> listener_;
    std::map<uint32_t, std::shared_ptr<DCameraStreamDataProcessProducer>> producers_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_STREAM_DATA_PROCESS_H
