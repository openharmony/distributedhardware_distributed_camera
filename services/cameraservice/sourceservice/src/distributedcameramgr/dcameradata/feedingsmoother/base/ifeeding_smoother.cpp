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
#include "ifeeding_smoother.h"
#include "distributed_camera_constants.h"
#include <sys/prctl.h>
#include "dcamera_utils_tools.h"
#include <cstdlib>
#include "distributed_hardware_log.h"
#include "smoother_constants.h"

namespace OHOS {
namespace DistributedHardware {
IFeedingSmoother::~IFeedingSmoother()
{
    std::lock_guard<std::mutex> lock(stateMutex_);
    if (state_ == SMOOTH_START) {
        StopSmooth();
    }
}

void IFeedingSmoother::PushData(const std::shared_ptr<IFeedableData>& data)
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (state_ == SMOOTH_STOP) {
            DHLOGD("Smoother stop, push data failed.");
            return;
        }
    }
    {
        std::lock_guard<std::mutex> lock(queueMutex_);
        dataQueue_.push(data);
    }
    if (statistician_ != nullptr) {
        statistician_->CalProcessTime(data);
    }
    smoothCon_.notify_one();
}

int32_t IFeedingSmoother::StartSmooth()
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (state_ == SMOOTH_START) {
            DHLOGD("Smoother is started.");
            return SMOOTH_IS_STARTED;
        }
        state_ = SMOOTH_START;
    }
    InitTimeStatistician();
    PrepareSmooth();
    smoothThread_ = std::thread([this]() { this->LooperSmooth(); });
    return SMOOTH_SUCCESS;
}

void IFeedingSmoother::LooperSmooth()
{
    prctl(PR_SET_NAME, LOOPER_SMOOTH.c_str());
    while (state_ == SMOOTH_START) {
        std::shared_ptr<IFeedableData> data = nullptr;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            smoothCon_.wait(lock, [this] {
                return (!dataQueue_.empty() || this->state_ == SMOOTH_STOP);
            });
            if (state_ == SMOOTH_STOP) {
                continue;
            }
            data = dataQueue_.front();
        }
        SmoothFeeding(data);
        int32_t ret = NotifySmoothFinished(data);
        if (ret == NOTIFY_FAILED) {
            DHLOGD("Smoother listener notify producer failed.");
            return;
        }
        {
            std::lock_guard<std::mutex> lock(queueMutex_);
            dataQueue_.pop();
        }
    }
}

void IFeedingSmoother::SmoothFeeding(const std::shared_ptr<IFeedableData>& data)
{
    CHECK_AND_RETURN_LOG(data == nullptr, "data is nullptr");
    int64_t enterTime = GetNowTimeStampUs();
    SetClockTime(enterTime);
    int64_t timeStamp = data->GetTimeStamp();
    if (timeStamp == 0) {
        return;
    }
    if (!CheckIsProcessInDynamicBalance() || !CheckIsTimeInit()) {
        RecordTime(enterTime, timeStamp);
        return;
    }

    if (!CheckIsBaselineInit()) {
        InitBaseline(timeStamp, clockTime_);
    }
    int64_t interval = timeStamp - lastTimeStamp_;
    int64_t elapse = enterTime - leaveTime_;
    int64_t render = enterTime - lastEnterTime_;
    int64_t delta = render - sleep_ - elapse;
    delta_ += delta;
    int64_t clock = timeStampBaseline_ + clockTime_ - clockBaseline_;
    sleep_ = interval - elapse;
    AdjustSleepTime(interval);
    SyncClock(timeStamp, interval, clock);
    {
        std::unique_lock<std::mutex> lock(sleepMutex_);
        sleepCon_.wait_for(lock, std::chrono::microseconds(sleep_), [this] {
            return (this->state_ == SMOOTH_STOP);
        });
        if (state_ == SMOOTH_STOP) {
            DHLOGD("Notify to interrupt sleep.");
            return;
        }
    }
    RecordTime(enterTime, timeStamp);
}

bool IFeedingSmoother::CheckIsProcessInDynamicBalance()
{
    if (isInDynamicBalance_.load()) {
        return true;
    }
    if (CheckIsProcessInDynamicBalanceOnce()) {
        dynamicBalanceCount_++;
    } else {
        dynamicBalanceCount_ = 0;
    }
    if (dynamicBalanceCount_ >= dynamicBalanceThre_) {
        isInDynamicBalance_.store(true);
        return true;
    }
    return false;
}

bool IFeedingSmoother::CheckIsProcessInDynamicBalanceOnce()
{
    if (statistician_ == nullptr) {
        return false;
    }
    int64_t feedInterval = statistician_->GetFeedInterval();
    int64_t averFeedInterval = statistician_->GetAverFeedInterval();
    int64_t averTimeStamapInterval = statistician_->GetAverTimeStampInterval();
    int64_t averIntervalDiff = averFeedInterval - averTimeStamapInterval;
    int64_t feedOnceDiff = feedInterval - averFeedInterval;
    return (averFeedInterval != 0) && (averTimeStamapInterval != 0) &&
        (llabs(averIntervalDiff) < averIntervalDiffThre_) && (llabs(feedOnceDiff) < feedOnceDiffThre_);
}

