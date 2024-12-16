/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dcamera_input_callback.h"

#include "camera_util.h"
#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraInputCallback::DCameraInputCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
{
}

void DCameraInputCallback::OnError(const int32_t errorType, const int32_t errorMsg) const
{
    DHLOGE("enter, errorType: %{public}d, errorMsg: %{public}d", errorType, errorMsg);
    if (callback_ == nullptr) {
        DHLOGE("StateCallback is null");
        return;
    }

    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = DCAMERA_MESSAGE;
    if (errorType == CameraStandard::CameraErrorCode::DEVICE_PREEMPTED) {
        event->eventResult_ = DCAMERA_EVENT_DEVICE_PREEMPT;
    } else {
        event->eventResult_ = DCAMERA_EVENT_DEVICE_ERROR;
    }
    callback_->OnStateChanged(event);
}
} // namespace DistributedHardware
} // namespace OHOS