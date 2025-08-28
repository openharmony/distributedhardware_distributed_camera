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

#ifndef OHOS_ICAMERA_SOURCE_DATA_PROCESS_PRODUCER_H
#define OHOS_ICAMERA_SOURCE_DATA_PROCESS_PRODUCER_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <deque>
#include <thread>
#include <atomic>
#include <chrono>
#include <ashmem.h>

#include "data_buffer.h"
#include "event_handler.h"
#include "v1_1/id_camera_provider.h"
#include "dcamera_feeding_smoother.h"
#include "idistributed_camera_source.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class DCameraStreamDataProcessProducer : public IFeedableDataProducer,
    public std::enable_shared_from_this<DCameraStreamDataProcessProducer> {
public:
    typedef enum {
        DCAMERA_PRODUCER_STATE_STOP = 0,
        DCAMERA_PRODUCER_STATE_START = 1,
    } DCameraProducerState;

    struct SyncSharedData {
        volatile int lock;
        uint64_t audio_current_pts;
        uint64_t audio_update_clock;
        float audio_speed;
        uint64_t video_current_pts;
        uint64_t video_update_clock;
        float video_speed;
        uint64_t sync_strategy;
        bool reset;
    };

    DCameraStreamDataProcessProducer(std::string devId, std::string dhId, int32_t streamId, DCStreamType streamType);
    ~DCameraStreamDataProcessProducer();
    void Start();
    void Stop();
    void FeedStream(const std::shared_ptr<DataBuffer>& buffer);
    virtual void OnSmoothFinished(const std::shared_ptr<IFeedableData>& data) override;
    void UpdateProducerWorkMode(const WorkModeParam& param);

private:
    void StartEvent();
    void LooperSnapShot();
    int32_t FeedStreamToDriver(const DHBase& dhBase, const std::shared_ptr<DataBuffer>& buffer);
    int32_t CheckSharedMemory(const DCameraBuffer& sharedMemory, const std::shared_ptr<DataBuffer>& buffer);
    void WritePtsAndAddBuffer(const std::shared_ptr<DataBuffer>& buffer);
    void SyncVideoThread();
    bool WaitForVideoFrame(std::shared_ptr<DataBuffer>& buffer);
    int32_t SyncVideoFrame(uint64_t videoPtsUs);
    void UpdateVideoClock(uint64_t videoPtsUs);

    const uint32_t DCAMERA_PRODUCER_MAX_BUFFER_SIZE = 30;
    const uint32_t DCAMERA_PRODUCER_RETRY_SLEEP_MS = 500;
    const uint32_t DCAMERA_MAX_SYNC_BUFFER_SIZE = 10;
    const uint32_t DCAMERA_SYNC_WATERMARK = 1;
    const uint32_t DCAMERA_SYNC_TIME_INTERVAL = 33;
    const uint32_t DCAMERA_NS_TO_MS = 1000000;
    const uint32_t DCAMERA_US_TO_MS = 1000;
    const uint32_t DCAMERA_TIME_DIFF_MAX = 5;
    const int32_t DCAMERA_TIME_DIFF_MIN = -80;

private:
    std::string devId_;
    std::string dhId_;

    std::thread eventThread_;
    std::thread producerThread_;
    std::condition_variable eventCon_;
    std::condition_variable producerCon_;
    std::mutex bufferMutex_;
    std::mutex eventMutex_;
    std::deque<std::shared_ptr<DataBuffer>> buffers_;
    DCameraProducerState state_;
    uint32_t interval_;
    uint32_t photoCount_;
    int32_t streamId_;
    DCStreamType streamType_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;

    sptr<IDCameraProvider> camHdiProvider_;
    std::unique_ptr<IFeedingSmoother> smoother_ = nullptr;
    std::shared_ptr<FeedingSmootherListener> smootherListener_ = nullptr;

    std::thread syncThread_;
    std::atomic<bool> syncRunning_;
    std::deque<std::shared_ptr<DataBuffer>> syncBufferQueue_;
    std::mutex syncBufferMutex_;
    std::condition_variable syncBufferCond_;
    std::atomic<bool> isFirstFrame_;
    std::chrono::steady_clock::time_point startTime_;
    WorkModeParam workModeParam_; // Audio-video synchronization fwk transfer structure
    std::mutex workModeParamMtx_;
    sptr<Ashmem> syncMem_ = nullptr; // Shared memory
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ICAMERA_SOURCE_DATA_PROCESS_PRODUCER_H
