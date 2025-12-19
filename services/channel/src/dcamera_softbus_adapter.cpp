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
#include "dcamera_utils_tools.h"
#include <random>
#include <sstream>
#include <iomanip>
#include "ffrt_inner.h"
#include <sys/prctl.h>
#ifdef DCAMERA_WAKEUP
#include "softbus_def.h"
#include "inner_socket.h"
#endif

namespace OHOS {
namespace DistributedHardware {
namespace {
static QosTV g_qosInfo[] = {
    { .qos = QOS_TYPE_MIN_BW, .value = DCAMERA_QOS_TYPE_MIN_BW },
    { .qos = QOS_TYPE_MAX_LATENCY, .value = DCAMERA_QOS_TYPE_MAX_LATENCY },
    { .qos = QOS_TYPE_MIN_LATENCY, .value = DCAMERA_QOS_TYPE_MIN_LATENCY}
};
static uint32_t g_QosTV_Param_Index = static_cast<uint32_t>(sizeof(g_qosInfo) / sizeof(QosTV));
#ifdef DCAMERA_WAKEUP
static TransWakeUpOnParam g_wakeUpParam = {
    .mode = PERIODIC_WAKE_UP_MODE,
    .level = HALF_WAKE_UP_LEVEL,
    .isEnabled = true,
};
static TransWakeUpOnParam g_wakeUpDisableParam = {
    .mode = PERIODIC_WAKE_UP_MODE,
    .level = HALF_WAKE_UP_LEVEL,
    .isEnabled = false,
};
static std::atomic<uint32_t> g_halfWakeupRef{0};
static const int32_t DCAMERA_OPT_TYPE_FAST_WAKE_UP = 10010;
#endif
static const int32_t HEX_WIDTH = 16;
static const int32_t SECONDS_TO_MS = 1000;
static const int32_t DEFAULT_TIMEOUT_MS = 30000;
}
IMPLEMENT_SINGLE_INSTANCE(DCameraSoftbusAdapter);
// LCOV_EXCL_START
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
// LCOV_EXCL_STOP
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
    sinkFrameInfo.rawTime_ = std::to_string(timeStamp);
    sinkFrameInfo.Marshal(jsonStr);
    DHLOGI("send videoPts=%{public}s to softbus,frameType:%{public}d", sinkFrameInfo.rawTime_.c_str(), frameType);
    StreamData ext = { const_cast<char *>(jsonStr.c_str()), jsonStr.length() };
    StreamFrameInfo param = { 0 };
    param.frameType = (frameType == AVCODEC_BUFFER_FLAG_NONE) ? SOFTBUS_VIDEO_P_FRAME : SOFTBUS_VIDEO_I_FRAME;
    param.seqNum = index;
    int32_t ret = SendStream(socket, &streamData, &ext, &param);
    if (ret != SOFTBUS_OK) {
        DHLOGD("SendSofbusStream failed, ret is %{public}d", ret);
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("send videoPts=%{public}s success,frameType:%{public}d,seqNum:%{public}d",
        sinkFrameInfo.rawTime_.c_str(), frameType, index);
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
    if (sinkFrameInfo.rawTime_.empty()) {
        frameInfo.rawTime = 0;
    } else {
        char *endptr = nullptr;
        errno = 0;
        long long raw_time_val = strtoll(sinkFrameInfo.rawTime_.c_str(), &endptr, 10);
        if (errno == ERANGE || (endptr != nullptr && *endptr != '\0') || endptr == sinkFrameInfo.rawTime_.c_str()) {
            DHLOGE("Failed to convert rawTime string to long long: %s", sinkFrameInfo.rawTime_.c_str());
            frameInfo.rawTime = 0;
        } else {
            frameInfo.rawTime = raw_time_val;
        }
    }
    DHLOGI("get videoPts=%{public}" PRId64 " from softbus", frameInfo.rawTime);
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
  
    ret = HandleConflictSession(socket, session, info.networkId);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    DCameraAccessConfigManager::GetInstance().SetCurrentNetworkId(peerNetworkId);
    if (session->GetPeerSessionName().find("_control") != std::string::npos) {
        DHLOGI("Control channel detected, triggering access authorization");
        RequestAndWaitForAuthorization(peerNetworkId);
    }

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
#ifdef DCAMERA_WAKEUP
    if (g_halfWakeupRef.fetch_add(1) == 0) {
        ret = SetSocketOpt(socket, OPT_LEVEL_SOFTBUS, static_cast<OptType>(DCAMERA_OPT_TYPE_FAST_WAKE_UP),
            &g_wakeUpParam, sizeof(TransWakeUpOnParam));
        CHECK_AND_RETURN_RET_LOG(ret != SOFTBUS_OK, ret, "SetSocketOpt failed");
        DHLOGI("SetSocketOpt success");
    }
#endif
    DHLOGI("sink bind socket end, socket: %{public}d", socket);
    return ret;
}

int32_t DCameraSoftbusAdapter::HandleConflictSession(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession> session, const std::string& networkId)
{
    DHLOGI("Starting conflict detection for socket: %{public}d, dhId: %{public}s",
           socket, GetAnonyString(session->GetMyDhId()).c_str());
    auto peerSessionName = session->GetPeerSessionName();
    if (peerSessionName.find("_control") != std::string::npos && trustSessionId_.controlSessionId_ != -1) {
        session->SetConflict(true);
        int32_t ret = session->OnSessionOpened(socket, networkId);
        if (ret != DCAMERA_OK) {
            DHLOGE("sink bind socket error, not find socket %{public}d", socket);
        }
        ExecuteConflictCleanupAsync(socket, session);
        return DCAMERA_DEVICE_BUSY;
    }
    
    if ((peerSessionName.find("_dataContinue") != std::string::npos && trustSessionId_.dataContinueSessionId_ != -1) ||
        (peerSessionName.find("_dataSnapshot") != std::string::npos && trustSessionId_.dataSnapshotSessionId_ != -1)) {
        {
            std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
            sinkSocketSessionMap_.erase(socket);
        }
        return DCAMERA_DEVICE_BUSY;
    }
    
    {
        std::lock_guard<std::mutex> autoLock(trustSessionIdLock_);
        if (peerSessionName.find("_control") != std::string::npos) {
            trustSessionId_.controlSessionId_ = socket;
            session->SetConflict(false);
        } else if (peerSessionName.find("_dataContinue") != std::string::npos) {
            trustSessionId_.dataContinueSessionId_ = socket;
        } else if (peerSessionName.find("_dataSnapshot") != std::string::npos) {
            trustSessionId_.dataSnapshotSessionId_ = socket;
        }
    }
    
    return DCAMERA_OK;
}

void DCameraSoftbusAdapter::ExecuteConflictCleanupAsync(int32_t socket,
    std::shared_ptr<DCameraSoftbusSession> session)
{
    DHLOGI("Submitting async cleanup task for socket: %{public}d", socket);
    
    ffrt::submit([this, socket, session]() {
        DHLOGI("Async cleanup: sending error notification for socket: %{public}d", socket);
        prctl(PR_SET_NAME, "DCamConflictCleanup");
        ReportCameraOperaterEvent(DCAMERA_CONFLICT_SEND_EVENT, GetAnonyString(session->GetPeerDevId()).c_str(),
            GetAnonyString(session->GetMyDhId()).c_str(), "operator start capture in used.");
        session->NotifyError(DCAMERA_MESSAGE,  DCAMERA_EVENT_DEVICE_IN_USE,
            std::string("operator start capture in used."));
        {
            std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
            sinkSocketSessionMap_.erase(socket);
        }
        {
            std::lock_guard<std::mutex> autoLock(trustSessionIdLock_);
            session->SetSessionId(trustSessionId_.controlSessionId_);
        }
        }, {}, {}, ffrt::task_attr().name("DCamConflictCleanup").qos(ffrt::qos_user_initiated));
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
    {
        std::lock_guard<std::mutex> autoLock(trustSessionIdLock_);
        if (trustSessionId_.controlSessionId_ == socket) {
            trustSessionId_.controlSessionId_ = -1;
            session->SetConflict(false);
        } else if (trustSessionId_.dataContinueSessionId_ == socket) {
            trustSessionId_.dataContinueSessionId_ = -1;
            session->SetConflict(false);
        } else if (trustSessionId_.dataSnapshotSessionId_ == socket) {
            trustSessionId_.dataSnapshotSessionId_ = -1;
            session->SetConflict(false);
        }
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
    {
        std::lock_guard<std::mutex> autoLock(sinkSocketLock_);
        sinkSocketSessionMap_.erase(socket);
    }
#ifdef DCAMERA_WAKEUP
    if (g_halfWakeupRef.fetch_sub(1) == 1) {
        ret = SetSocketOpt(socket, OPT_LEVEL_SOFTBUS, static_cast<OptType>(DCAMERA_OPT_TYPE_FAST_WAKE_UP),
            &g_wakeUpDisableParam, sizeof(TransWakeUpOnParam));
        CHECK_AND_RETURN_LOG(ret != SOFTBUS_OK, "SetSocketOpt failed");
        DHLOGI("SetSocketOpt success");
    }
#endif
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

std::string DCameraSoftbusAdapter::GenerateRequestId()
{
    std::random_device rd;
    std::mt19937_64 gen(rd());
    std::uniform_int_distribution<uint64_t> dis;

    uint64_t randomNum = dis(gen);
    auto now = std::chrono::system_clock::now();
    auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count();

    std::stringstream ss;
    ss << "req_" << std::hex << std::setfill('0') << std::setw(HEX_WIDTH) << timestamp
       << "_" << std::setw(HEX_WIDTH) << randomNum;

    std::string requestId = ss.str();
    DHLOGI("Generated requestId: %{public}s", GetAnonyString(requestId).c_str());
    return requestId;
}

void DCameraSoftbusAdapter::StartAuthorizationTimer(const std::string &requestId, int32_t timeOutMs)
{
    DHLOGI("Start authorization timer, requestId: %{public}s, timeout: %{public}d ms",
        GetAnonyString(requestId).c_str(), timeOutMs);

    CancelAuthorizationTimer(requestId);
    std::lock_guard<std::mutex> lock(authRequestMutex_);
    authTimerCancelFlags_[requestId] = false;

    auto timerThread = std::make_shared<std::thread>([this, requestId, timeOutMs]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeOutMs));

        bool shouldTimeout = false;
        {
            std::lock_guard<std::mutex> lock(authRequestMutex_);
            auto flagIt = authTimerCancelFlags_.find(requestId);
            if (flagIt != authTimerCancelFlags_.end() && !flagIt->second) {
                shouldTimeout = true;
            }
            authTimerCancelFlags_.erase(requestId);
        }

        if (shouldTimeout) {
            DHLOGI("Authorization timeout for requestId: %{public}s", GetAnonyString(requestId).c_str());
            HandleAuthorizationTimeout(requestId);
        }

        {
            std::lock_guard<std::mutex> lock(authRequestMutex_);
            authTimerThreads_.erase(requestId);
        }
    });

