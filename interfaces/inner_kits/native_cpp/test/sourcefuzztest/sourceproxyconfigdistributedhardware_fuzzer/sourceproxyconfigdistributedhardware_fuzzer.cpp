/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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

#include "sourceproxyconfigdistributedhardware_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SourceProxyConfigDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string key = fdp.ConsumeRandomLengthString();
    std::string value = fdp.ConsumeRandomLengthString();
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return;
    }
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    std::shared_ptr<DistributedCameraSourceProxy> dCSourceProxy =
        std::make_shared<DistributedCameraSourceProxy>(remoteObject);
    sptr<IDCameraSourceCallback> callback(new (std::nothrow) DCameraSourceCallback());

    dCSourceProxy->ConfigDistributedHardware(devId, dhId, key, value);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceProxyConfigDistributedHardwareFuzzTest(data, size);
    return 0;
}