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

#ifndef OHOS_DCAMERA_SOFTBUS_SESSION_H
#define OHOS_DCAMERA_SOFTBUS_SESSION_H

#include "event_handler.h"
#include <string>

#include "icamera_channel.h"
#include "icamera_channel_listener.h"
#include "transport/trans_type.h"

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    DCAMERA_SOFTBUS_STATE_CLOSED = 0,
    DCAMERA_SOFTBUS_STATE_OPENED = 1,
} DCameraSofbutState;

class DCameraSoftbusSession {
public:
    DCameraSoftbusSession();
    DCameraSoftbusSession(std::string myDhId, std::string myDevId, std::string mySessionName, std::string peerDevId,
        std::string peerSessionName, std::shared_ptr<ICameraChannelListener> listener, DCameraSessionMode mode);
    ~DCameraSoftbusSession();
    int32_t CloseSession();
    int32_t OnSessionOpened(int32_t socket, std::string networkId);
    int32_t OnSessionClose(int32_t sessionId);
    int32_t OnDataReceived(std::shared_ptr<DataBuffer>& buffer);
    int32_t SendData(DCameraSessionMode mode, std::shared_ptr<DataBuffer>& buffer);
    std::string GetPeerDevId();
    std::string GetPeerSessionName();
    std::string GetMySessionName();
    std::string GetMyDhId();
    int32_t GetSessionId();
    int32_t CreateSocketServer();
    int32_t BindSocketServer();
    void ReleaseSession();

private:
    struct SessionDataHeader {
        uint16_t version;
        uint8_t fragFlag;
        uint32_t dataType;
        uint32_t seqNum;
        uint32_t totalLen;
        uint16_t subSeq;
        uint32_t dataLen;
    };

    using DCameraSendFuc = int32_t (DCameraSoftbusSession::*)(std::shared_ptr<DataBuffer>& buffer);
    int32_t SendBytes(std::shared_ptr<DataBuffer>& buffer);
    int32_t SendStream(std::shared_ptr<DataBuffer>& buffer);
    void DealRecvData(std::shared_ptr<DataBuffer>& buffer);
    void PackRecvData(std::shared_ptr<DataBuffer>& buffer);
    void AssembleNoFrag(std::shared_ptr<DataBuffer>& buffer, SessionDataHeader& headerPara);
    void AssembleFrag(std::shared_ptr<DataBuffer>& buffer, SessionDataHeader& headerPara);
    int32_t CheckUnPackBuffer(SessionDataHeader& headerPara);
    void GetFragDataLen(uint8_t *ptrPacket, SessionDataHeader& headerPara);
    int32_t UnPackSendData(std::shared_ptr<DataBuffer>& buffer, DCameraSendFuc memberFunc);
    void MakeFragDataHeader(const SessionDataHeader& headPara, uint8_t *header, uint32_t len);
    void PostData(std::shared_ptr<DataBuffer>& buffer);
    uint16_t U16Get(const uint8_t *ptr);
    uint32_t U32Get(const uint8_t *ptr);
    void ResetAssembleFrag();
    void SetHeadParaDataLen(SessionDataHeader& headPara, const uint32_t totalLen, const uint32_t offset);

    enum {
        FRAG_NULL = 0,
        FRAG_START,
        FRAG_MID,
        FRAG_END,
        FRAG_START_END,
    };

    static const uint32_t BINARY_DATA_MAX_TOTAL_LEN = 100 * 1024 * 1024;
    static const uint32_t BINARY_DATA_MAX_LEN = 4 * 1024 * 1024;
    static const uint32_t BINARY_DATA_PACKET_MAX_LEN = 4 * 1024 * 1024;
    static const uint32_t BINARY_DATA_PACKET_RESERVED_BUFFER = 512;
    static const uint16_t PROTOCOL_VERSION = 1;
    static const uint16_t HEADER_UINT8_NUM = 1;
    static const uint16_t HEADER_UINT16_NUM = 2;
    static const uint16_t HEADER_UINT32_NUM = 4;
    static const uint16_t BINARY_HEADER_FRAG_LEN = 21;

    static const uint32_t BINARY_HEADER_FRAG_OFFSET = 2;
    static const uint32_t BINARY_HEADER_DATATYPE_OFFSET = 3;
    static const uint32_t BINARY_HEADER_SEQNUM_OFFSET = 7;
    static const uint32_t BINARY_HEADER_TOTALLEN_OFFSET = 11;
    static const uint32_t BINARY_HEADER_SUBSEQ_OFFSET = 15;
    static const uint32_t BINARY_HEADER_DATALEN_OFFSET = 17;

    std::shared_ptr<DataBuffer> packBuffer_;
    bool isWaiting_;
    uint32_t nowSeq_;
    uint32_t nowSubSeq_;
    uint32_t offset_;
    uint32_t totalLen_;

private:
    std::string myDhId_;
    std::string myDevId_;
    std::string mySessionName_;
    std::string peerDevId_;
    std::string peerSessionName_;
    std::shared_ptr<ICameraChannelListener> listener_;
    int32_t sessionId_;
    DCameraSofbutState state_;
    DCameraSessionMode mode_;
    std::map<DCameraSessionMode, DCameraSendFuc> sendFuncMap_;
    std::shared_ptr<AppExecFwk::EventHandler> eventHandler_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOFTBUS_SESSION_H
