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

#include "accesstoken_kit.h"
#include "dcamera_ipc_interface_code.h"
#include "dcamera_radar.h"
#include "distributed_camera_sink_stub.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "ipc_skeleton.h"
#include "dcamera_sink_callback_proxy.h"

namespace OHOS {
namespace DistributedHardware {
DistributedCameraSinkStub::DistributedCameraSinkStub() : IRemoteStub(true)
{
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::INIT_SINK)] =
        &DistributedCameraSinkStub::InitSinkInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::RELEASE_SINK)] =
        &DistributedCameraSinkStub::ReleaseSinkInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::SUBSCRIBE_LOCAL_HARDWARE)] =
        &DistributedCameraSinkStub::SubscribeLocalHardwareInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::UNSUBSCRIBE_LOCAL_HARDWARE)] =
        &DistributedCameraSinkStub::UnsubscribeLocalHardwareInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_CAPTURE)] =
        &DistributedCameraSinkStub::StopCaptureInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::CHANNEL_NEG)] =
        &DistributedCameraSinkStub::ChannelNegInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::GET_CAMERA_INFO)] =
        &DistributedCameraSinkStub::GetCameraInfoInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::OPEN_CHANNEL)] =
        &DistributedCameraSinkStub::OpenChannelInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::CLOSE_CHANNEL)] =
        &DistributedCameraSinkStub::CloseChannelInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE)] =
        &DistributedCameraSinkStub::PauseDistributedHardwareInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::RESUME_DISTRIBUTED_HARDWARE)] =
        &DistributedCameraSinkStub::ResumeDistributedHardwareInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_DISTRIBUTED_HARDWARE)] =
        &DistributedCameraSinkStub::StopDistributedHardwareInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_ACCESS_LISTENER)] =
        &DistributedCameraSinkStub::SetAccessListenerInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::REMOVE_ACCESS_LISTENER)] =
        &DistributedCameraSinkStub::RemoveAccessListenerInner;
    memberFuncMap_[static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_AUTHORIZATION_RESULT)] =
        &DistributedCameraSinkStub::SetAuthorizationResultInner;
}

DistributedCameraSinkStub::~DistributedCameraSinkStub()
{}

bool DistributedCameraSinkStub::HasEnableDHPermission()
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ENABLE_DISTRIBUTED_HARDWARE";
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        permissionName);
    return (result == Security::AccessToken::PERMISSION_GRANTED);
}

int32_t DistributedCameraSinkStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGD("remote request code: %{public}d", code);
    std::u16string desc = DistributedCameraSinkStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (desc != remoteDesc) {
        DHLOGE("remoteDesc is invalid!");
        return ERR_INVALID_DATA;
    }

    switch (code) {
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::INIT_SINK):
            return InitSinkInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::RELEASE_SINK):
            return ReleaseSinkInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::SUBSCRIBE_LOCAL_HARDWARE):
            return SubscribeLocalHardwareInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::UNSUBSCRIBE_LOCAL_HARDWARE):
            return UnsubscribeLocalHardwareInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_CAPTURE):
            return StopCaptureInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::CHANNEL_NEG):
            return ChannelNegInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::GET_CAMERA_INFO):
            return GetCameraInfoInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::OPEN_CHANNEL):
            return OpenChannelInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::CLOSE_CHANNEL):
            return CloseChannelInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::PAUSE_DISTRIBUTED_HARDWARE):
            return PauseDistributedHardwareInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::RESUME_DISTRIBUTED_HARDWARE):
            return ResumeDistributedHardwareInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::STOP_DISTRIBUTED_HARDWARE):
            return StopDistributedHardwareInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_ACCESS_LISTENER):
            return SetAccessListenerInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::REMOVE_ACCESS_LISTENER):
            return RemoveAccessListenerInner(data, reply);
        case static_cast<uint32_t>(IDCameraSinkInterfaceCode::SET_AUTHORIZATION_RESULT):
            return SetAuthorizationResultInner(data, reply);
        default:
            DHLOGE("Invalid OnRemoteRequest code=%{public}d", code);
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    return DCAMERA_NOT_FOUND;
}

