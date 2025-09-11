/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "callbacksinkonhardwarestatechangedinner_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_sink_callback.h"
#include "dcamera_sink_callback_stub.h"
#include "distributed_camera_constants.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
void CallbackSinkOnHardwareStateChangedInnerFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }
    MessageParcel pdata;
    MessageParcel reply;
    int32_t tempStrLen = 32;
    int32_t statusLen = 4;
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString(tempStrLen);
    std::string dhId = fdp.ConsumeRandomLengthString(tempStrLen);
    int32_t status = fdp.ConsumeIntegral<int32_t>() % statusLen;
    pdata.WriteInt32(status);
    pdata.WriteString(devId);
    pdata.WriteString(dhId);

    sptr<DCameraSinkCallback> dcameraSinkCallback(new (std::nothrow) DCameraSinkCallback());
    if (dcameraSinkCallback == nullptr) {
        return;
    }
    dcameraSinkCallback->OnHardwareStateChangedInner(pdata, reply);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CallbackSinkOnHardwareStateChangedInnerFuzzTest(data, size);
    return 0;
}
