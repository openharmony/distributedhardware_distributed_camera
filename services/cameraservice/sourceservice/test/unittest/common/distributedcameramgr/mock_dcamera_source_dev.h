/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
class MockDCameraSourceDev : public DCameraSourceDev {
public:
    MockDCameraSourceDev(std::string devId, std::string dhId, std::shared_ptr<ICameraStateListener>& stateLisener)
        : DCameraSourceDev(devId, dhId, stateLisener) {};
    ~MockDCameraSourceDev() = default;

    int32_t Register(std::shared_ptr<DCameraRegistParam>& param)
    {
        return DCAMERA_OK;
    }
    int32_t UnRegister(std::shared_ptr<DCameraRegistParam>& param)
    {
        return DCAMERA_OK;
    }
    int32_t OpenCamera()
    {
        return DCAMERA_OK;
    }
    int32_t CloseCamera()
    {
        return DCAMERA_OK;
    }
    int32_t ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
    {
        return DCAMERA_OK;
    }
    int32_t ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease)
    {
        isAllRelease = true;
        return DCAMERA_OK;
    }
    int32_t ReleaseAllStreams()
    {
        return DCAMERA_OK;
    }
    int32_t StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
    {
        return DCAMERA_OK;
    }
    int32_t StopCapture(std::vector<int>& streamIds, bool& isAllStop)
    {
        isAllStop = true;
        return DCAMERA_OK;
    }
    int32_t StopAllCapture()
    {
        return DCAMERA_OK;
    }
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        return DCAMERA_OK;
    }
    int32_t CameraEventNotify(std::shared_ptr<DCameraEvent>& events)
    {
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
