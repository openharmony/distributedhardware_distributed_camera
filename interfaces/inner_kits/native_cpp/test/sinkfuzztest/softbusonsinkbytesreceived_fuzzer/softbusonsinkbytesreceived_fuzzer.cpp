/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSinkBytesReceivedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(uint32_t))) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    int32_t socket = 1;
    const void *receivedData = reinterpret_cast<const void*>(data);
    uint32_t dataLen = *(reinterpret_cast<const uint32_t*>(data));
    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socket] = session;
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, receivedData, dataLen);
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

