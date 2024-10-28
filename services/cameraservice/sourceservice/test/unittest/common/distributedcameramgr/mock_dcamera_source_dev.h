/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_MOCKSOURCE_DEV_H
#define OHOS_DISTRIBUTED_CAMERA_MOCKSOURCE_DEV_H

#include "dcamera_source_dev.h"

#include "distributed_camera_errno.h"
#include "idistributed_camera_source.h"

namespace OHOS {
namespace DistributedHardware {
extern std::string g_regisStateStr;
extern std::string g_openStateStr;
extern std::string g_captureStateStr;
class MockDCameraSourceDev : public DCameraSourceDev {
public:
    MockDCameraSourceDev(std::string devId, std::string dhId, std::shared_ptr<ICameraStateListener>& stateLisener)
        : DCameraSourceDev(devId, dhId, stateLisener) {};
    ~MockDCameraSourceDev() = default;

    int32_t Register(std::shared_ptr<DCameraRegistParam>& param)
    {
        if (g_regisStateStr == "test008") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t UnRegister(std::shared_ptr<DCameraRegistParam>& param)
    {
        if (g_regisStateStr == "test008" || g_openStateStr == "test010" || g_captureStateStr == "test014") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t OpenCamera()
    {
        if (g_regisStateStr == "test008") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t CloseCamera()
    {
        if (g_openStateStr == "test010" || g_openStateStr == "test012" || g_captureStateStr == "test014" ||
            g_captureStateStr == "test017") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
    {
        if (g_openStateStr == "test010") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease)
    {
        isAllRelease = true;
        return DCAMERA_OK;
    }
    int32_t ReleaseAllStreams()
    {
        if (g_captureStateStr == "test018" || g_captureStateStr == "test020") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
    {
        if (g_captureStateStr == "test014") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t StopCapture(std::vector<int>& streamIds, bool& isAllStop)
    {
        if (g_captureStateStr == "test014") {
            return DCAMERA_BAD_VALUE;
        }
        isAllStop = true;
        return DCAMERA_OK;
    }
    int32_t StopAllCapture()
    {
        if (g_captureStateStr == "test016" || g_captureStateStr == "test019") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        if (g_openStateStr == "test010" || g_captureStateStr == "test014") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
    int32_t CameraEventNotify(std::shared_ptr<DCameraEvent>& events)
    {
        if (g_regisStateStr == "test008" || g_openStateStr == "test010" || g_openStateStr == "test011" ||
            g_captureStateStr == "test014" || g_captureStateStr == "test015") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

private:
    std::string devId_;
    std::string dhId_;
    std::shared_ptr<ICameraStateListener> stateLisener_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_MOCKSOURCE_DEV_H