bool IFeedingSmoother::CheckIsBaselineInit()
{
    return isBaselineInit_.load();
}

bool IFeedingSmoother::CheckIsTimeInit()
{
    return isTimeInit_.load();
}

void IFeedingSmoother::InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline)
{
    SetTimeStampBaseline(timeStampBaseline);
    SetClockBaseline(clockBaseline + bufferTime_);
    isBaselineInit_.store(true);
}

int32_t IFeedingSmoother::NotifySmoothFinished(const std::shared_ptr<IFeedableData>& data)
{
    if (listener_ == nullptr) {
        DHLOGE("Smoother listener is nullptr.");
        return NOTIFY_FAILED;
    }
    return listener_->OnSmoothFinished(data);
}

void IFeedingSmoother::AdjustSleepTime(const int64_t interval)
{
    const int64_t adjustThre = interval * adjustSleepFactor_;
    if (delta_ > adjustThre && sleep_ > 0) {
        int64_t sleep = sleep_ - adjustThre;
        delta_ -= (sleep < 0) ? sleep_ : adjustThre;
        sleep_ = sleep;
        DHLOGD("Delta more than thre, adjust sleep to %{public}" PRId64" us.", sleep_);
    } else if (delta_ < -adjustThre) {
        sleep_ += delta_;
        delta_ = 0;
        DHLOGD("Delta less than negative thre, adjust sleep to %{public}" PRId64" us.", sleep_);
    }
}

void IFeedingSmoother::SyncClock(const int64_t timeStamp, const int64_t interval, const int64_t clock)
{
    const int64_t waitThre = interval * waitClockFactor_;
    const int64_t trackThre = interval * trackClockFactor_;
    int64_t offset = timeStamp - sleep_ - clock;
    if (offset > waitThre || offset < -trackThre) {
        sleep_ += offset;
        DHLOGD("Offset is not in the threshold range, adjust sleep to %{public}" PRId64" us.", sleep_);
    }
    if (sleep_ < 0) {
        sleep_ = 0;
        DHLOGD("Sleep less than zero, adjust sleep to zero.");
    }
    DHLOGD("Offset is %{public}" PRId64" us, sleep is %{public}" PRId64" us after syncing clock.", offset, sleep_);
}

void IFeedingSmoother::RecordTime(const int64_t enterTime, const int64_t timeStamp)
{
    lastEnterTime_ = enterTime;
    lastTimeStamp_ = timeStamp;
    leaveTime_ = GetNowTimeStampUs();
    isTimeInit_.store(true);
}

int32_t IFeedingSmoother::StopSmooth()
{
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        if (state_ == SMOOTH_STOP) {
            DHLOGD("Smooth is stoped.");
            return SMOOTH_IS_STOPED;
        }
        state_ = SMOOTH_STOP;
    }
    smoothCon_.notify_one();
    sleepCon_.notify_one();
    if (smoothThread_.joinable()) {
        smoothThread_.join();
    }
    statistician_ = nullptr;
    UnregisterListener();

    std::queue<std::shared_ptr<IFeedableData>>().swap(dataQueue_);
    DHLOGD("Stop smooth success.");
    return SMOOTH_SUCCESS;
}

void IFeedingSmoother::InitTimeStatistician()
{
    if (statistician_ != nullptr) {
        return;
    }
    statistician_ = std::make_shared<TimeStatistician>();
}

void IFeedingSmoother::RegisterListener(const std::shared_ptr<FeedingSmootherListener>& listener)
{
    listener_ = listener;
}

void IFeedingSmoother::UnregisterListener()
{
    listener_ = nullptr;
}

void IFeedingSmoother::SetBufferTime(const int32_t time)
{
    bufferTime_ = time;
}

void IFeedingSmoother::SetDynamicBalanceThre(const uint8_t thre)
{
    dynamicBalanceThre_ = thre;
}

void IFeedingSmoother::SetProcessDynamicBalanceState(const bool state)
{
    isInDynamicBalance_.store(state);
}

void IFeedingSmoother::SetAverIntervalDiffThre(const uint32_t thre)
{
    averIntervalDiffThre_ = thre;
}

void IFeedingSmoother::SetFeedOnceDiffThre(const uint32_t thre)
{
    feedOnceDiffThre_ = thre;
}

void IFeedingSmoother::SetTimeStampBaseline(const int64_t timeStmapBaseline)
{
    timeStampBaseline_ = timeStmapBaseline;
}

void IFeedingSmoother::SetClockBaseline(const int64_t clockBaseline)
{
    clockBaseline_ = clockBaseline;
}

int64_t IFeedingSmoother::GetBufferTime()
{
    return bufferTime_;
}

int64_t IFeedingSmoother::GetClockTime()
{
    return clockTime_;
}

void IFeedingSmoother::SetClockTime(const int64_t clockTime)
{
    clockTime_ = clockTime;
}

void IFeedingSmoother::SetAdjustSleepFactor(const float factor)
{
    adjustSleepFactor_ = factor;
}

void IFeedingSmoother::SetWaitClockFactor(const float factor)
{
    waitClockFactor_ = factor;
}

void IFeedingSmoother::SetTrackClockFactor(const float factor)
{
    trackClockFactor_ = factor;
}
} // namespace DistributedHardware
} // namespace OHOS