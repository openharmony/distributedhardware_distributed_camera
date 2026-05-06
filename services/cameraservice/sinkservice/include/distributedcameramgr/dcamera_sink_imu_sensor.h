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
#include <mutex>
#include <queue>

#include "dcamera_utils_tools.h"
#include "dhfwk_single_instance.h"
 
namespace OHOS {
namespace DistributedHardware {
struct SensorData {
    int64_t timeStamp = -1;
    double data[3];
};

class DCameraSinkImuSensor {
FWK_DECLARE_SINGLE_INSTANCE_BASE(DCameraSinkImuSensor);
 
public:
    void SaveAccInfo(const SensorData data);
    void SaveGyroInfo(const SensorData data);
    std::string PackedImuData();
    void SetSinkEis(bool eis);
    bool GetSinkEis();
    int64_t GetMonoToBoottimeOffset();
 
private:
    DCameraSinkImuSensor() = default;
    ~DCameraSinkImuSensor();

    std::mutex mutex_;
    std::queue<SensorData> accInfoQueue;
    std::queue<SensorData> gyroInfoQueue;
    bool eis_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DCAMERA_SINK_IMU_SENSOR_H
