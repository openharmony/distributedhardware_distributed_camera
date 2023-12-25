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

#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_sink_frame_info.h"
#include "dcamera_softbus_adapter.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include <securec.h>
#include "softbus_bus_center.h"
#include "softbus_common.h"
#include "softbus_errcode.h"
#include "dcamera_utils_tools.h"
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraSoftbusAdapter);

static void DCameraSourceOnBind(int32_t socket, PeerSocketInfo info)
{
    return;
}

static void DCameraSourceOnShutDown(int32_t socket, ShutdownReason reason)
{
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(socket, reason);
    return;
}

static void DCameraSourceOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen);
    return;
}

static void DCameraSourceOnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().SourceOnMessage(socket, data, dataLen);
    return;
}

static void DCameraSourceOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, data, ext, param);
    return;
}

// sink
static void DCameraSinkOnBind(int32_t socket, PeerSocketInfo info)
{
    DCameraSoftbusAdapter::GetInstance().SinkOnBind(socket, info);
    return;
}

static void DCameraSinkOnShutDown(int32_t socket, ShutdownReason reason)
{
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socket, reason);
    return;
}

static void DCameraSinkOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(socket, data, dataLen);
    return;
}

static void DCameraSinkOnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().SinkOnMessage(socket, data, dataLen);
    return;
}

static void DCameraSinkOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(socket, data, ext, param);
    return;
}

DCameraSoftbusAdapter::DCameraSoftbusAdapter()
{
    sessionModeAndDataTypeMap_[DCAMERA_SESSION_MODE_CTRL] = TransDataType::DATA_TYPE_BYTES;
    sessionModeAndDataTypeMap_[DCAMERA_SESSION_MODE_VIDEO] = TransDataType::DATA_TYPE_VIDEO_STREAM;
    sessionModeAndDataTypeMap_[DCAMERA_SESSION_MODE_JPEG] = TransDataType::DATA_TYPE_RAW_STREAM;

    ISocketListener sourceListener;
    sourceListener.OnBind = DCameraSourceOnBind;
    sourceListener.OnShutdown = DCameraSourceOnShutDown;
    sourceListener.OnBytes = DCameraSourceOnBytes;
    sourceListener.OnMessage = DCameraSourceOnMessage;
    sourceListener.OnStream = DCameraSourceOnStream;
    sessListeners_[DCAMERA_CHANNLE_ROLE_SOURCE] = sourceListener;

    ISocketListener sinkListener;
    sinkListener.OnBind = DCameraSinkOnBind;
    sinkListener.OnShutdown = DCameraSinkOnShutDown;
    sinkListener.OnBytes = DCameraSinkOnBytes;
    sinkListener.OnMessage = DCameraSinkOnMessage;
    sinkListener.OnStream = DCameraSinkOnStream;
    sessListeners_[DCAMERA_CHANNLE_ROLE_SINK] = sinkListener;
}

DCameraSoftbusAdapter::~DCameraSoftbusAdapter()
{
}

