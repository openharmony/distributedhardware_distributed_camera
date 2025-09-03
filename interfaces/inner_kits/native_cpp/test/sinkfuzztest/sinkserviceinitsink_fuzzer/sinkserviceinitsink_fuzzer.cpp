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

#include "sinkserviceinitsink_fuzzer.h"
#include "distributed_camera_sink_service.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {

void SinkServiceInitSinkFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    int serviceId = fdp.ConsumeIntegral<int>();
    bool isServiceAvailable = fdp.ConsumeBool();
    auto sinkService = std::make_shared<DistributedCameraSinkService>(serviceId, isServiceAvailable);
    
    std::string param = fdp.ConsumeRemainingBytesAsString();
    sptr<IDCameraSinkCallback> callback(new DCameraSinkCallback());
    
    sinkService->InitSink(param, callback);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceInitSinkFuzzTest(data, size);
    return 0;
}