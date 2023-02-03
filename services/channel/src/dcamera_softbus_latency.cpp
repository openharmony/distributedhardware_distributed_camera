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

#include "dcamera_softbus_latency.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_softbus_adapter.h"
#include "softbus_bus_center.h"
#include "softbus_common.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraSoftbusLatency);

constexpr static int32_t MICROSECONDS = 1000;

static void OnTimeSyncResult(const TimeSyncResultInfo *info, int32_t retCode)
{
    int32_t microsecond = info->result.millisecond * MICROSECONDS + info->result.microsecond;
    DHLOGD("DCameraSoftbusLatency OnTimeSyncResult retcode %d, millisecond: %d, microsecond %d microsecond_ %d",
        retCode, info->result.millisecond, info->result.microsecond, microsecond);
    DCameraSoftbusLatency::GetInstance().SetTimeSyncInfo(microsecond);
}

int32_t DCameraSoftbusLatency::StartSoftbusTimeSync(const std::string& devId)
{
    DHLOGI("StartSoftbusTimeSync latency start.");
    if (refCount_ > REF_INITIAL) {
        refCount_++;
        DHLOGD("No need to start time offset, refCount just plus one and now is: %d.", refCount_.load());
        return DCAMERA_OK;
    }
    ITimeSyncCb timeSyncCb;
    timeSyncCb.onTimeSyncResult = OnTimeSyncResult;
    networkId_ = devId;
    int32_t ret = StartTimeSync(DCAMERA_PKG_NAME.c_str(), networkId_.c_str(), LOW_ACCURACY, NORMAL_PERIOD,
        &timeSyncCb);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusLatency:: StartSoftbusTimeSync failed networkId %s", networkId_.c_str());
    }
    DHLOGI("DCameraSoftbusLatency:: StartSoftbusTimeSync success ");
    return DCAMERA_OK;
}

int32_t DCameraSoftbusLatency::StopSoftbusTimeSync()
{
    DHLOGI("DCameraSoftbusLatency::StopSoftbusTimeSync start.");
    if (refCount_ == REF_INITIAL) {
        DHLOGD("No need to stop time offset, refCount is zero.");
        return DCAMERA_OK;
    }
    if (refCount_ > REF_NORMAL) {
        refCount_--;
        DHLOGD("No need to stop time offset, refCount just minus one and now is: %d.", refCount_.load());
        return DCAMERA_OK;
    }
    int32_t ret = StopTimeSync(DCAMERA_PKG_NAME.c_str(), networkId_.c_str());
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusLatency:: StopSoftbusTimeSync failed ret:%d", ret);
    }
    refCount_--;
    return DCAMERA_OK;
}

void DCameraSoftbusLatency::SetTimeSyncInfo(const int32_t microsecond)
{
    std::lock_guard<std::mutex> lock(micLock_);
    microsecond_ = microsecond;
}

int32_t DCameraSoftbusLatency::GetTimeSyncInfo()
{
    std::lock_guard<std::mutex> lock(micLock_);
    return microsecond_;
}
} // namespace DistributedHardware
}