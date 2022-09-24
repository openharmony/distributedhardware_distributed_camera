/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "distributed_camera_sink_stub.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DistributedCameraSinkStub::DistributedCameraSinkStub()
{
    memberFuncMap_[INIT_SINK] = &DistributedCameraSinkStub::InitSinkInner;
    memberFuncMap_[RELEASE_SINK] = &DistributedCameraSinkStub::ReleaseSinkInner;
    memberFuncMap_[SUBSCRIBE_LOCAL_HARDWARE] = &DistributedCameraSinkStub::SubscribeLocalHardwareInner;
    memberFuncMap_[UNSUBSCRIBE_LOCAL_HARDWARE] = &DistributedCameraSinkStub::UnsubscribeLocalHardwareInner;
    memberFuncMap_[STOP_CAPTURE] = &DistributedCameraSinkStub::StopCaptureInner;
    memberFuncMap_[CHANNEL_NEG] = &DistributedCameraSinkStub::ChannelNegInner;
    memberFuncMap_[GET_CAMERA_INFO] = &DistributedCameraSinkStub::GetCameraInfoInner;
    memberFuncMap_[OPEN_CHANNEL] = &DistributedCameraSinkStub::OpenChannelInner;
    memberFuncMap_[CLOSE_CHANNEL] = &DistributedCameraSinkStub::CloseChannelInner;
}

DistributedCameraSinkStub::~DistributedCameraSinkStub()
{}

int32_t DistributedCameraSinkStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGD("DistributedCameraSinkStub::OnRemoteRequest code: %d", code);
    std::u16string desc = DistributedCameraSinkStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (desc != remoteDesc) {
        DHLOGE("DistributedCameraSinkStub::OnRemoteRequest remoteDesc is invalid!");
        return ERR_INVALID_DATA;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc == memberFuncMap_.end()) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    auto memberFunc = itFunc->second;
    return (this->*memberFunc)(data, reply);
}

int32_t DistributedCameraSinkStub::InitSinkInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSinkStub::InitSinkInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string params = data.ReadString();
        if (params.empty() || params.size() > PARAM_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::InitSinkInner params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = InitSink(params);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::ReleaseSinkInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSinkStub::ReleaseSinkInner");
    int32_t ret = ReleaseSink();
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::SubscribeLocalHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSinkStub::SubscribeLocalHardwareInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string parameters = data.ReadString();
        if (parameters.empty() || parameters.size() > PARAM_MAX_SIZE || dhId.empty() ||
            dhId.size() > DID_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::SubscribeLocalHardwareInner params is invalid");
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
    DHLOGD("DistributedCameraSinkStub::UnsubscribeLocalHardwareInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::UnsubscribeLocalHardwareInner params is invalid");
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
    DHLOGD("DistributedCameraSinkStub::StopCaptureInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::StopCaptureInner params is invalid");
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
    DHLOGD("DistributedCameraSinkStub::ChannelNegInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string channelInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE || channelInfo.empty() ||
            channelInfo.size() > PARAM_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::ChannelNegInner params is invalid");
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
    DHLOGD("DistributedCameraSinkStub::GetCameraInfoInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string cameraInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::GetCameraInfoInner params is invalid");
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
    DHLOGD("DistributedCameraSinkStub::OpenChannelInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        std::string openInfo = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE || openInfo.empty()||
            openInfo.size() > PARAM_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::OpenChannelInner params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = OpenChannel(dhId, openInfo);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkStub::CloseChannelInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSinkStub::CloseChannelInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string dhId = data.ReadString();
        if (dhId.empty() || dhId.size() > DID_MAX_SIZE) {
            DHLOGE("DistributedCameraSinkStub::CloseChannelInner params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = CloseChannel(dhId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS