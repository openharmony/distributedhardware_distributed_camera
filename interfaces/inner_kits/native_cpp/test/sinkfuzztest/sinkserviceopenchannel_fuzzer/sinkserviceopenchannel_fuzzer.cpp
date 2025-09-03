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

#include "sinkserviceopenchannel_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <string>

#include "fuzzer/FuzzedDataProvider.h"

#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SinkServiceOpenChannelFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    bool serviceEnable = fdp.ConsumeBool();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string openInfo = fdp.ConsumeRemainingBytesAsString();

    std::shared_ptr<DistributedCameraSinkService> sinkService =
        std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, serviceEnable);
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    std::shared_ptr<DCameraSinkDev> sinkDevice = std::make_shared<DCameraSinkDev>(dhId, sinkCallback);
    sinkService->camerasMap_.emplace(dhId, sinkDevice);
    sinkService->OpenChannel(dhId, openInfo);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceOpenChannelFuzzTest(data, size);
    return 0;
}