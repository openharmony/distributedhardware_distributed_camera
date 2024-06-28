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

#include "dcamera_source_callback_stub.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "ipc_object_stub.h"
#include "ipc_types.h"
#include "message_parcel.h"
namespace OHOS { class MessageOption; }

namespace OHOS {
namespace DistributedHardware {
DCameraSourceCallbackStub::DCameraSourceCallbackStub() : IRemoteStub(true)
{
    memberFuncMap_[NOTIFY_REG_RESULT] = &DCameraSourceCallbackStub::NotifyRegResultInner;
    memberFuncMap_[NOTIFY_UNREG_RESULT] = &DCameraSourceCallbackStub::NotifyUnregResultInner;
    memberFuncMap_[NOTIFY_STATE_CHANGED] = &DCameraSourceCallbackStub::OnHardwareStateChangedInner;
    memberFuncMap_[NOTIFY_DATASYNC_TRIGGER] = &DCameraSourceCallbackStub::OnDataSyncTriggerInner;
}

DCameraSourceCallbackStub::~DCameraSourceCallbackStub()
{}

int32_t DCameraSourceCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGI("DCameraSourceCallbackStub OnRemoteRequest code: %{public}d", code);
    std::u16string desc = DCameraSourceCallbackStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (desc != remoteDesc) {
        DHLOGE("remoteDesc is invalid!");
        return ERR_INVALID_DATA;
    }

    switch (code) {
        case NOTIFY_REG_RESULT:
            return NotifyRegResultInner(data, reply);
        case NOTIFY_UNREG_RESULT:
            return NotifyUnregResultInner(data, reply);
        case NOTIFY_STATE_CHANGED:
            return OnHardwareStateChangedInner(data, reply);
        case NOTIFY_DATASYNC_TRIGGER:
            return OnDataSyncTriggerInner(data, reply);
        default:
            DHLOGE("Invalid OnRemoteRequest code=%{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DCAMERA_NOT_FOUND;
}

int32_t DCameraSourceCallbackStub::NotifyRegResultInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("DCameraSourceCallbackStub NotifyRegResultInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string devId = data.ReadString();
        std::string dhId = data.ReadString();
        std::string reqId = data.ReadString();
        int32_t status = data.ReadInt32();
        std::string result = data.ReadString();
        if (!CheckParams(devId, dhId, reqId, result)) {
            DHLOGE("input is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = OnNotifyRegResult(devId, dhId, reqId, status, result);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DCameraSourceCallbackStub::NotifyUnregResultInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("DCameraSourceCallbackStub NotifyUnregResultInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string devId = data.ReadString();
        std::string dhId = data.ReadString();
        std::string reqId = data.ReadString();
        int32_t status = data.ReadInt32();
        std::string result = data.ReadString();
        if (!CheckParams(devId, dhId, reqId, result)) {
            DHLOGE("input is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = OnNotifyUnregResult(devId, dhId, reqId, status, result);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DCameraSourceCallbackStub::OnHardwareStateChangedInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("DCameraSourceCallbackStub OnHardwareStateChangedInner");
    std::string devId = data.ReadString();
    std::string dhId = data.ReadString();
    int32_t status = data.ReadInt32();
    int32_t ret = OnHardwareStateChanged(devId, dhId, status);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DCameraSourceCallbackStub::OnDataSyncTriggerInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGI("DCameraSourceCallbackStub OnDataSyncTriggerInner");
    std::string devId = data.ReadString();
    int32_t ret = OnDataSyncTrigger(devId);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

bool DCameraSourceCallbackStub::CheckParams(const std::string& devId, const std::string& dhId, const std::string& reqId,
    const std::string& result)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE || result.size() > PARAM_MAX_SIZE) {
        DHLOGE("reqId or result is invalid");
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
