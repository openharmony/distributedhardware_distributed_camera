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

#include "distributed_camera_sink_proxy.h"

#include "parcel.h"

#include "anonymous_string.h"
#include "dcamera_ipc_interface_code.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DistributedCameraSinkProxy::InitSink(const std::string& params, const sptr<IDCameraSinkCallback> &sinkCallback)
{
    DHLOGI("start");
    if (params.empty() || params.size() > PARAM_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(params)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    if (sinkCallback != nullptr && sinkCallback->AsObject() != nullptr) {
        if (!data.WriteRemoteObject(sinkCallback->AsObject())) {
            DHLOGE("write sinkCallback failed");
            return DCAMERA_BAD_VALUE;
        }
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::INIT_SINK), data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DistributedCameraSinkProxy::ReleaseSink()
{
    DHLOGI("start");
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::RELEASE_SINK), data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DistributedCameraSinkProxy::SubscribeLocalHardware(const std::string& dhId, const std::string& parameters)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (parameters.empty() || parameters.size() > PARAM_MAX_SIZE || dhId.empty() ||
        dhId.size() > DID_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId) || !data.WriteString(parameters)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::SUBSCRIBE_LOCAL_HARDWARE),
        data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    
    int32_t result;

    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }

    return result;
}

int32_t DistributedCameraSinkProxy::UnsubscribeLocalHardware(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::UNSUBSCRIBE_LOCAL_HARDWARE), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::StopCapture(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option = { MessageOption::TF_ASYNC };
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_CAPTURE), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("async dhId: %{public}s", GetAnonyString(dhId).c_str());
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkProxy::ChannelNeg(const std::string& dhId, std::string& channelInfo)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE || channelInfo.empty() ||
        channelInfo.size() > PARAM_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId) || !data.WriteString(channelInfo)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::CHANNEL_NEG), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::GetCameraInfo(const std::string& dhId, std::string& cameraInfo)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId) || !data.WriteString(cameraInfo)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::GET_CAMERA_INFO), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::OpenChannel(const std::string& dhId, std::string& openInfo)
{
    DHLOGI("DistributedCameraSinkProxy OpenChannel Begin,dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE || openInfo.empty() ||
        openInfo.size() > PARAM_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId) || !data.WriteString(openInfo)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::OPEN_CHANNEL), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("DistributedCameraSinkProxy OpenChannel End,result: %{public}d", result);
    return result;
}

int32_t DistributedCameraSinkProxy::CloseChannel(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("params is invalid");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(dhId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::CLOSE_CHANNEL), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::PauseDistributedHardware(const std::string &networkId)
{
    DHLOGI("networkId: %{public}s", GetAnonyString(networkId).c_str());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::ResumeDistributedHardware(const std::string &networkId)
{
    DHLOGI("networkId: %{public}s", GetAnonyString(networkId).c_str());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::RESUME_DISTRIBUTED_HARDWARE), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::StopDistributedHardware(const std::string &networkId)
{
    DHLOGI("networkId: %{public}s", GetAnonyString(networkId).c_str());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(networkId)) {
        DHLOGE("write params failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_DISTRIBUTED_HARDWARE), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for code failed");
        return DCAMERA_BAD_VALUE;
    }
    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }
    return result;
}

int32_t DistributedCameraSinkProxy::SetAccessListener(const sptr<IAccessListener> &listener,
    int32_t timeOut, const std::string &pkgName)
{
    DHLOGI("SetAccessListener start, pkgName: %{public}s, timeOut: %{public}d", pkgName.c_str(), timeOut);
    if (listener == nullptr) {
        DHLOGE("listener is nullptr");
        return DCAMERA_BAD_VALUE;
    }

    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteRemoteObject(listener->AsObject())) {
        DHLOGE("write listener failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteInt32(timeOut)) {
        DHLOGE("write timeOut failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(pkgName)) {
        DHLOGE("write pkgName failed");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_ACCESS_LISTENER), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for SET_ACCESS_LISTENER failed, ipcResult: %{public}d", ipcResult);
        return DCAMERA_BAD_VALUE;
    }

    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGI("SetAccessListener end, result: %{public}d", result);
    return result;
}

int32_t DistributedCameraSinkProxy::RemoveAccessListener(const std::string &pkgName)
{
    DHLOGI("RemoveAccessListener start, pkgName: %{public}s", pkgName.c_str());
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(pkgName)) {
        DHLOGE("write pkgName failed");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::REMOVE_ACCESS_LISTENER), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for SET_ACCESS_LISTENER failed, ipcResult: %{public}d", ipcResult);
        return DCAMERA_BAD_VALUE;
    }

    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGI("RemoveAccessListener end, result: %{public}d", result);
    return result;
}

int32_t DistributedCameraSinkProxy::SetAuthorizationResult(const std::string &requestId, bool granted)
{
    DHLOGI("SetAuthorizationResult start, requestId: %{public}s, granted: %{public}d",
        GetAnonyString(requestId).c_str(), granted);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        DHLOGE("remote service is null");
        return DCAMERA_BAD_VALUE;
    }

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    if (!data.WriteInterfaceToken(DistributedCameraSinkProxy::GetDescriptor())) {
        DHLOGE("write token failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteString(requestId)) {
        DHLOGE("write requestId failed");
        return DCAMERA_BAD_VALUE;
    }
    if (!data.WriteBool(granted)) {
        DHLOGE("write granted failed");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ipcResult = remote->SendRequest(
        static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_AUTHORIZATION_RESULT), data, reply, option);
    if (ipcResult != DCAMERA_OK) {
        DHLOGE("SendRequest for SET_AUTHORIZATION_RESULT failed, ipcResult: %{public}d", ipcResult);
        return DCAMERA_BAD_VALUE;
    }

    int32_t result;
    if (!reply.ReadInt32(result)) {
        DHLOGE("read reply failed");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGI("SetAuthorizationResult end, result: %{public}d", result);
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS