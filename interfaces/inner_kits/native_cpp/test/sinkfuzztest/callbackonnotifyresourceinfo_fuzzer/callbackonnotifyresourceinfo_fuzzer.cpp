/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "callbackonnotifyresourceinfo_fuzzer.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "mock_component_resourceinfo.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {

const ResourceEventType VALID_EVENT_TYPES[] = {
    ResourceEventType::EVENT_TYPE_QUERY_RESOURCE,
    ResourceEventType::EVENT_TYPE_PULL_UP_PAGE,
    ResourceEventType::EVENT_TYPE_CLOSE_PAGE
};

void CallbackOnNotifyResourceInfoFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    ResourceEventType type = fdp.PickValueInArray(VALID_EVENT_TYPES);
    std::string subtype = fdp.ConsumeRandomLengthString();
    std::string networkId = fdp.ConsumeRandomLengthString();
    bool isSensitive = fdp.ConsumeBool();
    bool isSameAccout = fdp.ConsumeBool();
    
    std::shared_ptr<PrivacyResourcesListener> callback = std::make_shared<MockComponentResourceInfo>();

    sptr<DCameraSinkCallback> dcameraSinkCallback(new (std::nothrow) DCameraSinkCallback());
    if (dcameraSinkCallback == nullptr) {
        return;
    }
    dcameraSinkCallback->PushPrivacyResCallback(callback);
    dcameraSinkCallback->OnNotifyResourceInfo(type, subtype, networkId, isSensitive, isSameAccout);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::CallbackOnNotifyResourceInfoFuzzTest(data, size);
    return 0;
}

