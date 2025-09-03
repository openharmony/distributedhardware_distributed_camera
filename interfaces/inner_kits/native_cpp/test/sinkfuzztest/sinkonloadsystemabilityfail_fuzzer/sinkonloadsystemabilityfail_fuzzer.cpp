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

#include "sinkonloadsystemabilityfail_fuzzer.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <cstddef>
#include <cstdint>

#include "dcamera_sink_load_callback.h"
#include "dcamera_sink_handler.h"
#include "distributed_camera_constants.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SinkOnLoadSystemAbilityFailFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    int32_t systemAbilityId = fdp.ConsumeIntegral<int32_t>();
    std::string params = fdp.ConsumeRemainingBytesAsString();
    std::shared_ptr<DCameraSinkLoadCallback> callback = std::make_shared<DCameraSinkLoadCallback>(params);

    callback->OnLoadSystemAbilityFail(systemAbilityId);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkOnLoadSystemAbilityFailFuzzTest(data, size);
    return 0;
}