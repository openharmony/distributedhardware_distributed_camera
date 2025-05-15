/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#ifndef DCAMERA_LOW_LATENCY_H
#define DCAMERA_LOW_LATENCY_H

#include "distributed_hardware_fwk_kit.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraLowLatency {
DECLARE_SINGLE_INSTANCE_BASE(DCameraLowLatency);
public:
    int32_t EnableLowLatency();
    int32_t DisableLowLatency();

private:
    DCameraLowLatency() = default;
    ~DCameraLowLatency() = default;
    std::shared_ptr<DistributedHardwareFwkKit> GetDHFwkKit();

private:
    constexpr static int32_t REF_INITIAL = 0;
    constexpr static int32_t REF_NORMAL = 1;
    const std::string DH_TYPE = "dh_type";
    const std::string LOW_LATENCY_ENABLE = "low_latency_enable";
    std::shared_ptr<DistributedHardwareFwkKit> dHFwkKit_;
    std::mutex dHFwkKitMutex_;
    std::atomic<int> refCount_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_LOW_LATENCY_H