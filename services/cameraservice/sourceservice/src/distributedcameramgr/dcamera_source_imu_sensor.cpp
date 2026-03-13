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
}
}