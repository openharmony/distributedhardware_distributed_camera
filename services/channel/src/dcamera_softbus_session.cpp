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

#include "dcamera_softbus_session.h"

#include <securec.h>

#include "anonymous_string.h"
#include "dcamera_softbus_adapter.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_event_cmd.h"
#include "dcamera_protocol.h"
namespace OHOS {
namespace DistributedHardware {
DCameraSoftbusSession::DCameraSoftbusSession()
{
    sessionId_ = -1;
    state_ = DCAMERA_SOFTBUS_STATE_CLOSED;
    mode_ = DCAMERA_SESSION_MODE_CTRL;
    ResetAssembleFrag();
}

DCameraSoftbusSession::DCameraSoftbusSession(std::string myDhId, std::string myDevId, std::string mySessionName,
    std::string peerDevId, std::string peerSessionName, std::shared_ptr<ICameraChannelListener> listener,
    DCameraSessionMode mode)
    : myDhId_(myDhId), myDevId_(myDevId), mySessionName_(mySessionName), peerDevId_(peerDevId),
    peerSessionName_(peerSessionName), listener_(listener), sessionId_(-1), state_(DCAMERA_SOFTBUS_STATE_CLOSED),
    mode_(mode)
{
    sendFuncMap_[DCAMERA_SESSION_MODE_CTRL] = &DCameraSoftbusSession::SendBytes;
    sendFuncMap_[DCAMERA_SESSION_MODE_VIDEO] = &DCameraSoftbusSession::SendStream;
    sendFuncMap_[DCAMERA_SESSION_MODE_JPEG] = &DCameraSoftbusSession::SendBytes;
    auto runner = AppExecFwk::EventRunner::Create(mySessionName);
    eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    ResetAssembleFrag();
}

DCameraSoftbusSession::~DCameraSoftbusSession()
{
    if (sessionId_ != -1) {
        int32_t ret = DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSoftbusSession delete failed, ret: %{public}d, sessId: %{public}d peerDevId: %{public}s "
                "peerSessionName: %{public}s", ret, sessionId_, GetAnonyString(peerDevId_).c_str(),
                GetAnonyString(peerSessionName_).c_str());
        }
    }
    sendFuncMap_.clear();
    eventHandler_ = nullptr;
}

