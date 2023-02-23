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

#include "dcamera_stream_data_process_producer.h"

#include <chrono>
#include <securec.h>

#include "anonymous_string.h"
#include "dcamera_buffer_handle.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"


namespace OHOS {
namespace DistributedHardware {
DCameraStreamDataProcessProducer::DCameraStreamDataProcessProducer(std::string devId, std::string dhId,
    int32_t streamId, DCStreamType streamType)
    : devId_(devId), dhId_(dhId), streamId_(streamId), streamType_(streamType), eventHandler_(nullptr),
    camHdiProvider_(nullptr)
{
    DHLOGI("DCameraStreamDataProcessProducer Constructor devId %s dhId %s streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    state_ = DCAMERA_PRODUCER_STATE_STOP;
    interval_ = DCAMERA_PRODUCER_ONE_MINUTE_MS / DCAMERA_PRODUCER_FPS_DEFAULT;
}

DCameraStreamDataProcessProducer::~DCameraStreamDataProcessProducer()
{
    DHLOGI("DCameraStreamDataProcessProducer Destructor devId %s dhId %s state: %d streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), state_, streamType_, streamId_);
    if (state_ == DCAMERA_PRODUCER_STATE_START) {
        Stop();
    }
}

void DCameraStreamDataProcessProducer::Start()
{
    DHLOGI("DCameraStreamDataProcessProducer Start producer devId: %s dhId: %s streamType: %d streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider_ == nullptr) {
        DHLOGE("camHdiProvider_ is null.");
    }
    state_ = DCAMERA_PRODUCER_STATE_START;
    if (streamType_ == CONTINUOUS_FRAME) {
        eventThread_ = std::thread(&DCameraStreamDataProcessProducer::StartEvent, this);
        std::unique_lock<std::mutex> lock(eventMutex_);
        eventCon_.wait(lock, [this] {
            return eventHandler_ != nullptr;
        });
        producerThread_ = std::thread(&DCameraStreamDataProcessProducer::LooperContinue, this);
    } else {
        producerThread_ = std::thread(&DCameraStreamDataProcessProducer::LooperSnapShot, this);
    }
}

void DCameraStreamDataProcessProducer::Stop()
{
    DHLOGI("DCameraStreamDataProcessProducer Stop devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        state_ = DCAMERA_PRODUCER_STATE_STOP;
    }
    producerCon_.notify_one();
    if (streamType_ == CONTINUOUS_FRAME) {
        sleepCon_.notify_one();
        eventHandler_->GetEventRunner()->Stop();
        eventThread_.join();
        eventHandler_ = nullptr;
    }
    producerThread_.join();
    camHdiProvider_ = nullptr;
    DHLOGI("DCameraStreamDataProcessProducer Stop end devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

void DCameraStreamDataProcessProducer::FeedStream(const std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGD("DCameraStreamDataProcessProducer FeedStream devId %s dhId %s streamId: %d streamType: %d streamSize: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, streamType_, buffer->Size());
    std::lock_guard<std::mutex> lock(bufferMutex_);
    if (buffers_.size() >= DCAMERA_PRODUCER_MAX_BUFFER_SIZE) {
        DHLOGD("DCameraStreamDataProcessProducer FeedStream OverSize devId %s dhId %s streamType: %d streamSize: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, buffer->Size());
        buffers_.pop_front();
    }
    buffers_.push_back(buffer);
    int64_t time = GetNowTimeStampUs();
    FinetuneBaseline(time);
    CalculateAverFeedInterval(time);
    producerCon_.notify_one();
}

void DCameraStreamDataProcessProducer::StartEvent()
{
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

void DCameraStreamDataProcessProducer::LooperContinue()
{
    DHLOGI("LooperContinue producer devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;

    while (state_ == DCAMERA_PRODUCER_STATE_START) {
        std::shared_ptr<DataBuffer> buffer = nullptr;
        {
            std::unique_lock<std::mutex> lock(bufferMutex_);
            producerCon_.wait(lock, [this] {
                return (!buffers_.empty() || this->state_ == DCAMERA_PRODUCER_STATE_STOP);
            });
            if (state_ == DCAMERA_PRODUCER_STATE_STOP) {
                continue;
            }
            buffer = buffers_.front();
        }
        int64_t timeStamp = 0;
        if (!buffer->FindInt64(TIME_STAMP_US, timeStamp)) {
            DHLOGD("LooperContinue find %s failed.", TIME_STAMP_US.c_str());
        }
        ControlFrameRate(timeStamp);
        {
            std::lock_guard<std::mutex> lock(bufferMutex_);
            buffers_.pop_front();
        }
        auto feedFunc = [this, dhBase, buffer]() {
            FeedStreamToDriver(dhBase, buffer);
        };
        if (eventHandler_ != nullptr) {
            eventHandler_->PostTask(feedFunc);
        }
    }
    DHLOGI("LooperContinue producer end devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

void DCameraStreamDataProcessProducer::LooperSnapShot()
{
    DHLOGI("LooperSnapShot producer devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;

    while (state_ == DCAMERA_PRODUCER_STATE_START) {
        std::shared_ptr<DataBuffer> buffer = nullptr;
        {
            std::unique_lock<std::mutex> lock(bufferMutex_);
            producerCon_.wait(lock, [this] {
                return (!buffers_.empty() || state_ == DCAMERA_PRODUCER_STATE_STOP);
            });
            if (state_ == DCAMERA_PRODUCER_STATE_STOP) {
                continue;
            }

            if (!buffers_.empty()) {
                DHLOGI("LooperSnapShot producer get buffer devId: %s dhId: %s streamType: %d streamId: %d state: %d",
                    GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
                buffer = buffers_.front();
            }
        }

        if (buffer == nullptr) {
            DHLOGI("LooperSnapShot producer get buffer failed devId: %s dhId: %s streamType: %d streamId: %d state: %d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
            continue;
        }
        int32_t ret = FeedStreamToDriver(dhBase, buffer);
        if (ret != DCAMERA_OK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DCAMERA_PRODUCER_RETRY_SLEEP_MS));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(bufferMutex_);
            buffers_.pop_front();
        }
    }
    DHLOGI("LooperSnapShot producer end devId: %s dhId: %s streamType: %d streamId: %d state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

int32_t DCameraStreamDataProcessProducer::FeedStreamToDriver(const DHBase& dhBase,
    const std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGD("LooperFeed devId %s dhId %s streamSize: %d streamType: %d, streamId: %d", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str(), buffer->Size(), streamType_, streamId_);
    if (camHdiProvider_ == nullptr) {
        DHLOGI("camHdiProvider is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    DCameraBuffer sharedMemory;
    int32_t ret = camHdiProvider_->AcquireBuffer(dhBase, streamId_, sharedMemory);
    if (ret != SUCCESS) {
        DHLOGE("AcquireBuffer devId: %s dhId: %s streamId: %d ret: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }

    do {
        ret = CheckSharedMemory(sharedMemory, buffer);
        if (ret != DCAMERA_OK) {
            DHLOGE("CheckSharedMemory failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str());
            break;
        }
        sharedMemory.bufferHandle_->GetBufferHandle()->virAddr =
            DCameraMemoryMap(sharedMemory.bufferHandle_->GetBufferHandle());
        if (sharedMemory.bufferHandle_->GetBufferHandle()->virAddr == nullptr) {
            DHLOGE("mmap failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
        }
        ret = memcpy_s(sharedMemory.bufferHandle_->GetBufferHandle()->virAddr, sharedMemory.size_, buffer->Data(),
            buffer->Size());
        if (ret != EOK) {
            DHLOGE("memcpy_s devId: %s dhId: %s streamId: %d bufSize: %d, addressSize: %d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, buffer->Size(),
                sharedMemory.size_);
            break;
        }
        sharedMemory.size_ = buffer->Size();
    } while (0);

    ret = camHdiProvider_->ShutterBuffer(dhBase, streamId_, sharedMemory);
    if (sharedMemory.bufferHandle_ != nullptr) {
        DCameraMemoryUnmap(sharedMemory.bufferHandle_->GetBufferHandle());
    }
    if (ret != SUCCESS) {
        DHLOGE("ShutterBuffer devId: %s dhId: %s streamId: %d ret: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }
    DHLOGD("LooperFeed end devId %s dhId %s streamSize: %d streamType: %d, streamId: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), buffer->Size(), streamType_, streamId_);
    return ret;
}

int32_t DCameraStreamDataProcessProducer::CheckSharedMemory(const DCameraBuffer& sharedMemory,
    const std::shared_ptr<DataBuffer>& buffer)
{
    if (sharedMemory.bufferHandle_ == nullptr || sharedMemory.bufferHandle_->GetBufferHandle() == nullptr) {
        DHLOGE("bufferHandle read failed devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str());
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    if (buffer->Size() > sharedMemory.size_) {
        DHLOGE("sharedMemory devId: %s dhId: %s streamId: %d bufSize: %d, addressSize: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, buffer->Size(),
            sharedMemory.size_);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    return DCAMERA_OK;
}

void DCameraStreamDataProcessProducer::ControlFrameRate(const int64_t timeStamp)
{
    if (timeStamp == 0) {
        DHLOGD("TimeStamp is zero, just directlly display.");
        return;
    }
    int64_t enterTime = GetNowTimeStampUs();
    InitTime(timeStamp);
    int64_t duration = timeStamp - lastTimeStamp_;
    int64_t render = enterTime - leaveTime_;
    int64_t elapse = enterTime - lastEnterTime_;
    int64_t delta = elapse - sleep_ - render;
    delta_ += delta;
    int64_t clock = timeStampBaseline_ + enterTime - sysTimeBaseline_;
    sleep_ = duration - render;
    DHLOGD("Duration is %ld us, render is %ld us, elapse is %ld us, delta is %ld us, delta_ is %ld us, " +
        "clock is %ld us, sleep is %ld us.", duration, render, elapse, delta, delta_, clock, sleep_);

    ControlDisplay(timeStamp, duration, clock);
    lastTimeStamp_ = timeStamp;
    lastEnterTime_ = enterTime;
    leaveTime_ = GetNowTimeStampUs();
}

void DCameraStreamDataProcessProducer::InitTime(const int64_t timeStamp)
{
    if (lastTimeStamp_ == timeStamp) {
        sysTimeBaseline_ = 0;
        timeStampBaseline_ = 0;
    }
    if (lastTimeStamp_ == 0) {
        lastTimeStamp_ = timeStamp;
    }
    if (sysTimeBaseline_ == 0) {
        sysTimeBaseline_ = GetNowTimeStampUs();
    }
    if (leaveTime_ == 0) {
        leaveTime_ = sysTimeBaseline_;
    }
    if (lastEnterTime_ == 0) {
        lastEnterTime_ = sysTimeBaseline_;
    }
    if (timeStampBaseline_ == 0) {
        timeStampBaseline_ = timeStamp;
        DHLOGD("TimeStamp is %ld us, after initTime sysTimeBaseline is %ld us, timeStampBaseline is %ld us, " +
            "lastTimeStamp is %ld us, leaveTime is %ld us, lastEnterTime is %ld us.", timeStamp, sysTimeBaseline_,
            timeStampBaseline_, lastTimeStamp_, leaveTime_, lastEnterTime_);
    }
}

void DCameraStreamDataProcessProducer::ControlDisplay(const int64_t timeStamp, const int64_t duration,
    const int64_t clock)
{
    AdjustSleep(duration);
    int64_t offset = SyncClock(timeStamp, duration, clock);
    {
        std::unique_lock<std::mutex> lock(sleepMutex_);
        sleepCon_.wait_for(lock, std::chrono::microseconds(sleep_), [this] {
            return (this->state_ == DCAMERA_PRODUCER_STATE_STOP);
        });
        if (state_ == DCAMERA_PRODUCER_STATE_STOP) {
            DHLOGD("Notify to interrupt sleep.");
            return;
        }
    }
    LocateBaseline(timeStamp, duration, offset);
}

/*
 * Adjust sleep time to make it reasonable.
 * Count delta in internal process time consumption, sleep accuracy, etc.
 * The sleep time is adjusted to reduce the offset error when Synchronizing the clock.
 */
void DCameraStreamDataProcessProducer::AdjustSleep(const int64_t duration)
{
    const int64_t adjustThre_ = duration * ADJUST_SLEEP_FACTOR;
    if (delta_ > adjustThre_ && sleep_ > 0) {
        int64_t sleep = sleep_ - adjustThre_;
        delta_ -= (sleep < 0) ? sleep_ : adjustThre_;
        sleep_ = sleep;
        DHLOGD("Delta more than thre, adjust sleep to %ld us.", sleep_);
    } else if (delta_ < -adjustThre_) {
        sleep_ += delta_;
        delta_ = 0;
        DHLOGD("Delta less than negative thre, adjust sleep to %ld us.", sleep_);
    }
}

/*
 * Synchronize the timestamp with the clock.
 * Ahead of the threshold, waiting, behind the threshold, and catching up.
 */
int64_t DCameraStreamDataProcessProducer::SyncClock(const int64_t timeStamp, const int64_t duration,
    const int64_t clock)
{
    const int64_t waitThre_ = duration * WAIT_CLOCK_FACTOR;
    const int64_t trackThre_ = duration * TRACK_CLOCK_FACTOR;
    int64_t offset = timeStamp - sleep_ - clock;
    if (offset > waitThre_ || offset < -trackThre_) {
        sleep_ += offset;
        DHLOGD("Offset is not in the threshold range, adjust sleep to %ld us.", sleep_);
    }
    if (sleep_ < 0) {
        sleep_ = 0;
        DHLOGD("Sleep less than zero, adjust sleep to zero.");
    }
    DHLOGD("Offset is %ld us, sleep is %ld us after syncing clock.", offset, sleep_);
    return offset;
}

/*
 * Locate the time baseline near the time corresponding to the set display buffer size.
 */
void DCameraStreamDataProcessProducer::LocateBaseline(const int64_t timeStamp, const int64_t duration,
    const int64_t offset)
{
    const uint8_t normalSize_ = displayBufferSize_ + 1;
    const int64_t offsetThre_ = duration * OFFSET_FACTOR;
    const int64_t normalSleepThre_ = duration * NORMAL_SLEEP_FACTOR;
    const int64_t abnormalSleepThre_ = duration * ABNORMAL_SLEEP_FACTOR;
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        uint8_t size = buffers_.size();
        DHLOGD("Buffers curSize is %d.", size);
        if (size > normalSize_ && offset > -offsetThre_ && sleep_ > normalSleepThre_) {
            minusCount_++;
        } else {
            minusCount_ = 0;
        }

        if (minusCount_ >= MINUS_THRE) {
            finetuneTime_ = GetNowTimeStampUs();
            needFinetune_.store(true);
            int64_t time = 0;
            buffers_.at(size - normalSize_)->FindInt64(TIME_STAMP_US, time);
            time -= timeStamp;
            sysTimeBaseline_ -= time;
            minusCount_ = 0;
            DHLOGD("MinusCount more than minus thre, minus %ld us.", time);
        }

        if (size == normalSize_ && offset > -offsetThre_ && sleep_ < abnormalSleepThre_) {
            plusCount_++;
        } else {
            plusCount_ = 0;
        }

        if (plusCount_ >= PLUS_THRE) {
            int64_t time = normalSleepThre_ - sleep_;
            sysTimeBaseline_ += time;
            plusCount_ = 0;
            DHLOGD("PlusCount more than plus thre, plus %ld us.", time);
        }
    }
}

void DCameraStreamDataProcessProducer::SetDisplayBufferSize(const uint8_t size)
{
    displayBufferSize_ = size;
}

void DCameraStreamDataProcessProducer::FinetuneBaseline(const int64_t time)
{
    if (!needFinetune_.load()) {
        return;
    }
    int64_t finetuneTime = time - finetuneTime_;
    finetuneTime_ = (finetuneTime > averFeedInterval_) ? (averFeedInterval_ * FINETUNE_TIME_FACTOR) : finetuneTime;
    sysTimeBaseline_ += finetuneTime_;
    needFinetune_.store(false);
    DHLOGD("Finetune baseline %ld us.", finetuneTime_);
}

void DCameraStreamDataProcessProducer::CalculateAverFeedInterval(const int64_t time)
{
    int64_t interval = time - feedTime_;
    if (feedTime_ != 0) {
        intervalCount_ += interval;
        averFeedInterval_ = intervalCount_ / index_;
    }
    feedTime_ = time;
    index_++;
    DHLOGD("AverFeedInterval is %ld us, interval is %ld us.", averFeedInterval_, interval);
}
} // namespace DistributedHardware
} // namespace OHOS
