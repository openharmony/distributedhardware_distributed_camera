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

#include "softbusonsourcestreamreceived_fuzzer.h"

#include "dcamera_softbus_adapter.h"

namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSourceStreamReceivedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    const StreamData receivedData = {
        const_cast<char*>(reinterpret_cast<const char*>(data)), static_cast<int>(size)
    };
    const StreamData ext = {
        const_cast<char*>(reinterpret_cast<const char*>(data)), static_cast<int>(size)
    };
    const StreamFrameInfo param = {
        *(reinterpret_cast<const int*>(data)), *(reinterpret_cast<const int64_t*>(data)),
        *(reinterpret_cast<const int*>(data)), *(reinterpret_cast<const int*>(data)),
        *(reinterpret_cast<const int*>(data)), *(reinterpret_cast<const int*>(data)),
        *(reinterpret_cast<const int*>(data)), nullptr
    };
    int32_t socket = 1;
    auto session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_[socket] = session;

    DCameraSoftbusAdapter::GetInstance().SourceOnStream(sessionId, &receivedData, &ext, &param);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusOnSourceStreamReceivedFuzzTest(data, size);
    return 0;
}

