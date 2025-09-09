/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "callbackonremoterequest_fuzzer.h"
#include "dcamera_source_callback.h"
#include "dcamera_source_callback_stub.h"
#include "distributed_camera_constants.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void CallbackOnRemoteRequestFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    MessageParcel pdata;
    MessageParcel reply;
    MessageOption option;
    uint32_t code = fdp.ConsumeIntegral<uint32_t>();
    int32_t status = fdp.ConsumeIntegral<int32_t>();
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string reqId = fdp.ConsumeRandomLengthString();
    std::string dataStr = fdp.ConsumeRemainingBytesAsString();
    pdata.WriteInt32(status);
    pdata.WriteString(devId);
    pdata.WriteString(dhId);
    pdata.WriteString(reqId);
    pdata.WriteString(dataStr);

    sptr<DCameraSourceCallback> dcameraSourceCallback(new (std::nothrow) DCameraSourceCallback());
    if (dcameraSourceCallback == nullptr) {
        return;
    }
    dcameraSourceCallback->OnRemoteRequest(code, pdata, reply, option);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::CallbackOnRemoteRequestFuzzTest(data, size);
    return 0;
}