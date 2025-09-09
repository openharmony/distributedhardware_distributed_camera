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

#include "softbusonsinkmessagereceived_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>
#include <vector>

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSinkMessageReceivedFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    std::vector<uint8_t> messageBytes = fdp.ConsumeRemainingBytes<uint8_t>();
    const void* receivedData = messageBytes.data();
    uint32_t dataLen = static_cast<uint32_t>(messageBytes.size());

    DCameraSoftbusAdapter::GetInstance().SinkOnMessage(sessionId, receivedData, dataLen);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SoftbusOnSinkMessageReceivedFuzzTest(data, size);
    return 0;
}