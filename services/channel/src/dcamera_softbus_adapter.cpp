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
#include "distributed_camera_allconnect_manager.h"
#include "dcamera_event_cmd.h"
#include "dcamera_protocol.h"
#include "cJSON.h"

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

void DCameraSoftbusAdapter::ReplaceSuffix(std::string &mySessNmRep, const std::string &suffix,
                                          const std::string &replacement)
{
    DHLOGI("replacing suffix in mySessionName: %{public}s", GetAnonyString(mySessNmRep).c_str());
    bool isModified = false;
    if (mySessNmRep.length() >= suffix.length() &&
        mySessNmRep.compare(mySessNmRep.length() - suffix.length(), suffix.length(), suffix) == 0) {
        mySessNmRep.replace(mySessNmRep.length() - suffix.length(), suffix.length(), replacement);
        isModified = true;
    }
    DHLOGI("suffix replaced? %{public}s - Modified: %{public}s",
        isModified ? "Y" : "N", GetAnonyString(mySessNmRep).c_str());
}

int32_t DCameraSoftbusAdapter::CreatSoftBusSinkSocketServer(std::string mySessionName, DCAMERA_CHANNEL_ROLE role,
    DCameraSessionMode sessionMode, std::string peerDevId, std::string peerSessionName)
{
    DHLOGI("create socket server start, mySessionName: %{public}s,peerSessionName: %{public}s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    SocketInfo serverSocketInfo {};
    if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
        serverSocketInfo = {
            .name =  const_cast<char*>(mySessionName.c_str()),
            .peerName = const_cast<char*>(peerSessionName.c_str()),
            .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
            .pkgName = const_cast<char*>(PKG_NAME.c_str()),
            .dataType = sessionModeAndDataTypeMap_[sessionMode],
        };
    } else {
        serverSocketInfo = {
            .name =  const_cast<char*>(mySessionName.c_str()),
            .pkgName = const_cast<char*>(PKG_NAME.c_str()),
            .dataType = sessionModeAndDataTypeMap_[sessionMode],
        };
    }
    int32_t socketId = Socket(serverSocketInfo);
    if (socketId < 0) {
        DHLOGE("create socket server error, socket is invalid, socketId: %{public}d", socketId);
        return DCAMERA_BAD_VALUE;
    }
    {
        std::lock_guard<std::mutex> autoLock(mySocketSetLock_);
        if (mySocketSet_.find(socketId) != mySocketSet_.end()) {
            DHLOGI("current socketId had Listened");
            return DCAMERA_OK;
        }
        mySocketSet_.insert(socketId);
    }
    int32_t ret = Listen(socketId, g_qosInfo, g_QosTV_Param_Index, &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("create socket server error, ret: %{public}d", ret);
        Shutdown(socketId);
        return DCAMERA_BAD_VALUE;
    }
    if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        std::string peerDevIdMySessionName = peerDevId + std::string("_") + mySessionName;
        peerDevIdMySessionNameMap_[peerDevIdMySessionName] = mySessionName;
    } else {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        mySessionNameMapV2_[mySessionName] = mySessionName;
    }
    DHLOGI("create socket server end, mySessionName: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(mySessionName).c_str(), GetAnonyString(peerSessionName).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CreateSoftBusSourceSocketClient(std::string myDhId, std::string myDevId,
    std::string peerSessionName, std::string peerDevId, DCameraSessionMode sessionMode, DCAMERA_CHANNEL_ROLE role)
{
    DHLOGI("create socket client start, myDevId: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    SocketInfo clientSocketInfo {};
    std::string myDevIdPeerSessionName = "";
    std::string srcSessionName = "";
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        myDevIdPeerSessionName = myDevId + std::string("_") + peerSessionName;
        clientSocketInfo = {
            .name = const_cast<char*>(myDevIdPeerSessionName.c_str()),
            .peerName = const_cast<char*>(peerSessionName.c_str()),
            .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
            .pkgName = const_cast<char*>(PKG_NAME.c_str()),
            .dataType = sessionModeAndDataTypeMap_[sessionMode],
        };
    } else {
        srcSessionName = peerSessionName + "_receiver";
        peerSessionName = peerSessionName + "_sender";
        clientSocketInfo = {
            .name = const_cast<char*>(srcSessionName.c_str()),
            .peerName = const_cast<char*>(peerSessionName.c_str()),
            .peerNetworkId = const_cast<char*>(peerDevId.c_str()),
            .pkgName = const_cast<char*>(PKG_NAME.c_str()),
            .dataType = sessionModeAndDataTypeMap_[sessionMode],
        };
    }
    DHLOGD("create socket client myDevIdPeerSessionName: %{public}s, srcSessionName: %{public}s",
        GetAnonyString(myDevIdPeerSessionName).c_str(), GetAnonyString(srcSessionName).c_str());
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
    if (peerSessionName.find("_control") != std::string::npos && DCameraAllConnectManager::IsInited()) {
        DCameraAllConnectManager::GetInstance().PublishServiceState(peerDevId, myDhId, SCM_CONNECTED);
        DCameraAllConnectManager::SetSourceNetworkId(peerDevId, sourceSocketId_);
    }
    DHLOGI("DCamera allconnect create socket client publish scm connected success, dhId: %{public}s",
        GetAnonyString(myDhId).c_str());
    DHLOGI("create socket client end, myDevId: %{public}s, peerSessionName: %{public}s",
        GetAnonyString(myDevId).c_str(), GetAnonyString(peerSessionName).c_str());
    return socketId;
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
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, DCAMERA_BAD_VALUE, "Data buffer is null");
    return SendBytes(socket, buffer->Data(), buffer->Size());
}

