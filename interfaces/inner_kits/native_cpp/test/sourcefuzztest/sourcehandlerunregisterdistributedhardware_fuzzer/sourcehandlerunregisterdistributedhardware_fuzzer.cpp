/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "sourcehandlerunregisterdistributedhardware_fuzzer.h"
#include <cstddef>
#include <cstdint>
#include "dcamera_source_handler.h"
#include "distributed_camera_constants.h"
#include "mock_component_disable.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void SourceHandlerUnregisterDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    const int doubleNum = 2;
    std::string devId = fdp.ConsumeRandomLengthString(fdp.remaining_bytes() / doubleNum);
    std::string dhId = fdp.ConsumeRemainingBytesAsString();

    std::shared_ptr<UnregisterCallback> uncallback;
    if (fdp.ConsumeBool()) {
        uncallback = std::make_shared<MockComponentDisable>();
    } else {
        uncallback = nullptr;
    }
    DCameraSourceHandler::GetInstance().UnregisterDistributedHardware(devId, dhId, uncallback);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceHandlerUnregisterDistributedHardwareFuzzTest(data, size);
    return 0;
}