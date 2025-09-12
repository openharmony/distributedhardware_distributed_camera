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

#include "onsourcelocalcamsrvdied_fuzzer.h"

#include <cstddef>
#include <cstdint>
#include <array>
#include "dcamera_source_handler.h"
#include "dcamera_source_handler_ipc.h"
#include "distributed_camera_constants.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "fuzzer/FuzzedDataProvider.h"
#include "ipc_object_stub.h"
namespace OHOS {
namespace DistributedHardware {
class MockRemoteObject : public OHOS::IPCObjectStub {
public:
    explicit MockRemoteObject() : IPCObjectStub(u"mock.remote.object") {}
    ~MockRemoteObject() = default;

    int OnRemoteRequest(uint32_t code, OHOS::MessageParcel& data,
                        OHOS::MessageParcel& reply, OHOS::MessageOption& option) override
    {
        return 0;
    }
};

void OnSourceLocalCamSrvDiedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    sptr<IRemoteObject> knownObject = new MockRemoteObject();
    sptr<IRemoteObject> unknownObject = new MockRemoteObject();
    sptr<IRemoteObject> nullSptr = nullptr;

    const std::array<sptr<IRemoteObject>, 3> choices = {
        knownObject,
        unknownObject,
        nullSptr
    };

    int choice = fdp.PickValueInArray({0, 1, 2});

    wptr<IRemoteObject> objectToDie = choices[choice];

    DCameraSourceHandlerIpc::GetInstance().OnSourceLocalCamSrvDied(objectToDie);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::OnSourceLocalCamSrvDiedFuzzTest(data, size);
    return 0;
}