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

#include "dcamera_client_demo.h"

using namespace OHOS;
using namespace OHOS::Camera;
using namespace OHOS::CameraStandard;
using namespace OHOS::DistributedHardware;

static sptr<CameraInfo> GetCameraInfo(sptr<CameraManager> cameraManager)
{
    std::vector<sptr<CameraInfo>> cameraObjList = cameraManager->GetCameras();
    for (auto& info : cameraObjList) {
        DHLOGI("Distributed Camera Demo: %s position: %d, camera type: %d, connection type: %d",
            info->GetID().c_str(), info->GetPosition(), info->GetCameraType(), info->GetConnectionType());
    }

    return cameraObjList.back();
}

static void InitCaptureInfo(std::shared_ptr<DCameraCaptureInfo>& captureInfo)
{
    captureInfo->width_ = CAPTURE_WIDTH;
    captureInfo->height_ = CAPTURE_HEIGTH;
    captureInfo->format_ = 1;
}

static void SetPhotoSurface(sptr<Surface>& photoSurface, std::shared_ptr<DCameraCaptureInfo> photoInfo)
{
    std::shared_ptr<ResultCallback> photoResultCallback = std::make_shared<DCameraDemoPhotoResultCallback>();
    photoSurface->SetDefaultWidthAndHeight(photoInfo->width_, photoInfo->height_);
    photoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(photoInfo->format_));
    std::shared_ptr<DCameraPhotoSurfaceListener> photoListener =
        std::make_shared<DCameraPhotoSurfaceListener>(photoSurface, photoResultCallback);
    photoSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)photoListener);
}

static void SetPreviewSurface(sptr<Surface>& previewSurface, std::shared_ptr<DCameraCaptureInfo> previewInfo)
{
    std::shared_ptr<ResultCallback> previewResultCallback = std::make_shared<DCameraDemoPreviewResultCallback>();
    previewSurface->SetDefaultWidthAndHeight(previewInfo->width_, previewInfo->height_);
    previewSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(previewInfo->format_));
    std::shared_ptr<DCameraVideoSurfaceListener> previewListener =
        std::make_shared<DCameraVideoSurfaceListener>(previewSurface, previewResultCallback);
    previewSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)previewListener);
}

static void SetVideoSurface(sptr<Surface>& videoSurface, std::shared_ptr<DCameraCaptureInfo> videoInfo)
{
    std::shared_ptr<ResultCallback> videoResultCallback = std::make_shared<DCameraDemoVideoResultCallback>();
    videoSurface->SetDefaultWidthAndHeight(videoInfo->width_, videoInfo->height_);
    videoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(videoInfo->format_));
    std::shared_ptr<DCameraVideoSurfaceListener> videoListener =
        std::make_shared<DCameraVideoSurfaceListener>(videoSurface, videoResultCallback);
    videoSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)videoListener);
}

static void SetCaptureSettings(std::shared_ptr<PhotoCaptureSetting>& photoCaptureSettings)
{
    DHLOGI("Distributed Camera Demo: SetCaptureSettings");
    // rotation
    PhotoCaptureSetting::RotationConfig rotation = PhotoCaptureSetting::RotationConfig::Rotation_0;
    photoCaptureSettings->SetRotation(rotation);
    // jpeg quality
    PhotoCaptureSetting::QualityLevel quality = PhotoCaptureSetting::QualityLevel::HIGH_QUALITY;
    photoCaptureSettings->SetQuality(quality);
    // gps coordinates
    double gpsCoordinates[3] = { 33.52256, 56.32586, 22.665132 };
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->latitude = gpsCoordinates[LATITUDE];
    location->longitude = gpsCoordinates[LONGITUDE];
    location->altitude = gpsCoordinates[ALTITUDE];
    photoCaptureSettings->SetLocation(location);
}

static void SetFocusAndExposure(sptr<CaptureInput>& cameraInput)
{
    ((sptr<CameraInput> &)cameraInput)->LockForControl();
    camera_focus_mode_enum_t focusMode = OHOS_CAMERA_FOCUS_MODE_AUTO;
    camera_exposure_mode_enum_t exposureMode = OHOS_CAMERA_EXPOSURE_MODE_AUTO;
    int32_t exposureValue = 0;
    std::vector<int32_t> biasRange = ((sptr<CameraInput> &)cameraInput)->GetExposureBiasRange();
    if (!biasRange.empty()) {
        DHLOGI("Distributed Camera Demo: biasRange.size(): %d", biasRange.size());
        exposureValue = biasRange[0];
    }
    ((sptr<CameraInput> &)cameraInput)->SetFocusMode(focusMode); // 对焦模式
    ((sptr<CameraInput> &)cameraInput)->SetExposureMode(exposureMode); // 曝光模式
    ((sptr<CameraInput> &)cameraInput)->SetExposureBias(exposureValue); // 曝光补偿
    ((sptr<CameraInput> &)cameraInput)->UnlockForControl();
}

