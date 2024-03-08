/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dcamera_sink_callback_stub.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
namespace OHOS { class MessageOption; }

namespace OHOS {
namespace DistributedHardware {
DCameraSinkCallbackStub::DCameraSinkCallbackStub() : IRemoteStub(true)
{
    memberFuncMap_[NOTIFY_RESOURCEINFO] = &DCameraSinkCallbackStub::OnNotifyResourceInfoInner;
}

DCameraSinkCallbackStub::~DCameraSinkCallbackStub()
{}

int32_t DCameraSinkCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGI("DCameraSinkCallbackStub OnRemoteRequest code: %{public}u", code);
    std::u16string desc = DCameraSinkCallbackStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (desc != remoteDesc) {
        DHLOGE("remoteDesc is invalid!");
        return ERR_INVALID_DATA;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc == memberFuncMap_.end()) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    auto memberFunc = itFunc->second;
    return (this->*memberFunc)(data, reply);
}

int32_t DCameraSinkCallbackStub::OnNotifyResourceInfoInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("DCameraSinkCallbackStub OnNotifyResourceInfoInner");
    int32_t ret = DCAMERA_OK;
    bool isSensitive;
    bool isSameAccout;
    do {
        ResourceEventType type = static_cast<ResourceEventType>(data.ReadInt32());
        std::string subtype = data.ReadString();
        std::string networkId = data.ReadString();
        ret = OnNotifyResourceInfo(type, subtype, networkId, isSensitive, isSameAccout);
    } while (0);
    reply.WriteInt32(ret);
    reply.WriteBool(isSensitive);
    reply.WriteBool(isSameAccout);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
