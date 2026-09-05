/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sinkserviceconfigdistributedhardware_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include <fuzzer/FuzzedDataProvider.h>

#include "cJSON.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"

namespace OHOS {
namespace DistributedHardware {

void SinkServiceConfigDHFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string key = fdp.ConsumeRandomLengthString();
    std::string value = fdp.ConsumeRemainingBytesAsString();
    auto sinkService = std::make_shared<DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    if (sinkService == nullptr) {
        return;
    }
    sinkService->ConfigDistributedHardware(devId, dhId, key, value);
}

void SinkServiceConfigDHEnableParamFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string value = fdp.ConsumeRemainingBytesAsString();
    auto sinkService = std::make_shared<DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    if (sinkService == nullptr) {
        return;
    }
    sinkService->ConfigDistributedHardware("", dhId, KEY_ENABLE_INIT_PARAM, value);
    sinkService->enableFirstTokenId_ = 0;
    sinkService->ConfigDistributedHardware("", dhId, KEY_ENABLE_INIT_PARAM, value);
}

void SinkServiceConfigDHInvalidJsonFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }
    std::string value(reinterpret_cast<const char*>(data), size);
    auto sinkService = std::make_shared<DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    if (sinkService == nullptr) {
        return;
    }
    sinkService->ConfigDistributedHardware("", "camera_0", KEY_ENABLE_INIT_PARAM, value);
    sinkService->ConfigDistributedHardware("", "camera_0", "other_key", value);
}

void SinkServiceConfigDHValidJsonFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    uint32_t tokenId = fdp.ConsumeIntegral<uint32_t>();
    std::string dhId = fdp.ConsumeRandomLengthString();
    cJSON* root = cJSON_CreateObject();
    if (root == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(root, KEY_TOKEN_ID, static_cast<double>(tokenId));
    char* jsonStr = cJSON_PrintUnformatted(root);
    if (jsonStr == nullptr) {
        cJSON_Delete(root);
        return;
    }
    std::string value(jsonStr);
    auto sinkService = std::make_shared<DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    if (sinkService == nullptr) {
        cJSON_Delete(root);
        free(jsonStr);
        return;
    }
    sinkService->enableFirstTokenId_ = 0;
    sinkService->ConfigDistributedHardware("", dhId, KEY_ENABLE_INIT_PARAM, value);
    cJSON_Delete(root);
    free(jsonStr);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceConfigDHFuzzTest(data, size);
    OHOS::DistributedHardware::SinkServiceConfigDHEnableParamFuzzTest(data, size);
    OHOS::DistributedHardware::SinkServiceConfigDHInvalidJsonFuzzTest(data, size);
    OHOS::DistributedHardware::SinkServiceConfigDHValidJsonFuzzTest(data, size);
    return 0;
}
