/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dcamera_hisysevent_adapter.h"

#include "hisysevent.h"
#include "securec.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t MSG_MAX_LEN = 2048;
constexpr int32_t ENUM_STREAMTYPE_LEN = 2;
constexpr int32_t ENUM_ENCODETYPE_LEN = 4;
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;
const std::string ENUM_STREAMTYPE_STRINGS[] = {
    "CONTINUOUS_FRAME", "SNAPSHOT_FRAME"
};
const std::string ENUM_ENCODETYPE_STRINGS[] = {
    "ENCODE_TYPE_NULL", "ENCODE_TYPE_H264", "ENCODE_TYPE_H265", "ENCODE_TYPE_JPEG"
};
}

void ReportDcamerInitFail(const std::string& eventName, int32_t errCode, const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::FAULT,
        "ERRCODE", errCode,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

void ReportRegisterCameraFail(const std::string& eventName, const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::FAULT,
        "DEVID", devId,
        "DHID", dhId,
        "VERSION", version,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %d, devId %s, dhId %s, errMsg %s.", ret, GetAnonyString(devId).c_str(),
            dhId.c_str(), errMsg.c_str());
    }
}

void ReportDcamerOptFail(const std::string& eventName, int32_t errCode, const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::FAULT,
        "ERRCODE", errCode,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

void ReportSaEvent(const std::string& eventName, int32_t saId, const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "SAID", saId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

void ReportRegisterCameraEvent(const std::string& eventName, const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "VERSION", version,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

void ReportCameraOperaterEvent(const std::string& eventName, const std::string& devId, const std::string& dhId,
    const std::string& errMsg)
{
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

void ReportStartCaptureEvent(const std::string& eventName, EventCaptureInfo& capture, const std::string& errMsg)
{
    if (capture.encodeType_ < 0 || capture.encodeType_ >= ENUM_ENCODETYPE_LEN ||
        capture.type_ < 0 || capture.type_ >= ENUM_STREAMTYPE_LEN) {
        DHLOGE("Invalid capture parameters.");
        return;
    }
    int32_t ret = HiSysEventWrite(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA,
        eventName,
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "WIDTH", capture.width_,
        "HEIGHT", capture.height_,
        "FORMAT", capture.format_,
        "ISCAPTURE", capture.isCapture_ ? "true" : "false",
        "ENCODETYPE", ENUM_ENCODETYPE_STRINGS[capture.encodeType_],
        "STREAMTYPE", ENUM_STREAMTYPE_STRINGS[capture.type_],
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%{public}d, errMsg %{public}s.", ret, errMsg.c_str());
    }
}

std::string CreateMsg(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char msg[MSG_MAX_LEN] = {0};
    if (vsnprintf_s(msg, sizeof(msg), sizeof(msg) - 1, format, args) < 0) {
        DHLOGE("failed to call vsnprintf_s");
        va_end(args);
        return "";
    }
    va_end(args);
    return msg;
}
} // namespace DistributedHardware
} // namespace OHOS