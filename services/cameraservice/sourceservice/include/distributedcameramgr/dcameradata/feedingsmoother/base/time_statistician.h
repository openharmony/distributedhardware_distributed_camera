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

#ifndef OHOS_TIME_STATISTICIAN_H
#define OHOS_TIME_STATISTICIAN_H

#include "ifeedable_data.h"
#include <memory>
namespace OHOS {
namespace DistributedHardware {
class TimeStatistician {
public:
    virtual ~TimeStatistician() = default;
    virtual void CalProcessTime(const std::shared_ptr<IFeedableData>& data);
    void CalAverFeedInterval(const int64_t feedTime);
    void CalAverTimeStampInterval(const int64_t timeStamp);

public:
    int64_t GetAverFeedInterval();
    int64_t GetAverTimeStampInterval();
    int64_t GetFeedInterval();
    int64_t GetTimeStampInterval();

public:
    uint32_t feedIndex_ = 0;
    int64_t averFeedInterval_ = 0;
    int64_t lastFeedTime_ = 0;
    int64_t feedTime_ = 0;
    int64_t feedIntervalSum_ = 0;
    int64_t feedInterval_ = 0;

    uint32_t timeStampIndex_ = 0;
    int64_t averTimeStampInterval_ = 0;
    int64_t lastTimeStamp_ = 0;
    int64_t timeStamp_ = 0;
    int64_t timeStampIntervalSum_ = 0;
    int64_t timeStampInterval_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_TIME_STATISTICIAN_H