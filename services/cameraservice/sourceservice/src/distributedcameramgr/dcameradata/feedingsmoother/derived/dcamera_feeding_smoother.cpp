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
#include "dcamera_feeding_smoother.h"
#include "distributed_hardware_log.h"
#include <memory>
#include "data_buffer.h"
#include "dcamera_utils_tools.h"
#include "feeding_smoother_listener.h"
#include "smoother_constants.h"
namespace OHOS {
namespace DistributedHardware {
void DCameraFeedingSmoother::PrepareSmooth()
{
    SetProcessDynamicBalanceState(false);
    SetBufferTime(SMOOTH_BUFFER_TIME_US);
    SetDynamicBalanceThre(DYNAMIC_BALANCE_THRE);
    SetAverIntervalDiffThre(AVER_INTERVAL_DIFF_THRE_US);
    SetFeedOnceDiffThre(FEED_ONCE_DIFF_THRE_US);
}

void DCameraFeedingSmoother::InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline)
{
    CHECK_AND_RETURN_LOG(dCameraStatistician_ == nullptr, "dCameraStatistician_ is null.");
    int64_t clock = dCameraStatistician_->GetRecvTime() + dCameraStatistician_->GetAverRecv2FeedTime()
        + GetBufferTime();
    SetTimeStampBaseline(timeStampBaseline);
    SetClockBaseline(clock);
}

void DCameraFeedingSmoother::InitTimeStatistician()
{
    if (statistician_ != nullptr) {
        return;
    }
    dCameraStatistician_ = std::make_shared<DCameraTimeStatistician>();
    statistician_ = dCameraStatistician_;
}

int32_t DCameraFeedingSmoother::NotifySmoothFinished(const std::shared_ptr<IFeedableData>& data)
{
    int64_t finishSmoothT = GetNowTimeStampUs();
    std::shared_ptr<DataBuffer> buffer = std::reinterpret_pointer_cast<DataBuffer>(data);
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, NOTIFY_FAILED, "buffer is nullptr.");
    buffer->frameInfo_.timePonit.finishSmooth = finishSmoothT;
    CHECK_AND_RETURN_RET_LOG(dCameraStatistician_ == nullptr, NOTIFY_FAILED, "dCameraStatistician_ is null.");
    dCameraStatistician_->CalWholeProcessTime(buffer);
    if (listener_ == nullptr) {
        DHLOGE("Smoother listener is nullptr.");
        return NOTIFY_FAILED;
    }
    return listener_->OnSmoothFinished(buffer);
}
} // namespace DistributedHardware
} // namespace OHOS