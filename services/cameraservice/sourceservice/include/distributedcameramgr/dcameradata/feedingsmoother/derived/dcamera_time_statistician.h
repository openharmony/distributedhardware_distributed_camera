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

#ifndef OHOS_DCAMERA_TIME_STATISTICIAN_H
#define OHOS_DCAMERA_TIME_STATISTICIAN_H

#include "ifeedable_data.h"
#include "time_statistician.h"
#include "data_buffer.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraTimeStatistician : public TimeStatistician {
public:
    virtual ~DCameraTimeStatistician() override = default;

public:
    virtual void CalProcessTime(const std::shared_ptr<IFeedableData>& data) override;
    void CalWholeProcessTime(const std::shared_ptr<DataBuffer>& data);
public:
    int64_t GetAverEncodeTime();
    int64_t GetAverTransTime();
    int64_t GetAverDecodeTime();
    int64_t GetAverDecode2ScaleTime();
    int64_t GetAverScaleTime();
    int64_t GetAverRecv2FeedTime();
    int64_t GetAverSmoothTime();
    int64_t GetRecv2FeedTime();
    int64_t GetAverSourceTime();
    int64_t GetAverWholeTime();
    int64_t GetRecvTime();
    void SetRecvTime(const int64_t recvTime);
    void SetFrameIndex(const int32_t index);
    int64_t CalAverValue(int64_t& value, int64_t& valueSum);

private:
    int32_t frameIndex_ = -1;
    int64_t averEncodeTime_ = 0;
    int64_t encodeTimeSum_ = 0;
    int64_t averTransTime_ = 0;
    int64_t transTimeSum_ = 0;
    int64_t averDecodeTime_ = 0;
    int64_t decodeTimeSum_ = 0;
    int64_t averDecode2ScaleTime_ = 0;
    int64_t decode2ScaleTimeSum_ = 0;
    int64_t averScaleTime_ = 0;
    int64_t scaleTimeSum_ = 0;
    int64_t averSmoothTime_ = 0;
    int64_t smoothTimeSum_ = 0;
    int64_t averRecv2FeedTime_ = 0;
    int64_t recv2FeedTimeSum_ = 0;
    int64_t averSourceTime_ = 0;
    int64_t sourceTimeSum_ = 0;
    int64_t averWholeTime_ = 0;
    int64_t wholeTimeSum_ = 0;
    int64_t recvTime_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_TIME_STATISTICIAN_H