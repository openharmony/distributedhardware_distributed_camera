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
#include "dcamera_utils_tools.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t SECONDS_TO_MICROSECONDS = 1000000;

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

CaOnResultCallback::CaOnResultCallback(const char* Name) : Name_(Name)
{
}
 
void CaOnResultCallback::OnResult(const uint64_t timestamp,
    const std::shared_ptr<Camera::CameraMetadata> &result) const
{
    DHLOGI("OnResult start for:%{public}s", Name_);
    auto metaData = result->get();
    camera_metadata_item item{};
    auto ret = Camera::FindCameraMetadataItem(metaData, OHOS_STATUS_SENSOR_EXPOSURE_TIME, &item);
    DHLOGI("Get exp time.replace");
    if (ret == CAM_META_SUCCESS) {
        int32_t numerator = item.data.r->numerator;
        int32_t denominator = item.data.r->denominator;
        uint32_t value = static_cast<uint32_t>(numerator / (denominator / SECONDS_TO_MICROSECONDS));
        DCameraExpoTime::GetInstance().SetExpoTime(value);
    }
}
} // namespace DistributedHardware
} // namespace OHOS