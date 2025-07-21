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

#include "dcamera_source_state_machine.h"

#include <memory>

#include "dcamera_source_state_factory.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceStateMachine::DCameraSourceStateMachine(std::shared_ptr<DCameraSourceDev>& camDev) : camDev_(camDev)
{
    DHLOGI("DCameraSourceStateMachine Create");
}

DCameraSourceStateMachine::~DCameraSourceStateMachine()
{
    DHLOGI("DCameraSourceStateMachine Delete");
}

int32_t DCameraSourceStateMachine::Execute(DCAMERA_EVENT eventType, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(currentState_ == nullptr, DCAMERA_BAD_VALUE,
        "DCameraSourceStateMachine currentState_ is nullptr, please check the state machine initialization");
    DHLOGI("In state %{public}d execute event %{public}d", currentState_->GetStateType(), eventType);
    std::shared_ptr<DCameraSourceDev> camDev = camDev_.lock();
    if (camDev == nullptr) {
        DHLOGE("DCameraSourceStateMachine execute failed, camDev is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    auto tempState = currentState_;
    int32_t ret = tempState->Execute(camDev, event.GetEventType(), event);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceStateMachine currentState_: %{public}d execute event: %{public}d failed",
            tempState->GetStateType(), event.GetEventType());
    }
    return ret;
}

void DCameraSourceStateMachine::UpdateState(DCameraStateType stateType)
{
    if (currentState_ != nullptr && stateType != DCAMERA_STATE_INIT) {
        DHLOGI("DCameraSourceStateMachine update state from %{public}d to %{public}d",
            currentState_->GetStateType(), stateType);
    } else {
        DHLOGI("DCameraSourceStateMachine update state %{public}d", stateType);
    }
    auto stateMachine = std::shared_ptr<DCameraSourceStateMachine>(shared_from_this());
    currentState_ = DCameraSourceStateFactory::GetInstance().CreateState(stateType, stateMachine);
}

int32_t DCameraSourceStateMachine::GetCameraState()
{
    CHECK_AND_RETURN_RET_LOG(currentState_ == nullptr, DCAMERA_BAD_VALUE,
        "DCameraSourceStateMachine currentState_ is nullptr");
    DHLOGI("GetCameraState In state %{public}d", currentState_->GetStateType());
    return currentState_->GetStateType();
}
} // namespace DistributedHardware
} // namespace OHOS
