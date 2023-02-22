/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "data_buffer.h"
#include "event_handler.h"
#include "v1_0/id_camera_provider.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_0;
class DCameraStreamDataProcessProducer {
public:
    typedef enum {
        DCAMERA_PRODUCER_STATE_STOP = 0,
        DCAMERA_PRODUCER_STATE_START = 1,
    } DCameraProducerState;

    DCameraStreamDataProcessProducer(std::string devId, std::string dhId, int32_t streamId, DCStreamType streamType);
    ~DCameraStreamDataProcessProducer();
    void Start();
    void Stop();
    void FeedStream(const std::shared_ptr<DataBuffer>& buffer);
    void UpdateInterval(uint32_t fps);

private:
    void StartEvent();
    void LooperContinue();
    void LooperSnapShot();
    int32_t FeedStreamToDriver(const DHBase& dhBase, const std::shared_ptr<DataBuffer>& buffer);
    int32_t CheckSharedMemory(const DCameraBuffer& sharedMemory, const std::shared_ptr<DataBuffer>& buffer);
    void ControlFrameRate(const int64_t timeStamp);
    void InitTime(const int64_t timeStamp);
    void ControlDisplay(const int64_t timeStamp, const int64_t duration, const int64_t clock);
    void AdjustSleep(const int64_t duration);
    int64_t SyncClock(const int64_t timeStamp, const int64_t duration, const int64_t clock);
    void LocateBaseline(const int64_t timeStamp, const int64_t duration, const int64_t offset);
    void SetDisplayBufferSize(const uint8_t size);
    void CalculateAverFeedInterval(const int64_t time);
    void FinetuneBaseline(const int64_t time);

    const uint32_t DCAMERA_PRODUCER_MAX_BUFFER_SIZE = 30;
    const uint32_t DCAMERA_PRODUCER_RETRY_SLEEP_MS = 500;

private:
    std::string devId_;
    std::string dhId_;

    std::thread eventThread_;
    std::thread producerThread_;
    std::condition_variable eventCon_;
    std::condition_variable producerCon_;
    std::condition_variable sleepCon_;
    std::mutex bufferMutex_;
    std::mutex eventMutex_;
    std::mutex sleepMutex_;
    std::deque<std::shared_ptr<DataBuffer>> buffers_;
    DCameraProducerState state_;
    uint32_t interval_;
    int32_t streamId_;
    DCStreamType streamType_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;

    sptr<IDCameraProvider> camHdiProvider_;

    constexpr static float ADJUST_SLEEP_FACTOR = 0.1;
    constexpr static float WAIT_CLOCK_FACTOR = 0.1;
    constexpr static float TRACK_CLOCK_FACTOR = 0.2;
    constexpr static float OFFSET_FACTOR = 2.0;
    constexpr static float NORMAL_SLEEP_FACTOR = 0.8;
    constexpr static float ABNORMAL_SLEEP_FACTOR = 0.2;
    constexpr static float FINETUNE_TIME_FACTOR = 0.5;
    constexpr static uint8_t PLUS_THRE = 10;
    constexpr static uint8_t MINUS_THRE = 20;
    std::atomic<bool> needFinetune_ = false;
    uint8_t minusCount_ = 0;
    uint8_t plusCount_ = 0;
    uint8_t displayBufferSize_ = 0;
    int64_t lastTimeStamp_ = 0;
    int64_t leaveTime_ = 0;
    int64_t lastEnterTime_ = 0;
    int64_t sleep_ = 0;
    int64_t delta_ = 0;
    int64_t timeStampBaseline_ = 0;
    int64_t sysTimeBaseline_ = 0;
    int64_t offset_ = 0;
    int64_t averFeedInterval_ = 0;
    int64_t feedTime_ = 0;
    int64_t intervalCount_ = 0;
    int64_t finetuneTime_ = 0;
    int64_t index_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ICAMERA_SOURCE_DATA_PROCESS_PRODUCER_H