int main() {
    DHLOGI("========== Distributed Camera Demo Start ==========");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraDemoStateCallback>();

    DHLOGI("Distributed Camera Demo: Create CameraManager and CaptureSession");
    sptr<CameraManager> cameraManager = CameraManager::GetInstance();
    cameraManager->SetCallback(std::make_shared<DCameraManagerCallback>());
    sptr<CaptureSession> captureSession = cameraManager->CreateCaptureSession();
    captureSession->SetCallback(std::make_shared<DCameraSessionCallback>(stateCallback));

    DHLOGI("Distributed Camera Demo: Create CameraInfo and CaptureInput");
    sptr<CameraInfo> cameraInfo = GetCameraInfo(cameraManager);
    sptr<CaptureInput> cameraInput = cameraManager->CreateCameraInput(cameraInfo);
    std::shared_ptr<DemoDCameraInputCallback> inputCallback = std::make_shared<DemoDCameraInputCallback>(stateCallback);
    ((sptr<CameraInput> &)cameraInput)->SetErrorCallback(inputCallback);
    ((sptr<CameraInput> &)cameraInput)->SetFocusCallback(inputCallback);

    std::shared_ptr<DCameraCaptureInfo> photoInfo = std::make_shared<DCameraCaptureInfo>();
    InitCaptureInfo(photoInfo);

    std::shared_ptr<DCameraCaptureInfo> previewInfo = std::make_shared<DCameraCaptureInfo>();
    InitCaptureInfo(previewInfo);

    std::shared_ptr<DCameraCaptureInfo> videoInfo = std::make_shared<DCameraCaptureInfo>();
    InitCaptureInfo(videoInfo);

    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %d, height = %d, format = %d",
        photoInfo->width_, photoInfo->height_, photoInfo->format_);
    sptr<Surface> photoSurface = Surface::CreateSurfaceAsConsumer();
    SetPhotoSurface(photoSurface, photoInfo);
    sptr<CaptureOutput> photoOutput = cameraManager->CreatePhotoOutput(photoSurface);
    ((sptr<PhotoOutput> &)photoOutput)->SetCallback(std::make_shared<DemoDCameraPhotoCallback>(stateCallback));

    std::shared_ptr<PhotoCaptureSetting> photoCaptureSettings = std::make_shared<PhotoCaptureSetting>();
    SetCaptureSettings(photoCaptureSettings);

    DHLOGI("Distributed Camera Demo: Create PreviewOutput, width = %d, height = %d, format = %d",
        previewInfo->width_, previewInfo->height_, previewInfo->format_);
    sptr<Surface> previewSurface = Surface::CreateSurfaceAsConsumer();
    SetPreviewSurface(previewSurface, previewInfo);
    sptr<CaptureOutput> previewOutput = cameraManager->CreateCustomPreviewOutput(previewSurface,
        previewInfo->width_, previewInfo->height_);
    ((sptr<PreviewOutput> &)previewOutput)->SetCallback(std::make_shared<DemoDCameraPreviewCallback>(stateCallback));

    DHLOGI("Distributed Camera Demo: Create VideoOutput, width = %d, height = %d, format = %d",
        videoInfo->width_, videoInfo->height_, videoInfo->format_);
    sptr<Surface> videoSurface = Surface::CreateSurfaceAsConsumer();
    SetVideoSurface(videoSurface, videoInfo);
    sptr<CaptureOutput> videoOutput = cameraManager->CreateVideoOutput(videoSurface);
    ((sptr<VideoOutput> &)videoOutput)->SetCallback(std::make_shared<DemoDCameraVideoCallback>(stateCallback));

    captureSession->BeginConfig();
    captureSession->AddInput(cameraInput);
    captureSession->AddOutput(photoOutput);
    captureSession->AddOutput(previewOutput);
    captureSession->CommitConfig();
    captureSession->Start();
    sleep(SLEEP_FIVE_SECOND);

    SetFocusAndExposure(cameraInput);
    sleep(SLEEP_FIVE_SECOND);

    ((sptr<PhotoOutput> &)photoOutput)->Capture(photoCaptureSettings);
    sleep(SLEEP_TWENTY_SECOND);

    captureSession->Stop();
    captureSession->Release();
    cameraInput->Release();
    cameraManager->SetCallback(nullptr);

    DHLOGI("========== Distributed Camera Demo End ==========");
    return 0;
}