int32_t DCameraSoftbusAdapter::SendSofbusStream(int32_t socket, std::shared_ptr<DataBuffer>& buffer)
{
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, DCAMERA_BAD_VALUE, "Data buffer is null");
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
    if (ret != DCAMERA_OK || session == nullptr) {
        DHLOGE("source bind socket can not find socket %{public}d", socket);
        return DCAMERA_NOT_FOUND;
    }
    ret = session->OnSessionOpened(socket, info.networkId);
    if (ret != DCAMERA_OK) {
        DHLOGE("source bind socket failed, ret: %{public}d socket: %{public}d", ret, socket);
    }
    if (session->GetPeerSessionName().find("_control") != std::string::npos && DCameraAllConnectManager::IsInited()) {
        ret = DCameraAllConnectManager::GetInstance().PublishServiceState(info.networkId,
            session->GetMyDhId(), SCM_CONNECTED);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect sourceonBind publish scm connected failed, ret: %{public}d", ret);
        }
        DHLOGI("DCamera allconnect sourceonBind publish scm connected success, dhId: %{public}s",
            GetAnonyString(session->GetMyDhId()).c_str());
        DCameraAllConnectManager::SetSourceNetworkId(info.networkId, socket);
    }
    DHLOGI("source bind socket end, socket: %{public}d end", socket);
    return ret;
}

