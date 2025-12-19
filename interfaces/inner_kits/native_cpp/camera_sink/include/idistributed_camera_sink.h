/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_IDISTRIBUTED_CAMERA_SINK_H
#define OHOS_IDISTRIBUTED_CAMERA_SINK_H

#include "idcamera_sink_callback.h"
#include "iremote_broker.h"
#include "iaccess_listener.h"

namespace OHOS {
namespace DistributedHardware {
class IDistributedCameraSink : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedhardware.distributedcamerasink");

    IDistributedCameraSink() = default;
    virtual ~IDistributedCameraSink() = default;
    virtual int32_t InitSink(const std::string& params, const sptr<IDCameraSinkCallback> &sinkCallback) = 0;
    virtual int32_t ReleaseSink() = 0;
    virtual int32_t SubscribeLocalHardware(const std::string& dhId, const std::string& parameters) = 0;
    virtual int32_t UnsubscribeLocalHardware(const std::string& dhId) = 0;
    virtual int32_t StopCapture(const std::string& dhId) = 0;
    virtual int32_t ChannelNeg(const std::string& dhId, std::string& channelInfo) = 0;
    virtual int32_t GetCameraInfo(const std::string& dhId, std::string& cameraInfo) = 0;
    virtual int32_t OpenChannel(const std::string& dhId, std::string& openInfo) = 0;
    virtual int32_t CloseChannel(const std::string& dhId) = 0;
    virtual int32_t PauseDistributedHardware(const std::string &networkId) = 0;
    virtual int32_t ResumeDistributedHardware(const std::string &networkId) = 0;
    virtual int32_t StopDistributedHardware(const std::string &networkId) = 0;
    virtual int32_t SetAccessListener(const sptr<IAccessListener> &listener, int32_t timeOut,
        const std::string &pkgName) = 0;
    virtual int32_t RemoveAccessListener(const std::string &pkgName) = 0;
    virtual int32_t SetAuthorizationResult(const std::string &requestId, bool granted) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IDISTRIBUTED_CAMERA_SINK_H