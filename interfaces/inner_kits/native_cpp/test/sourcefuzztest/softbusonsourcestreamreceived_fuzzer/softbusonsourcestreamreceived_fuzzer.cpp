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

#include "softbusonsourcestreamreceived_fuzzer.h"
#include "dcamera_softbus_adapter.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <vector>
#include <memory>

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSourceStreamReceivedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    size_t receivedDataLength = fdp.ConsumeIntegralInRange<size_t>(0, fdp.remaining_bytes());

    std::vector<uint8_t> receivedDataBytes = fdp.ConsumeBytes<uint8_t>(receivedDataLength);
    std::vector<char> receivedDataCharBuf(receivedDataBytes.begin(), receivedDataBytes.end());
    StreamData receivedData;
    receivedData.buf = receivedDataCharBuf.data();
    receivedData.bufLen = static_cast<int>(receivedDataCharBuf.size());

    size_t extLength = fdp.remaining_bytes();
    std::vector<uint8_t> extBytes = fdp.ConsumeBytes<uint8_t>(extLength);
    std::vector<char> extCharBuf(extBytes.begin(), extBytes.end());
    StreamData ext;
    ext.buf = extCharBuf.data();
    ext.bufLen = static_cast<int>(extCharBuf.size());

    StreamFrameInfo param = {};
    const int16_t tvCOuntMax = 16;
    param.frameType = fdp.ConsumeIntegral<int32_t>();
    param.timeStamp = fdp.ConsumeIntegral<int64_t>();
    param.seqNum = fdp.ConsumeIntegral<int32_t>();
    param.seqSubNum = fdp.ConsumeIntegral<int32_t>();
    param.level = fdp.ConsumeIntegral<int32_t>();
    param.bitMap = fdp.ConsumeIntegral<int32_t>();
    param.tvCount = fdp.ConsumeIntegralInRange<int32_t>(0, tvCOuntMax);
    std::vector<TV> tvListBuffer(param.tvCount);
    for (int32_t i = 0; i < param.tvCount; ++i) {
        tvListBuffer[i].type = fdp.ConsumeIntegral<int32_t>();

        tvListBuffer[i].value = fdp.ConsumeIntegral<int64_t>();
    }
    param.tvList = tvListBuffer.data();

    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    int32_t socket = fdp.ConsumeIntegral<int32_t>();
    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_[socket] = session;

    DCameraSoftbusAdapter::GetInstance().SourceOnStream(sessionId, &receivedData, &ext, &param);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SoftbusOnSourceStreamReceivedFuzzTest(data, size);
    return 0;
}