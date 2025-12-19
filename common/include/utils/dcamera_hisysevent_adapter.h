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

#ifndef OHOS_DCAMERA_HISYSEVENT_ADAPTER_H
#define OHOS_DCAMERA_HISYSEVENT_ADAPTER_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
const std::string DCAMERA_INIT_FAIL = "DCAMERA_INIT_FAIL";
const std::string DCAMERA_REGISTER_FAIL = "DCAMERA_REGISTER_FAIL";
const std::string DCAMERA_OPT_FAIL = "DCAMERA_OPT_FAIL";

const std::string INIT_SA_EVENT = "DCAMERA_INIT";
const std::string RELEASE_SA_EVENT = "DCAMERA_EXIT";
const std::string REGIST_CAMERA_EVENT = "DCAMERA_REGISTER";
const std::string UNREGIST_CAMERA_EVENT = "DCAMERA_UNREGISTER";
const std::string OPEN_CAMERA_EVENT = "DCAMERA_OPEN";
const std::string CLOSE_CAMERA_EVENT = "DCAMERA_CLOSE";
const std::string START_CAPTURE_EVENT = "DCAMERA_CAPTURE";
const std::string DCAMERA_CONFLICT_SEND_EVENT = "DCAMERA_CONFLICT_SEND";
const std::string DCAMERA_CONFLICT_RECEIVE_EVENT = "DCAMERA_CONFLICT_RECEIVE";
enum DcameraHisyseventErrno : int32_t  {
    DCAMERA_SA_ERROR = 0,
    DCAMERA_HDF_ERROR = 1,
    DCAMERA_CREATE_SESSIONSVR_ERROR = 2,
    DCAMERA_OPEN_SESSION_ERROR = 3,
    DCAMERA_SINK_OPEN_CAM_ERROR = 4,
    DCAMERA_ENCODE_ERROR = 5,
    DCAMERA_DECODE_ERROR = 6,
};

using EventCaptureInfo = struct _EventCaptureInfo {
    int32_t width_;
    int32_t height_;
    int32_t format_;
    bool isCapture_;
    int32_t encodeType_;
    int32_t type_;
};

void ReportDcamerInitFail(const std::string& eventName, int32_t errCode, const std::string& errMsg);
void ReportRegisterCameraFail(const std::string& eventName, const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg);
void ReportDcamerOptFail(const std::string& eventName, int32_t errCode, const std::string& errMsg);

void ReportSaEvent(const std::string& eventName, int32_t saId, const std::string& errMsg);
void ReportRegisterCameraEvent(const std::string& eventName, const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg);
void ReportCameraOperaterEvent(const std::string& eventName, const std::string& devId, const std::string& dhId,
    const std::string& errMsg);
void ReportStartCaptureEvent(const std::string& eventName, EventCaptureInfo& capture, const std::string& errMsg);

std::string CreateMsg(const char *format, ...);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HISYSEVENT_ADAPTER_H