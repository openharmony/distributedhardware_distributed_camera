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

#include "dcamera_channel_source_impl.h"

#include "dcamera_softbus_adapter.h"

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraChannelSourceImpl::DCameraChannelSourceImpl()
{
}

DCameraChannelSourceImpl::~DCameraChannelSourceImpl()
{
}

int32_t DCameraChannelSourceImpl::CloseSession()
{
    DHLOGI("DCameraChannelSourceImpl CloseSession name: %{public}s", GetAnonyString(mySessionName_).c_str());
    if (softbusSessions_.empty()) {
        DHLOGE("DCameraChannelSourceImpl CloseSession %{public}s failed", GetAnonyString(mySessionName_).c_str());
        return DCAMERA_BAD_OPERATE;
    }
    int32_t ret = DCAMERA_OK;
    for (auto iter = softbusSessions_.begin(); iter != softbusSessions_.end(); iter++) {
        int32_t retOpen = (*iter)->CloseSession();
        if (retOpen != DCAMERA_OK) {
            DHLOGE("DCameraChannelSourceImpl CloseSession %{public}s failed, ret: %{public}d",
                GetAnonyString(mySessionName_).c_str(), retOpen);
            ret = DCAMERA_BAD_OPERATE;
        }
    }

    return ret;
}

int32_t DCameraChannelSourceImpl::CreateSession(std::vector<DCameraIndex>& camIndexs, std::string sessionFlag,
    DCameraSessionMode sessionMode, std::shared_ptr<ICameraChannelListener>& listener)
{
    if (camIndexs.size() > DCAMERA_MAX_NUM || listener == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    if (!softbusSessions_.empty()) {
        DHLOGI("DCameraChannelSourceImpl session has already create %{public}s", sessionFlag.c_str());
        return DCAMERA_OK;
    }
    camIndexs_.assign(camIndexs.begin(), camIndexs.end());
    listener_ = listener;
    mySessionName_ = SESSION_HEAD + sessionFlag;
    mode_ = sessionMode;
    std::string myDevId;
    DCameraSoftbusAdapter::GetInstance().GetLocalNetworkId(myDevId);
    DHLOGI("DCameraChannelSourceImpl CreateSession Start, name: %{public}s devId: %{public}s",
        GetAnonyString(mySessionName_).c_str(), GetAnonyString(myDevId).c_str());
    for (auto iter = camIndexs.begin(); iter != camIndexs.end(); iter++) {
        std::string peerDevId = (*iter).devId_;
        std::string peerSessionName = SESSION_HEAD + (*iter).dhId_ + std::string("_") + sessionFlag;
        // source_bind
        int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDevId, peerSessionName,
            peerDevId, sessionMode, DCAMERA_CHANNLE_ROLE_SOURCE);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraChannelSourceImpl CreateSession failed, ret: %{public}d", ret);
            return ret;
        }
        int32_t sourceSocket = DCameraSoftbusAdapter::GetInstance().GetSourceSocketId();
        std::shared_ptr<DCameraSoftbusSession> softbusSess = std::make_shared<DCameraSoftbusSession>(myDevId,
            mySessionName_, peerDevId, peerSessionName, listener, sessionMode);
        softbusSess->OnSessionOpened(sourceSocket);
        DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(sourceSocket, softbusSess);
        softbusSessions_.push_back(softbusSess);
        DCameraSoftbusAdapter::GetInstance().sourceSessions_[peerDevId + peerSessionName] = softbusSess;
    }
    DHLOGI("DCameraChannelSourceImpl CreateSession End");
    return DCAMERA_OK;
}

int32_t DCameraChannelSourceImpl::ReleaseSession()
{
    DHLOGI("DCameraChannelSourceImpl ReleaseSession name: %{public}s", GetAnonyString(mySessionName_).c_str());
    for (auto iter = softbusSessions_.begin(); iter != softbusSessions_.end(); iter++) {
        std::string sessKey = (*iter)->GetPeerDevId() + (*iter)->GetPeerSessionName();
        DCameraSoftbusAdapter::GetInstance().sourceSessions_.erase(sessKey);
    }
    std::vector<std::shared_ptr<DCameraSoftbusSession>>().swap(softbusSessions_);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(mySessionName_);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraChannelSourceImpl ReleaseSession %{public}s failed, ret: %{public}d",
            GetAnonyString(mySessionName_).c_str(), ret);
    }
    return ret;
}

int32_t DCameraChannelSourceImpl::SendData(std::shared_ptr<DataBuffer>& buffer)
{
    if (softbusSessions_.empty()) {
        DHLOGE("DCameraChannelSourceImpl SendData %{public}s failed", GetAnonyString(mySessionName_).c_str());
        return DCAMERA_BAD_OPERATE;
    }
    int32_t ret = DCAMERA_OK;
    for (auto iter = softbusSessions_.begin(); iter != softbusSessions_.end(); iter++) {
        int32_t retSend = (*iter)->SendData(mode_, buffer);
        if (retSend != DCAMERA_OK) {
            DHLOGE("DCameraChannelSourceImpl SendData %{public}s failed, ret: %{public}d",
                GetAnonyString(mySessionName_).c_str(), retSend);
            ret = DCAMERA_BAD_OPERATE;
        }
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
