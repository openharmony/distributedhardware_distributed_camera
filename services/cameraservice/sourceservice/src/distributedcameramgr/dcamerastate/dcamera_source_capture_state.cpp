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

#include "dcamera_source_capture_state.h"

#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceCaptureState::DCameraSourceCaptureState(std::shared_ptr<DCameraSourceStateMachine>& stateMachine)
    : stateMachine_(stateMachine)
{
    memberFuncMap_[DCAMERA_EVENT_REGIST] = &DCameraSourceCaptureState::DoRegisterTask;
    memberFuncMap_[DCAMERA_EVENT_UNREGIST] = &DCameraSourceCaptureState::DoUnregisterTask;
    memberFuncMap_[DCAMERA_EVENT_OPEN] = &DCameraSourceCaptureState::DoOpenTask;
    memberFuncMap_[DCAMERA_EVENT_CLOSE] = &DCameraSourceCaptureState::DoCloseTask;
    memberFuncMap_[DCAMERA_EVENT_START_CAPTURE] = &DCameraSourceCaptureState::DoStartCaptureTask;
    memberFuncMap_[DCAMERA_EVENT_STOP_CAPTURE] = &DCameraSourceCaptureState::DoStopCaptureTask;
    memberFuncMap_[DCAMERA_EVENT_UPDATE_SETTINGS] = &DCameraSourceCaptureState::DoUpdateSettingsTask;
    memberFuncMap_[DCAMERA_EVENT_NOFIFY] = &DCameraSourceCaptureState::DoEventNofityTask;
}

int32_t DCameraSourceCaptureState::Execute(std::shared_ptr<DCameraSourceDev>& camDev, DCAMERA_EVENT eventType,
    DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceCaptureState execute %{public}d", eventType);
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
        case DCAMERA_EVENT_START_CAPTURE:
            ret = DoStartCaptureTask(camDev, event);
            break;
        case DCAMERA_EVENT_STOP_CAPTURE:
            ret = DoStopCaptureTask(camDev, event);
            break;
        case DCAMERA_EVENT_UPDATE_SETTINGS:
            ret = DoUpdateSettingsTask(camDev, event);
            break;
        case DCAMERA_EVENT_NOFIFY:
            ret = DoEventNofityTask(camDev, event);
            break;
        default:
            DHLOGE("DCameraSourceCaptureState execute %{public}d in wrong state", eventType);
            return DCAMERA_WRONG_STATE;
    }
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceCaptureState execute %{public}d failed, ret: %{public}d", eventType, ret);
    }
    return ret;
}

DCameraStateType DCameraSourceCaptureState::GetStateType()
{
    return DCAMERA_STATE_CAPTURE;
}

int32_t DCameraSourceCaptureState::DoRegisterTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceCaptureState DoRegisterTask Idempotent");
    return DCAMERA_OK;
}

int32_t DCameraSourceCaptureState::DoUnregisterTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
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

    ret = camDev->StopAllCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopAllCapture failed: %{public}d", ret);
        return ret;
    }

    ret = camDev->CloseCamera();
    if (ret != DCAMERA_OK) {
        DHLOGE("CloseCamera failed, ret: %{public}d", ret);
        return ret;
    }

    ret = camDev->ReleaseAllStreams();
    if (ret != DCAMERA_OK) {
        DHLOGE("ReleaseAllStreams failed, ret: %{public}d", ret);
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

int32_t DCameraSourceCaptureState::DoOpenTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    DHLOGI("DCameraSourceCaptureState DoOpenTask Idempotent");
    return DCAMERA_OK;
}

int32_t DCameraSourceCaptureState::DoCloseTask(std::shared_ptr<DCameraSourceDev>& camDev, DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    int32_t ret = camDev->StopAllCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopAllCapture failed, ret: %{public}d", ret);
        return ret;
    }

    ret = camDev->ReleaseAllStreams();
    if (ret != DCAMERA_OK) {
        DHLOGE("ReleaseAllStreams failed, ret: %{public}d", ret);
        return ret;
    }

    ret = camDev->CloseCamera();
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

int32_t DCameraSourceCaptureState::DoStartCaptureTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::vector<std::shared_ptr<DCCaptureInfo>> captureInfos;
    int32_t ret = event.GetCaptureInfos(captureInfos);
    if (ret != DCAMERA_OK) {
        return ret;
    }

    ret = camDev->StartCapture(captureInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceCaptureState DoStartCaptureTask failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceCaptureState::DoStopCaptureTask(std::shared_ptr<DCameraSourceDev>& camDev,
    DCameraSourceEvent& event)
{
    CHECK_AND_RETURN_RET_LOG(camDev == nullptr, DCAMERA_BAD_VALUE, "camDev is nullptr");
    std::vector<int> streamIds;
    int32_t ret = event.GetStreamIds(streamIds);
    if (ret != DCAMERA_OK) {
        return ret;
    }

    bool isAllStop = false;
    ret = camDev->StopCapture(streamIds, isAllStop);
    if (ret != DCAMERA_OK) {
        DHLOGE("StopCapture failed, ret: %{public}d", ret);
        return ret;
    }

    if (isAllStop) {
        std::shared_ptr<DCameraSourceStateMachine> stateMachine = stateMachine_.lock();
        if (stateMachine == nullptr) {
            DHLOGE("can not get stateMachine");
            return DCAMERA_BAD_VALUE;
        }
        stateMachine->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceCaptureState::DoUpdateSettingsTask(std::shared_ptr<DCameraSourceDev>& camDev,
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

int32_t DCameraSourceCaptureState::DoEventNofityTask(std::shared_ptr<DCameraSourceDev>& camDev,
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
