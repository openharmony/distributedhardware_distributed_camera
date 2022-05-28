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

#ifndef OHOS_DCAMERA_HISYSEVENT_ADAPTER_H
#define OHOS_DCAMERA_HISYSEVENT_ADAPTER_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
void ReportLoadSaFail(int32_t saId, const std::string& errMsg);
void ReportHDFFail(const std::string& errMsg);
void ReportRegisterHardwareFail(const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg);
void ReportSoftbusSessionServerFail(const std::string pkgName, std::string sessionName, const std::string& errMsg);
void ReportSoftbusSessionFail(std::string mySessName, std::string peerSessName, std::string peerDevId,
    const std::string& errMsg);
void ReportCameraOperaterFail(const std::string& dhId, const std::string& errMsg);
void ReportStartVideoEncoderFail(int32_t width, int32_t height, std::string format, const std::string& errMsg);
void ReportStopVideoEncoderFail(const std::string& errMsg);
void ReportStartVideoDecoderFail(int32_t width, int32_t height, std::string format, const std::string& errMsg);
void ReportStopVideoDecoderFail(const std::string& errMsg);
void ReportInitSaEvent(int32_t saId, const std::string& errMsg);
void ReportReleaseSaEvent(const std::string& errMsg);
void ReportLoadHDFEvent(const std::string& errMsg);
void ReportUnLoadHDFEvent(const std::string& errMsg);
void ReportRegisterCameraEvent(const std::string& devId, const std::string& dhId,
    std::string version, const std::string& errMsg);
void ReportUnRegisterCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg);
void ReportOpenCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg);
void ReportCloseCameraEvent(const std::string& devId, const std::string& dhId, const std::string& errMsg);
void ReportConfigStreamsEvent(int32_t streamId, int32_t width, int32_t height, int32_t format,
    std::string encodeType, std::string streamType, const std::string& errMsg);
void ReportReleaseStreamsEvent(int32_t streamId, const std::string& errMsg);
void ReportStartCaptureEvent(int32_t width, int32_t height, int32_t format, std::string isCapture,
    std::string encodeType, std::string streamType, const std::string& errMsg);
void ReportStopCaptureEvent(int32_t streamId, const std::string& errMsg);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HISYSEVENT_ADAPTER_H