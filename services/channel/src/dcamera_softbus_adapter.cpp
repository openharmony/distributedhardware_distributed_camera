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

static int32_t DCameraSourceOnSessionOpend(int32_t sessionId, int32_t result)
{
    return DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
}

static void DCameraSourceOnSessionClosed(int32_t sessionId)
{
    DCameraSoftbusAdapter::GetInstance().OnSourceSessionClosed(sessionId);
    return;
}

static void DCameraSourceOnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().OnSourceBytesReceived(sessionId, data, dataLen);
    return;
}

static void DCameraSourceOnMessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().OnSourceMessageReceived(sessionId, data, dataLen);
    return;
}

static void DCameraSourceOnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    DCameraSoftbusAdapter::GetInstance().OnSourceStreamReceived(sessionId, data, ext, param);
    return;
}

static int32_t DCameraSinkOnSessionOpend(int32_t sessionId, int32_t result)
{
    return DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
}

static void DCameraSinkOnSessionClosed(int32_t sessionId)
{
    DCameraSoftbusAdapter::GetInstance().OnSinkSessionClosed(sessionId);
    return;
}

static void DCameraSinkOnBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().OnSinkBytesReceived(sessionId, data, dataLen);
    return;
}

static void DCameraSinkOnMessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    DCameraSoftbusAdapter::GetInstance().OnSinkMessageReceived(sessionId, data, dataLen);
    return;
}

static void DCameraSinkOnStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    DCameraSoftbusAdapter::GetInstance().OnSinkStreamReceived(sessionId, data, ext, param);
    return;
}

DCameraSoftbusAdapter::DCameraSoftbusAdapter()
{
    ISessionListener sourceListener;
    sourceListener.OnSessionOpened = DCameraSourceOnSessionOpend;
    sourceListener.OnSessionClosed = DCameraSourceOnSessionClosed;
    sourceListener.OnBytesReceived = DCameraSourceOnBytesReceived;
    sourceListener.OnMessageReceived = DCameraSourceOnMessageReceived;
    sourceListener.OnStreamReceived = DCameraSourceOnStreamReceived;
    sessListeners_[DCAMERA_CHANNLE_ROLE_SOURCE] = sourceListener;

    ISessionListener sinkListener;
    sinkListener.OnSessionOpened = DCameraSinkOnSessionOpend;
    sinkListener.OnSessionClosed = DCameraSinkOnSessionClosed;
    sinkListener.OnBytesReceived = DCameraSinkOnBytesReceived;
    sinkListener.OnMessageReceived = DCameraSinkOnMessageReceived;
    sinkListener.OnStreamReceived = DCameraSinkOnStreamReceived;
    sessListeners_[DCAMERA_CHANNLE_ROLE_SINK] = sinkListener;
}

DCameraSoftbusAdapter::~DCameraSoftbusAdapter()
{
}

