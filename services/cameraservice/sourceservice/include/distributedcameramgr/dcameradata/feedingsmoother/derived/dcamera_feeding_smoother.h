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

#ifndef OHOS_DCAMERA_FEEDING_SMOOTHER_H
#define OHOS_DCAMERA_FEEDING_SMOOTHER_H
#include "ifeeding_smoother.h"
#include "dcamera_time_statistician.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraFeedingSmoother : public IFeedingSmoother {
public:
    virtual ~DCameraFeedingSmoother() override = default;
    virtual void PrepareSmooth() override;
    virtual void InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline) override;
    virtual void InitTimeStatistician() override;
    virtual int32_t NotifySmoothFinished(const std::shared_ptr<IFeedableData>& data) override;

private:
    constexpr static uint8_t DYNAMIC_BALANCE_THRE = 3;
    constexpr static int32_t SMOOTH_BUFFER_TIME_US = 20000;
    constexpr static uint32_t AVER_INTERVAL_DIFF_THRE_US = 2000;
    constexpr static uint32_t FEED_ONCE_DIFF_THRE_US = 10000;
    std::shared_ptr<DCameraTimeStatistician> dCameraStatistician_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_FEEDING_SMOOTHER_H