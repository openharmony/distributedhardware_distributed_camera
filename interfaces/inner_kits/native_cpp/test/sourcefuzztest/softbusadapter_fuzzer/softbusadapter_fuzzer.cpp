/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "softbusadapter_fuzzer.h"

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusCreatSoftBusSinkSocketServerFuzzTest(const uint8_t* data, size_t size)
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
    DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDevId, peerSessName, peerDevId,
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
    DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDevId, peerSessName, peerDevId,
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
    return 0;
}
