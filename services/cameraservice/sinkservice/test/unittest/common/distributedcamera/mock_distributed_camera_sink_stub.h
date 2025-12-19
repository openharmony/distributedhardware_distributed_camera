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

#ifndef OHOS_MOCK_DISTRIBUTED_CAMERA_SINK_STUB_H
#define OHOS_MOCK_DISTRIBUTED_CAMERA_SINK_STUB_H

#include "distributed_camera_errno.h"
#include "distributed_camera_sink_stub.h"

namespace OHOS {
namespace DistributedHardware {
class MockDistributedCameraSinkStub : public DistributedCameraSinkStub {
public:
    MockDistributedCameraSinkStub() = default;
    virtual ~MockDistributedCameraSinkStub() = default;
    int32_t InitSink(const std::string &params, const sptr<IDCameraSinkCallback> &sinkCallback)
    {
        (void)params;
        (void)sinkCallback;
        return DCAMERA_OK;
    }

    int32_t ReleaseSink()
    {
        return DCAMERA_OK;
    }

    int32_t SubscribeLocalHardware(const std::string &dhId, const std::string &param)
    {
        (void)dhId;
        (void)param;
        return DCAMERA_OK;
    }

    int32_t UnsubscribeLocalHardware(const std::string &dhId)
    {
        (void)dhId;
        return DCAMERA_OK;
    }

    int32_t StopCapture(const std::string &dhId)
    {
        (void)dhId;
        return DCAMERA_OK;
    }

    int32_t ChannelNeg(const std::string &dhId, std::string& channelInfo)
    {
        (void)dhId;
        (void)channelInfo;
        return DCAMERA_OK;
    }

    int32_t GetCameraInfo(const std::string &dhId, std::string& cameraInfo)
    {
        (void)dhId;
        (void)cameraInfo;
        return DCAMERA_OK;
    }

    int32_t OpenChannel(const std::string &dhId, std::string& openInfo)
    {
        (void)dhId;
        (void)openInfo;
        return DCAMERA_OK;
    }

    int32_t CloseChannel(const std::string &dhId)
    {
        (void)dhId;
        return DCAMERA_OK;
    }

    int32_t PauseDistributedHardware(const std::string &networkId)
    {
        (void)networkId;
        return DCAMERA_OK;
    }

    int32_t ResumeDistributedHardware(const std::string &networkId)
    {
        (void)networkId;
        return DCAMERA_OK;
    }

    int32_t StopDistributedHardware(const std::string &networkId)
    {
        (void)networkId;
        return DCAMERA_OK;
    }

    int32_t SetAccessListener(const sptr<IAccessListener> &listener, int32_t timeOut, const std::string &pkgName)
    {
        (void)listener;
        (void)timeOut;
        (void)pkgName;
        return DCAMERA_OK;
    }
    int32_t RemoveAccessListener(const std::string &pkgName)
    {
        (void)pkgName;
        return DCAMERA_OK;
    }
    int32_t SetAuthorizationResult(const std::string &requestId, bool granted)
    {
        (void)requestId;
        (void)granted;
        return DCAMERA_OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_DISTRIBUTED_CAMERA_SINK_STUB_H