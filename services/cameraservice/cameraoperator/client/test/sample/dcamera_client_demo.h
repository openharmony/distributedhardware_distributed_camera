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

#ifndef OHOS_DCAMERA_CLIENT_DEMO_H
#define OHOS_DCAMERA_CLIENT_DEMO_H

#include <fcntl.h>
#include <functional>
#include <securec.h>
#include <sys/time.h>
#include <unistd.h>

#include "camera_device_ability_items.h"
#include "camera_metadata_info.h"
#include "capture_input.h"
#include "capture_output.h"

#include "anonymous_string.h"
#include "dcamera_input_callback.h"
#include "dcamera_manager_callback.h"
#include "dcamera_photo_callback.h"
#include "dcamera_photo_surface_listener.h"
#include "dcamera_preview_callback.h"
#include "dcamera_session_callback.h"
#include "dcamera_utils_tools.h"
#include "dcamera_video_callback.h"
#include "dcamera_video_surface_listener.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"

static int32_t FILE_PERMISSIONS_FLAG = 00766;
const uint32_t CAPTURE_WIDTH = 640;
const uint32_t CAPTURE_HEIGTH = 480;
const int32_t LATITUDE = 0;
const int32_t LONGITUDE = 1;
const int32_t ALTITUDE = 2;
const int32_t SLEEP_FIVE_SECOND = 5;
const int32_t SLEEP_TWENTY_SECOND = 20;

namespace OHOS {
namespace DistributedHardware {
class DemoDCameraPhotoCallback : public CameraStandard::PhotoCallback {
public:
    DemoDCameraPhotoCallback(const std::shared_ptr<StateCallback>& callback): callback_(callback)
    {
    }
    void OnCaptureStarted(const int32_t captureID) const
    {
        DHLOGI("DemoDCameraPhotoCallback::OnCaptureStarted captureID: %d", captureID);
    }
    void OnCaptureEnded(const int32_t captureID, int32_t frameCount) const
    {
        DHLOGI("DemoDCameraPhotoCallback::OnCaptureEnded captureID: %d frameCount: %d", captureID, frameCount);
    }
    void OnFrameShutter(const int32_t captureId, const uint64_t timestamp) const
    {
        DHLOGI("DemoDCameraPhotoCallback::OnFrameShutter captureID: %d timestamp: %llu", captureId, timestamp);
    }
    void OnCaptureError(const int32_t captureId, const int32_t errorCode) const
    {
        DHLOGI("DemoDCameraPhotoCallback::OnCaptureError captureID: %d errorCode: %d", captureId, errorCode);
    }

private:
    std::shared_ptr<StateCallback> callback_;
};

class DemoDCameraPreviewCallback : public CameraStandard::PreviewCallback {
public:
    DemoDCameraPreviewCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
    {
    }
    void OnFrameStarted() const {
        DHLOGI("DemoDCameraPreviewCallback::OnFrameStarted.");
    }
    void OnFrameEnded(const int32_t frameCount) const  {
        DHLOGI("DemoDCameraPreviewCallback::OnFrameEnded frameCount: %d", frameCount);
    }
    void OnError(const int32_t errorCode) const {
        DHLOGI("DemoDCameraPreviewCallback::OnError errorCode: %d", errorCode);
    }

private:
    std::shared_ptr<StateCallback> callback_;
};

class DemoDCameraVideoCallback : public CameraStandard::VideoCallback {
public:
    DemoDCameraVideoCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
    {
    }
    void OnFrameStarted() const {
        DHLOGI("DemoDCameraVideoCallback::OnFrameStarted.");
    }
    void OnFrameEnded(const int32_t frameCount) const  {
        DHLOGI("DemoDCameraVideoCallback::OnFrameEnded frameCount: %d", frameCount);
    }
    void OnError(const int32_t errorCode) const {
        DHLOGI("DemoDCameraVideoCallback::OnError errorCode: %d", errorCode);
    }

private:
    std::shared_ptr<StateCallback> callback_;
};

class DemoDCameraInputCallback : public CameraStandard::ErrorCallback, public CameraStandard::FocusCallback {
public:
    explicit DemoDCameraInputCallback(const std::shared_ptr<StateCallback>& callback) : callback_(callback)
    {

    }
    void OnError(const int32_t errorType, const int32_t errorMsg) const
    {
        DHLOGI("DemoDCameraInputCallback::OnError errorType: %d errorMsg: %d", errorType, errorMsg);
    }
    void OnFocusState(FocusState state)
    {
    }
private:
    std::shared_ptr<StateCallback> callback_;
};

class DCameraDemoStateCallback : public StateCallback {
public:
    void OnStateChanged(std::shared_ptr<DCameraEvent>& event) override
    {
        DHLOGI("DCameraDemoStateCallback::OnStateChanged type: %d, result: %d",
            event->eventType_, event->eventResult_);
    }

    void OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings) override;
};

class DCameraDemoPhotoResultCallback : public ResultCallback {
public:
    void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer) override;
    void OnVideoResult(std::shared_ptr<DataBuffer>& buffer) override
    {
        DHLOGI("DCameraDemoPhotoResultCallback::OnVideoResult");
    }
};

class DCameraDemoPreviewResultCallback : public ResultCallback {
public:
    void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer) override
    {
        DHLOGI("DCameraDemoPreviewResultCallback::OnPhotoResult");
    }

    void OnVideoResult(std::shared_ptr<DataBuffer>& buffer) override;
};

class DCameraDemoVideoResultCallback : public ResultCallback {
public:
    void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer) override
    {
        DHLOGI("DCameraDemoVideoResultCallback::OnPhotoResult");
    }

    void OnVideoResult(std::shared_ptr<DataBuffer>& buffer) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_CLIENT_DEMO_H