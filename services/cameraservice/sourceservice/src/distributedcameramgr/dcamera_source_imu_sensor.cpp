/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "dcamera_source_imu_sensor.h"
#include "distributed_hardware_log.h"
#include "anonymous_string.h"

namespace OHOS {
namespace DistributedHardware {
FWK_IMPLEMENT_SINGLE_INSTANCE(DCameraSrcImuSensor);

DCameraSrcImuSensor::~DCameraSrcImuSensor()
{}

void DCameraSrcImuSensor::SetInitParam(const std::string& param)
{
    param_ = param;
}
 
std::string DCameraSrcImuSensor::GetInitParam()
{
    return param_;
}

void DCameraSrcImuSensor::SetSrcEis(bool eis)
{
    eis_ = eis;
}

bool DCameraSrcImuSensor::GetSrcEis()
{
    return eis_;
}

void DCameraSrcImuSensor::SetAREnable(std::string devID, bool arEnable)
{
    std::lock_guard<std::mutex> lock(arEnableMutex_);
    arEnableMap_.insert_or_assign(devID, arEnable);
    DHLOGI("DcameraSrcImuSensor set ar enable %{public}d, devId:%{public}s", arEnable, GetAnonyString(devID).c_str());
}

bool DCameraSrcImuSensor::GetAREnable(std::string devID)
{
    std::lock_guard<std::mutex> lock(arEnableMutex_);
    auto it = arEnableMap_.find(devID);
    if (it != arEnableMap_.end()) {
        return it->second;
    }
    return false;
}
}
}
