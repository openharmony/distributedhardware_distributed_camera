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

#include "sourceserviceregisterdhext_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>
#include <memory>
#include <unistd.h>

#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {

void SourceServiceRegisterDHExtFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    EnableParam param;
    param.sourceVersion = fdp.ConsumeRandomLengthString();
    param.sourceAttrs = fdp.ConsumeRandomLengthString();
    param.sinkVersion = fdp.ConsumeRandomLengthString();
    param.sinkAttrs = fdp.ConsumeRemainingBytesAsString();
    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    if (sourceService == nullptr) {
        return;
    }
    sourceService->RegisterDistributedHardware(devId, dhId, reqId, param);
    uint32_t time = 500000;
    usleep(time);
}

void SourceServiceRegisterDHMultiCallFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    EnableParam param;
    param.sourceVersion = fdp.ConsumeRandomLengthString();
    param.sourceAttrs = fdp.ConsumeRandomLengthString();
    param.sinkVersion = fdp.ConsumeRandomLengthString();
    param.sinkAttrs = fdp.ConsumeRemainingBytesAsString();
    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    if (sourceService == nullptr) {
        return;
    }
    sourceService->RegisterDistributedHardware(devId, dhId, reqId, param);
    sourceService->UnregisterDistributedHardware(devId, dhId, reqId);
}

void SourceServiceRegisterDHEmptyParamFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    EnableParam param;
    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    if (sourceService == nullptr) {
        return;
    }
    sourceService->RegisterDistributedHardware(devId, dhId, reqId, param);
    sourceService->UnregisterDistributedHardware(devId, dhId, reqId);
    uint32_t time = 500000;
    usleep(time);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceRegisterDHExtFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceRegisterDHMultiCallFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceRegisterDHEmptyParamFuzzTest(data, size);
    return 0;
}
