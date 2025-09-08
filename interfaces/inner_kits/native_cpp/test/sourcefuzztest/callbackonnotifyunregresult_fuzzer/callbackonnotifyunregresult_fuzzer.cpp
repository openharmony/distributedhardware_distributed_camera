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

#include "callbackonnotifyunregresult_fuzzer.h"
#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "mock_component_disable.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void CallbackOnNotifyRegResultFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    int32_t status = fdp.ConsumeIntegral<int32_t>();
    std::string dataStr = fdp.ConsumeRemainingBytesAsString();

    std::shared_ptr<UnregisterCallback> uncallback = std::make_shared<MockComponentDisable>();

    sptr<DCameraSourceCallback> dcameraSourceCallback(new (std::nothrow) DCameraSourceCallback());
    if (dcameraSourceCallback == nullptr) {
        return;
    }
    dcameraSourceCallback->PushUnregCallback(reqId, uncallback);
    dcameraSourceCallback->OnNotifyRegResult(devId, dhId, reqId, status, dataStr);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::CallbackOnNotifyRegResultFuzzTest(data, size);
    return 0;
}