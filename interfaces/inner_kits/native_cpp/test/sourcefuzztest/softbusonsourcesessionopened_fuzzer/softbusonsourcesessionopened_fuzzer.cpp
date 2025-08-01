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

#include "softbusonsourcesessionopened_fuzzer.h"

#include "dcamera_softbus_adapter.h"
#include <fuzzer/FuzzedDataProvider.h>
namespace OHOS {
namespace DistributedHardware {
void SoftbusOnSourceSessionOpenedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    int32_t sessionId = *(reinterpret_cast<const int32_t*>(data));
    std::string peerSessionName(reinterpret_cast<const char*>(data), size);
    std::string peerDevId(reinterpret_cast<const char*>(data), size);
    std::string pkgName = "ohos.dhardware.dcamera";
    PeerSocketInfo socketInfo = {
        .name = const_cast<char*>(peerSessionName.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .pkgName = const_cast<char*>(pkgName.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, socketInfo);
    DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, socketInfo);

    FuzzedDataProvider fdp(data, size);
    int32_t socket = fdp.ConsumeIntegral<int32_t>();
    std::shared_ptr<DCameraSoftbusSession> session = nullptr;
    DCameraSoftbusAdapter::GetInstance().SinkOnBind(socket, socketInfo);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusOnSourceSessionOpenedFuzzTest(data, size);
    return 0;
}

