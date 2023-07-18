/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_IFEEDING_SMOOTHER_H
#define OHOS_IFEEDING_SMOOTHER_H

#include <condition_variable>
#include <mutex>
#include <queue>
#include <thread>
#include <memory>
#include "feeding_smoother_listener.h"
#include "ifeedable_data.h"
#include "time_statistician.h"
#include "smoother_constants.h"

namespace OHOS {
namespace DistributedHardware {
class IFeedingSmoother {
public:
    virtual ~IFeedingSmoother();
    virtual void PrepareSmooth() = 0;
    virtual void InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline);
    virtual void InitTimeStatistician();
    virtual int32_t NotifySmoothFinished(const std::shared_ptr<IFeedableData>& data);
    virtual void SetClockTime(const int64_t clockTime);

public:
    void PushData(const std::shared_ptr<IFeedableData>& data);
    int32_t StartSmooth();
    int32_t StopSmooth();
    void RegisterListener(const std::shared_ptr<FeedingSmootherListener>& listener);
    void UnregisterListener();
    void SetBufferTime(const int32_t time);
    void SetAverIntervalDiffThre(const uint32_t thre);
    void SetDynamicBalanceThre(const uint8_t thre);
    void SetFeedOnceDiffThre(const uint32_t thre);
    void SetTimeStampBaseline(const int64_t timeStmapBaseline);
    void SetClockBaseline(const int64_t clockBaseline);
    void SetBaselineInitState(const bool state);
    void SetProcessDynamicBalanceState(const bool state);
    void SetTimeInitState(const bool state);
    void SetAdjustSleepFactor(const float factor);
    void SetWaitClockFactor(const float factor);
    void SetTrackClockFactor(const float factor);
    int64_t GetBufferTime();
    int64_t GetClockTime();

private:
    void AdjustSleepTime(const int64_t interval);
    bool CheckIsBaselineInit();
    bool CheckIsTimeInit();
    bool CheckIsProcessInDynamicBalance();
    bool CheckIsProcessInDynamicBalanceOnce();
    void LooperSmooth();
    void RecordTime(const int64_t enterTime, const int64_t timeStamp);
    void SmoothFeeding(const std::shared_ptr<IFeedableData>& data);
    void SyncClock(const int64_t timeStamp, const int64_t timeStampInterval, const int64_t clock);

protected:
    std::queue<std::shared_ptr<IFeedableData>> dataQueue_;
    std::shared_ptr<TimeStatistician> statistician_ = nullptr;
    std::shared_ptr<FeedingSmootherListener> listener_ = nullptr;

private:
    SmoothState state_ = SMOOTH_STOP;
    std::thread smoothThread_;
    std::condition_variable smoothCon_;
    std::condition_variable sleepCon_;
    std::mutex queueMutex_;
    std::mutex sleepMutex_;
    std::mutex stateMutex_;
    std::atomic<bool> isInDynamicBalance_ = true;
    std::atomic<bool> isBaselineInit_ = false;
    std::atomic<bool> isTimeInit_ = false;

    float adjustSleepFactor_ = 0.1;
    float waitClockFactor_ = 0.1;
    float trackClockFactor_ = 0.2;
    uint8_t dynamicBalanceThre_ = 0;
    uint8_t dynamicBalanceCount_ = 0;
    uint32_t averIntervalDiffThre_ = 0;
    uint32_t feedOnceDiffThre_ = 0;
    int64_t bufferTime_ = 0;
    int64_t lastEnterTime_ = 0;
    int64_t lastTimeStamp_ = 0;
    int64_t leaveTime_ = 0;
    int64_t timeStampBaseline_ = 0;
    int64_t clockTime_ = 0;
    int64_t clockBaseline_ = 0;
    int64_t delta_ = 0;
    int64_t sleep_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IFEEDING_SMOOTHER_H