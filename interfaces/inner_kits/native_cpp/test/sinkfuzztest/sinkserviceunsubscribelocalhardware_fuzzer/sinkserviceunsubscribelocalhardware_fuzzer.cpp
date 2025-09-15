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

#include "sinkserviceunsubscribelocalhardware_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void SinkServiceUnsubscribeLocalHardwareFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    bool isSystemAbility = fdp.ConsumeBool();
    std::shared_ptr<DistributedCameraSinkService> sinkService =
        std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, isSystemAbility);
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    const int maxDhIdLen = 32;
    std::string dhId_to_insert = fdp.ConsumeRandomLengthString(maxDhIdLen);
    if (!dhId_to_insert.empty()) {
        std::shared_ptr<DCameraSinkDev> sinkDevice = std::make_shared<DCameraSinkDev>(dhId_to_insert, sinkCallback);
        sinkService->camerasMap_.emplace(dhId_to_insert, sinkDevice);
    }
    std::string dhId_to_unsubscribe;
    if (!dhId_to_insert.empty() && fdp.ConsumeBool()) {
        dhId_to_unsubscribe = dhId_to_insert;
    } else {
        dhId_to_unsubscribe = fdp.ConsumeRandomLengthString(maxDhIdLen);
    }
    sinkService->UnsubscribeLocalHardware(dhId_to_unsubscribe);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceUnsubscribeLocalHardwareFuzzTest(data, size);
    return 0;
}