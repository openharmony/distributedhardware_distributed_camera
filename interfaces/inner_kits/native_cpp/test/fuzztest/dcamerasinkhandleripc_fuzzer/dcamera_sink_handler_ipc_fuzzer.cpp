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

#include "dcamera_sink_handler_ipc_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "dcamera_sink_handler.h"
#include "dcamera_sink_handler_ipc.h"
#include "distributed_camera_constants.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void DCameraSinkHandlerIpcFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size <= 0)) {
        return;
    }

    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    wptr<IRemoteObject> remote (remoteObject);

    DCameraSinkHandlerIpc::GetInstance().Init();
    DCameraSinkHandlerIpc::GetInstance().GetSinkLocalDHMS();
    DCameraSinkHandlerIpc::GetInstance().OnSinkLocalDmsDied(remote);
    DCameraSinkHandlerIpc::GetInstance().UnInit();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DCameraSinkHandlerIpcFuzzTest(data, size);
    return 0;
}

