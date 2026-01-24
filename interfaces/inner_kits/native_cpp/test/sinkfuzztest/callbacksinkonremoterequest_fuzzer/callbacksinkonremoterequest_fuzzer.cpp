/*
 * Copyright (c) 2024-2026 Huawei Device Co., Ltd.
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

#include "callbacksinkonremoterequest_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_sink_callback.h"
#include "dcamera_sink_callback_stub.h"
#include "distributed_camera_constants.h"
#include "iremote_object.h"
#include "message_option.h"
#include "message_parcel.h"

namespace OHOS {
namespace DistributedHardware {
const uint32_t DC_RESOURCE_SIZE = 3;
const ResourceEventType resourceEventType[DC_RESOURCE_SIZE] {
    ResourceEventType::EVENT_TYPE_QUERY_RESOURCE,
    ResourceEventType::EVENT_TYPE_PULL_UP_PAGE,
    ResourceEventType::EVENT_TYPE_CLOSE_PAGE
};

void CallbackSinkOnRemoteRequestFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel pdata;
    MessageParcel reply;
    MessageOption option;
    FuzzedDataProvider fdp(data, size);
    uint32_t code = 0;
    ResourceEventType resType = fdp.PickValueInArray(resourceEventType);
    std::string subtype = fdp.ConsumeRandomLengthString(64);
    std::string networkId = fdp.ConsumeRandomLengthString(64);
    bool isSensitive = fdp.ConsumeBool();
    bool isSameAccout = fdp.ConsumeBool();
    pdata.WriteInt32(static_cast<int32_t>(resType));
    pdata.WriteString(subtype);
    pdata.WriteString(networkId);
    pdata.ReadBool(isSensitive);
    pdata.ReadBool(isSameAccout);

    sptr<DCameraSinkCallback> dcameraSinkCallback(new (std::nothrow) DCameraSinkCallback());
    if (dcameraSinkCallback == nullptr) {
        return;
    }
    dcameraSinkCallback->memberFuncMap_[code] = &DCameraSinkCallbackStub::OnNotifyResourceInfoInner;
    dcameraSinkCallback->OnRemoteRequest(code, pdata, reply, option);
}

void CallbackSinkOnNotifyResourceInfoInnerFuzzTest(const uint8_t* data, size_t size)
{
    MessageParcel pdata;
    MessageParcel reply;
    FuzzedDataProvider fdp(data, size);
    ResourceEventType resType = fdp.PickValueInArray(resourceEventType);
    std::string subtype = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    bool isSensitive = fdp.ConsumeBool();
    bool isSameAccout = fdp.ConsumeBool();
    pdata.WriteInt32(static_cast<int32_t>(resType));
    pdata.WriteString(subtype);
    pdata.WriteString(networkId);
    pdata.ReadBool(isSensitive);
    pdata.ReadBool(isSameAccout);

    sptr<DCameraSinkCallback> dcameraSinkCallback(new (std::nothrow) DCameraSinkCallback());
    if (dcameraSinkCallback == nullptr) {
        return;
    }
    dcameraSinkCallback->OnNotifyResourceInfoInner(pdata, reply);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::CallbackSinkOnRemoteRequestFuzzTest(data, size);
    OHOS::DistributedHardware::CallbackSinkOnNotifyResourceInfoInnerFuzzTest(data, size);
    return 0;
}
