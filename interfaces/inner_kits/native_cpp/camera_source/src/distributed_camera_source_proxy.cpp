/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "distributed_camera_source_proxy.h"

#include "anonymous_string.h"
#include "dcamera_ipc_interface_code.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DistributedCameraSourceProxy::InitSource(const std::string& params,
    const sptr<IDCameraSourceCallback>& callback)
{
    DHLOGI("start");
    if (params.empty() || params.size() > PARAM_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DistributedCameraSourceProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!data.WriteString(params)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }

    if (callback != nullptr && !data.WriteRemoteObject(callback->AsObject())) {
        DHLOGE("write callback failed");
        return DCAMERA_BAD_VALUE;
    }

    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::INIT_SOURCE), data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DistributedCameraSourceProxy::ReleaseSource()
{
    DHLOGI("start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DistributedCameraSourceProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::RELEASE_SOURCE), data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DistributedCameraSourceProxy::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    DHLOGI("devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    if (!CheckRegParams(devId, dhId, reqId, param)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DistributedCameraSourceProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!data.WriteString(devId) || !data.WriteString(dhId) || !data.WriteString(reqId) ||
        !data.WriteString(param.sinkVersion) || !data.WriteString(param.sinkAttrs)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::REGISTER_DISTRIBUTED_HARDWARE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

bool DistributedCameraSourceProxy::CheckRegParams(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE) {
        DHLOGE("reqId is invalid");
        return false;
    }

    if (param.sinkVersion.empty() || param.sinkVersion.size() > PARAM_MAX_SIZE ||
        param.sinkAttrs.empty() || param.sinkAttrs.size() > PARAM_MAX_SIZE) {
        DHLOGE("param is invalid");
        return false;
    }
    return true;
}

int32_t DistributedCameraSourceProxy::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId)
{
    DHLOGI("devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    if (!CheckUnregParams(devId, dhId, reqId)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DistributedCameraSourceProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!data.WriteString(devId) || !data.WriteString(dhId) || !data.WriteString(reqId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::UNREGISTER_DISTRIBUTED_HARDWARE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

bool DistributedCameraSourceProxy::CheckUnregParams(const std::string& devId, const std::string& dhId,
    const std::string& reqId)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE) {
        DHLOGE("reqId is invalid");
        return false;
    }
    return true;
}

int32_t DistributedCameraSourceProxy::DCameraNotify(const std::string& devId, const std::string& dhId,
    std::string& events)
{
    DHLOGI("DCameraNotify devId: %{public}s dhId: %{public}s events: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), events.c_str());
    if (!CheckNotifyParams(devId, dhId, events)) {
        DHLOGE("input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("DistributedCameraSourceProxy remote service null");
        return DCAMERA_BAD_VALUE;
    }
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }

    if (!data.WriteString(devId) || !data.WriteString(dhId) || !data.WriteString(events)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::CAMERA_NOTIFY),
        data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

bool DistributedCameraSourceProxy::CheckNotifyParams(const std::string& devId, const std::string& dhId,
    std::string& events)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return false;
    }

    if (events.empty() || events.size() > PARAM_MAX_SIZE) {
        DHLOGE("events is invalid");
        return false;
    }
    return true;
}

int32_t DistributedCameraSourceProxy::UpdateDistributedHardwareWorkMode(const std::string& devId,
    const std::string& dhId, const WorkModeParam& param)
{
    DHLOGI("devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    if (!CheckUpdateParams(devId, dhId, param)) {
        DHLOGE("workmode param is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    CHECK_AND_RETURN_RET_LOG(remote == nullptr, DCAMERA_BAD_VALUE, "remote service is null");
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSourceProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    bool writeRet = !data.WriteString(devId) || !data.WriteString(dhId) || !data.WriteFileDescriptor(param.fd) ||
        !data.WriteInt32(param.sharedMemLen) || !data.WriteUint32(param.scene) || !data.WriteInt32(param.isAVsync);
    CHECK_AND_RETURN_RET_LOG(writeRet, DCAMERA_BAD_VALUE, "write params failed");
    remote->SendRequest(static_cast<uint32_t>(IDCameraSourceInterfaceCode::UPDATE_WORKMODE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

bool DistributedCameraSourceProxy::CheckUpdateParams(const std::string& devId, const std::string& dhId,
    const WorkModeParam& param)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("input params is invalid");
        return false;
    }
    if (param.fd < 0 || param.sharedMemLen < 0) {
        DHLOGE("workmode param is invalid");
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
