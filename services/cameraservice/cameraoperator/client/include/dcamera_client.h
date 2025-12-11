/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_CLIENT_H
#define OHOS_DCAMERA_CLIENT_H

#include "icamera_operator.h"

#include <queue>

#include "camera_info.h"
#include "camera_input.h"
#include "camera_manager.h"
#include "camera_output_capability.h"
#include "capture_input.h"
#include "capture_output.h"
#include "capture_session.h"
#include "photo_output.h"
#include "preview_output.h"

#include "dcamera_photo_surface_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraClient : public ICameraOperator {
public:
    explicit DCameraClient(const std::string& dhId);
    ~DCameraClient() override;

    int32_t Init() override;
    int32_t UnInit() override;
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings) override;
    int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
        sptr<Surface>& surface, int32_t sceneMode) override;
    int32_t PrepareCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode) override;
    int32_t CommitCapture(sptr<Surface>& surface) override;
    int32_t StopCapture() override;
    int32_t SetStateCallback(std::shared_ptr<StateCallback>& callback) override;
    int32_t SetResultCallback(std::shared_ptr<ResultCallback>& callback) override;
    int32_t PauseCapture() override;
    int32_t ResumeCapture() override;

private:
    int32_t ConfigCaptureSession(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode);
    int32_t ConfigCaptureSessionInner();
    int32_t CreateCaptureOutput(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos);
    int32_t CreatePhotoOutput(std::shared_ptr<DCameraCaptureInfo>& info);
    int32_t CreatePreviewOutput(std::shared_ptr<DCameraCaptureInfo>& info);
    int32_t StartCaptureInner(std::shared_ptr<DCameraCaptureInfo>& info);
    int32_t StartPhotoOutput(std::shared_ptr<DCameraCaptureInfo>& info);
    void FindCameraMetadata(const std::string& metadataStr);
    void SetPhotoCaptureRotation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
        std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting);
    void SetPhotoCaptureQuality(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
        std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting);
    void SetPhotoCaptureLocation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
        std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting);
    void StopOutput();
    void ReleaseCaptureSession();
    int32_t CameraServiceErrorType(const int32_t errorType);
    CameraStandard::CameraFormat ConvertToCameraFormat(int32_t format);
    void UpdateSettingCache(const std::string& metadataStr);
    void GetFpsRanges();

private:
    constexpr static uint32_t DCAMERA_MAX_METADATA_SIZE = 20;
    constexpr static uint32_t DCAMERA_FPS_SIZE = 2;
    constexpr static int32_t DCAMERA_MAX_FPS = 30;

    bool isInit_;
    std::string cameraId_;
    std::queue<std::string> cameraMetadatas_;
    sptr<IConsumerSurface> photoSurface_;
    sptr<Surface> previewSurface_;
    sptr<CameraStandard::CameraDevice> cameraInfo_;
    sptr<CameraStandard::CameraManager> cameraManager_;
    sptr<CameraStandard::CaptureSession> captureSession_;
    sptr<CameraStandard::CaptureInput> cameraInput_;
    sptr<CameraStandard::CaptureOutput> photoOutput_;
    sptr<CameraStandard::CaptureOutput> previewOutput_;
    sptr<DCameraPhotoSurfaceListener> photoListener_;
    std::shared_ptr<StateCallback> stateCallback_;
    std::shared_ptr<ResultCallback> resultCallback_;
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfosCache_;
    std::vector<int32_t> fpsRanges_ = {};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_CLIENT_H
