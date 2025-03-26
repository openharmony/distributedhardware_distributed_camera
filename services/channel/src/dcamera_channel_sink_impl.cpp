/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dcamera_channel_sink_impl.h"

#include "dcamera_softbus_adapter.h"
#include "dcamera_utils_tools.h"

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraChannelSinkImpl::DCameraChannelSinkImpl()
{
    softbusSession_ = nullptr;
}

DCameraChannelSinkImpl::~DCameraChannelSinkImpl()
{
}

int32_t DCameraChannelSinkImpl::CloseSession()
{
    DHLOGI("DCameraChannelSinkImpl CloseSession name: %{public}s", GetAnonyString(mySessionName_).c_str());
    if (softbusSession_ == nullptr) {
        DHLOGE("DCameraChannelSinkImpl CloseSession %{public}s failed", GetAnonyString(mySessionName_).c_str());
        return DCAMERA_BAD_OPERATE;
    }
    int32_t ret = softbusSession_->CloseSession();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraChannelSinkImpl CloseSession %{public}s ret: %{public}d",
            GetAnonyString(mySessionName_).c_str(), ret);
    }

    return ret;
}

int32_t DCameraChannelSinkImpl::CreateSession(std::vector<DCameraIndex>& camIndexs, std::string sessionFlag,
    DCameraSessionMode sessionMode, std::shared_ptr<ICameraChannelListener>& listener)
{
    if (camIndexs.size() > DCAMERA_MAX_NUM || listener == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    std::string myDevId;
    int32_t ret = GetLocalDeviceNetworkId(myDevId);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraChannelSinkImpl get local device networkId error, ret%{public}d", ret);
        return ret;
    }
    camIndexs_.assign(camIndexs.begin(), camIndexs.end());
    listener_ = listener;
    if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
        mySessionName_ = SESSION_HEAD + camIndexs[0].dhId_ + std::string("_") + sessionFlag;
    } else {
        mySessionName_ = SESSION_HEAD + camIndexs[0].dhId_ + std::string("_") + sessionFlag + "_sender";
    }
    mode_ = sessionMode;
    std::string peerDevId = camIndexs[0].devId_;
    std::string dhId = camIndexs[0].dhId_;
    std::string peerSessionName = SESSION_HEAD + sessionFlag;
    DHLOGI("DCameraChannelSinkImpl CreateSession Listen Start, devId: %{public}s", GetAnonyString(myDevId).c_str());
    // sink_server_listen
    softbusSession_ = std::make_shared<DCameraSoftbusSession>(dhId, myDevId, mySessionName_, peerDevId,
        peerSessionName, listener, sessionMode);
    ret = softbusSession_->CreateSocketServer();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraChannelSinkImpl CreateSession Error, ret %{public}d", ret);
        return ret;
    }
    DCameraSoftbusAdapter::GetInstance().sinkSessions_[mySessionName_] = softbusSession_;
    DHLOGI("DCameraChannelSinkImpl CreateSession Listen End, devId: %{public}s", GetAnonyString(myDevId).c_str());
    return DCAMERA_OK;
}

int32_t DCameraChannelSinkImpl::ReleaseSession()
{
    DHLOGI("DCameraChannelSinkImpl ReleaseSession name: %{public}s", GetAnonyString(mySessionName_).c_str());
    if (softbusSession_ == nullptr) {
        return DCAMERA_OK;
    }
    softbusSession_->ReleaseSession();
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.erase(softbusSession_->GetMySessionName());
    softbusSession_ = nullptr;
    return DCAMERA_OK;
}

int32_t DCameraChannelSinkImpl::SendData(std::shared_ptr<DataBuffer>& buffer)
{
    if (softbusSession_ == nullptr) {
        DHLOGE("DCameraChannelSinkImpl SendData %{public}s failed", GetAnonyString(mySessionName_).c_str());
        return DCAMERA_BAD_OPERATE;
    }
    int32_t ret = softbusSession_->SendData(mode_, buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraChannelSinkImpl SendData %{public}s failed, ret: %{public}d",
            GetAnonyString(mySessionName_).c_str(), ret);
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
