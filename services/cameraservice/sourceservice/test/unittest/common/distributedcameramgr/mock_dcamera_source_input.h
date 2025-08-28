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

#ifndef OHOS_MOCK_DCAMERA_SOURCE_INPUT_H
#define OHOS_MOCK_DCAMERA_SOURCE_INPUT_H

#include "icamera_controller.h"
#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
class MockDCameraSourceInput : public ICameraInput {
public:
    int32_t ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease)
    {
        return DCAMERA_OK;
    }
    int32_t ReleaseAllStreams()
    {
        return DCAMERA_OK;
    }
    int32_t StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t StopCapture(std::vector<int>& streamIds, bool& isAllStop)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t StopAllCapture()
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t OpenChannel(std::vector<DCameraIndex>& indexs)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t CloseChannel()
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t Init()
    {
        return DCAMERA_OK;
    }
    int32_t UnInit()
    {
        return DCAMERA_OK;
    }
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        return DCAMERA_BAD_OPERATE;
    }
    int32_t StopDistributedHardware(const std::string &networkId)
    {
        return DCAMERA_OK;
    }
    int32_t UpdateWorkMode(const WorkModeParam& param)
    {
        return DCAMERA_OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_DCAMERA_SOURCE_INPUT_H
