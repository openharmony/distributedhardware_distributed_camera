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

#ifndef OHOS_DISTRIBUTED_CAMERA_SINK_STUB_H
#define OHOS_DISTRIBUTED_CAMERA_SINK_STUB_H

#include <map>
#include "iremote_stub.h"

#include "idistributed_camera_sink.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSinkStub : public IRemoteStub<IDistributedCameraSink> {
public:
    DistributedCameraSinkStub();
    virtual ~DistributedCameraSinkStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t InitSinkInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReleaseSinkInner(MessageParcel &data, MessageParcel &reply);
    int32_t SubscribeLocalHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnsubscribeLocalHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopCaptureInner(MessageParcel &data, MessageParcel &reply);
    int32_t ChannelNegInner(MessageParcel &data, MessageParcel &reply);
    int32_t GetCameraInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t OpenChannelInner(MessageParcel &data, MessageParcel &reply);
    int32_t CloseChannelInner(MessageParcel &data, MessageParcel &reply);
    bool HasEnableDHPermission();
    int32_t PauseDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t ResumeDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t StopDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAccessListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t RemoveAccessListenerInner(MessageParcel &data, MessageParcel &reply);
    int32_t SetAuthorizationResultInner(MessageParcel &data, MessageParcel &reply);
    bool HasAccessDHPermission();

    using DCameraFunc = int32_t (DistributedCameraSinkStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, DCameraFunc> memberFuncMap_;

    const size_t DID_MAX_SIZE = 256;
    const size_t PARAM_MAX_SIZE = 50 * 1024 * 1024;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_SINK_STUB_H