void DCameraSoftbusAdapter::SourceOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("source on shutdown socket start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(socket, session);
    if (ret != DCAMERA_OK || session == nullptr) {
        DHLOGE("SourceOnShutDown can not find socket %{public}d", socket);
        return;
    }
    session->OnSessionClose(socket);
    if (session->GetPeerSessionName().find("_control") != std::string::npos && DCameraAllConnectManager::IsInited()) {
        DCameraAllConnectManager::RemoveSourceNetworkId(socket);
    }
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
    if (ret != DCAMERA_OK || session == nullptr) {
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
    if (ret != DCAMERA_OK || session == nullptr) {
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
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, DCAMERA_BAD_VALUE, "Data buffer is null");
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
        CHECK_AND_RETURN_RET_LOG(session == nullptr, DCAMERA_BAD_VALUE, "Softbus session is null");
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
    if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        auto sessionNameIter = peerDevIdMySessionNameMap_.find(info.name);
        if (sessionNameIter == peerDevIdMySessionNameMap_.end()) {
            DHLOGE("find session by peer socket error, socket %{public}d", socket);
            return DCAMERA_NOT_FOUND;
        }
        mySessionName = sessionNameIter->second;
    } else {
        std::lock_guard<std::mutex> autoLock(mySessionNamePeerDevIdLock_);
        mySessionName = info.name;
        ReplaceSuffix(mySessionName, "_receiver", "_sender");
        auto sessionNameIter = mySessionNameMapV2_.find(mySessionName);
        if (sessionNameIter == mySessionNameMapV2_.end()) {
            DHLOGE("find session by peer socket error, socket %{public}d", socket);
            return DCAMERA_NOT_FOUND;
        }
        mySessionName = sessionNameIter->second;
        if (mySessionName.empty()) {
            DHLOGE("find mySessionName is empty");
            return DCAMERA_BAD_VALUE;
        }
    }
    auto iter = sinkSessions_.find(mySessionName);
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
    if (ret != DCAMERA_OK || session == nullptr) {
        DHLOGE("sink bind socket error, can not find socket %{public}d", socket);
        return DCAMERA_NOT_FOUND;
    }
    std::string peerNetworkId = info.networkId;
    bool isInvalid = false;
    CHECK_AND_RETURN_RET_LOG(CheckOsType(peerNetworkId, isInvalid) != DCAMERA_OK && isInvalid, DCAMERA_BAD_VALUE,
        "CheckOsType failed or invalid osType");
    ret = session->OnSessionOpened(socket, info.networkId);
    if (ret != DCAMERA_OK) {
        DHLOGE("sink bind socket error, not find socket %{public}d", socket);
    }
    if (session->GetPeerSessionName().find("_control") != std::string::npos && DCameraAllConnectManager::IsInited()) {
        ret = DCameraAllConnectManager::GetInstance().PublishServiceState(info.networkId,
            session->GetMyDhId(), SCM_CONNECTED);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect sink on bind, publish service state failed %{public}d", ret);
        }
        DHLOGI("DCamera allconnect sink on bind publish scm connected success, dhId: %{public}s",
            GetAnonyString(session->GetMyDhId()).c_str());
        DCameraAllConnectManager::SetSinkNetWorkId(info.networkId, socket);
    }
    DHLOGI("sink bind socket end, socket: %{public}d", socket);
    return ret;
}

int32_t DCameraSoftbusAdapter::ParseValueFromCjson(std::string args, std::string key)
{
    DHLOGD("ParseValueFromCjson");
    cJSON *jParam = cJSON_Parse(args.c_str());
    CHECK_NULL_RETURN(jParam == nullptr, DCAMERA_BAD_VALUE);
    cJSON *retItem = cJSON_GetObjectItemCaseSensitive(jParam, key.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG(retItem == nullptr || !cJSON_IsNumber(retItem),
        DCAMERA_BAD_VALUE, jParam, "Not found key result");
    int32_t ret = retItem->valueint;
    cJSON_Delete(jParam);
    return ret;
}

int32_t DCameraSoftbusAdapter::CheckOsType(const std::string &networkId, bool &isInvalid)
{
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DeviceInitCallback>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(DCAMERA_PKG_NAME, initCallback);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, DCAMERA_BAD_VALUE, "InitDeviceManager failed, ret: %{public}d", ret);
    std::vector<DistributedHardware::DmDeviceInfo> dmDeviceInfoList;
    ret = DeviceManager::GetInstance().GetTrustedDeviceList(DCAMERA_PKG_NAME, "", dmDeviceInfoList);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, DCAMERA_BAD_VALUE,
        "Get device manager trusted device list fail, errCode %{public}d", ret);
    for (const auto& dmDeviceInfo : dmDeviceInfoList) {
        if (dmDeviceInfo.networkId == networkId) {
            int32_t osType = ParseValueFromCjson(dmDeviceInfo.extraData, KEY_OS_TYPE);
            if (osType != VALID_OS_TYPE && osType != DCAMERA_BAD_VALUE) {
                isInvalid = true;
            }
            DHLOGI("remote found, osType: %{public}d, isInvalid: %{public}d", osType, isInvalid);
            return DCAMERA_OK;
        }
    }
    DHLOGI("remote not found.");
    return DCAMERA_OK;
}