int32_t DCameraSoftbusAdapter::CreatSoftBusSinkSocketServer(std::string mySessionName, DCAMERA_CHANNEL_ROLE role,
    DCameraSessionMode sessionMode, std::string peerDevId, std::string peerSessionName)
{
    DHLOGI("CreatSoftBusSinkSocketServer start, mySessionName: %s,peerSessionName: %s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    {
        std::lock_guard<std::mutex> autoLock(mySessionNameLock_);
        if (mySessionNameSet_.find(mySessionName) ==mySessionNameSet_.end()) {
            mySessionNameSet_.insert(mySessionName);
        } else {
            DHLOGI("current mySessionName had Listened");
            return DCAMERA_OK;
        }
    }
    QosTV qos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 160 * 1024 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 4000 },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 2000}
    };
    SocketInfo serverSocketInfo = {
        .name = (char *)mySessionName.c_str(),
        .pkgName = (char *)PKG_NAME.c_str(),
        .peerNetworkId = (char *)peerDevId.c_str(),
        .peerName = (char *)peerSessionName.c_str(),
        .dataType = sessionModeAndDataTypeMap_[sessionMode],
    };
    int socketId = Socket(serverSocketInfo);
    if (socketId < 0) {
        DHLOGE("DCameraSoftbusAdapter CreatSoftBusSinkSocketServer Error, socket is invalid");
        return DCAMERA_BAD_VALUE;
    }
    int ret = Listen(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter CreatSoftBusSinkSocketServer Error");
        Shutdown(socketId);
        return DCAMERA_BAD_VALUE;
    }
    {
        std::string peerDevIdMySessionName = peerDevId + std::string("_") + mySessionName;
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        peerDevIdMySessionNameMap_[peerDevIdMySessionName] = mySessionName;
    }
    DHLOGI("CreatSoftBusSinkSocketServer End, mySessionName: %s, peerSessionName: %s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CreateSoftBusSourceSocketClient(std::string myDevId, std::string peerSessionName,
    std::string peerDevId, DCameraSessionMode sessionMode, DCAMERA_CHANNEL_ROLE role)
{
    DHLOGI("CreateSoftBusSourceSocketClient start, myDevId: %s, peerSessionName: %s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    QosTV qos[] = {
        { .qos = QOS_TYPE_MIN_BW, .value = 160 * 1024 * 1024 },
        { .qos = QOS_TYPE_MAX_LATENCY, .value = 4000 },
        { .qos = QOS_TYPE_MIN_LATENCY, .value = 2000}
    };
    std::string myDevIdPeerSessionName = myDevId + std::string("_") + peerSessionName;
    SocketInfo clientSocketInfo = {
        .name = (char *)myDevIdPeerSessionName.c_str(),
        .pkgName = (char *)PKG_NAME.c_str(),
        .peerNetworkId = (char *)peerDevId.c_str(),
        .peerName = (char *)peerSessionName.c_str(),
        .dataType = sessionModeAndDataTypeMap_[sessionMode],
    };
    int socketId = Socket(clientSocketInfo);
    if (socketId < 0) {
        DHLOGE("DCameraSoftbusAdapter CreateSoftBusSourceSocketClient Error, socket is invalid");
        return DCAMERA_BAD_VALUE;
    }
    int ret = Bind(socketId, qos, sizeof(qos) / sizeof(qos[0]), &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter CreateSoftBusSourceSocketClient Error");
        Shutdown(socketId);
        return DCAMERA_BAD_VALUE;
    }
    sourceSocketId_ = socketId;
    DHLOGI("DCameraSoftbusAdapter CreateSoftBusSourceSocketClient End, myDevId: %s, peerSessionName: %s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DestroySoftbusSessionServer(std::string sessionName)
{
    std::lock_guard<std::mutex> autoLock(optLock_);
    if (sessionTotal_.find(sessionName) == sessionTotal_.end()) {
        DHLOGI("DCameraSoftbusAdapter sessionName already destroy %s", sessionName.c_str());
        return DCAMERA_OK;
    }

    sessionTotal_[sessionName]--;
    DHLOGI("DCameraSoftbusAdapter sessionName destroy %s totalnum: %d", sessionName.c_str(),
        sessionTotal_[sessionName]);
    uint32_t total_ = sessionTotal_[sessionName];
    if (total_ == 0) {
        sessionTotal_.erase(sessionName);
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CloseSoftbusSession(int32_t socket)
{
    DHLOGI("Shutdown softbus socket start: %d", socket);
    Shutdown(socket); // shutdown socket
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    sinkSocketIdSessionMap_.erase(socket);
    sourceSocketIdSessionMap_.erase(socket);
    DHLOGI("Shutdown softbus socket: %d end", socket);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::SendSofbusBytes(int32_t socket, std::shared_ptr<DataBuffer>& buffer)
{
    return SendBytes(socket, buffer->Data(), buffer->Size());
}

int32_t DCameraSoftbusAdapter::SendSofbusStream(int32_t socket, std::shared_ptr<DataBuffer>& buffer)
{
    StreamData streamData = { reinterpret_cast<char *>(buffer->Data()), buffer->Size() };
    int64_t timeStamp;
    if (!buffer->FindInt64(TIME_STAMP_US, timeStamp)) {
        DHLOGD("SendSofbusStream find %s failed.", TIME_STAMP_US.c_str());
    }
    int32_t frameType;
    if (!buffer->FindInt32(FRAME_TYPE, frameType)) {
        DHLOGD("SendSofbusStream find %s failed.", FRAME_TYPE.c_str());
    }
    int32_t index;
    if (!buffer->FindInt32(INDEX, index)) {
        DHLOGD("SendSofbusStream find %s failed.", INDEX.c_str());
    }
    int64_t startEncodeT;
    if (!buffer->FindInt64(START_ENCODE_TIME_US, startEncodeT)) {
        DHLOGD("SendSofbusStream find %s failed.", START_ENCODE_TIME_US.c_str());
    }
    int64_t finishEncodeT;
    if (!buffer->FindInt64(FINISH_ENCODE_TIME_US, finishEncodeT)) {
        DHLOGD("SendSofbusStream find %s failed.", FINISH_ENCODE_TIME_US.c_str());
    }
    std::string jsonStr = "";
    DCameraSinkFrameInfo sinkFrameInfo;
    sinkFrameInfo.pts_ = timeStamp;
    sinkFrameInfo.type_ = frameType;
    sinkFrameInfo.index_ = index;
    sinkFrameInfo.startEncodeT_ = startEncodeT;
    sinkFrameInfo.finishEncodeT_ = finishEncodeT;
    sinkFrameInfo.sendT_ = GetNowTimeStampUs();
    sinkFrameInfo.Marshal(jsonStr);
    StreamData ext = { const_cast<char *>(jsonStr.c_str()), jsonStr.length() };
    StreamFrameInfo param = { 0 };
    int32_t ret = SendStream(socket, &streamData, &ext, &param);
    if (ret != SOFTBUS_OK) {
        DHLOGD("SendSofbusStream failed, ret is %d", ret);
        return DCAMERA_BAD_VALUE;
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusGetSessionById(int32_t sessionId,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    DHLOGI("get softbus session by sessionId: %d", sessionId);
    std::lock_guard<std::mutex> autoLock(idMapLock_);
    auto iter = sessionIdMap_.find(sessionId);
    if (iter == sessionIdMap_.end()) {
        DHLOGE("get softbus session by id not find session %d", sessionId);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusSourceGetSession(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    auto iter =  sourceSocketIdSessionMap_.find(socket);
    if (iter == sourceSocketIdSessionMap_.end()) {
        DHLOGE("DCameraSoftbusAdapter DCameraSoftbusSourceGetSession can not find socket %d", socket);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

// source
int32_t DCameraSoftbusAdapter::SourceOnBind(int32_t socket, PeerSocketInfo info)
{
    DHLOGI("DCameraSoftbusAdapter SourceOnBind socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SourceOnBind not find socket %d", socket);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpened(socket, info);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SourceOnBind failed %d socket: %d", ret, socket);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(socket, session);
    }
    DHLOGI("DCameraSoftbusAdapter SourceOnBind socket: %d end", socket);
    return ret;
}

void DCameraSoftbusAdapter::SourceOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("DCameraSoftbusAdapter SourceOnShutDown start socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SourceOnShutDown not find socket %d", socket);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    session->OnSessionClose(socket);
    DHLOGI("DCameraSoftbusAdapter SourceOnShutDown end socket: %d end", socket);
    return;
}

void DCameraSoftbusAdapter::SourceOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter SourceOnBytes dataLen: %d, socket: %d", dataLen, socket);
        return;
    }
    DHLOGI("DCameraSoftbusAdapter SourceOnBytes Start socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SourceOnBytes not find session %d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter SourceOnBytes memcpy_s failed ret: %d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    DHLOGI("DCameraSoftbusAdapter SourceOnBytes end socket: %d", socket);
    return;
}

void DCameraSoftbusAdapter::SourceOnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    (void)socket;
    (void)data;
    (void)dataLen;
    return;
}

void DCameraSoftbusAdapter::SourceOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *param)
{
    int64_t recvT = GetNowTimeStampUs();
    if (data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter::OnSourceStreamReceived, data is null, socket: %d.", socket);
        return;
    }
    DHLOGI("DCameraSoftbusAdapter SourceOnStream Start socket: %d", socket);
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("DCameraSoftbusAdapter OnSourceStreamReceived dataLen: %d, socket: %d", dataLen, socket);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceStreamReceived not find socket %d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(data->bufLen);
    buffer->SetInt64(RECV_TIME_US, recvT);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceStreamReceived memcpy_s failed ret: %d", ret);
        return;
    }
    ret = HandleSourceStreamExt(buffer, ext);
    if (ret != DCAMERA_OK) {
        DHLOGE("Handle source stream ext failed, ret is: %d", ret);
    }
    session->OnDataReceived(buffer);
    DHLOGI("DCameraSoftbusAdapter SourceOnStream End socket: %d", socket);
}

int32_t DCameraSoftbusAdapter::HandleSourceStreamExt(std::shared_ptr<DataBuffer>& buffer, const StreamData *ext)
{
    if (ext == nullptr) {
        DHLOGE("Source stream ext is null.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t extLen = ext->bufLen;
    if (extLen <= 0 || extLen > DCAMERA_MAX_RECV_EXT_LEN) {
        DHLOGD("ExtLen is: %d.", extLen);
        return DCAMERA_BAD_VALUE;
    }

    std::string jsonStr(reinterpret_cast<const char*>(ext->buf), ext->bufLen);
    DCameraSinkFrameInfo sinkFrameInfo;
    int32_t ret = sinkFrameInfo.Unmarshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("Unmarshal sinkFrameInfo failed.");
        return DCAMERA_BAD_VALUE;
    }
    int64_t recvT;
    if (!buffer->FindInt64(RECV_TIME_US, recvT)) {
        DHLOGD("HandleSourceStreamExt find %s failed.", RECV_TIME_US.c_str());
    }
    DCameraFrameInfo frameInfo;
    frameInfo.type = sinkFrameInfo.type_;
    frameInfo.pts = sinkFrameInfo.pts_;
    frameInfo.index = sinkFrameInfo.index_;
    frameInfo.ver = sinkFrameInfo.ver_;
    frameInfo.timePonit.startEncode = sinkFrameInfo.startEncodeT_;
    frameInfo.timePonit.finishEncode = sinkFrameInfo.finishEncodeT_;
    frameInfo.timePonit.send = sinkFrameInfo.sendT_;
    frameInfo.timePonit.recv = recvT;
    buffer->frameInfo_ = frameInfo;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::GetSinkSocketId()
{
    return sinkSocketId_;
}

int32_t DCameraSoftbusAdapter::GetSourceSocketId()
{
    return sourceSocketId_;
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusSinkGetSession(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    DHLOGI("DCameraSoftbusAdapter DCameraSoftbusSinkGetSession Start, socket: %d", socket);
    auto iter = sinkSocketIdSessionMap_.find(socket);
    if (iter == sinkSocketIdSessionMap_.end()) {
        DHLOGE("DCameraSoftbusAdapter DCameraSoftbusSinkGetSession can not find socket %d", socket);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    DHLOGI("DCameraSoftbusAdapter DCameraSoftbusSinkGetSession End, socket: %d", socket);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftBusGetSessionByPeerSocket(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession> &session, PeerSocketInfo info)
{
    DHLOGI("DCameraSoftbusAdapter DCameraSoftBusGetSessionByPeerSocket Start, socket %d", socket);
    std::string mySessionName = "";
    auto sessionNameIter = peerDevIdMySessionNameMap_.find(info.name);
    if (sessionNameIter == peerDevIdMySessionNameMap_.end()) {
        DHLOGI("DCameraSoftbusAdapter DCameraSoftBusGetSessionByPeerSocket error, socket %d", socket);
        return DCAMERA_NOT_FOUND;
    }
    mySessionName = sessionNameIter->second;
    auto iter = sinkSessions_.find(std::string(mySessionName));
    if (iter == sinkSessions_.end()) {
        DHLOGI("DCameraSoftbusAdapter DCameraSoftBusGetSessionByPeerSocket error, mySessionName %s",
            GetAnonyString(mySessionName).c_str());
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    sinkSocketIdSessionMap_[socket] = session;
    DHLOGI("DCameraSoftbusAdapter DCameraSoftBusGetSessionByPeerSocket End, socket %d", socket);
    return DCAMERA_OK;
}

// sink
int32_t DCameraSoftbusAdapter::SinkOnBind(int32_t socket, PeerSocketInfo info)
{
    DHLOGI("DCameraSoftbusAdapter SinkOnBind Start socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftBusGetSessionByPeerSocket(socket, session, info);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnBind not find socket %d", socket);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpened(socket, info);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnBind not find socket %d", socket);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(socket, session);
    }
    DHLOGI("DCameraSoftbusAdapter SinkOnBind End socket: %d", socket);
    return ret;
}

void DCameraSoftbusAdapter::SinkOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("DCameraSoftbusAdapter SinkOnShutDown Start socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnShutDown not find session %d", socket);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    session->OnSessionClose(socket);
    DHLOGI("DCameraSoftbusAdapter SinkOnShutDown End socket: %d", socket);
    return;
}

void DCameraSoftbusAdapter::SinkOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter SinkOnBytes dataLen: %d, socket: %d", dataLen, socket);
        return;
    }
    DHLOGI("DCameraSoftbusAdapter SinkOnBytes Start socket: %d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnBytes not find session %d", socket);
        return;
    }
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnBytes memcpy_s failed ret: %d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    DHLOGI("DCameraSoftbusAdapter SinkOnBytes End socket: %d", socket);
    return;
}

void DCameraSoftbusAdapter::SinkOnMessage(int32_t socket, const void *data, uint32_t dataLen)
{
    return;
}

void DCameraSoftbusAdapter::SinkOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *param)
{
    if (data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter::SinkOnStream, data is null, socket: %d.", socket);
        return;
    }
    DHLOGI("DCameraSoftbusAdapter SinkOnStream Start socket: %d", socket);
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("DCameraSoftbusAdapter SinkOnStream dataLen: %d socket: %d", dataLen, socket);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnStream not find session %d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(data->bufLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter SinkOnStream memcpy_s failed ret: %d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    DHLOGI("DCameraSoftbusAdapter SinkOnStream End socket: %d", socket);
    return;
}

int32_t DCameraSoftbusAdapter::GetLocalNetworkId(std::string& myDevId)
{
    NodeBasicInfo basicInfo = { { 0 } };
    int32_t ret = GetLocalNodeDeviceInfo(PKG_NAME.c_str(), &basicInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter GetLocalNodeDeviceInfo failed ret: %d", ret);
        return ret;
    }

    myDevId = std::string(basicInfo.networkId);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
