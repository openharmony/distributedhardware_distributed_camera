/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_SOFTBUS_ADAPTER_H
#define OHOS_DCAMERA_SOFTBUS_ADAPTER_H

#include <mutex>
#include <map>
#include <set>
#include <unistd.h>

#include "dcamera_softbus_session.h"
#include "icamera_channel.h"
#include "single_instance.h"
#include "socket.h"
#include "trans_type.h"

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    DCAMERA_CHANNLE_ROLE_SOURCE = 0,
    DCAMERA_CHANNLE_ROLE_SINK = 1,
} DCAMERA_CHANNEL_ROLE;

class DCameraSoftbusAdapter {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSoftbusAdapter);

public:
    int32_t CreatSoftBusSinkSocketServer(std::string mySessionName, DCAMERA_CHANNEL_ROLE role,
        DCameraSessionMode sessionMode, std::string peerDevId, std::string peerSessionName);
    int32_t CreateSoftBusSourceSocketClient(std::string myDevId, std::string peerSessionName,
        std::string peerDevId, DCameraSessionMode sessionMode, DCAMERA_CHANNEL_ROLE role);

    int32_t DestroySoftbusSessionServer(std::string sessionName);
    int32_t CloseSoftbusSession(int32_t socket);
    int32_t SendSofbusBytes(int32_t socket, std::shared_ptr<DataBuffer> &buffer);
    int32_t SendSofbusStream(int32_t socket, std::shared_ptr<DataBuffer> &buffer);
    int32_t GetLocalNetworkId(std::string &myDevId);

    int32_t SourceOnBind(int32_t socket, PeerSocketInfo info);
    void SourceOnShutDown(int32_t socket, ShutdownReason reason);
    void SourceOnBytes(int32_t socket, const void *data, uint32_t dataLen);
    void SourceOnMessage(int32_t socket, const void *data, uint32_t dataLen);
    void SourceOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *param);

    int32_t SinkOnBind(int32_t socket, PeerSocketInfo info);
    void SinkOnShutDown(int32_t socket, ShutdownReason reason);
    void SinkOnBytes(int32_t socket, const void *data, uint32_t dataLen);
    void SinkOnMessage(int32_t socket, const void *data, uint32_t dataLen);
    void SinkOnStream(int32_t socket, const StreamData *data, const StreamData *ext,
        const StreamFrameInfo *param);

    int32_t HandleSourceStreamExt(std::shared_ptr<DataBuffer>& buffer, const StreamData *ext);
    int32_t GetSourceSocketId();
    void RecordSourceSocketSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession> session);

public:
    std::map<std::string, std::shared_ptr<DCameraSoftbusSession>> sourceSessions_;
    std::map<std::string, std::shared_ptr<DCameraSoftbusSession>> sinkSessions_;

private:
    DCameraSoftbusAdapter();
    ~DCameraSoftbusAdapter();

    int32_t DCameraSoftBusGetSessionByPeerSocket(int32_t socket, std::shared_ptr<DCameraSoftbusSession> &session,
        PeerSocketInfo info);
    int32_t DCameraSoftbusSourceGetSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession>& session);
    int32_t DCameraSoftbusSinkGetSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession>& session);
    int32_t DCameraSoftbusGetSessionById(int32_t socket, std::shared_ptr<DCameraSoftbusSession>& session);

private:
    std::mutex optLock_;
    const std::string PKG_NAME = "ohos.dhardware.dcamera";
    static const uint32_t DCAMERA_SESSION_NAME_MAX_LEN = 128;
    std::map<DCAMERA_CHANNEL_ROLE, ISocketListener> sessListeners_;
    std::map<std::string, uint32_t> sessionTotal_;
    static const uint32_t DCAMERA_LINK_TYPE_MAX = 4;
    static const uint32_t DCAMERA_LINK_TYPE_INDEX_2 = 2;
    std::mutex idMapLock_;
    std::map<int32_t, std::shared_ptr<DCameraSoftbusSession>> sessionIdMap_;

    int32_t sourceSocketId_ = -1;
    std::map<DCameraSessionMode, TransDataType> sessionModeAndDataTypeMap_;
    std::mutex mySessionNamePeerDevIdLock_;
    std::map<std::string, std::string> peerDevIdMySessionNameMap_;
    std::mutex mySessionNameLock_;
    std::set<std::string> mySessionNameSet_;

    std::mutex sinkSocketLock_;
    std::map<int32_t, std::shared_ptr<DCameraSoftbusSession>> sinkSocketSessionMap_;
    std::mutex sourceSocketLock_;
    std::map<int32_t, std::shared_ptr<DCameraSoftbusSession>> sourceSocketSessionMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOFTBUS_ADAPTER_H
