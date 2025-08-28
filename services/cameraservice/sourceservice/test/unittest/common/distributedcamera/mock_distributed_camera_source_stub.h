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

#ifndef OHOS_MOCK_DISTRIBUTED_CAMERA_SOURCE_STUB_H
#define OHOS_MOCK_DISTRIBUTED_CAMERA_SOURCE_STUB_H

#include "distributed_camera_errno.h"
#include "distributed_camera_source_stub.h"

namespace OHOS {
namespace DistributedHardware {
class MockDistributedCameraSourceStub : public DistributedCameraSourceStub {
public:
    MockDistributedCameraSourceStub() = default;
    virtual ~MockDistributedCameraSourceStub() = default;
    int32_t InitSource(const std::string& params, const sptr<IDCameraSourceCallback>& callback)
    {
        (void)params;
        (void)callback;
        return DCAMERA_OK;
    }

    int32_t ReleaseSource()
    {
        return DCAMERA_OK;
    }

    int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& reqId, const EnableParam& param)
    {
        (void)devId;
        (void)dhId;
        (void)reqId;
        (void)param;
        return DCAMERA_OK;
    }

    int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& reqId)
    {
        (void)devId;
        (void)dhId;
        (void)reqId;
        return DCAMERA_OK;
    }

    int32_t DCameraNotify(const std::string& devId, const std::string& dhId,
        std::string& events)
    {
        (void)devId;
        (void)dhId;
        (void)events;
        return DCAMERA_OK;
    }

    int32_t UpdateDistributedHardwareWorkMode(const std::string& devId, const std::string& dhId,
        const WorkModeParam& param)
    {
        (void)devId;
        (void)dhId;
        (void)param;
        return DCAMERA_OK;
    }
};


} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_DISTRIBUTED_CAMERA_SOURCE_STUB_H