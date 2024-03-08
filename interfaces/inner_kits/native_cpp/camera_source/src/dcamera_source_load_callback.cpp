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

#include "dcamera_source_load_callback.h"

#include "dcamera_hisysevent_adapter.h"
#include "dcamera_source_handler.h"
#include "distributed_hardware_log.h"
#include "distributed_camera_constants.h"
namespace OHOS { class IRemoteObject; }

namespace OHOS {
namespace DistributedHardware {
DCameraSourceLoadCallback::DCameraSourceLoadCallback(const std::string& params) : params_(params) {}
void DCameraSourceLoadCallback::OnLoadSystemAbilitySuccess(int32_t systemAbilityId,
    const sptr<IRemoteObject>& remoteObject)
{
    DHLOGI("OnLoadSystemAbilitySuccess systemAbilityId: %{public}d, IRmoteObject result: %{public}s",
        systemAbilityId, (remoteObject != nullptr) ? "true" : "false");
    if (systemAbilityId != DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID) {
        DHLOGE("start systemabilityId is not sourceSAId!");
        return;
    }
    if (remoteObject == nullptr) {
        DHLOGE("remoteObject is null.");
        return;
    }
    DCameraSourceHandler::GetInstance().FinishStartSA(params_);
}

void DCameraSourceLoadCallback::OnLoadSystemAbilityFail(int32_t systemAbilityId)
{
    DHLOGI("OnLoadSystemAbilityFail systemAbilityId: %{public}d.", systemAbilityId);
    if (systemAbilityId != DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID) {
        DHLOGE("start systemabilityId is not sourceSAId!");
        ReportDcamerInitFail(DCAMERA_INIT_FAIL, DCAMERA_SA_ERROR,
            CreateMsg("dcamera source OnLoadSystemAbilityFail, systemAbilityId: %d", systemAbilityId));
        return;
    }
    DCameraSourceHandler::GetInstance().FinishStartSAFailed(systemAbilityId);
}
}
}