int32_t DistributedCameraSinkStub::InitSinkInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasEnableDHPermission()) {
            DHLOGE("The caller has no ENABLE_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        std::string params = data.ReadString();
        if (params.empty() || params.size() > PARAM_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        sptr<IRemoteObject> remoteObject = data.ReadRemoteObject();
        if (remoteObject == nullptr) {
            DHLOGE("Read ReadRemoteObject failed.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }

        sptr<DCameraSinkCallbackProxy> dCameraSinkCallbackProxy(new DCameraSinkCallbackProxy(remoteObject));
        ret = InitSink(params, dCameraSinkCallbackProxy);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::ReleaseSinkInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasEnableDHPermission()) {
            DHLOGE("The caller has no ENABLE_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = ReleaseSink();
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::SubscribeLocalHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string parameters = data.ReadString();
        if (parameters.empty() || parameters.size() > PARAM_MAX_SIZE || dhId.empty() ||
            dhId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = SubscribeLocalHardware(dhId, parameters);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::UnsubscribeLocalHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = UnsubscribeLocalHardware(dhId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::StopCaptureInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = StopCapture(dhId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::ChannelNegInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string channelInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE || channelInfo.empty() ||
            channelInfo.size() > PARAM_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = ChannelNeg(dhId, channelInfo);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::GetCameraInfoInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string cameraInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = GetCameraInfo(dhId, cameraInfo);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::OpenChannelInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSinkStub OpenChannelInner Begin");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string openInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE || openInfo.empty()||
            openInfo.size() > PARAM_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = OpenChannel(dhId, openInfo);
    } while (0);
    reply.WriteInt32(ret);
    DHLOGD("DistributedCameraSinkStub OpenChannelInner End");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::CloseChannelInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = CloseChannel(dhId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

bool DistributedCameraSinkStub::HasAccessDHPermission()
{
    Security::AccessToken::AccessTokenID callerToken = IPCSkeleton::GetCallingTokenID();
    const std::string permissionName = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    int32_t result = Security::AccessToken::AccessTokenKit::VerifyAccessToken(callerToken,
        permissionName);
    return (result == Security::AccessToken::PERMISSION_GRANTED);
}

int32_t DistributedCameraSinkStub::PauseDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        std::string networkId = data.ReadString();
        if (networkId.empty() || networkId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = PauseDistributedHardware(networkId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::ResumeDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        std::string networkId = data.ReadString();
        if (networkId.empty() || networkId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = ResumeDistributedHardware(networkId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::StopDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        std::string networkId = data.ReadString();
        if (networkId.empty() || networkId.size() > DID_MAX_SIZE) {
            DHLOGE("params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = StopDistributedHardware(networkId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::SetAccessListenerInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("SetAccessListenerInner enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }

        sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
        if (remoteObj == nullptr) {
            DHLOGE("Read remote listener object failed");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        sptr<IAccessListener> listener = iface_cast<IAccessListener>(remoteObj);
        int32_t timeOut = data.ReadInt32();
        std::string pkgName = data.ReadString();
        ret = SetAccessListener(listener, timeOut, pkgName);
    } while (0);

    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::RemoveAccessListenerInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("RemoveAccessListenerInner enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        std::string pkgName = data.ReadString();
        ret = RemoveAccessListener(pkgName);
    } while (0);

    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::SetAuthorizationResultInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("SetAuthorizationResultInner enter");
    int32_t ret = DCAMERA_OK;
    do {
        if (!HasAccessDHPermission()) {
            DHLOGE("The caller has no ACCESS_DISTRIBUTED_HARDWARE permission.");
            ret = DCAMERA_BAD_VALUE;
            break;
        }

        std::string requestId = data.ReadString();
        bool granted = data.ReadBool();
        ret = SetAuthorizationResult(requestId, granted);
    } while (0);

    reply.WriteInt32(ret);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS