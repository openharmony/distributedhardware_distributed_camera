/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_MOCK_DCAMERA_SINK_CONTROLLER_H
#define OHOS_MOCK_DCAMERA_SINK_CONTROLLER_H

#include "icamera_controller.h"
#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
class MockDCameraSourceController : public ICameraController {
public:
    explicit MockDCameraSourceController()
    {
    }

    ~MockDCameraSourceController()
    {
    }

    int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode)
    {
        return DCAMERA_OK;
    }
    int32_t StopCapture()
    {
        return DCAMERA_OK;
    }
    int32_t ChannelNeg(std::shared_ptr<DCameraChannelInfo>& info)
    {
        return DCAMERA_OK;
    }
    int32_t DCameraNotify(std::shared_ptr<DCameraEvent>& events)
    {
        return DCAMERA_OK;
    }
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        return DCAMERA_OK;
    }
    int32_t GetCameraInfo(std::shared_ptr<DCameraInfo>& camInfo)
    {
        return DCAMERA_OK;
    }
    int32_t OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo)
    {
        return DCAMERA_OK;
    }
    int32_t CloseChannel()
    {
        return DCAMERA_OK;
    }
    int32_t Init(std::vector<DCameraIndex>& indexs)
    {
        return DCAMERA_OK;
    }
    int32_t UnInit()
    {
        return DCAMERA_OK;
    }
    int32_t PauseDistributedHardware(const std::string &networkId)
    {
        return DCAMERA_OK;
    }
    int32_t ResumeDistributedHardware(const std::string &networkId)
    {
        return DCAMERA_OK;
    }
    int32_t StopDistributedHardware(const std::string &networkId)
    {
        return DCAMERA_OK;
    }
    void SetTokenId(uint64_t token)
    {
    }
};
class MockDCameraSourceControllerRetErr : public MockDCameraSourceController {
public:
    int32_t OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t CloseChannel()
    {
        return DCAMERA_BAD_OPERATE;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_DCAMERA_SINK_CONTROLLER_H
