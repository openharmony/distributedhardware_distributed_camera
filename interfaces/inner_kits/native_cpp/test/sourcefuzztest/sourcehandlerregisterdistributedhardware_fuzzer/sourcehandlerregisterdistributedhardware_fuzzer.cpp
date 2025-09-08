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

#include "sourcehandlerregisterdistributedhardware_fuzzer.h"
#include "dcamera_source_handler.h"
#include "distributed_camera_constants.h"
#include "mock_component_enable.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "refbase.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <string>
#include <vector>
#include <cstddef>
#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
void SourceHandlerRegisterDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string sourceVersion = fdp.ConsumeRandomLengthString();
    std::string sourceAttrs = fdp.ConsumeRandomLengthString();
    std::string sinkVersion = fdp.ConsumeRandomLengthString();
    std::string sinkAttrs = fdp.ConsumeRemainingBytesAsString();

    EnableParam param;
    param.sourceVersion = sourceVersion;
    param.sourceAttrs = sourceAttrs;
    param.sinkVersion = sinkVersion;
    param.sinkAttrs = sinkAttrs;
    std::shared_ptr<RegisterCallback> callback = std::make_shared<MockComponentEnable>();

    DCameraSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceHandlerRegisterDistributedHardwareFuzzTest(data, size);
    return 0;
}