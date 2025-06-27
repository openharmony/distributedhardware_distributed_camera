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

#include "softbusonsourcebytesreceived_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSourceBytesReceivedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    const void *receivedData = reinterpret_cast<const void*>(data);
    uint32_t dataLen = fdp.ConsumeIntegral<uint32_t>();
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionId, receivedData, dataLen);
    
    std::string testStr = "test_suffix";
    std::string randomSuffix = fdp.ConsumeRandomLengthString(10);
    std::string randomReplacement = fdp.ConsumeRandomLengthString(10);
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(testStr, randomSuffix, randomReplacement);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusOnSourceBytesReceivedFuzzTest(data, size);
    return 0;
}