    authTimerThreads_[requestId] = timerThread;
    timerThread->detach();
}

void DCameraSoftbusAdapter::CancelAuthorizationTimer(const std::string &requestId)
{
    DHLOGI("Cancel authorization timer, requestId: %{public}s", GetAnonyString(requestId).c_str());

    std::lock_guard<std::mutex> lock(authRequestMutex_);

    auto flagIt = authTimerCancelFlags_.find(requestId);
    if (flagIt != authTimerCancelFlags_.end()) {
        flagIt->second = true;
    }
}

void DCameraSoftbusAdapter::HandleAuthorizationTimeout(const std::string &requestId)
{
    DHLOGE("Authorization timeout, requestId: %{public}s", GetAnonyString(requestId).c_str());

    std::string networkId;
    {
        std::lock_guard<std::mutex> lock(authRequestMutex_);
        auto it = pendingAuthRequests_.find(requestId);
        if (it != pendingAuthRequests_.end()) {
            networkId = it->second;
            pendingAuthRequests_.erase(it);
        }
    }

    if (networkId.empty()) {
        DHLOGW("Authorization request not found for timeout, requestId: %{public}s",
            GetAnonyString(requestId).c_str());
        return;
    }

    if (DCameraAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId)) {
        DHLOGI("Authorization result already exists for device: %{public}s, cover last result",
            GetAnonyString(networkId).c_str());
    }

    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, false);
}

