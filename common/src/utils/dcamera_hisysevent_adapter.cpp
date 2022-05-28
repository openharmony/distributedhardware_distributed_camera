/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
using HiSysEventNameSpace = OHOS::HiviewDFX::HiSysEvent;
const std::string DOMAIN_STR = std::string(HiSysEventNameSpace::Domain::DISTRIBUTED_CAMERA);
}

void ReportLoadSaFail(int32_t saId, const std::string& errMsg)
{
    int ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "SA_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "SAID", saId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %d, saId %d, errMsg %s.", ret, saId, errMsg.c_str());
    }
}

void ReportHDFFail(const std::string& errMsg)
{
    int ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "HDF_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportRegisterHardwareFail(const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "REGISTER_HARDWARE_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "DEVID", devId,
        "DHID", dhId,
        "VERSION", version,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error ret %d, devId %s, dhId %s, errMsg %s.", ret, devId.c_str(),
            dhId.c_str(), errMsg.c_str());
    }
}

void ReportSoftbusSessionServerFail(const std::string pkgName, std::string sessionName, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "SOFTBUS_SESSIONSERVER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "PKGNAME", pkgName,
        "SESSIONNAME", sessionName,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportSoftbusSessionFail(std::string mySessName, std::string peerSessName, std::string peerDevId,
    const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "SOFTBUS_SESSION_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "MYSESSIONNAME", mySessName,
        "PEERSESSIONNAME", peerSessName,
        "PEERDEVID", peerDevId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportCameraOperaterFail(const std::string& dhId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "CAMERA_OPERATER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "DHID", dhId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, dhId %s, errMsg %s.", ret, dhId.c_str(), errMsg.c_str());
    }
}

void ReportStartVideoEncoderFail(int32_t width, int32_t height, std::string format, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "START_VIDEO_ENCODER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "WIDTH", width,
        "HEIGHT", height,
        "FORMAT", format,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportStopVideoEncoderFail(const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "STOP_VIDEO_ENCODER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportStartVideoDecoderFail(int32_t width, int32_t height, std::string format, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "START_VIDEO_DECODER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "WIDTH", width,
        "HEIGHT", height,
        "FORMAT", format,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportStopVideoDecoderFail(const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "STOP_VIDEO_DECODER_ERROR",
        HiSysEventNameSpace::EventType::FAULT,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportInitSaEvent(int32_t saId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "INIT_SA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "SAID", saId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportReleaseSaEvent(const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "RELEASE_SA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportLoadHDFEvent(const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "LOAD_HDF_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportUnLoadHDFEvent(const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "UNLOAD_HDF_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportRegisterCameraEvent(const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "REGIST_CAMERA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "VERSION", version,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportUnRegisterCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "UNREGIST_CAMERA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportOpenCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "OPEN_CAMERA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportCloseCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "CLOSE_CAMERA_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "DEVID", devId,
        "DHID", dhId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportConfigStreamsEvent(int32_t streamId, int32_t width, int32_t height, int32_t format,
    std::string encodeType, std::string streamType, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "CONFIG_STREAMS_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "STREAMID", streamId,
        "WIDTH", width,
        "HEIGHT", height,
        "FORMAT", format,
        "ENCODETYPE", encodeType,
        "STREAMTYPE", streamType,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportReleaseStreamsEvent(int32_t streamId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "RELEASE_STREAMS_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "STREAMID", streamId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportStartCaptureEvent(int32_t width, int32_t height, int32_t format, std::string isCapture,
    std::string encodeType, std::string streamType, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "START_CAPTURE_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "WIDTH", width,
        "HEIGHT", height,
        "FORMAT", format,
        "ISCAPTURE", isCapture,
        "ENCODETYPE", encodeType,
        "STREAMTYPE", streamType,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}

void ReportStopCaptureEvent(int32_t streamId, const std::string& errMsg)
{
    int32_t ret = HiSysEventNameSpace::Write(DOMAIN_STR,
        "STOP_CAPTURE_EVENT",
        HiSysEventNameSpace::EventType::BEHAVIOR,
        "STREAMID", streamId,
        "MSG", errMsg);
    if (ret != DCAMERA_OK) {
        DHLOGE("Write HiSysEvent error, ret:%d, errMsg %s.", ret, errMsg.c_str());
    }
}
} // namespace DistributedHardware
} // namespace OHOS