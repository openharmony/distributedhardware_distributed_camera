/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "allconnectmanager_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "distributed_camera_allconnect_manager.h"

constexpr size_t maxStringLength = 64;
namespace OHOS {
namespace DistributedHardware {
void DCameraAllConnectManagerInitFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
#ifdef __LP64__
    constexpr const char *tmpAllConnectSoPath = "/system/lib64/";
#else
    constexpr const char *tmpAllConnectSoPath = "/system/lib/";
#endif
    std::string randomPath = fuzzedData.ConsumeRandomLengthString(PATH_MAX);
    std::string allConnectSoPath = tmpAllConnectSoPath;
    allConnectSoPath = randomPath;

    DCameraAllConnectManager::GetInstance().InitDCameraAllConnectManager();
}

void DCameraAllConnectManagerUnInitFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    void* randomDllHandle = reinterpret_cast<void*>(fuzzedData.ConsumeIntegral<uintptr_t>());
    const_cast<void*&>(DCameraAllConnectManager::GetInstance().dllHandle_) = randomDllHandle;

    DCameraAllConnectManager::GetInstance().UnInitDCameraAllConnectManager();
}

void DCameraAllConnectManagerIsInitedFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    bool randomInitState = fuzzedData.ConsumeBool();
    const_cast<bool&>(DCameraAllConnectManager::GetInstance().bInited_) = randomInitState;

    DCameraAllConnectManager::GetInstance().IsInited();
}

void DCameraAllConnectManagerPublishServiceStateFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string peerNetworkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    std::string dhId = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    DCameraCollaborationBussinessStatus state = static_cast<DCameraCollaborationBussinessStatus>(
        fuzzedData.ConsumeIntegralInRange<int32_t>(SCM_IDLE, SCM_CONNECTED));

    DCameraAllConnectManager::GetInstance().PublishServiceState(peerNetworkId, dhId, state);
}

void DCameraAllConnectManagerApplyAdvancedResourceFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string peerNetworkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    DCameraCollaborationResourceRequestInfoSets resourceRequest;
    resourceRequest.remoteHardwareListSize = fuzzedData.ConsumeIntegral<uint32_t>();
    resourceRequest.localHardwareListSize = fuzzedData.ConsumeIntegral<uint32_t>();
    resourceRequest.communicationRequest = nullptr;

    DCameraAllConnectManager::GetInstance().ApplyAdvancedResource(peerNetworkId, &resourceRequest);
}

void DCameraAllConnectManagerGetAllConnectSoLoadFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
#ifdef __LP64__
    constexpr const char *tmpAllConnectSoPath = "/system/lib64/";
#else
    constexpr const char *tmpAllConnectSoPath = "/system/lib/";
#endif
    std::string randomPath = fuzzedData.ConsumeRandomLengthString(PATH_MAX + 10);
    std::string allConnectSoPath = tmpAllConnectSoPath;
    allConnectSoPath = randomPath;

    DCameraAllConnectManager::GetInstance().GetAllConnectSoLoad();
}

void DCameraAllConnectManagerRegisterLifecycleCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string serviceName = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    const_cast<std::string&>(DCameraAllConnectManager::SERVICE_NAME) = serviceName;

    DCameraAllConnectManager::GetInstance().RegisterLifecycleCallback();
}

void DCameraAllConnectManagerUnRegisterLifecycleCallbackFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string serviceName = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    const_cast<std::string&>(DCameraAllConnectManager::SERVICE_NAME) = serviceName;

    DCameraAllConnectManager::GetInstance().UnRegisterLifecycleCallback();
}

void DCameraAllConnectManagerOnStopFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string peerNetworkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);

    DCameraAllConnectManager::GetInstance().OnStop(peerNetworkId.c_str());
}

void DCameraAllConnectManagerRemoveSinkNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    int32_t sessionId = fuzzedData.ConsumeIntegral<int32_t>();

    DCameraAllConnectManager::GetInstance().RemoveSinkNetworkId(sessionId);
}

void DCameraAllConnectManagerRemoveSourceNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    int32_t sessionId = fuzzedData.ConsumeIntegral<int32_t>();
    DCameraAllConnectManager::GetInstance().RemoveSourceNetworkId(sessionId);
}

void DCameraAllConnectManagerSetSourceNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    int32_t socket = fuzzedData.ConsumeIntegral<int32_t>();

    DCameraAllConnectManager::GetInstance().SetSourceNetworkId(networkId, socket);
}

void DCameraAllConnectManagerSetSinkNetWorkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);
    int32_t socket = fuzzedData.ConsumeIntegral<int32_t>();

    DCameraAllConnectManager::GetInstance().SetSinkNetWorkId(networkId, socket);
}

void DCameraAllConnectManagerGetSinkDevIdBySocketFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    int32_t socket = fuzzedData.ConsumeIntegral<int32_t>();

    DCameraAllConnectManager::GetInstance().GetSinkDevIdBySocket(socket);
}

void DCameraAllConnectManagerGetSinkSocketByNetWorkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);

    DCameraAllConnectManager::GetInstance().GetSinkSocketByNetWorkId(networkId);
}

void DCameraAllConnectManagerGetSourceSocketByNetworkIdFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    FuzzedDataProvider fuzzedData(data, size);
    std::string networkId = fuzzedData.ConsumeRandomLengthString(maxStringLength);

    DCameraAllConnectManager::GetInstance().GetSourceSocketByNetworkId(networkId);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DCameraAllConnectManagerInitFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerUnInitFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerIsInitedFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerPublishServiceStateFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerApplyAdvancedResourceFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerGetAllConnectSoLoadFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerRegisterLifecycleCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerUnRegisterLifecycleCallbackFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerOnStopFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerRemoveSinkNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerRemoveSourceNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerSetSourceNetworkIdFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerSetSinkNetWorkIdFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerGetSinkDevIdBySocketFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerGetSinkSocketByNetWorkIdFuzzTest(data, size);
    OHOS::DistributedHardware::DCameraAllConnectManagerGetSourceSocketByNetworkIdFuzzTest(data, size);
    return 0;
}