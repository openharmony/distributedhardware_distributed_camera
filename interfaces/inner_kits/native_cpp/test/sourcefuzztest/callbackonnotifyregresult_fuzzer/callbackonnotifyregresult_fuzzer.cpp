/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "callbackonnotifyregresult_fuzzer.h"

#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "mock_component_enable.h"

namespace OHOS {
namespace DistributedHardware {
void CallbackOnNotifyRegResultFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    std::string devId(reinterpret_cast<const char*>(data), size);
    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string reqId(reinterpret_cast<const char*>(data), size);
    int32_t status = *(reinterpret_cast<const int32_t*>(data));
    std::string dataStr(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<RegisterCallback> callback = std::make_shared<MockComponentEnable>();

    sptr<DCameraSourceCallback> dcameraSourceCallback(new (std::nothrow) DCameraSourceCallback());
    dcameraSourceCallback->PushRegCallback(reqId, callback);
    dcameraSourceCallback->OnNotifyRegResult(devId, dhId, reqId, status, dataStr);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CallbackOnNotifyRegResultFuzzTest(data, size);
    return 0;
}

