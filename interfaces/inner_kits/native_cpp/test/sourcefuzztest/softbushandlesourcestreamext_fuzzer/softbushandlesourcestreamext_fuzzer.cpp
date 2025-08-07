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

#include "softbushandlesourcestreamext_fuzzer.h"
#include "dcamera_softbus_adapter.h"
#include "data_buffer.h"

namespace OHOS {
namespace DistributedHardware {

void SoftbusHandleSourceStreamExtFuzzTest(const uint8_t* data, size_t size)
{
    if (data == nullptr || size == 0) {
        return;
    }

    auto buffer = std::make_shared<DataBuffer>(0);
    
    const StreamData ext = {
        const_cast<char*>(reinterpret_cast<const char*>(data)), static_cast<int>(size)
    };

    DCameraSoftbusAdapter::GetInstance().HandleSourceStreamExt(buffer, &ext);
}

} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SoftbusHandleSourceStreamExtFuzzTest(data, size);
    return 0;
}