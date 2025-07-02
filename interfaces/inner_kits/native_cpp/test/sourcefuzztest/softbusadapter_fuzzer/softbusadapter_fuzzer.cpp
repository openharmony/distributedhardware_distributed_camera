/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include <fuzzer/FuzzedDataProvider.h>

#include "softbusadapter_fuzzer.h"

#include "dcamera_softbus_adapter.h"

namespace OHOS {
inline size_t GenerateBufferSize(FuzzedDataProvider& fuzzedData)
{
    constexpr size_t minBufferSize = 1;
    constexpr size_t maxBufferSize = 1024;
    return fuzzedData.ConsumeIntegralInRange<size_t>(minBufferSize, maxBufferSize);
}
namespace DistributedHardware {
void SoftbusCreatSoftBusSinkSocketServerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size < sizeof(uint32_t)) {
        return;
    }

    std::string sessionName = "sourcetest02";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    
    int32_t sessionID = *(reinterpret_cast<const int32_t*>(data));
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionID, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    uint32_t Len = static_cast<uint32_t>(size);
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionID, data, Len);
    DCameraSoftbusAdapter::GetInstance().SourceOnMessage(sessionID, data, Len);

    int32_t socket = 1234;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(socket, session);
}

void SoftbusCreateSoftBusSourceSocketClientFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = "bb536a637105409e904d4da83790a4a7";
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId, peerSessName, peerDevId,
        sessionMode, role);
}

void SoftbusDestroySoftbusSessionServerFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    std::string sessionName = "sourcetest02";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
}

void SoftbusCloseSoftbusSessionFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    std::string sessionName = "sourcetest04";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest04";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = "bb536a637105409e904d4da83790a4a7";
    DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(mySessName, role,
        sessionMode, peerDevId, peerSessName);
    int32_t sessionId = 1;
    DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId);
}

void SoftbusSendSofbusStreamFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = "bb536a637105409e904d4da83790a4a7";
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId, peerSessName, peerDevId,
        sessionMode, role);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t sessionId = 2;
    DCameraSoftbusAdapter::GetInstance().SendSofbusStream(sessionId, dataBuffer);
}

void SoftbusGetLocalNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    std::string devId = "bb536a637105409e904d4da83790a4a7";
    DCameraSoftbusAdapter::GetInstance().GetLocalNetworkId(devId);
}

void FuzzReplaceSuffix(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }
    int32_t tempStrLen = 64;
    int32_t tempShortStrLen = 16;
    FuzzedDataProvider fuzzedData(data, size);
    std::string mySessNmRep = fuzzedData.ConsumeRandomLengthString(tempStrLen);
    std::string suffix = fuzzedData.ConsumeRandomLengthString(tempShortStrLen);
    std::string replacement = fuzzedData.ConsumeRandomLengthString(tempShortStrLen);
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
}

void FuzzSendSofbusBytes(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }
    FuzzedDataProvider fuzzedData(data, size);
    int32_t socket = fuzzedData.ConsumeIntegral<int32_t>();
    size_t bufferSize = GenerateBufferSize(fuzzedData);

    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(bufferSize);
    fuzzedData.ConsumeData(buffer->Data(), bufferSize);
    DCameraSoftbusAdapter::GetInstance().SendSofbusBytes(socket, buffer);
}

void FuzzRecordSourceSocketSession(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }
    FuzzedDataProvider fuzzedData(data, size);
    int32_t socket = fuzzedData.ConsumeIntegral<int32_t>();
    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(socket, session);
}

void FuzzCloseSessionWithNetWorkId(const uint8_t *data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }
    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(64);
    DCameraSoftbusAdapter::GetInstance().CloseSessionWithNetWorkId(networkId);
}

void DCameraSoftbusAdapterCheckOsTypeFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    int32_t tempStrLen = 64;
    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(tempStrLen);
    bool isInvalid = false;

    DCameraSoftbusAdapter::GetInstance().CheckOsType(networkId, isInvalid);
}

void DCameraSoftbusAdapterParseValueFromCjsonFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    int32_t tempJsonStrLen = 256;
    int32_t tempStrLen = 64;
    FuzzedDataProvider fuzzedData(data, size);
    std::string jsonStr = fuzzedData.ConsumeRandomLengthString(tempJsonStrLen);
    std::string key = fuzzedData.ConsumeRandomLengthString(tempStrLen);

    DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusCreatSoftBusSinkSocketServerFuzzTest(data, size);
    OHOS::DistributedHardware::SoftbusCreateSoftBusSourceSocketClientFuzzTest(data, size);
    OHOS::DistributedHardware::SoftbusDestroySoftbusSessionServerFuzzTest(data, size);
    OHOS::DistributedHardware::SoftbusCloseSoftbusSessionFuzzTest(data, size);
    OHOS::DistributedHardware::SoftbusSendSofbusStreamFuzzTest(data, size);
    OHOS::DistributedHardware::SoftbusGetLocalNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::FuzzReplaceSuffix(data, size);
    OHOS::DistributedHardware::FuzzSendSofbusBytes(data, size);
    OHOS::DistributedHardware::FuzzRecordSourceSocketSession(data, size);
    OHOS::DistributedHardware::FuzzCloseSessionWithNetWorkId(data, size);
    OHOS::DistributedHardware::DCameraSoftbusAdapterCheckOsTypeFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraSoftbusAdapterParseValueFromCjsonFuzzTest(data, size);
    return 0;
}
