/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dcamera_session_callback.h"

#include "dcamera_event_cmd.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSessionCallback::DCameraSessionCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
{
}

void DCameraSessionCallback::OnError(int32_t errorCode)
{
    DHLOGE("enter, errorCode: %{public}d", errorCode);
    if (callback_ == nullptr) {
        DHLOGE("StateCallback is null");
        return;
    }

    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = DCAMERA_MESSAGE;
    event->eventResult_ = DCAMERA_EVENT_DEVICE_ERROR;
    callback_->OnStateChanged(event);
}

void DCameraSessionCallback::OnFocusState(FocusState state)
{
    DHLOGI("enter, state: %{public}d", state);
    if (callback_ == nullptr) {
        DHLOGE("StateCallback is null");
        return;
    }

    auto iter = focusStateMap_.find(state);
    if (iter == focusStateMap_.end()) {
        DHLOGE("focusStateMap find %{public}d state failed", state);
        return;
    }

    int32_t dataCountStartNum = 1;
    uint8_t focusState = iter->second;
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata =
        std::make_shared<Camera::CameraMetadata>(CAMERA_META_DATA_ITEM_CAPACITY, CAMERA_META_DATA_DATA_CAPACITY);
    if (!cameraMetadata->addEntry(OHOS_CONTROL_FOCUS_STATE, &focusState, dataCountStartNum)) {
        DHLOGE("cameraMetadata add entry failed");
        return;
    }

    std::string abilityString = Camera::MetadataUtils::EncodeToString(cameraMetadata);
    std::string encodeString = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());

    std::shared_ptr<DCameraSettings> dcSetting = std::make_shared<DCameraSettings>();
    dcSetting->type_ = DCSettingsType::METADATA_RESULT;
    dcSetting->value_ = encodeString;
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    settings.push_back(dcSetting);
}
} // namespace DistributedHardware
} // namespace OHOS