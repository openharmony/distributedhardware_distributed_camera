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

static sptr<Surface> g_photoSurface = nullptr;
static sptr<Surface> g_previewSurface = nullptr;
static sptr<Surface> g_videoSurface = nullptr;
static sptr<CaptureOutput> g_photoOutput = nullptr;
static sptr<CaptureOutput> g_previewOutput = nullptr;
static sptr<CaptureOutput> g_videoOutput = nullptr;
static std::shared_ptr<DCameraPhotoSurfaceListener> g_photoListener = nullptr;
static std::shared_ptr<DCameraVideoSurfaceListener> g_previewListener = nullptr;
static std::shared_ptr<DCameraVideoSurfaceListener> g_videoListener = nullptr;
static std::shared_ptr<ResultCallback> g_photoResultCallback = nullptr;
static std::shared_ptr<ResultCallback> g_previewResultCallback = nullptr;
static std::shared_ptr<ResultCallback> g_videoResultCallback = nullptr;

static sptr<CameraInfo> GetCameraInfo(sptr<CameraManager> cameraManager)
{
    std::vector<sptr<CameraInfo>> cameraObjList = cameraManager->GetCameras();
    for (auto& info : cameraObjList) {
        DHLOGI("Distributed Camera Demo: %s, position: %d, camera type: %d, connection type: %d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType(),
            info->GetConnectionType());
    }

    return cameraObjList.back();
}

static void InitPhotoInfo(std::shared_ptr<DCameraCaptureInfo>& photoInfo)
{
    photoInfo->width_ = CAPTURE_WIDTH;
    photoInfo->height_ = CAPTURE_HEIGTH;
    photoInfo->format_ = PHOTO_FORMAT;
}

static void InitPreviewInfo(std::shared_ptr<DCameraCaptureInfo>& previewInfo)
{
    previewInfo->width_ = CAPTURE_WIDTH;
    previewInfo->height_ = CAPTURE_HEIGTH;
    previewInfo->format_ = VIDEO_FORMAT;
}

static void InitVideoInfo(std::shared_ptr<DCameraCaptureInfo>& videoInfo)
{
    videoInfo->width_ = CAPTURE_WIDTH;
    videoInfo->height_ = CAPTURE_HEIGTH;
    videoInfo->format_ = VIDEO_FORMAT;
}

static void SetPhotoOutput(sptr<CameraManager>& cameraManager, std::shared_ptr<DCameraCaptureInfo>& photoInfo,
    std::shared_ptr<StateCallback>& stateCallback)
{
    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %d, height = %d, format = %d",
        photoInfo->width_, photoInfo->height_, photoInfo->format_);
    g_photoSurface = Surface::CreateSurfaceAsConsumer();
    g_photoResultCallback = std::make_shared<DCameraDemoPhotoResultCallback>();
    g_photoSurface->SetDefaultWidthAndHeight(photoInfo->width_, photoInfo->height_);
    g_photoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(photoInfo->format_));
    g_photoListener = std::make_shared<DCameraPhotoSurfaceListener>(g_photoSurface, g_photoResultCallback);
    g_photoSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)g_photoListener);
    g_photoOutput = cameraManager->CreatePhotoOutput(g_photoSurface);
    ((sptr<PhotoOutput> &)g_photoOutput)->SetCallback(std::make_shared<DemoDCameraPhotoCallback>(stateCallback));
}

static void SetPreviewOutput(sptr<CameraManager>& cameraManager, std::shared_ptr<DCameraCaptureInfo>& previewInfo,
    std::shared_ptr<StateCallback>& stateCallback)
{
    DHLOGI("Distributed Camera Demo: Create PreviewOutput, width = %d, height = %d, format = %d",
        previewInfo->width_, previewInfo->height_, previewInfo->format_);
    g_previewSurface = Surface::CreateSurfaceAsConsumer();
    g_previewResultCallback = std::make_shared<DCameraDemoPreviewResultCallback>();
    g_previewSurface->SetDefaultWidthAndHeight(previewInfo->width_, previewInfo->height_);
    g_previewSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(previewInfo->format_));
    g_previewListener = std::make_shared<DCameraVideoSurfaceListener>(g_previewSurface, g_previewResultCallback);
    g_previewSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)g_previewListener);
    g_previewOutput = cameraManager->CreateCustomPreviewOutput(g_previewSurface,
        previewInfo->width_, previewInfo->height_);
    ((sptr<PreviewOutput> &)g_previewOutput)->SetCallback(std::make_shared<DemoDCameraPreviewCallback>(stateCallback));
}

static void SetVideoOutput(sptr<CameraManager>& cameraManager, std::shared_ptr<DCameraCaptureInfo>& videoInfo,
    std::shared_ptr<StateCallback>& stateCallback)
{
    DHLOGI("Distributed Camera Demo: Create VideoOutput, width = %d, height = %d, format = %d",
        videoInfo->width_, videoInfo->height_, videoInfo->format_);
    g_videoSurface = Surface::CreateSurfaceAsConsumer();
    g_videoResultCallback = std::make_shared<DCameraDemoVideoResultCallback>();
    g_videoSurface->SetDefaultWidthAndHeight(videoInfo->width_, videoInfo->height_);
    g_videoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(videoInfo->format_));
    g_videoListener = std::make_shared<DCameraVideoSurfaceListener>(g_videoSurface, g_videoResultCallback);
    g_videoSurface->RegisterConsumerListener((sptr<IBufferConsumerListener> &)g_videoListener);
    g_videoOutput = cameraManager->CreateVideoOutput(g_videoSurface);
    ((sptr<VideoOutput> &)g_videoOutput)->SetCallback(std::make_shared<DemoDCameraVideoCallback>(stateCallback));
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
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->latitude = 22.306; // 22.306: latitude
    location->longitude = 52.12; // 52.12:longitude
    location->altitude = 2.365; // 2.365: altitude
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
    ((sptr<CameraInput> &)cameraInput)->SetFocusMode(focusMode);
    ((sptr<CameraInput> &)cameraInput)->SetExposureMode(exposureMode);
    ((sptr<CameraInput> &)cameraInput)->SetExposureBias(exposureValue);
    ((sptr<CameraInput> &)cameraInput)->UnlockForControl();
}

int main()
{
    DHLOGI("========== Distributed Camera Demo Start ==========");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraDemoStateCallback>();

    sptr<CameraManager> cameraManager = CameraManager::GetInstance();
    cameraManager->SetCallback(std::make_shared<DemoDCameraManagerCallback>());
    sptr<CaptureSession> captureSession = cameraManager->CreateCaptureSession();
    captureSession->SetCallback(std::make_shared<DemoDCameraSessionCallback>(stateCallback));

    sptr<CameraInfo> cameraInfo = GetCameraInfo(cameraManager);
    sptr<CaptureInput> cameraInput = cameraManager->CreateCameraInput(cameraInfo);
    std::shared_ptr<DemoDCameraInputCallback> inputCallback = std::make_shared<DemoDCameraInputCallback>(stateCallback);
    ((sptr<CameraInput> &)cameraInput)->SetErrorCallback(inputCallback);
    ((sptr<CameraInput> &)cameraInput)->SetFocusCallback(inputCallback);

    std::shared_ptr<DCameraCaptureInfo> photoInfo = std::make_shared<DCameraCaptureInfo>();
    InitPhotoInfo(photoInfo);

    std::shared_ptr<DCameraCaptureInfo> previewInfo = std::make_shared<DCameraCaptureInfo>();
    InitPreviewInfo(previewInfo);

    std::shared_ptr<DCameraCaptureInfo> videoInfo = std::make_shared<DCameraCaptureInfo>();
    InitVideoInfo(videoInfo);

    SetPhotoOutput(cameraManager, photoInfo, stateCallback);

    std::shared_ptr<PhotoCaptureSetting> photoCaptureSettings = std::make_shared<PhotoCaptureSetting>();
    SetCaptureSettings(photoCaptureSettings);

    SetPreviewOutput(cameraManager, previewInfo, stateCallback);

    SetVideoOutput(cameraManager, videoInfo, stateCallback);

    captureSession->BeginConfig();
    captureSession->AddInput(cameraInput);
    captureSession->AddOutput(g_photoOutput);
    captureSession->AddOutput(g_previewOutput);
    captureSession->CommitConfig();
    captureSession->Start();
    sleep(SLEEP_FIVE_SECOND);

    SetFocusAndExposure(cameraInput);
    sleep(SLEEP_FIVE_SECOND);

    ((sptr<PhotoOutput> &)g_photoOutput)->Capture(photoCaptureSettings);
    sleep(SLEEP_TWENTY_SECOND);

    captureSession->Stop();
    captureSession->Release();
    cameraInput->Release();
    cameraManager->SetCallback(nullptr);

    DHLOGI("========== Distributed Camera Demo End ==========");
    return 0;
}