int32_t DCameraSoftbusAdapter::CreateSoftbusSessionServer(std::string sessionName, DCAMERA_CHANNEL_ROLE role)
{
    std::lock_guard<std::mutex> autoLock(optLock_);
    if (sessionTotal_.find(sessionName) != sessionTotal_.end()) {
        sessionTotal_[sessionName]++;
        DHLOGI("DCameraSoftbusAdapter sessionName already exist %s totalnum: %d", sessionName.c_str(),
            sessionTotal_[sessionName]);
        return DCAMERA_OK;
    }

    int32_t ret = CreateSessionServer(PKG_NAME.c_str(), sessionName.c_str(), &sessListeners_[role]);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter CreateSessionServer failed, ret: %d", ret);
        ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_CREATE_SESSIONSVR_ERROR,
            CreateMsg("create session server failed, sessionName: %s", sessionName.c_str()));
        return ret;
    }
    sessionTotal_[sessionName]++;
    DHLOGI("DCameraSoftbusAdapter sessionName create %s totalnum: %d", sessionName.c_str(),
        sessionTotal_[sessionName]);
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
        RemoveSessionServer(PKG_NAME.c_str(), sessionName.c_str());
        sessionTotal_.erase(sessionName);
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::OpenSoftbusSession(std::string mySessName, std::string peerSessName,
    int32_t sessionMode, std::string peerDevId)
{
    SessionAttribute attr = { 0 };
    int32_t ret = ConstructSessionAttribute(sessionMode, attr);
    if (ret != DCAMERA_OK) {
        DHLOGE("ConstructSessionAttribute failed, ret is: %d", ret);
        return DCAMERA_BAD_VALUE;
    }
    int32_t sessionId = OpenSession(mySessName.c_str(), peerSessName.c_str(), peerDevId.c_str(), "0", &attr);
    if (sessionId < 0) {
        DHLOGE("DCameraSoftbusAdapter OpenSoftbusSession failed %d", sessionId);
        ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_OPEN_SESSION_ERROR,
            CreateMsg("open softbus session failed, mySessName: %s, peerSessName: %s, peerDevId: %s",
            mySessName.c_str(), peerSessName.c_str(), peerDevId.c_str()));
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::CloseSoftbusSession(int32_t sessionId)
{
    DHLOGI("close softbus sessionId: %d", sessionId);
    CloseSession(sessionId);
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(sessionId);
    }
    DHLOGI("close softbus sessionId: %d end", sessionId);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::SendSofbusBytes(int32_t sessionId, std::shared_ptr<DataBuffer>& buffer)
{
    return SendBytes(sessionId, buffer->Data(), buffer->Size());
}

int32_t DCameraSoftbusAdapter::SendSofbusStream(int32_t sessionId, std::shared_ptr<DataBuffer>& buffer)
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
    int32_t ret = SendStream(sessionId, &streamData, &ext, &param);
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

int32_t DCameraSoftbusAdapter::DCameraSoftbusSourceGetSession(int32_t sessionId,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    char peerSessionName[DCAMERA_SESSION_NAME_MAX_LEN] = "";
    char peerDevId[NETWORK_ID_BUF_LEN] = "";
    int32_t ret = GetPeerSessionName(sessionId, peerSessionName, sizeof(peerSessionName));
    if (ret != DCAMERA_OK) {
        DHLOGI("DCameraSoftbusAdapter DCameraSoftbusSourceGetSession sessionId: %d GetPeerSessionName failed: %d",
            sessionId, ret);
        return ret;
    }

    ret = GetPeerDeviceId(sessionId, peerDevId, sizeof(peerDevId));
    if (ret != DCAMERA_OK) {
        DHLOGI("DCameraSoftbusAdapter DCameraSoftbusSourceGetSession sessionId: %d GetPeerDeviceId failed: %d",
            sessionId, ret);
        return ret;
    }

    auto iter = sourceSessions_.find(std::string(peerDevId) + std::string(peerSessionName));
    if (iter == sourceSessions_.end()) {
        DHLOGE("DCameraSoftbusAdapter DCameraSoftbusSourceGetSession not find session %d", sessionId);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::OnSourceSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("DCameraSoftbusAdapter OnSourceSessionOpened sessionId: %d, result: %d", sessionId, result);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceSessionOpened not find session %d", sessionId);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpend(sessionId, result);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceSessionOpened failed %d sessionId: %d", ret, sessionId);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(sessionId, session);
    }
    DHLOGI("DCameraSoftbusAdapter OnSourceSessionOpened sessionId: %d, result: %d end", sessionId, result);
    return ret;
}

void DCameraSoftbusAdapter::OnSourceSessionClosed(int32_t sessionId)
{
    DHLOGI("DCameraSoftbusAdapter OnSourceSessionClosed sessionId: %d", sessionId);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceSessionClosed not find session %d", sessionId);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(sessionId);
    }
    session->OnSessionClose(sessionId);
    DHLOGI("DCameraSoftbusAdapter OnSourceSessionClosed sessionId: %d end", sessionId);
    return;
}

void DCameraSoftbusAdapter::OnSourceBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter OnSourceBytesReceived dataLen: %d, sessionId: %d", dataLen, sessionId);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceBytesReceived not find session %d", sessionId);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceBytesReceived memcpy_s failed ret: %d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    return;
}

void DCameraSoftbusAdapter::OnSourceMessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    (void)sessionId;
    (void)data;
    (void)dataLen;
    return;
}

void DCameraSoftbusAdapter::OnSourceStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    int64_t recvT = GetNowTimeStampUs();
    if (data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter::OnSourceStreamReceived, data is null, sessionId: %d.", sessionId);
        return;
    }
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("DCameraSoftbusAdapter OnSourceStreamReceived dataLen: %d, sessionId: %d", dataLen, sessionId);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSourceGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSourceStreamReceived not find session %d", sessionId);
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

int32_t DCameraSoftbusAdapter::DCameraSoftbusSinkGetSession(int32_t sessionId,
    std::shared_ptr<DCameraSoftbusSession>& session)
{
    char mySessionName[DCAMERA_SESSION_NAME_MAX_LEN] = "";
    int ret = GetMySessionName(sessionId, mySessionName, sizeof(mySessionName));
    if (ret != DCAMERA_OK) {
        DHLOGI("DCameraSoftbusAdapter DCameraSoftbusSinkGetSession sessionId: %d GetPeerSessionName failed: %d",
            sessionId, ret);
        return ret;
    }

    auto iter = sinkSessions_.find(std::string(mySessionName));
    if (iter == sinkSessions_.end()) {
        DHLOGE("DCameraSoftbusAdapter DCameraSoftbusSinkGetSession not find session %d", sessionId);
        return DCAMERA_NOT_FOUND;
    }
    session = iter->second;
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::OnSinkSessionOpened(int32_t sessionId, int32_t result)
{
    DHLOGI("DCameraSoftbusAdapter OnSinkSessionOpened sessionId: %d, result: %d", sessionId, result);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkSessionOpened not find session %d", sessionId);
        return DCAMERA_NOT_FOUND;
    }

    ret = session->OnSessionOpend(sessionId, result);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkSessionOpened not find session %d", sessionId);
    } else {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.emplace(sessionId, session);
    }
    DHLOGI("DCameraSoftbusAdapter OnSinkSessionOpened sessionId: %d, result: %d end", sessionId, result);
    return ret;
}

