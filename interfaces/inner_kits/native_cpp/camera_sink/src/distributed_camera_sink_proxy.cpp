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
    if (!data.WriteRemoteObject(sinkCallback->AsObject())) {
        DHLOGE("write sinkCallback failed");
        return DCAMERA_BAD_VALUE;
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::SUBSCRIBE_LOCAL_HARDWARE), data, reply,
        option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::UNSUBSCRIBE_LOCAL_HARDWARE), data, reply,
        option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_CAPTURE), data, reply, option);
    int32_t result = reply.ReadInt32();
    DHLOGI("async dhId: %{public}s", GetAnonyString(dhId).c_str());
    return result;
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::CHANNEL_NEG), data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}

int32_t DistributedCameraSinkProxy::GetCameraInfo(const std::string& dhId, std::string& cameraInfo)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("parmas is invalid");
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::GET_CAMERA_INFO), data, reply, option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::OPEN_CHANNEL), data, reply, option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::CLOSE_CHANNEL), data, reply, option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::RESUME_DISTRIBUTED_HARDWARE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
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
    remote->SendRequest(static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_DISTRIBUTED_HARDWARE),
        data, reply, option);
    int32_t result = reply.ReadInt32();
    return result;
}
} // namespace DistributedHardware
} // namespace OHOS