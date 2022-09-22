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

#include "distributed_camera_source_stub.h"

#include "dcamera_source_callback_proxy.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DistributedCameraSourceStub::DistributedCameraSourceStub()
{
    memberFuncMap_[INIT_SOURCE] = &DistributedCameraSourceStub::InitSourceInner;
    memberFuncMap_[RELEASE_SOURCE] = &DistributedCameraSourceStub::ReleaseSourceInner;
    memberFuncMap_[REGISTER_DISTRIBUTED_HARDWARE] = &DistributedCameraSourceStub::RegisterDistributedHardwareInner;
    memberFuncMap_[UNREGISTER_DISTRIBUTED_HARDWARE] = &DistributedCameraSourceStub::UnregisterDistributedHardwareInner;
    memberFuncMap_[CAMERA_NOTIFY] = &DistributedCameraSourceStub::DCameraNotifyInner;
}

DistributedCameraSourceStub::~DistributedCameraSourceStub()
{}

int32_t DistributedCameraSourceStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
{
    DHLOGD("DistributedCameraSourceStub OnRemoteRequest code: %u", code);
    std::u16string desc = DistributedCameraSourceStub::GetDescriptor();
    std::u16string remoteDesc = data.ReadInterfaceToken();
    if (desc != remoteDesc) {
        DHLOGE("DistributedCameraSourceStub::OnRemoteRequest remoteDesc is invalid!");
        return ERR_INVALID_DATA;
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc == memberFuncMap_.end()) {
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }

    auto memberFunc = itFunc->second;
    return (this->*memberFunc)(data, reply);
}

int32_t DistributedCameraSourceStub::InitSourceInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSourceStub InitSourceInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string params = data.ReadString();
        if (params.empty() || params.size() > PARAM_MAX_SIZE) {
            DHLOGE("DistributedCameraSourceStub InitSourceInner input params is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        sptr<IRemoteObject> remoteObj = data.ReadRemoteObject();
        if (remoteObj == nullptr) {
            DHLOGE("DistributedCameraSourceStub initSource read object failed");
            ret = DCAMERA_BAD_VALUE;
            break;
        }

        sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObj));
        if (callbackProxy == nullptr) {
            DHLOGE("DistributedCameraSourceStub initSource get proxy failed");
            ret = DCAMERA_BAD_VALUE;
            break;
        }

        ret = InitSource(params, callbackProxy);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceStub::ReleaseSourceInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSourceStub ReleaseSourceInner");
    (void)data;
    int32_t ret = ReleaseSource();
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceStub::RegisterDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSourceStub RegisterDistributedHardwareInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string devId = data.ReadString();
        std::string dhId = data.ReadString();
        std::string reqId = data.ReadString();
        EnableParam params;
        params.version = data.ReadString();
        params.attrs = data.ReadString();
        if (!CheckRegParams(devId, dhId, reqId, params)) {
            DHLOGE("DistributedCameraSourceStub RegisterDistributedHardwareInner input is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = RegisterDistributedHardware(devId, dhId, reqId, params);
        DHLOGI("DistributedCameraSourceStub RegisterDistributedHardware %d", ret);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

bool DistributedCameraSourceStub::CheckRegParams(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckRegParams devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckRegParams reqId is invalid");
        return false;
    }

    if (param.version.empty() || param.version.size() > PARAM_MAX_SIZE ||
        param.attrs.empty() || param.attrs.size() > PARAM_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckRegParams param is invalid");
        return false;
    }
    return true;
}

int32_t DistributedCameraSourceStub::UnregisterDistributedHardwareInner(MessageParcel &data, MessageParcel &reply)
{
    DHLOGD("DistributedCameraSourceStub UnregisterDistributedHardwareInner");
    int32_t ret = DCAMERA_OK;
    do {
        std::string devId = data.ReadString();
        std::string dhId = data.ReadString();
        std::string reqId = data.ReadString();
        if (!CheckUnregParams(devId, dhId, reqId)) {
            DHLOGE("DistributedCameraSourceService UnregisterDistributedHardware input is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = UnregisterDistributedHardware(devId, dhId, reqId);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

bool DistributedCameraSourceStub::CheckUnregParams(const std::string& devId, const std::string& dhId,
    const std::string& reqId)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckUnregParams devId or dhId is invalid");
        return false;
    }

    if (reqId.empty() || reqId.size() > DID_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckUnregParams reqId is invalid");
        return false;
    }
    return true;
}

int32_t DistributedCameraSourceStub::DCameraNotifyInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = DCAMERA_OK;
    do {
        std::string devId = data.ReadString();
        std::string dhId = data.ReadString();
        std::string events = data.ReadString();
        if (!CheckNotifyParams(devId, dhId, events)) {
            DHLOGE("DistributedCameraSourceStub DCameraNotifyInner input is invalid");
            ret = DCAMERA_BAD_VALUE;
            break;
        }
        ret = DCameraNotify(devId, dhId, events);
    } while (0);
    reply.WriteInt32(ret);
    return DCAMERA_OK;
}

bool DistributedCameraSourceStub::CheckNotifyParams(const std::string& devId, const std::string& dhId,
    std::string& events)
{
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckNotifyParams devId or dhId is invalid");
        return false;
    }

    if (events.empty() || events.size() > PARAM_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceStub CheckNotifyParams events is invalid");
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS
