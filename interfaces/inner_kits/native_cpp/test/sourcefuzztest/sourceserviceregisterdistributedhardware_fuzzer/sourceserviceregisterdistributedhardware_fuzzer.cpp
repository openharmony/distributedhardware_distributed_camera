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

#include "sourceserviceregisterdistributedhardware_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include <string>
#include <memory>

namespace OHOS {
namespace DistributedHardware {
void SourceServiceRegisterDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    
    EnableParam param;
    param.sourceVersion = fdp.ConsumeRandomLengthString();
    param.sourceAttrs = fdp.ConsumeRandomLengthString();
    param.sinkVersion = fdp.ConsumeRandomLengthString();
    param.sinkAttrs = fdp.ConsumeRandomLengthString();

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    if (sourceService == nullptr) {
        return;
    }

    sourceService->RegisterDistributedHardware(devId, dhId, reqId, param);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceRegisterDistributedHardwareFuzzTest(data, size);
    return 0;
}