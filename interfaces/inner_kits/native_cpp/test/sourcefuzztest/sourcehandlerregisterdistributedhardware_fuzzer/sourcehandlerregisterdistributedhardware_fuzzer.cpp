/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <cstddef>
#include <cstdint>

#include "dcamera_source_handler.h"
#include "distributed_camera_constants.h"
#include "mock_component_enable.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "refbase.h"

namespace OHOS {
namespace DistributedHardware {
void SourceHandlerRegisterDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }
    std::string devId(reinterpret_cast<const char*>(data), size);
    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string sourceVersion(reinterpret_cast<const char*>(data), size);
    std::string sourceAttrs(reinterpret_cast<const char*>(data), size);
    std::string sinkVersion(reinterpret_cast<const char*>(data), size);
    std::string sinkAttrs(reinterpret_cast<const char*>(data), size);
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

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SourceHandlerRegisterDistributedHardwareFuzzTest(data, size);
    return 0;
}