void DCameraSoftbusAdapter::ProcessAuthorizationResult(const std::string &requestId, bool granted)
{
    DHLOGI("Process authorization result");
    CancelAuthorizationTimer(requestId);

    std::string networkId;
    {
        std::lock_guard<std::mutex> lock(authRequestMutex_);
        auto it = pendingAuthRequests_.find(requestId);
        if (it != pendingAuthRequests_.end()) {
            networkId = it->second;
            pendingAuthRequests_.erase(it);
        }
    }

    if (networkId.empty()) {
        DHLOGE("Authorization request not found or already processed: %{public}s",
            GetAnonyString(requestId).c_str());
        return;
    }

    if (DCameraAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId)) {
        DHLOGW("Authorization result already exists for device: %{public}s, cover last result",
            GetAnonyString(networkId).c_str());
    }

    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, granted);
}

int32_t DCameraSoftbusAdapter::RequestAndWaitForAuthorization(const std::string &peerNetworkId)
{
    DHLOGI("Request authorization, networkId: %{public}s", GetAnonyString(peerNetworkId).c_str());

    sptr<IAccessListener> listener = DCameraAccessConfigManager::GetInstance().GetAccessListener();
    if (listener == nullptr) {
        DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(peerNetworkId, true);
        DHLOGI("listener is null, no authorization config, allow by default");
        return DCAMERA_OK;
    }
    int32_t timeOut = DCameraAccessConfigManager::GetInstance().GetAccessTimeOut();
    std::string pkgName = DCameraAccessConfigManager::GetInstance().GetAccessPkgName();
    if (pkgName.empty()) {
        DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(peerNetworkId, true);
        DHLOGI("package name is null, no authorization config, allow by default");
        return DCAMERA_OK;
    }

    std::string requestId = GenerateRequestId();
    {
        std::lock_guard<std::mutex> lock(authRequestMutex_);
        pendingAuthRequests_[requestId] = peerNetworkId;
    }

    int32_t timeOutMs = (timeOut > 0) ? timeOut * SECONDS_TO_MS : DEFAULT_TIMEOUT_MS;
    StartAuthorizationTimer(requestId, timeOutMs);
    AuthDeviceInfo remoteDevInfo;
    remoteDevInfo.networkId = peerNetworkId;
    remoteDevInfo.deviceName = "Remote Device";
    remoteDevInfo.deviceType = 0;
    listener->OnRequestHardwareAccess(requestId, remoteDevInfo, DHType::CAMERA, pkgName);
    DHLOGI("Authorization request sent");
    return DCAMERA_OK;
}

void DeviceInitCallback::OnRemoteDied()
{
    DHLOGI("DeviceInitCallback OnRemoteDied");
}
} // namespace DistributedHardware
} // namespace OHOS