void DCameraSoftbusAdapter::OnSinkSessionClosed(int32_t sessionId)
{
    DHLOGI("DCameraSoftbusAdapter OnSinkSessionClosed sessionId: %d", sessionId);
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusGetSessionById(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkSessionClosed not find session %d", sessionId);
        return;
    }
    {
        std::lock_guard<std::mutex> autoLock(idMapLock_);
        sessionIdMap_.erase(sessionId);
    }
    session->OnSessionClose(sessionId);
    return;
}

void DCameraSoftbusAdapter::OnSinkBytesReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    if (dataLen == 0 || dataLen > DCAMERA_MAX_RECV_DATA_LEN || data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter OnSinkBytesReceived dataLen: %d, sessionId: %d", dataLen, sessionId);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkBytesReceived not find session %d", sessionId);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(dataLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), data, dataLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkBytesReceived memcpy_s failed ret: %d", ret);
        return;
    }
    session->OnDataReceived(buffer);
    return;
}

void DCameraSoftbusAdapter::OnSinkMessageReceived(int32_t sessionId, const void *data, uint32_t dataLen)
{
    return;
}

void DCameraSoftbusAdapter::OnSinkStreamReceived(int32_t sessionId, const StreamData *data, const StreamData *ext,
    const StreamFrameInfo *param)
{
    if (data == nullptr) {
        DHLOGE("DCameraSoftbusAdapter::OnSinkStreamReceived, data is null, sessionId: %d.", sessionId);
        return;
    }
    int32_t dataLen = data->bufLen;
    if (dataLen <= 0 || dataLen > static_cast<int32_t>(DCAMERA_MAX_RECV_DATA_LEN)) {
        DHLOGE("DCameraSoftbusAdapter OnSinkStreamReceived dataLen: %d sessionId: %d", dataLen, sessionId);
        return;
    }
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    int32_t ret = DCameraSoftbusSinkGetSession(sessionId, session);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkStreamReceived not find session %d", sessionId);
        return;
    }

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(data->bufLen);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(data->buf), data->bufLen);
    if (ret != EOK) {
        DHLOGE("DCameraSoftbusAdapter OnSinkStreamReceived memcpy_s failed ret: %d", ret);
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
        DHLOGE("DCameraSoftbusAdapter GetLocalNodeDeviceInfo failed ret: %d", ret);
        return ret;
    }

    myDevId = std::string(basicInfo.networkId);
    return DCAMERA_OK;
}

int32_t DCameraSoftbusAdapter::ConstructSessionAttribute(int32_t sessionMode, SessionAttribute& attr)
{
    int dataType = TYPE_STREAM;
    int streamType = INVALID;
    switch (sessionMode) {
        case DCAMERA_SESSION_MODE_CTRL:
            dataType = TYPE_BYTES;
            break;
        case DCAMERA_SESSION_MODE_VIDEO:
            streamType = COMMON_VIDEO_STREAM;
            break;
        case DCAMERA_SESSION_MODE_JPEG:
            streamType = RAW_STREAM;
            break;
        default:
            DHLOGE("Bad sessionMode %d", sessionMode);
            return DCAMERA_BAD_VALUE;
    }
    attr.dataType = dataType;
    attr.linkTypeNum = DCAMERA_LINK_TYPE_MAX;
    LinkType linkTypeList[DCAMERA_LINK_TYPE_MAX] = {
        LINK_TYPE_WIFI_P2P,
        LINK_TYPE_WIFI_WLAN_5G,
        LINK_TYPE_WIFI_WLAN_2G,
        LINK_TYPE_BR,
    };

    if (dataType == TYPE_BYTES) {
        linkTypeList[0] = LINK_TYPE_WIFI_WLAN_2G;
        linkTypeList[DCAMERA_LINK_TYPE_INDEX_2] = LINK_TYPE_WIFI_P2P;
    }
    int32_t ret = memcpy_s(attr.linkType, DCAMERA_LINK_TYPE_MAX * sizeof(LinkType), linkTypeList,
        DCAMERA_LINK_TYPE_MAX * sizeof(LinkType));
    if (ret != EOK) {
        DHLOGE("LinkType memcpy_s failed %d", ret);
        return DCAMERA_BAD_VALUE;
    }
    attr.attr.streamAttr.streamType = streamType;
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
