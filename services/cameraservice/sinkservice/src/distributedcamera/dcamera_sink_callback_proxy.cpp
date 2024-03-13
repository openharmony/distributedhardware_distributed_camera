/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dcamera_sink_callback_proxy.h"

#include "parcel.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraSinkCallbackProxy::OnNotifyResourceInfo(const ResourceEventType &type, const std::string &subtype,
    const std::string &networkId, bool &isSensitive, bool &isSameAccout)
{
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DCameraSinkCallbackProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel req;
    MessageParcel reply;
    MessageOption option;
    if (!req.WriteInterfaceToken(DCameraSinkCallbackProxy::GetDescriptor())) {
        DHLOGE("Write token failed");
        return DCAMERA_BAD_VALUE;
    }

    int32_t resType = static_cast<int32_t>(type);
    if (!req.WriteInt32(resType) || !req.WriteString(subtype) || !req.WriteString(networkId)) {
        DHLOGE("DCameraSinkCallbackProxy InitSink write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(NOTIFY_RESOURCEINFO, req, reply, option);
    int32_t result = reply.ReadInt32();
    isSensitive = reply.ReadBool();
    isSameAccout = reply.ReadBool();
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS
