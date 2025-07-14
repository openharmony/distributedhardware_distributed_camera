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

#include "dcamera_source_regist_state.h"

#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceRegistState::DCameraSourceRegistState(std::shared_ptr<DCameraSourceStateMachine>& stateMachine)
    : stateMachine_(stateMachine)
{
    memberFuncMap_[DCAMERA_EVENT_REGIST] = &DCameraSourceRegistState::DoRegisterTask;
    memberFuncMap_[DCAMERA_EVENT_UNREGIST] = &DCameraSourceRegistState::DoUnregisterTask;
    memberFuncMap_[DCAMERA_EVENT_OPEN] = &DCameraSourceRegistState::DoOpenTask;
    memberFuncMap_[DCAMERA_EVENT_CLOSE] = &DCameraSourceRegistState::DoCloseTask;
    memberFuncMap_[DCAMERA_EVENT_NOFIFY] = &DCameraSourceRegistState::DoEventNofityTask;
    memberFuncMap_[DCAMERA_EVENT_GET_FULLCAPS] = &DCameraSourceRegistState::DoGetFullCaps;
}

int32_t DCameraSourceRegistState::Execute(std::shared_ptr<DCameraSourceDev>& camDev, DCAMERA_EVENT eventType,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceRegistState execute %{public}d", eventType);
    int32_t ret = DCAMERA_NOT_FOUND;
    switch (eventType) {
        case DCAMERA_EVENT_REGIST:
            ret = DoRegisterTask(camDev, event);
            break;
        case DCAMERA_EVENT_UNREGIST:
            ret = DoUnregisterTask(camDev, event);
            break;
        case DCAMERA_EVENT_OPEN:
            ret = DoOpenTask(camDev, event);
            break;
        case DCAMERA_EVENT_CLOSE:
            ret = DoCloseTask(camDev, event);
            break;
        case DCAMERA_EVENT_NOFIFY:
            ret = DoEventNofityTask(camDev, event);
            break;
        case DCAMERA_EVENT_GET_FULLCAPS:
            ret = DoGetFullCaps(camDev, event);
            break;
        default:
            DHLOGE("DCameraSourceRegistState execute %{public}d in wrong state", eventType);
            return DCAMERA_WRONG_STATE;
    }
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState execute %{public}d failed, ret: %{public}d", eventType, ret);
    }
    return ret;
}

DCameraStateType DCameraSourceRegistState::GetStateType()
{
    return DCAMERA_STATE_REGIST;
}

int32_t DCameraSourceRegistState::DoRegisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    DHLOGI("DCameraSourceRegistState DoRegisterTask");
    std::shared_ptr<DCameraRegistParam> param;
    int32_t ret = event.GetDCameraRegistParam(param);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState GetDCameraRegistParam failed");
        return ret;
    }
    ret = camDev->Register(param);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState Register failed");
        return ret;
    }
    return DCAMERA_OK;
}


int32_t DCameraSourceRegistState::DoUnregisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::shared_ptr<DCameraRegistParam> param;
    int32_t ret = event.GetDCameraRegistParam(param);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    ret = camDev->UnRegister(param);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        DHLOGE("DCameraSourceRegistState DoUnregisterTask can not get stateMachine");
        return DCAMERA_BAD_VALUE;
    }
    stateMachine->UpdateState(DCAMERA_STATE_INIT);
    return DCAMERA_OK;
}

int32_t DCameraSourceRegistState::DoOpenTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    int32_t ret = camDev->OpenCamera();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState OpenCamera failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceRegistState::DoCloseTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceRegistState DoCloseTask Idempotent");
    return DCAMERA_OK;
}

int32_t DCameraSourceRegistState::DoEventNofityTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::shared_ptr<DCameraEvent> camEvent;
    int32_t ret = event.GetCameraEvent(camEvent);
    if (ret != DCAMERA_OK) {
        return ret;
    }

    ret = camDev->CameraEventNotify(camEvent);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState DoEventNofityTask CameraEventNotify failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceRegistState::DoGetFullCaps(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceRegistState DoGetFullCaps enter.");
    if (camDev == nullptr) {
        DHLOGE("DCameraSourceRegistState camDev is null.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = camDev->GetFullCaps();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceRegistState DoGetFullCaps GetFullCaps failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