int32_t DCameraSoftbusSession::CloseSession()
{
    DHLOGI("close session sessionId: %{public}d peerDevId: %{public}s peerSessionName: %{public}s", sessionId_,
        GetAnonyString(peerDevId_).c_str(), GetAnonyString(peerSessionName_).c_str());
    if (sessionId_ == -1) {
        DHLOGI("current session has already close peerDevId: %{public}s peerSessionName: %{public}s",
            GetAnonyString(peerDevId_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_OK;
    }
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
    if (ret != DCAMERA_OK) {
        DHLOGE("close session failed, ret: %{public}d, peerDevId: %{public}s peerSessionName: %{public}s", ret,
            GetAnonyString(peerDevId_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return ret;
    }

    sessionId_ = -1;
    state_ = DCAMERA_SOFTBUS_STATE_CLOSED;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusSession::OnSessionOpened(int32_t socket, std::string networkId)
{
    DHLOGI("open current session start, socket: %{public}d", socket);
    sessionId_ = socket;
    state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    CHECK_AND_RETURN_RET_LOG(listener_ == nullptr, DCAMERA_BAD_VALUE, "listener_ is null.");
    if (isConflict_) {
        DHLOGI("OnSessionOpened session is in conflict state, not notify connected event, socket: %{public}d", socket);
        return DCAMERA_OK;
    }
    listener_->OnSessionState(DCAMERA_CHANNEL_STATE_CONNECTED, networkId);
    DHLOGI("open current session end, socket: %{public}d", socket);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusSession::OnSessionClose(int32_t sessionId)
{
    DHLOGI("OnSessionClose sessionId: %{public}d peerDevId: %{public}s peerSessionName: %{public}s", sessionId,
        GetAnonyString(peerDevId_).c_str(), GetAnonyString(peerSessionName_).c_str());
    sessionId_ = -1;
    state_ = DCAMERA_SOFTBUS_STATE_CLOSED;
    CHECK_AND_RETURN_RET_LOG(listener_ == nullptr, DCAMERA_BAD_VALUE, "listener_ is null.");
    if (isConflict_) {
        DHLOGI("OnSessionClose session is in conflict state,socket: %{public}d", sessionId);
        return DCAMERA_OK;
    }
    listener_->OnSessionState(DCAMERA_CHANNEL_STATE_DISCONNECTED, "");
    return DCAMERA_OK;
}

int32_t DCameraSoftbusSession::OnDataReceived(std::shared_ptr<DataBuffer>& buffer)
{
    auto recvDataFunc = [this, buffer]() mutable {
        DealRecvData(buffer);
    };
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(recvDataFunc);
    }
    return DCAMERA_OK;
}

void DCameraSoftbusSession::DealRecvData(std::shared_ptr<DataBuffer>& buffer)
{
    if (mode_ == DCAMERA_SESSION_MODE_VIDEO) {
        PostData(buffer);
        return;
    }
    PackRecvData(buffer);
    return;
}

void DCameraSoftbusSession::PackRecvData(std::shared_ptr<DataBuffer>& buffer)
{
    if (buffer == nullptr) {
        DHLOGE("Data buffer is null");
        return;
    }
    uint64_t bufferSize;
    if (buffer->Size() < BINARY_HEADER_FRAG_LEN) {
        bufferSize = static_cast<uint64_t>(buffer->Size());
        DHLOGE("pack recv data error, size: %{public}" PRIu64", sess: %{public}s peerSess: %{public}s",
            bufferSize, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return;
    }
    uint8_t *ptrPacket = buffer->Data();
    SessionDataHeader headerPara;
    GetFragDataLen(ptrPacket, headerPara);
    if (buffer->Size() != (headerPara.dataLen + BINARY_HEADER_FRAG_LEN) || headerPara.dataLen > headerPara.totalLen ||
        headerPara.dataLen > BINARY_DATA_MAX_LEN || headerPara.totalLen > BINARY_DATA_MAX_TOTAL_LEN) {
        bufferSize = static_cast<uint64_t>(buffer->Size());
        DHLOGE("pack recv data failed, size: %{public}" PRIu64", dataLen: %{public}d, totalLen: %{public}d sess: "
            "%{public}s peerSess: %{public}s", bufferSize, headerPara.dataLen, headerPara.totalLen,
            GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return;
    }
    bufferSize = static_cast<uint64_t>(buffer->Size());
    DHLOGD("pack recv data Assemble, size: %{public}" PRIu64", dataLen: %{public}d, totalLen: %{public}d, nowTime: "
        "%{public}" PRId64" start", bufferSize, headerPara.dataLen, headerPara.totalLen, GetNowTimeStampUs());
    if (headerPara.fragFlag == FRAG_START_END) {
        AssembleNoFrag(buffer, headerPara);
    } else {
        AssembleFrag(buffer, headerPara);
    }
    bufferSize = static_cast<uint64_t>(buffer->Size());
    DHLOGD("pack recv data Assemble, size: %{public}" PRIu64", dataLen: %{public}d, totalLen: %{public}d, nowTime: "
        "%{public}" PRId64" end", bufferSize, headerPara.dataLen, headerPara.totalLen, GetNowTimeStampUs());
}

void DCameraSoftbusSession::AssembleNoFrag(std::shared_ptr<DataBuffer>& buffer, SessionDataHeader& headerPara)
{
    if (headerPara.dataLen != headerPara.totalLen) {
        DHLOGE("DCameraSoftbusSession PackRecvData failed, dataLen: %{public}d, totalLen: %{public}d, sess: "
            "%{public}s peerSess: %{public}s",
            headerPara.dataLen, headerPara.totalLen, GetAnonyString(mySessionName_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
        return;
    }
    if (buffer == nullptr) {
        DHLOGE("Data buffer is null");
        return;
    }
    std::shared_ptr<DataBuffer> postData = std::make_shared<DataBuffer>(headerPara.dataLen);
    int32_t ret = memcpy_s(postData->Data(), postData->Size(), buffer->Data() + BINARY_HEADER_FRAG_LEN,
        buffer->Size() - BINARY_HEADER_FRAG_LEN);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusSession PackRecvData failed, ret: %{public}d, sess: %{public}s peerSess: %{public}s",
            ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return;
    }
    PostData(postData);
}

void DCameraSoftbusSession::AssembleFrag(std::shared_ptr<DataBuffer>& buffer, SessionDataHeader& headerPara)
{
    if (buffer == nullptr) {
        DHLOGE("Data buffer is null");
        return;
    }
    if (headerPara.fragFlag == FRAG_START) {
        isWaiting_ = true;
        nowSeq_ = headerPara.seqNum;
        nowSubSeq_ = headerPara.subSeq;
        offset_ = 0;
        totalLen_ = headerPara.totalLen;
        packBuffer_ = std::make_shared<DataBuffer>(headerPara.totalLen);
        int32_t ret = memcpy_s(packBuffer_->Data(), packBuffer_->Size(), buffer->Data() + BINARY_HEADER_FRAG_LEN,
            buffer->Size() - BINARY_HEADER_FRAG_LEN);
        if (ret != EOK) {
            DHLOGE("DCameraSoftbusSession AssembleFrag failed, ret: %{public}d, sess: %{public}s peerSess: %{public}s",
                ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
            ResetAssembleFrag();
            return;
        }
        offset_ += headerPara.dataLen;
    }

    if (headerPara.fragFlag == FRAG_MID || headerPara.fragFlag == FRAG_END) {
        int32_t ret = CheckUnPackBuffer(headerPara);
        if (ret != DCAMERA_OK) {
            ResetAssembleFrag();
            return;
        }

        nowSubSeq_ = headerPara.subSeq;
        ret = memcpy_s(packBuffer_->Data() + offset_, packBuffer_->Size() - offset_,
            buffer->Data() + BINARY_HEADER_FRAG_LEN, buffer->Size() - BINARY_HEADER_FRAG_LEN);
        if (ret != EOK) {
            DHLOGE("DCameraSoftbusSession AssembleFrag failed, memcpy_s ret: %{public}d, sess: %{public}s peerSess: "
                "%{public}s", ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
            ResetAssembleFrag();
            return;
        }
        offset_ += headerPara.dataLen;
    }

    if (headerPara.fragFlag == FRAG_END) {
        PostData(packBuffer_);
        ResetAssembleFrag();
    }
}

int32_t DCameraSoftbusSession::CheckUnPackBuffer(SessionDataHeader& headerPara)
{
    if (!isWaiting_) {
        DHLOGE("DCameraSoftbusSession AssembleFrag failed, not start one, sess: %{public}s peerSess: %{public}s",
            GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    if (nowSeq_ != headerPara.seqNum) {
        DHLOGE("DCameraSoftbusSession AssembleFrag seq error nowSeq: %{public}d actualSeq: %{public}d, sess: "
            "%{public}s peerSess: %{public}s", nowSeq_, headerPara.seqNum, GetAnonyString(mySessionName_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    if (nowSubSeq_ + 1 != headerPara.subSeq) {
        DHLOGE("DCameraSoftbusSession AssembleFrag subSeq error nowSeq: %{public}d actualSeq: %{public}d, "
            "sess: %{public}s peerSess: %{public}s",
            nowSubSeq_, headerPara.subSeq, GetAnonyString(mySessionName_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    if (totalLen_ < headerPara.dataLen + offset_) {
        DHLOGE("DCameraSoftbusSession AssembleFrag len error cap: %{public}d size: %{public}d, dataLen: "
            "%{public}d sess: %{public}s peerSess: %{public}s", totalLen_, offset_, headerPara.dataLen,
            GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    return DCAMERA_OK;
}

void DCameraSoftbusSession::ResetAssembleFrag()
{
    isWaiting_ = false;
    nowSeq_ = 0;
    nowSubSeq_ = 0;
    offset_ = 0;
    totalLen_ = 0;
    packBuffer_ = nullptr;
}

void DCameraSoftbusSession::PostData(std::shared_ptr<DataBuffer>& buffer)
{
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    buffers.push_back(buffer);
    CHECK_AND_RETURN_LOG(listener_ == nullptr, "listener_ is null.");
    listener_->OnDataReceived(buffers);
}

void DCameraSoftbusSession::GetFragDataLen(uint8_t *ptrPacket, SessionDataHeader& headerPara)
{
    headerPara.version = U16Get(ptrPacket);
    headerPara.fragFlag = ptrPacket[BINARY_HEADER_FRAG_OFFSET];
    headerPara.dataType = U32Get(ptrPacket + BINARY_HEADER_DATATYPE_OFFSET);
    headerPara.seqNum = U32Get(ptrPacket + BINARY_HEADER_SEQNUM_OFFSET);
    headerPara.totalLen = U32Get(ptrPacket + BINARY_HEADER_TOTALLEN_OFFSET);
    headerPara.subSeq = U16Get(ptrPacket + BINARY_HEADER_SUBSEQ_OFFSET);
    headerPara.dataLen = U32Get(ptrPacket + BINARY_HEADER_DATALEN_OFFSET);
}

uint16_t DCameraSoftbusSession::U16Get(const uint8_t *ptr)
{
    return (ptr[0] << DCAMERA_SHIFT_8) | ptr[1];
}

uint32_t DCameraSoftbusSession::U32Get(const uint8_t *ptr)
{
    return (ptr[0] << DCAMERA_SHIFT_24) | (ptr[1] << DCAMERA_SHIFT_16) | (ptr[2] << DCAMERA_SHIFT_8) | ptr[3];
}

int32_t DCameraSoftbusSession::SendData(DCameraSessionMode mode, std::shared_ptr<DataBuffer>& buffer)
{
    auto itFunc = sendFuncMap_.find(mode);
    if (itFunc == sendFuncMap_.end()) {
        return DCAMERA_NOT_FOUND;
    }
    auto memberFunc = itFunc->second;
    switch (mode) {
        case DCAMERA_SESSION_MODE_VIDEO:
            return SendStream(buffer);
        case DCAMERA_SESSION_MODE_CTRL:
        case DCAMERA_SESSION_MODE_JPEG:
            return UnPackSendData(buffer, memberFunc);
        default:
            return UnPackSendData(buffer, memberFunc);
    }
    return DCAMERA_NOT_FOUND;
}

int32_t DCameraSoftbusSession::CreateSocketServer()
{
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(mySessionName_,
        DCAMERA_CHANNLE_ROLE_SINK, mode_, peerDevId_, peerSessionName_);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusSession CreateSocketServer Error, ret %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusSession::BindSocketServer()
{
    int32_t socketId = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId_, myDevId_,
        peerSessionName_, peerDevId_, mode_, DCAMERA_CHANNLE_ROLE_SOURCE);
    if (socketId == 0 || socketId == DCAMERA_BAD_VALUE) {
        DHLOGE("DCameraSoftbusSession BindSocketServer Error, socketId %{public}d", socketId);
        return socketId;
    }
    OnSessionOpened(socketId, myDevId_);
    return socketId;
}

void DCameraSoftbusSession::ReleaseSession()
{
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(mySessionName_);
    DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId_);
}

int32_t DCameraSoftbusSession::UnPackSendData(std::shared_ptr<DataBuffer>& buffer, DCameraSendFuc memberFunc)
{
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, DCAMERA_BAD_VALUE, "Data buffer is null");
    uint16_t subSeq = 0;
    uint32_t seq = 0;
    uint32_t totalLen = buffer->Size();
    SessionDataHeader headPara = { PROTOCOL_VERSION, FRAG_START, mode_, seq, totalLen, subSeq };
    if (buffer->Size() <= BINARY_DATA_PACKET_MAX_LEN) {
        headPara.fragFlag = FRAG_START_END;
        headPara.dataLen = buffer->Size();
        std::shared_ptr<DataBuffer> unpackData = std::make_shared<DataBuffer>(buffer->Size() + BINARY_HEADER_FRAG_LEN);
        MakeFragDataHeader(headPara, unpackData->Data(), BINARY_HEADER_FRAG_LEN);
        int32_t ret = memcpy_s(unpackData->Data() + BINARY_HEADER_FRAG_LEN, unpackData->Size() - BINARY_HEADER_FRAG_LEN,
            buffer->Data(), buffer->Size());
        if (ret != EOK) {
            DHLOGE("UnPackSendData START_END memcpy_s failed, ret: %{public}d, sess: %{public}s peerSess: %{public}s",
                ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
            return ret;
        }
        return SendBytes(unpackData);
    }
    uint32_t offset = 0;
    while (totalLen > offset) {
        SetHeadParaDataLen(headPara, totalLen, offset);
        uint64_t bufferSize = static_cast<uint64_t>(buffer->Size());
        DHLOGD("DCameraSoftbusSession UnPackSendData, size: %" PRIu64", dataLen: %{public}d, totalLen: %{public}d, "
            "nowTime: %{public}" PRId64" start:", bufferSize, headPara.dataLen, headPara.totalLen, GetNowTimeStampUs());
        std::shared_ptr<DataBuffer> unpackData =
            std::make_shared<DataBuffer>(headPara.dataLen + BINARY_HEADER_FRAG_LEN);
        MakeFragDataHeader(headPara, unpackData->Data(), BINARY_HEADER_FRAG_LEN);
        int ret = memcpy_s(unpackData->Data() + BINARY_HEADER_FRAG_LEN, unpackData->Size() - BINARY_HEADER_FRAG_LEN,
            buffer->Data() + offset, headPara.dataLen);
        if (ret != EOK) {
            DHLOGE("DCameraSoftbusSession UnPackSendData memcpy_s failed, ret: %{public}d, sess: %{public}s peerSess: "
                "%{public}s", ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
            return ret;
        }
        ret = SendBytes(unpackData);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSoftbusSession sendData failed, ret: %{public}d, sess: %{public}s peerSess: %{public}s",
                ret, GetAnonyString(mySessionName_).c_str(), GetAnonyString(peerSessionName_).c_str());
            return ret;
        }
        DHLOGD("DCameraSoftbusSession UnPackSendData, size: %" PRIu64", dataLen: %{public}d, totalLen: %{public}d, "
            "nowTime: %{public}" PRId64" end:", bufferSize, headPara.dataLen, headPara.totalLen, GetNowTimeStampUs());
        headPara.subSeq++;
        headPara.fragFlag = FRAG_MID;
        offset += headPara.dataLen;
    }
    return DCAMERA_OK;
}

void DCameraSoftbusSession::SetHeadParaDataLen(SessionDataHeader& headPara, const uint32_t totalLen,
    const uint32_t offset)
{
    if (totalLen >= offset) {
        if (totalLen - offset > BINARY_DATA_PACKET_MAX_LEN) {
            headPara.dataLen = BINARY_DATA_PACKET_MAX_LEN - BINARY_DATA_PACKET_RESERVED_BUFFER;
        } else {
            headPara.fragFlag = FRAG_END;
            headPara.dataLen = totalLen - offset;
        }
    }
}

void DCameraSoftbusSession::MakeFragDataHeader(const SessionDataHeader& headPara, uint8_t *header, uint32_t len)
{
    uint32_t headerLen = sizeof(uint8_t) * HEADER_UINT8_NUM + sizeof(uint16_t) * HEADER_UINT16_NUM +
        sizeof(uint32_t) * HEADER_UINT32_NUM;
    if (headerLen > len) {
        DHLOGE("MakeFragDataHeader %{public}d over len %{public}d", headerLen, len);
        return;
    }
    uint32_t i = 0;
    header[i++] = headPara.version >> DCAMERA_SHIFT_8;
    header[i++] = headPara.version & UINT16_SHIFT_MASK_0;
    header[i++] = headPara.fragFlag;
    header[i++] = (headPara.dataType & UINT32_SHIFT_MASK_24) >> DCAMERA_SHIFT_24;
    header[i++] = (headPara.dataType & UINT32_SHIFT_MASK_16) >> DCAMERA_SHIFT_16;
    header[i++] = (headPara.dataType & UINT32_SHIFT_MASK_8) >> DCAMERA_SHIFT_8;
    header[i++] = (headPara.dataType & UINT32_SHIFT_MASK_0);
    header[i++] = (headPara.seqNum & UINT32_SHIFT_MASK_24) >> DCAMERA_SHIFT_24;
    header[i++] = (headPara.seqNum & UINT32_SHIFT_MASK_16) >> DCAMERA_SHIFT_16;
    header[i++] = (headPara.seqNum & UINT32_SHIFT_MASK_8) >> DCAMERA_SHIFT_8;
    header[i++] = (headPara.seqNum & UINT32_SHIFT_MASK_0);
    header[i++] = (headPara.totalLen & UINT32_SHIFT_MASK_24) >> DCAMERA_SHIFT_24;
    header[i++] = (headPara.totalLen & UINT32_SHIFT_MASK_16) >> DCAMERA_SHIFT_16;
    header[i++] = (headPara.totalLen & UINT32_SHIFT_MASK_8) >> DCAMERA_SHIFT_8;
    header[i++] = (headPara.totalLen & UINT32_SHIFT_MASK_0);
    header[i++] = headPara.subSeq >> DCAMERA_SHIFT_8;
    header[i++] = headPara.subSeq & UINT16_SHIFT_MASK_0;
    header[i++] = (headPara.dataLen & UINT32_SHIFT_MASK_24) >> DCAMERA_SHIFT_24;
    header[i++] = (headPara.dataLen & UINT32_SHIFT_MASK_16) >> DCAMERA_SHIFT_16;
    header[i++] = (headPara.dataLen & UINT32_SHIFT_MASK_8) >> DCAMERA_SHIFT_8;
    header[i++] = (headPara.dataLen & UINT32_SHIFT_MASK_0);
}

int32_t DCameraSoftbusSession::SendBytes(std::shared_ptr<DataBuffer>& buffer)
{
    if (state_ != DCAMERA_SOFTBUS_STATE_OPENED) {
        DHLOGE("DCameraSoftbusSession SendBytes session state %{public}d is not opened sessionId: %{public}d "
            "peerDev: %{public}s peerName: %{public}s", state_, sessionId_, GetAnonyString(peerDevId_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_WRONG_STATE;
    }

    int32_t ret = DCameraSoftbusAdapter::GetInstance().SendSofbusBytes(sessionId_, buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusSession SendBytes sessionId: %{public}d failed: %{public}d peerDevId: %{public}s "
            "peerSessionName: %{public}s", sessionId_, ret, GetAnonyString(peerDevId_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
    }
    return ret;
}

int32_t DCameraSoftbusSession::SendStream(std::shared_ptr<DataBuffer>& buffer)
{
    if (state_ != DCAMERA_SOFTBUS_STATE_OPENED) {
        DHLOGE("DCameraSoftbusSession SendStream session state %{public}d is not opened sessionId: %{public}d "
            "peerDev: %{public}s peerName: %{public}s", state_, sessionId_, GetAnonyString(peerDevId_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
        return DCAMERA_WRONG_STATE;
    }

    int32_t ret = DCameraSoftbusAdapter::GetInstance().SendSofbusStream(sessionId_, buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusSession SendStream sessionId: %{public}d failed: %{public}d peerDevId: %{public}s "
            "peerSessionName: %{public}s", sessionId_, ret, GetAnonyString(peerDevId_).c_str(),
            GetAnonyString(peerSessionName_).c_str());
    }
    return ret;
}

std::string DCameraSoftbusSession::GetPeerDevId()
{
    return peerDevId_;
}

std::string DCameraSoftbusSession::GetPeerSessionName()
{
    return peerSessionName_;
}

std::string DCameraSoftbusSession::GetMySessionName()
{
    return mySessionName_;
}

int32_t DCameraSoftbusSession::GetSessionId()
{
    return sessionId_;
}

void DCameraSoftbusSession::SetSessionId(int32_t sessionId)
{
    sessionId_ = sessionId;
}

std::string DCameraSoftbusSession::GetMyDhId()
{
    return myDhId_;
}

void DCameraSoftbusSession::SetConflict(bool isConflict)
{
    isConflict_ = isConflict;
}

int32_t DCameraSoftbusSession::NotifyError(int32_t eventType, int32_t eventReason, const std::string& detail)
{
    DHLOGI("NotifyError eventType: %{public}d, eventReason: %{public}d", eventType, eventReason);
    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = eventType;
    event->eventResult_ = eventReason;
    event->eventContent_ = detail;
      
    DCameraEventCmd errorCmd;
    errorCmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
    errorCmd.dhId_ = myDhId_;
    errorCmd.command_ = DCAMERA_PROTOCOL_CMD_STATE_NOTIFY;
    errorCmd.value_ = event;
       
    std::string jsonStr = "";
    int32_t ret = errorCmd.Marshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("NotifyError Marshal failed,dhId: %{public}s, ret: %{public}d",
               GetAnonyString(myDhId_).c_str(), ret);
        return ret;
    }
      
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(),
        reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())), jsonStr.length() + 1);
    CHECK_AND_RETURN_RET_LOG(ret != EOK, DCAMERA_BAD_VALUE, "NotifyError memcpy_s failed, ret: %{public}d", ret);
    ret = SendData(DCAMERA_SESSION_MODE_CTRL, buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("NotifyError SendData failed, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(myDhId_).c_str(), ret);
        return ret;
    }
    DHLOGI("NotifyError SendData success, dhId: %{public}s", GetAnonyString(myDhId_).c_str());
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
