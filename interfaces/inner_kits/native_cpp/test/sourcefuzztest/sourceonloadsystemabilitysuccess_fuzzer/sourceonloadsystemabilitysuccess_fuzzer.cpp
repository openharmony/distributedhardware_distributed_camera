/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "sourceonloadsystemabilitysuccess_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "dcamera_source_load_callback.h"
#include "dcamera_source_handler.h"
#include "distributed_camera_constants.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SourceOnLoadSystemAbilitySuccessFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < (sizeof(int32_t)))) {
        return;
    }
    int32_t systemAbilityId = *(reinterpret_cast<const int32_t *>(data));
    std::string params(reinterpret_cast<const char*>(data), size);
    std::shared_ptr<DCameraSourceLoadCallback> callback = std::make_shared<DCameraSourceLoadCallback>(params);

    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);

    callback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SourceOnLoadSystemAbilitySuccessFuzzTest(data, size);
    return 0;
}