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

#include "dcamera_source_callback_proxy.h"

#include "parcel.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraSourceCallbackProxy::OnNotifyRegResult(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    if (!CheckParams(devId, dhId, reqId, data)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DCameraSourceCallbackProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel req;
    MessageParcel reply;
    MessageOption option;
    if (!req.WriteInterfaceToken(DCameraSourceCallbackProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!req.WriteString(devId) || !req.WriteString(dhId) || !req.WriteString(reqId) ||
        !req.WriteInt32(status) || !req.WriteString(data)) {
        DHLOGE("DistributedCameraSourceProxy InitSource write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(NOTIFY_REG_RESULT, req, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DCameraSourceCallbackProxy::OnNotifyUnregResult(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    if (!CheckParams(devId, dhId, reqId, data)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DCameraSourceCallbackProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel req;
    MessageParcel reply;
    MessageOption option;
    if (!req.WriteInterfaceToken(DCameraSourceCallbackProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!req.WriteString(devId) || !req.WriteString(dhId) || !req.WriteString(reqId) ||
        !req.WriteInt32(status) || !req.WriteString(data)) {
        DHLOGE("DistributedCameraSourceProxy InitSource write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(NOTIFY_UNREG_RESULT, req, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DCameraSourceCallbackProxy::OnHardwareStateChanged(const std::string &devId,
    const std::string &dhId, int32_t status)
{
    if (!CheckParams(devId, dhId, status)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DCameraSourceCallbackProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel req;
    MessageParcel reply;
    MessageOption option;
    if (!req.WriteInterfaceToken(DCameraSourceCallbackProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!req.WriteString(devId) || !req.WriteString(dhId) || !req.WriteInt32(status)) {
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(NOTIFY_STATE_CHANGED, req, reply, option);
    return reply.ReadInt32();
}

int32_t DCameraSourceCallbackProxy::OnDataSyncTrigger(const std::string &devId)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE) {
        DHLOGE("devId is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DCameraSourceCallbackProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel req;
    MessageParcel reply;
    MessageOption option;
    if (!req.WriteInterfaceToken(DCameraSourceCallbackProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!req.WriteString(devId)) {
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(NOTIFY_DATASYNC_TRIGGER, req, reply, option);
    return reply.ReadInt32();
}

bool DCameraSourceCallbackProxy::CheckParams(const std::string& devId, const std::string& dhId, int32_t status)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }
    if (status < 0) {
        DHLOGE("status in invalid.");
        return false;
    }
    return true;
}

bool DCameraSourceCallbackProxy::CheckParams(const std::string& devId, const std::string& dhId,
    const std::string& reqId, std::string& data)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE || data.size() > PARAM_MAX_SIZE) {
        DHLOGE("reqId or data is invalid");
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
