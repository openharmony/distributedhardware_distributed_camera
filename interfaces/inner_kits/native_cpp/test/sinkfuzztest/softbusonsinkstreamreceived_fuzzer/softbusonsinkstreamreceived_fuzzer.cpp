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

#include "softbusonsinkstreamreceived_fuzzer.h"
#include "dcamera_softbus_adapter.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <vector>
#include <memory>

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSinkStreamReceivedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    const size_t minParamsSize = sizeof(int32_t) * 2 + sizeof(int) * 6 + sizeof(int64_t);
    if (fdp.remaining_bytes() < minParamsSize) {
        return;
    }

    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t socket = fdp.ConsumeIntegral<int32_t>();

    const StreamFrameInfo param = {
        fdp.ConsumeIntegral<int>(),
        fdp.ConsumeIntegral<int64_t>(),
        fdp.ConsumeIntegral<int>(),
        fdp.ConsumeIntegral<int>(),
        fdp.ConsumeIntegral<int>(),
        fdp.ConsumeIntegral<int>(),
        fdp.ConsumeIntegral<int>(),
        nullptr
    };

    const int32_t doubleNum = 2;
    size_t halfRemainingBytes = fdp.remaining_bytes() / doubleNum;

    std::vector<uint8_t> receivedDataBuffer;
    if (halfRemainingBytes > 0) {
        receivedDataBuffer = fdp.ConsumeBytes<uint8_t>(halfRemainingBytes);
    }
    StreamData receivedData = {
        reinterpret_cast<char*>(receivedDataBuffer.data()),
        static_cast<int>(receivedDataBuffer.size())
    };

    std::vector<uint8_t> extBuffer;
    if (fdp.remaining_bytes() > 0) {
        extBuffer = fdp.ConsumeRemainingBytes<uint8_t>();
    }
    StreamData ext = {
        reinterpret_cast<char*>(extBuffer.data()),
        static_cast<int>(extBuffer.size())
    };

    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socket] = session;
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(sessionId, &receivedData, &ext, &param);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SoftbusOnSinkStreamReceivedFuzzTest(data, size);
    return 0;
}