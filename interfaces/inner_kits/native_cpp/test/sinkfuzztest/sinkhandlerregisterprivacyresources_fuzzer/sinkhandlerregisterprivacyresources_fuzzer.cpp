/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "sinkhandlerregisterprivacyresources_fuzzer.h"
#include "dcamera_sink_handler.h"
#include "distributed_camera_constants.h"
#include "mock_component_resourceinfo.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void SinkHandlerRegisterPrivacyResourcesFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    const size_t maxRegistrations = 5;
    size_t numRegistrations = fdp.ConsumeIntegralInRange<size_t>(0, maxRegistrations);

    for (size_t i = 0; i < numRegistrations; ++i) {
        std::shared_ptr<PrivacyResourcesListener> callback = std::make_shared<MockComponentResourceInfo>();
        DCameraSinkHandler::GetInstance().RegisterPrivacyResources(callback);
    }
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkHandlerRegisterPrivacyResourcesFuzzTest(data, size);
    return 0;
}