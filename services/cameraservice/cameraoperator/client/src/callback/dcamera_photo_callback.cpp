/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "dcamera_photo_callback.h"

#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraPhotoCallback::DCameraPhotoCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
{
}

void DCameraPhotoCallback::OnCaptureStarted(const int32_t captureID) const
{
    DHLOGI("enter, captureID: %{public}d", captureID);
}

void DCameraPhotoCallback::OnCaptureStarted(const int32_t captureID, uint32_t exposureTime) const
{
    DHLOGI("enter, captureID: %{public}d, exposureTime: %{public}u", captureID, exposureTime);
}

void DCameraPhotoCallback::OnCaptureEnded(const int32_t captureID, const int32_t frameCount) const
{
    DHLOGI("enter, captureID: %{public}d, frameCount: %{public}d", captureID, frameCount);
}

void DCameraPhotoCallback::OnFrameShutter(const int32_t captureId, const uint64_t timestamp) const
{
    DHLOGI("enter, captureId: %{public}d, timestamp: %{public}" PRIu64, captureId, timestamp);
}

void DCameraPhotoCallback::OnFrameShutterEnd(const int32_t captureId, const uint64_t timestamp) const
{
    DHLOGI("enter, captureId: %{public}d, timestamp: %{public}" PRIu64, captureId, timestamp);
}

void DCameraPhotoCallback::OnCaptureReady(const int32_t captureId, const uint64_t timestamp) const
{
    DHLOGI("enter, captureId: %{public}d, timestamp: %{public}" PRIu64, captureId, timestamp);
}

void DCameraPhotoCallback::OnEstimatedCaptureDuration(const int32_t duration) const
{
    DHLOGI("enter, duration: %{public}d", duration);
}

void DCameraPhotoCallback::OnOfflineDeliveryFinished(const int32_t captureId) const
{
    DHLOGI("enter, captureId: %{public}d", captureId);
}

void DCameraPhotoCallback::OnCaptureError(const int32_t captureId, const int32_t errorCode) const
{
    DHLOGE("enter, captureId: %{public}d, errorCode: %{public}d", captureId, errorCode);
    if (callback_ == nullptr) {
        DHLOGE("StateCallback is null");
        return;
    }

    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = DCAMERA_MESSAGE;
    event->eventResult_ = DCAMERA_EVENT_DEVICE_ERROR;
    callback_->OnStateChanged(event);
}
} // namespace DistributedHardware
} // namespace OHOS