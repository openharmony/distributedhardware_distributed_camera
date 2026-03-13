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

#include "cJSON.h"
#include "dcamera_sink_imu_sensor.h"
#include "distributed_hardware_log.h"
#include "dcamera_utils_tools.h"

namespace OHOS {
namespace DistributedHardware {
FWK_IMPLEMENT_SINGLE_INSTANCE(DCameraSinkImuSensor);
constexpr int32_t IMU_X = 0;
constexpr int32_t IMU_Y = 1;
constexpr int32_t IMU_Z = 2;

DCameraSinkImuSensor::~DCameraSinkImuSensor()
{}

void DCameraSinkImuSensor::SaveAccInfo(const SensorData data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    accInfos.push_back(data);
}

void DCameraSinkImuSensor::SaveGyroInfo(const SensorData data)
{
    std::lock_guard<std::mutex> lock(mutex_);
    gyroInfos.push_back(data);
}

std::string DCameraSinkImuSensor::PackedImuData()
{
    std::lock_guard<std::mutex> lock(mutex_);
    std::string jsonStr;
    cJSON *imuInfo = cJSON_CreateObject();
    if (imuInfo == nullptr) {
        return "";
    }
    cJSON_AddNumberToObject(imuInfo, "exposuretime", DCameraExpoTime::GetInstance().GetExpoTime());
    cJSON* accArray = cJSON_CreateArray();
    if (accArray != nullptr) {
        for (const auto& accData : accInfos) {
            cJSON* accItem = cJSON_CreateObject();
            if (accItem != nullptr) {
                cJSON_AddNumberToObject(accItem, "timestamp", accData.timeStamp);
                cJSON_AddNumberToObject(accItem, "x", accData.data[IMU_X]);
                cJSON_AddNumberToObject(accItem, "y", accData.data[IMU_Y]);
                cJSON_AddNumberToObject(accItem, "z", accData.data[IMU_Z]);
                cJSON_AddItemToArray(accArray, accItem);
            }
        }
        cJSON_AddItemToObject(imuInfo, "imuAccData", accArray);
    }
    accInfos.clear();

    cJSON* gyroArray = cJSON_CreateArray();
    if (gyroArray != nullptr) {
        for (const auto& gyroData : gyroInfos) {
            cJSON* gyroItem = cJSON_CreateObject();
            if (gyroItem != nullptr) {
                cJSON_AddNumberToObject(gyroItem, "timestamp", gyroData.timeStamp);
                cJSON_AddNumberToObject(gyroItem, "x", gyroData.data[IMU_X]);
                cJSON_AddNumberToObject(gyroItem, "y", gyroData.data[IMU_Y]);
                cJSON_AddNumberToObject(gyroItem, "z", gyroData.data[IMU_Z]);
                cJSON_AddItemToArray(gyroArray, gyroItem);
            }
        }
        cJSON_AddItemToObject(imuInfo, "imuGyroData", gyroArray);
    }
    gyroInfos.clear();
    char* data = cJSON_Print(imuInfo);
    if (data == nullptr) {
        cJSON_Delete(imuInfo);
        return "";
    }
    jsonStr = std::string(data);
    cJSON_Delete(imuInfo);
    cJSON_free(data);
    return jsonStr;
}

void DCameraSinkImuSensor::SetSinkEis(bool eis)
{
    eis_ = eis;
}

bool DCameraSinkImuSensor::GetSinkEis()
{
    return eis_;
}
}
}
