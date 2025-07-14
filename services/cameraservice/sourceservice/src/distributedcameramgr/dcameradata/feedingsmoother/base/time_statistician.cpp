/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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
#include "time_statistician.h"
#include "distributed_hardware_log.h"
#include "dcamera_utils_tools.h"

namespace OHOS {
namespace DistributedHardware {
void TimeStatistician::CalProcessTime(const std::shared_ptr<IFeedableData>& data)
{
    CHECK_AND_RETURN_LOG(data == nullptr, "data is nullptr");
    int64_t feedTime  = GetNowTimeStampUs();
    int64_t timeStamp = data->GetTimeStamp();
    CalAverFeedInterval(feedTime);
    CalAverTimeStampInterval(timeStamp);
}

void TimeStatistician::CalAverFeedInterval(const int64_t feedTime)
{
    feedTime_ = feedTime;
    feedInterval_ = feedTime_ - lastFeedTime_;
    if (lastFeedTime_ == 0) {
        lastFeedTime_ = feedTime_;
        return;
    }
    feedIndex_++;
    if (feedIntervalSum_ > INT64_MAX - feedInterval_) {
        DHLOGE("feedIntervalSum_ overflow");
        return;
    }
    feedIntervalSum_ += feedInterval_;
    averFeedInterval_ = feedIntervalSum_ / static_cast<int64_t>(feedIndex_);
    lastFeedTime_ = feedTime_;
}

void TimeStatistician::CalAverTimeStampInterval(const int64_t timeStamp)
{
    timeStamp_ = timeStamp;
    timeStampInterval_ = timeStamp_ - lastTimeStamp_;
    if (lastTimeStamp_ == 0) {
        lastTimeStamp_ = timeStamp_;
        return;
    }
    timeStampIndex_++;
    if (timeStampIntervalSum_ > INT64_MAX - timeStampInterval_) {
        DHLOGE("timeStampIntervalSum_ overflow");
        return;
    }
    timeStampIntervalSum_ += timeStampInterval_;
    averTimeStampInterval_ = timeStampIntervalSum_ / static_cast<int64_t>(timeStampIndex_);
    lastTimeStamp_ = timeStamp_;
}

int64_t TimeStatistician::GetAverFeedInterval()
{
    return averFeedInterval_;
}

int64_t TimeStatistician::GetAverTimeStampInterval()
{
    return averTimeStampInterval_;
}

int64_t TimeStatistician::GetFeedInterval()
{
    return feedInterval_;
}

int64_t TimeStatistician::GetTimeStampInterval()
{
    return timeStampInterval_;
}
} // namespace DistributedHardware
} // namespace OHOS