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
#include "softbus_error_code.h"
#include "dcamera_utils_tools.h"
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {
namespace {
static QosTV g_qosInfo[] = {
    { .qos = QOS_TYPE_MIN_BW, .value = DCAMERA_QOS_TYPE_MIN_BW },
    { .qos = QOS_TYPE_MAX_LATENCY, .value = DCAMERA_QOS_TYPE_MAX_LATENCY },
    { .qos = QOS_TYPE_MIN_LATENCY, .value = DCAMERA_QOS_TYPE_MIN_LATENCY}
};
static uint32_t g_QosTV_Param_Index = static_cast<uint32_t>(sizeof(g_qosInfo) / sizeof(QosTV));
}
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
    sessionModeAndDataTypeMap_[DCAMERA_SESSION_MODE_JPEG] = TransDataType::DATA_TYPE_BYTES;

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
    DHLOGI("create socket server start, mySessionName: %{public}s,peerSessionName: %{public}s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    {
        std::lock_guard<std::mutex> autoLock(mySessionNameLock_);
        if (mySessionNameSet_.find(mySessionName) != mySessionNameSet_.end()) {
            DHLOGI("current mySessionName had Listened");
            return DCAMERA_OK;
        }
        mySessionNameSet_.insert(mySessionName);
    }
    SocketInfo serverSocketInfo = {
        .name =  const_cast<char*>(mySessionName.c_str()),
        .peerName = const_cast<char*>(peerSessionName.c_str()),
        .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(PKG_NAME.c_str()),
        .dataType = sessionModeAndDataTypeMap_[sessionMode],
    };
    int socketId = Socket(serverSocketInfo);
    if (socketId < 0) {
        DHLOGE("create socket server error, socket is invalid");
        return DCAMERA_BAD_VALUE;
    }
    int ret = Listen(socketId, g_qosInfo, g_QosTV_Param_Index, &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("create socket server error");
        Shutdown(socketId);
        return DCAMERA_BAD_VALUE;
    }
    {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        std::string peerDevIdMySessionName = peerDevId + std::string("_") + mySessionName;
        peerDevIdMySessionNameMap_[peerDevIdMySessionName] = mySessionName;
    }
    DHLOGI("create socket server end, mySessionName: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CreateSoftBusSourceSocketClient(std::string myDevId, std::string peerSessionName,
    std::string peerDevId, DCameraSessionMode sessionMode, DCAMERA_CHANNEL_ROLE role)
{
    DHLOGI("create socket client start, myDevId: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    std::string myDevIdPeerSessionName = myDevId + std::string("_") + peerSessionName;
    SocketInfo clientSocketInfo = {
        .name = const_cast<char*>(myDevIdPeerSessionName.c_str()),
        .peerName = const_cast<char*>(peerSessionName.c_str()),
        .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(PKG_NAME.c_str()),
        .dataType = sessionModeAndDataTypeMap_[sessionMode],
    };
    int socketId = Socket(clientSocketInfo);
    if (socketId < 0) {
        DHLOGE("create socket client error, socket is invalid");
        return DCAMERA_BAD_VALUE;
    }
    int ret = Bind(socketId, g_qosInfo, g_QosTV_Param_Index, &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("create socket client error");
        Shutdown(socketId);
        return DCAMERA_BAD_VALUE;
    }
    sourceSocketId_ = socketId;
    DHLOGI("create socket client end, myDevId: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DestroySoftbusSessionServer(std::string sessionName)
{
    std::lock_guard<std::mutex> autoLock(optLock_);
    if (sessionTotal_.find(sessionName) == sessionTotal_.end()) {
        DHLOGI("current sessionName already destroy, sessionName: %{public}s", GetAnonyString(sessionName).c_str());
        return DCAMERA_OK;
    }

    sessionTotal_[sessionName]--;
    DHLOGI("sessionName destroy %{public}s totalnum: %{public}d", GetAnonyString(sessionName).c_str(),
        sessionTotal_[sessionName]);
    uint32_t total_ = sessionTotal_[sessionName];
    if (total_ == 0) {
        sessionTotal_.erase(sessionName);
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CloseSoftbusSession(int32_t socket)
{
    DHLOGI("Shutdown softbus socket start: %{public}d", socket);
    Shutdown(socket); // shutdown socket
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    {
        std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
        sinkSocketSessionMap_.erase(socket);
    }
    {
        std::lock_guard<std::mutex> autoLock(sourceSocketLock_);
        sourceSocketSessionMap_.erase(socket);
    }
    DHLOGI("Shutdown softbus socket: %{public}d end", socket);
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
        DHLOGD("SendSofbusStream find %{public}s failed.", TIME_STAMP_US.c_str());
    }
    int32_t frameType;
    if (!buffer->FindInt32(FRAME_TYPE, frameType)) {
        DHLOGD("SendSofbusStream find %{public}s failed.", FRAME_TYPE.c_str());
    }
    int32_t index;
    if (!buffer->FindInt32(INDEX, index)) {
        DHLOGD("SendSofbusStream find %{public}s failed.", INDEX.c_str());
    }
    int64_t startEncodeT;
    if (!buffer->FindInt64(START_ENCODE_TIME_US, startEncodeT)) {
        DHLOGD("SendSofbusStream find %{public}s failed.", START_ENCODE_TIME_US.c_str());
    }
    int64_t finishEncodeT;
    if (!buffer->FindInt64(FINISH_ENCODE_TIME_US, finishEncodeT)) {
        DHLOGD("SendSofbusStream find %{public}s failed.", FINISH_ENCODE_TIME_US.c_str());
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
        DHLOGD("SendSofbusStream failed, ret is %{public}d", ret);
        return DCAMERA_BAD_VALUE;
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusGetSessionById(int32_t sessionId,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    DHLOGI("get softbus session by sessionId: %{public}d", sessionId);
    std::lock_guard<std::mutex> autoLock(idMapLock_);
    auto iter = sessionIdMap_.find(sessionId);
    if (iter == sessionIdMap_.end()) {
        DHLOGE("get softbus session by id not find session %{public}d", sessionId);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusSourceGetSession(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    std::lock_guard<std::mutex> autoLock(sourceSocketLock_);
    auto iter =  sourceSocketSessionMap_.find(socket);
    if (iter == sourceSocketSessionMap_.end()) {
        DHLOGE("source can not find current socket %{public}d", socket);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

// source
int32_t DCameraSoftbusAdapter::SourceOnBind(int32_t socket, PeerSocketInfo info)
{
    DHLOGI("source bind socket begin, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("source bind socket can not find socket %{public}d", socket);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpened(socket);
    if (ret != DCAMERA_OK) {
        DHLOGE("source bind socket failed, ret: %{public}d socket: %{public}d", ret, socket);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(socket, session);
    }
    DHLOGI("source bind socket end, socket: %{public}d end", socket);
    return ret;
}

void DCameraSoftbusAdapter::SourceOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("source on shutdown socket start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("SourceOnShutDown can not find socket %{public}d", socket);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    session->OnSessionClose(socket);
    DHLOGI("source on shutdown socket end socket: %{public}d end", socket);
    return;
}

void DCameraSoftbusAdapter::SourceOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("source callback send bytes error, dataLen: %{public}d, socket: %{public}d", dataLen, socket);
        return;
    }
    DHLOGI("source callback send bytes start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("source callback send bytes not find session %{public}d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("source callback send bytes memcpy_s failed ret: %{public}d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    DHLOGI("source callback send bytes end, socket: %{public}d", socket);
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
        DHLOGE("SourceOnStream Error, data is null, socket: %{public}d.", socket);
        return;
    }
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("SourceOnStream Error, dataLen: %{public}d, socket: %{public}d", dataLen, socket);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("SourceOnStream not find socket %{public}d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(data->bufLen);
    buffer->SetInt64(RECV_TIME_US, recvT);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (ret != EOK) {
        DHLOGE("SourceOnStream memcpy_s failed ret: %{public}d", ret);
        return;
    }
    ret = HandleSourceStreamExt(buffer, ext);
    if (ret != DCAMERA_OK) {
        DHLOGE("Handle source stream ext failed, ret is: %{public}d", ret);
    }
    session->OnDataReceived(buffer);
}

int32_t DCameraSoftbusAdapter::HandleSourceStreamExt(std::shared_ptr<DataBuffer>& buffer, const StreamData *ext)
{
    if (ext == nullptr) {
        DHLOGE("Source stream ext is null.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t extLen = ext->bufLen;
    if (extLen <= 0 || extLen > DCAMERA_MAX_RECV_EXT_LEN) {
        DHLOGD("ExtLen is: %{public}d.", extLen);
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
        DHLOGD("HandleSourceStreamExt find %{public}s failed.", RECV_TIME_US.c_str());
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

int32_t DCameraSoftbusAdapter::GetSourceSocketId()
{
    return sourceSocketId_;
}

void DCameraSoftbusAdapter::RecordSourceSocketSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession> session)
{
    if (session == nullptr) {
        DHLOGE("RecordSourceSocketSession error, session is null");
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(sourceSocketLock_);
        sourceSocketSessionMap_[socket] = session;
    }
}

int32_t DCameraSoftbusAdapter::DCameraSoftbusSinkGetSession(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    DHLOGI("sink find session start, socket: %{public}d", socket);
    {
        std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
        auto iter = sinkSocketSessionMap_.find(socket);
        if (iter == sinkSocketSessionMap_.end()) {
            DHLOGE("sink can not find socket %{public}d", socket);
            return DCAMERA_NOT_FOUND;
        }
        session = iter->second;
        if (session->GetSessionId() < 0) {
            DHLOGE("sink find session error, current sessionId is invalid");
            return DCAMERA_BAD_VALUE;
        }
    }
    DHLOGI("sink find session end, socket: %{public}d", socket);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::DCameraSoftBusGetSessionByPeerSocket(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession> &session, PeerSocketInfo info)
{
    DHLOGI("find session by peer socket start, socket %{public}d", socket);
    std::string mySessionName = "";
    {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        auto sessionNameIter = peerDevIdMySessionNameMap_.find(info.name);
        if (sessionNameIter == peerDevIdMySessionNameMap_.end()) {
            DHLOGE("find session by peer socket error, socket %{public}d", socket);
            return DCAMERA_NOT_FOUND;
        }
        mySessionName = sessionNameIter->second;
    }
    auto iter = sinkSessions_.find(std::string(mySessionName));
    if (iter == sinkSessions_.end()) {
        DHLOGE("find session by peer socket error, mySessionName %{public}s",
            GetAnonyString(mySessionName).c_str());
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    {
        std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
        sinkSocketSessionMap_[socket] = session;
    }
    DHLOGI("find session by peer socket end, socket %{public}d", socket);
    return DCAMERA_OK;
}

// sink
int32_t DCameraSoftbusAdapter::SinkOnBind(int32_t socket, PeerSocketInfo info)
{
    DHLOGI("sink bind socket start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftBusGetSessionByPeerSocket(socket, session, info);
    if (ret != DCAMERA_OK) {
        DHLOGE("sink bind socket error, can not find socket %{public}d", socket);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpened(socket);
    if (ret != DCAMERA_OK) {
        DHLOGE("sink bind socket error, not find socket %{public}d", socket);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(socket, session);
    }
    DHLOGI("sink bind socket end, socket: %{public}d", socket);
    return ret;
}

void DCameraSoftbusAdapter::SinkOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("sink on shutdown socket start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("sink on shutdown socket can not find socket %{public}d", socket);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(socket);
    }
    session->OnSessionClose(socket);
    DHLOGI("sink on shutdown socket end, socket: %{public}d", socket);
    return;
}

void DCameraSoftbusAdapter::SinkOnBytes(int32_t socket, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("sink on bytes error, dataLen: %{public}d, socket: %{public}d", dataLen, socket);
        return;
    }
    DHLOGI("sink on bytes start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("sink on bytes error, can not find session %{public}d", socket);
        return;
    }
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("sink on bytes memcpy_s failed ret: %{public}d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    DHLOGI("sink on bytes end, socket: %{public}d", socket);
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
        DHLOGE("SinkOnStream error, data is null, socket: %{public}d.", socket);
        return;
    }
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("SinkOnStream error, dataLen: %{public}d socket: %{public}d", dataLen, socket);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(socket, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("SinkOnStream error, can not find socket %{public}d", socket);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(data->bufLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (ret != EOK) {
        DHLOGE("SinkOnStream error, memcpy_s failed ret: %{public}d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    return;
}

int32_t DCameraSoftbusAdapter::GetLocalNetworkId(std::string& myDevId)
{
    NodeBasicInfo basicInfo = { { 0 } };
    int32_t ret = GetLocalNodeDeviceInfo(PKG_NAME.c_str(), &basicInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("GetLocalNodeDeviceInfo failed ret: %{public}d", ret);
        return ret;
    }

    myDevId = std::string(basicInfo.networkId);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
