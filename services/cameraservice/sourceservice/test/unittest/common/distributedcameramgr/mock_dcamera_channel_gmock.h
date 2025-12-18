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

#ifndef OHOS_MOCK_CAMERA_CHANNEL_GMOCK_H
#define OHOS_MOCK_CAMERA_CHANNEL_GMOCK_H

#include <gmock/gmock.h>
#include "distributed_camera_errno.h"
#include "icamera_channel.h"

namespace OHOS {
namespace DistributedHardware {
class MockCameraChannelGMock : public ICameraChannel {
public:
    explicit MockCameraChannelGMock()
    {
    }

    ~MockCameraChannelGMock()
    {
    }

    MOCK_METHOD0(OpenSession, int32_t());
    MOCK_METHOD0(CloseSession, int32_t());
    MOCK_METHOD4(CreateSession, int32_t(std::vector<DCameraIndex>&, std::string,
        DCameraSessionMode, std::shared_ptr<ICameraChannelListener>&));
    MOCK_METHOD0(ReleaseSession, int32_t());
    MOCK_METHOD1(SendData, int32_t(std::shared_ptr<DataBuffer>&));
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_CAMERA_CHANNEL_GMOCK_H
