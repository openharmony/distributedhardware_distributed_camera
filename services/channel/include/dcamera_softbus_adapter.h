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

#ifndef OHOS_DCAMERA_SOFTBUS_ADAPTER_H
#define OHOS_DCAMERA_SOFTBUS_ADAPTER_H

#include <mutex>
#include <map>
#include <set>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <condition_variable>

#include "dcamera_softbus_session.h"
#include "icamera_channel.h"
#include "single_instance.h"
#include "socket.h"
#include "trans_type.h"
#include "device_manager.h"
#include "device_type.h"
#include "iaccess_listener.h"

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
    int32_t CreateSoftBusSourceSocketClient(std::string myDhId, std::string myDevId, std::string peerSessionName,
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
    void RecordSourceSocketSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession> session);

    void CloseSessionWithNetWorkId(const std::string &networkId);
    void ProcessAuthorizationResult(const std::string &requestId, bool granted);
public:
    std::map<std::string, std::shared_ptr<DCameraSoftbusSession>> sinkSessions_;

private:
    DCameraSoftbusAdapter();
    ~DCameraSoftbusAdapter();

    int32_t DCameraSoftBusGetSessionByPeerSocket(int32_t socket, std::shared_ptr<DCameraSoftbusSession> &session,
        PeerSocketInfo info);
    int32_t DCameraSoftbusSourceGetSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession>& session);
    int32_t DCameraSoftbusSinkGetSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession>& session);
    void ReplaceSuffix(std::string &mySessNmRep, const std::string &suffix, const std::string &replacement);
    int32_t CheckOsType(const std::string &networkId, bool &isInvalid);
    int32_t ParseValueFromCjson(std::string args, std::string key);
    std::string GenerateRequestId();
    void StartAuthorizationTimer(const std::string &requestId, int32_t timeOutMs);
    void CancelAuthorizationTimer(const std::string &requestId);
    void HandleAuthorizationTimeout(const std::string &requestId);
    int32_t RequestAndWaitForAuthorization(const std::string &peerNetworkId);
    int32_t HandleConflictSession(int32_t socket, std::shared_ptr<DCameraSoftbusSession> session,
        const std::string& networkId);
    void ExecuteConflictCleanupAsync(int32_t socket, std::shared_ptr<DCameraSoftbusSession> session);
private:
    std::mutex optLock_;
    const std::string PKG_NAME = "ohos.dhardware.dcamera";
    static const uint32_t DCAMERA_SESSION_NAME_MAX_LEN = 128;
    std::map<DCAMERA_CHANNEL_ROLE, ISocketListener> sessListeners_;
    std::map<std::string, uint32_t> sessionTotal_;
    static const uint32_t DCAMERA_LINK_TYPE_MAX = 4;
    static const uint32_t DCAMERA_LINK_TYPE_INDEX_2 = 2;
    static const uint32_t AVCODEC_BUFFER_FLAG_NONE = 0;
    static const uint32_t SOFTBUS_VIDEO_I_FRAME = 1;
    static const uint32_t SOFTBUS_VIDEO_P_FRAME = 2;

    int32_t sourceSocketId_ = -1;
    std::mutex trustSessionIdLock_;
    struct TrustSessionId {
        int32_t controlSessionId_ = -1;
        int32_t dataContinueSessionId_ = -1;
        int32_t dataSnapshotSessionId_ = -1;
    };
    TrustSessionId trustSessionId_;
    std::map<DCameraSessionMode, TransDataType> sessionModeAndDataTypeMap_;
    std::mutex mySessionNamePeerDevIdLock_;
    std::map<std::string, std::string> peerDevIdMySessionNameMap_;
    std::map<std::string, std::string> mySessionNameMapV2_;
    std::mutex mySocketSetLock_;
    std::set<int32_t> mySocketSet_;

    std::mutex sinkSocketLock_;
    std::map<int32_t, std::shared_ptr<DCameraSoftbusSession>> sinkSocketSessionMap_;
    std::mutex sourceSocketLock_;
    std::map<int32_t, std::shared_ptr<DCameraSoftbusSession>> sourceSocketSessionMap_;

    // Authorization mechanism members
    std::mutex authRequestMutex_;
    std::map<std::string, std::shared_ptr<std::thread>> authTimerThreads_;
    std::map<std::string, bool> authTimerCancelFlags_;
    std::map<std::string, std::string> pendingAuthRequests_;  // requestId -> networkId mapping
};

class DeviceInitCallback : public DmInitCallback {
    void OnRemoteDied() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOFTBUS_ADAPTER_H
