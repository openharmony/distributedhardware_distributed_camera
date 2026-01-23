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

#include "sinkservicesetauthorizationresult_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <cstddef>
#include <cstdint>

#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include <string>

namespace OHOS {
namespace DistributedHardware {
void SinkServiceSetAuthorizationResultFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    std::string dhId = fdp.ConsumeRandomLengthString(64);
    std::shared_ptr<DistributedCameraSinkService> sinkService =
        std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    std::shared_ptr<DCameraSinkDev> sinkDevice = std::make_shared<DCameraSinkDev>(dhId, sinkCallback);
    sinkService->camerasMap_.emplace(dhId, sinkDevice);
    std::string requestId = fdp.ConsumeRandomLengthString();
    bool granted = fdp.ConsumeBool();

    sinkService->SetAuthorizationResult(requestId, granted);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceSetAuthorizationResultFuzzTest(data, size);
    return 0;
}
