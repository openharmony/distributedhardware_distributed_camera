/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dcamera_source_init_state.h"

#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceInitState::DCameraSourceInitState(std::shared_ptr<DCameraSourceStateMachine>& stateMachine)
    : stateMachine_(stateMachine)
{
    memberFuncMap_[DCAMERA_EVENT_REGIST] = &DCameraSourceInitState::DoRegisterTask;
    memberFuncMap_[DCAMERA_EVENT_UNREGIST] = &DCameraSourceInitState::DoUnregisterTask;
}

int32_t DCameraSourceInitState::Execute(std::shared_ptr<DCameraSourceDev>& camDev, DCAMERA_EVENT eventType,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceInitState execute %{public}d", eventType);
    int32_t ret = DCAMERA_NOT_FOUND;
    switch (eventType) {
        case DCAMERA_EVENT_REGIST:
            ret = DoRegisterTask(camDev, event);
            break;
        case DCAMERA_EVENT_UNREGIST:
            ret = DoUnregisterTask(camDev, event);
            break;
        default:
            DHLOGE("DCameraSourceInitState execute %{public}d in wrong state", eventType);
            return DCAMERA_WRONG_STATE;
    }
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInitState execute %{public}d failed, ret: %{public}d", eventType, ret);
    }
    return ret;
}

DCameraStateType DCameraSourceInitState::GetStateType()
{
    return DCAMERA_STATE_INIT;
}

int32_t DCameraSourceInitState::DoRegisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::shared_ptr<DCameraRegistParam> param;
    int32_t ret = event.GetDCameraRegistParam(param);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    ret = camDev->Register(param);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        DHLOGE("DCameraSourceInitState can not get stateMachine");
        return DCAMERA_BAD_VALUE;
    }
    stateMachine->UpdateState(DCAMERA_STATE_REGIST);
    return DCAMERA_OK;
}

int32_t DCameraSourceInitState::DoUnregisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceInitState DoUnregisterTask Idempotent");
    (void)camDev;
    (void)event;
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
