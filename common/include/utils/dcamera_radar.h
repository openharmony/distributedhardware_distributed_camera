/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_COMMON_INCLUDE_UTILS_DCAMERA_RADAR_H
#define OHOS_DISTRIBUTED_CAMERA_COMMON_INCLUDE_UTILS_DCAMERA_RADAR_H

#include <cstdint>
#include <chrono>
#include <string>
#include <vector>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
const std::string ORG_PKG_NAME = "ohos.dhardware.dcamera";
const std::string DSOFTBUS_PKG_NAME = "dsoftbus";
const std::string ORG_PKG = "ORG_PKG";
const std::string FUNC = "FUNC";
const std::string BIZ_SCENE = "BIZ_SCENE";
const std::string BIZ_STAGE = "BIZ_STAGE";
const std::string STAGE_RES = "STAGE_RES";
const std::string BIZ_STATE = "BIZ_STATE";
const std::string TO_CALL_PKG = "TO_CALL_PKG";
const std::string HOST_PKG = "HOST_PKG";
const std::string ERROR_CODE = "ERROR_CODE";
const std::string DISTRIBUTED_CAMERA_BEHAVIOR = "DISTRIBUTED_CAMERA_BEHAVIOR";
constexpr char DISTRIBUTED_CAMERA[] = "DISTCAMERA";

enum class BizScene : int32_t {
    CAMERA_INIT = 1,
    CAMERA_OPEN = 2,
    CAMERA_CLOSE = 3,
    CAMERA_UNINIT = 4,
};

enum class StageRes : int32_t {
    STAGE_IDLE = 0,
    STAGE_SUCC = 1,
    STAGE_FAIL = 2,
    STAGE_CANCEL = 3,
    STAGE_UNKNOW = 4,
};

enum class BizState : int32_t {
    BIZ_STATE_START = 1,
    BIZ_STATE_END = 2,
};

enum class CameraInit : int32_t {
    SERVICE_INIT = 1,
    SOURCE_CAMERA_INIT = 2,
    LOAD_HDF_DRIVER = 3,
};

enum class CameraOpen : int32_t {
    OPEN_CAMERA = 1,
    OPEN_CHANNEL = 2,
    CONFIG_STREAMS = 3,
    OPEN_DATA_CHANNEL = 4,
    INIT_DECODE = 5,
    START_CAPTURE = 6,
};

enum class CameraClose : int32_t {
    STOP_CAPTURE = 1,
    RELEASE_STREAM = 2,
    CLOSE_CAMERA = 3,
};

enum class CameraUnInit : int32_t {
    UNLOAD_HDF_DRIVER = 1,
    SERVICE_RELEASE = 2,
};

class DcameraRadar {
    DECLARE_SINGLE_INSTANCE(DcameraRadar);
public:
    bool ReportDcameraInit(const std::string& func, CameraInit bizStage, BizState bizState, int32_t errCode);
    bool ReportDcameraInitProgress(const std::string& func, CameraInit bizStage, int32_t errCode);
    bool ReportDcameraOpen(const std::string& func, CameraOpen bizStage, BizState bizState, int32_t errCode);
    bool ReportDcameraOpenProgress(const std::string& func, CameraOpen bizStage, int32_t errCode);
    bool ReportDcameraClose(const std::string& func, CameraClose bizStage, BizState bizState, int32_t errCode);
    bool ReportDcameraCloseProgress(const std::string& func, CameraClose bizStage, int32_t errCode);
    bool ReportDcameraUnInit(const std::string& func, CameraUnInit bizStage, BizState bizState, int32_t errCode);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_COMMON_INCLUDE_UTILS_DCAMERA_RADAR_H