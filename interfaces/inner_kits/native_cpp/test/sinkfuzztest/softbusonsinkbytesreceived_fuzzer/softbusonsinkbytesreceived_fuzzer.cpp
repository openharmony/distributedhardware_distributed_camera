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

#include "softbusonsinkbytesreceived_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSinkBytesReceivedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    size_t part1_size = fdp.ConsumeIntegralInRange<size_t>(0, fdp.remaining_bytes());
    std::vector<uint8_t> part1_data = fdp.ConsumeBytes<uint8_t>(part1_size);
    FuzzedDataProvider fdp1(part1_data.data(), part1_data.size());

    int32_t sessionId = fdp1.ConsumeIntegral<int32_t>();
    int32_t socket = fdp1.ConsumeIntegral<int32_t>();

    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socket] = session;
    
    std::vector<uint8_t> receivedData = fdp1.ConsumeRemainingBytes<uint8_t>();
    
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(
        sessionId,
        receivedData.data(),
        static_cast<uint32_t>(receivedData.size())
    );

    std::vector<uint8_t> part2_data = fdp.ConsumeRemainingBytes<uint8_t>();
    FuzzedDataProvider fdp2(part2_data.data(), part2_data.size());

    std::string testStr = fdp2.ConsumeRandomLengthString(256);
    std::string randomSuffix = fdp2.ConsumeRandomLengthString(32);
    std::string randomReplacement = fdp2.ConsumeRandomLengthString(32);

    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(testStr, randomSuffix, randomReplacement);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusOnSinkBytesReceivedFuzzTest(data, size);
    return 0;
}

