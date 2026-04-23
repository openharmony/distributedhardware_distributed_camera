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

#ifndef DCAMERA_SINK_IMU_SENSOR_H
#define DCAMERA_SINK_IMU_SENSOR_H

#include <string>
#include <vector>
#include <map>
#include <mutex>
#include "dhfwk_single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSrcImuSensor {
FWK_DECLARE_SINGLE_INSTANCE_BASE(DCameraSrcImuSensor);

public:
    void SetInitParam(const std::string& param);
    std::string GetInitParam();
    void SetSrcEis(bool eis);
    bool GetSrcEis();
    void SetAREnable(std::string devID, bool arEnable);
    bool GetAREnable(std::string devID);

private:
    DCameraSrcImuSensor() = default;
    ~DCameraSrcImuSensor();

    std::string param_;
    bool eis_ = false;
    std::map<std::string, bool> arEnableMap_;
    std::mutex arEnableMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DCAMERA_SINK_IMU_SENSOR_H
