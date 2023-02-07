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

#ifndef DCAMERA_SOFTBUS_LATENCY_H
#define DCAMERA_SOFTBUS_LATENCY_H

#include <string>
#include <mutex>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSoftbusLatency {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSoftbusLatency);
public:
    int32_t StartSoftbusTimeSync(const std::string& devId);
    int32_t StopSoftbusTimeSync();
    void SetTimeSyncInfo(const int32_t microsecond);
    int32_t GetTimeSyncInfo();
private:
    DCameraSoftbusLatency() = default;
    ~DCameraSoftbusLatency() = default;

private:
    std::string networkId_;
    int32_t microsecond_ = 0;
    constexpr static int32_t REF_INITIAL = 0;
    constexpr static int32_t REF_NORMAL = 1;
    std::atomic<int> refCount_ = 0;
    std::mutex micLock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DCAMERA_SOFTBUS_LATENCY_H