void DCameraSoftbusAdapter::SinkOnShutDown(int32_t socket, ShutdownReason reason)
{
    DHLOGI("sink on shutdown socket start, socket: %{public}d", socket);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(socket, session);
    if (ret != DCAMERA_OK || session == nullptr) {
        DHLOGE("sink on shutdown socket can not find socket %{public}d", socket);
        return;
    }
    session->OnSessionClose(socket);
    if (session->GetPeerSessionName().find("_control") != std::string::npos && DCameraAllConnectManager::IsInited()) {
        std::string devId = DCameraAllConnectManager::GetSinkDevIdBySocket(socket);
        if (!devId.empty()) {
            ret = DCameraAllConnectManager::GetInstance().PublishServiceState(devId, session->GetMyDhId(), SCM_IDLE);
            if (ret != DCAMERA_OK) {
                DHLOGE("DCamera allconnect sinkDown PublishServiceState failed, ret: %{public}d, devId: %{public}s ",
                    ret, GetAnonyString(devId).c_str());
            }
        }
        DHLOGI("DCamera allconnect sinkdown publish scm idle success, dhId: %{public}s",
            GetAnonyString(session->GetMyDhId()).c_str());
        DCameraAllConnectManager::RemoveSinkNetworkId(socket);
    }

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
    if (ret != DCAMERA_OK || session == nullptr) {
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
    if (ret != DCAMERA_OK || session == nullptr) {
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
void DCameraSoftbusAdapter::CloseSessionWithNetWorkId(const std::string &networkId)
{
    DHLOGI("DCamera allconnect CloseSessionWithNetworkId begin");
    if (networkId.empty()) {
        DHLOGE("DCamera allconnect peerNetworkId is empty");
        return;
    }
    int32_t  sessionId = DCameraAllConnectManager::GetSinkSocketByNetWorkId(networkId);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCAMERA_OK;
    bool bSinkConflict = false;
    if (sessionId != -1) {
        ret = DCameraSoftbusSinkGetSession(sessionId, session);
        bSinkConflict = true;
    } else {
        sessionId = DCameraAllConnectManager::GetSourceSocketByNetworkId(networkId);
        if (sessionId != -1) {
            ret = DCameraSoftbusSourceGetSession(sessionId, session);
        } else {
            DHLOGE("DCamera allconnect CloseSessionWithNetWorkId can not find socket");
            return;
        }
    }
    if (ret != DCAMERA_OK || session == nullptr) {
        DHLOGE("DCamera allconnect CloseSessionWithNetWorkId can not find session %{public}d", sessionId);
        return;
    }
    session->OnSessionClose(sessionId);
    Shutdown(sessionId);
    if (bSinkConflict) {
        ret = DCameraAllConnectManager::GetInstance().PublishServiceState(networkId, session->GetMyDhId(), SCM_IDLE);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect CloseSessionWithNetworkId publish service state failed");
        }
        DHLOGI("DCamera allconnect close session publish scm idle success, dhId: %{public}s",
            GetAnonyString(session->GetMyDhId()).c_str());
    }
}

void DeviceInitCallback::OnRemoteDied()
{
    DHLOGI("DeviceInitCallback OnRemoteDied");
}
} // namespace DistributedHardware
} // namespace OHOS
