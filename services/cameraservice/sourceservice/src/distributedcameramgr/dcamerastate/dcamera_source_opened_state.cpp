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

#include "dcamera_source_opened_state.h"

#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceOpenedState::DCameraSourceOpenedState(std::shared_ptr<DCameraSourceStateMachine>& stateMachine)
    : stateMachine_(stateMachine)
{
    memberFuncMap_[DCAMERA_EVENT_REGIST] = &DCameraSourceOpenedState::DoRegisterTask;
    memberFuncMap_[DCAMERA_EVENT_UNREGIST] = &DCameraSourceOpenedState::DoUnregisterTask;
    memberFuncMap_[DCAMERA_EVENT_OPEN] = &DCameraSourceOpenedState::DoOpenTask;
    memberFuncMap_[DCAMERA_EVENT_CLOSE] = &DCameraSourceOpenedState::DoCloseTask;
    memberFuncMap_[DCAMERA_EVENT_CONFIG_STREAMS] = &DCameraSourceOpenedState::DoConfigStreamsTask;
    memberFuncMap_[DCAMERA_EVENT_RELEASE_STREAMS] = &DCameraSourceOpenedState::DoReleaseStreamsTask;
    memberFuncMap_[DCAMERA_EVENT_UPDATE_SETTINGS] = &DCameraSourceOpenedState::DoUpdateSettingsTask;
    memberFuncMap_[DCAMERA_EVENT_NOFIFY] = &DCameraSourceOpenedState::DoEventNofityTask;
}

int32_t DCameraSourceOpenedState::Execute(std::shared_ptr<DCameraSourceDev>& camDev, DCAMERA_EVENT eventType,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceOpenedState execute %{public}d", eventType);
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
        case DCAMERA_EVENT_CONFIG_STREAMS:
            ret = DoConfigStreamsTask(camDev, event);
            break;
        case DCAMERA_EVENT_RELEASE_STREAMS:
            ret = DoReleaseStreamsTask(camDev, event);
            break;
        case DCAMERA_EVENT_UPDATE_SETTINGS:
            ret = DoUpdateSettingsTask(camDev, event);
            break;
        case DCAMERA_EVENT_NOFIFY:
            ret = DoEventNofityTask(camDev, event);
            break;
        default:
            DHLOGE("DCameraSourceOpenedState execute %{public}d in wrong state", eventType);
            return DCAMERA_WRONG_STATE;
    }
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceOpenedState execute %{public}d failed, ret: %{public}d", eventType, ret);
    }
    return ret;
}

DCameraStateType DCameraSourceOpenedState::GetStateType()
{
    return DCAMERA_STATE_OPENED;
}

int32_t DCameraSourceOpenedState::DoRegisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceOpenedState DoRegisterTask Idempotent");
    (void)camDev;
    (void)event;
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoUnregisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
    camEvent->eventType_ = DCAMERA_MESSAGE;
    camEvent->eventResult_ = DCAMERA_EVENT_CHANNEL_DISCONNECTED;
    int32_t ret = camDev->CameraEventNotify(camEvent);
    if (ret != DCAMERA_OK) {
        DHLOGE("CameraEventNotify failed: %{public}d", ret);
        return ret;
    }

    ret = camDev->CloseCamera();
    if (ret != DCAMERA_OK) {
        DHLOGE("CloseCamera failed: %{public}d", ret);
        return ret;
    }

    std::shared_ptr<DCameraRegistParam> param;
    ret = event.GetDCameraRegistParam(param);
    if (ret != DCAMERA_OK) {
        return ret;
    }
    ret = camDev->UnRegister(param);
    if (ret != DCAMERA_OK) {
        DHLOGE("UnRegister failed: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        DHLOGE("can not get stateMachine");
        return DCAMERA_BAD_VALUE;
    }
    stateMachine->UpdateState(DCAMERA_STATE_INIT);
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoConfigStreamsTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::vector<std::shared_ptr<DCStreamInfo>> streamInfos;
    int32_t ret = event.GetStreamInfos(streamInfos);
    if (ret != DCAMERA_OK) {
        return ret;
    }

    ret = camDev->ConfigStreams(streamInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigStreams failed, ret: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        DHLOGE("can not get stateMachine");
        return DCAMERA_BAD_VALUE;
    }
    stateMachine->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoReleaseStreamsTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceOpenedState DoReleaseStreamsTask Idempotent");
    (void)camDev;
    (void)event;
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoUpdateSettingsTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    int32_t ret = event.GetCameraSettings(settings);
    if (ret != DCAMERA_OK) {
        return ret;
    }

    ret = camDev->UpdateSettings(settings);
    if (ret != DCAMERA_OK) {
        DHLOGE("UpdateSettings failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoOpenTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceOpenedState DoOpenTask Idempotent");
    (void)camDev;
    (void)event;
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoCloseTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    int32_t ret = camDev->CloseCamera();
    if (ret != DCAMERA_OK) {
        DHLOGE("CloseCamera failed, ret: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
    if (stateMachine == nullptr) {
        DHLOGE("can not get stateMachine");
        return DCAMERA_BAD_VALUE;
    }
    stateMachine->UpdateState(DCAMERA_STATE_REGIST);
    return DCAMERA_OK;
}

int32_t DCameraSourceOpenedState::DoEventNofityTask(std::shared_ptr<DCameraSourceDev>& camDev,
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
        DHLOGE("CameraEventNotify failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
