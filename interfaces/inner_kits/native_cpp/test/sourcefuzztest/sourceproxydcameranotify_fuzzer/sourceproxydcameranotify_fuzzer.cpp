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

#include "sourceproxydcameranotify_fuzzer.h"
#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void SourceProxyDCameraNotifyFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string events = fdp.ConsumeRemainingBytesAsString();
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return;
    }
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    if (remoteObject == nullptr) {
        return;
    }
    std::shared_ptr<DistributedCameraSourceProxy> dCSourceProxy =
        std::make_shared<DistributedCameraSourceProxy>(remoteObject);

    dCSourceProxy->DCameraNotify(devId, dhId, events);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceProxyDCameraNotifyFuzzTest(data, size);
    return 0;
}