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

constexpr double LATITUDE = 22.306;
constexpr double LONGITUDE = 52.12;
constexpr double ALTITUDE = 2.365;
constexpr int32_t PHOTO_WIDTH = 640;
constexpr int32_t PHOTO_HEIGTH = 480;
constexpr int32_t PREVIEW_WIDTH = 640;
constexpr int32_t PREVIEW_HEIGTH = 480;
constexpr int32_t VIDEO_WIDTH = 640;
constexpr int32_t VIDEO_HEIGTH = 480;
constexpr int32_t SLEEP_FIVE_SECOND = 5;
constexpr int32_t SLEEP_TWENTY_SECOND = 20;

static sptr<CameraInfo> g_cameraInfo = nullptr;
static sptr<CameraManager> g_cameraManager = nullptr;
static sptr<CaptureInput> g_cameraInput = nullptr;
static sptr<CaptureOutput> g_photoOutput = nullptr;
static sptr<CaptureOutput> g_previewOutput = nullptr;
static sptr<CaptureOutput> g_videoOutput = nullptr;
static sptr<CaptureSession> g_captureSession = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_photoInfo = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_previewInfo = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_videoInfo = nullptr;

#ifdef PRODUCT_M40
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
#else
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
#endif

static int32_t InitCameraStandard()
{
    g_cameraManager = CameraManager::GetInstance();
    g_cameraManager->SetCallback(std::make_shared<DemoDCameraManagerCallback>());

    g_captureSession = g_cameraManager->CreateCaptureSession();
    g_captureSession->SetCallback(std::make_shared<DemoDCameraSessionCallback>());

    std::vector<sptr<CameraInfo>> cameraObjList = g_cameraManager->GetCameras();
    for (auto info : cameraObjList) {
        DHLOGI("Camera: %s, position: %d, camera type: %d, connection type: %d", GetAnonyString(info->GetID()).c_str(),
            info->GetPosition(), info->GetCameraType(), info->GetConnectionType());
        // OHOS_CAMERA_POSITION_FRONT or OHOS_CAMERA_POSITION_BACK
        if ((info->GetPosition() == OHOS_CAMERA_POSITION_FRONT) &&
            (info->GetConnectionType() == OHOS_CAMERA_CONNECTION_TYPE_REMOTE)) {
            g_cameraInfo = info;
            break;
        }
    }

    if (g_cameraInfo == nullptr) {
        DHLOGE("Distributed Camera Demo: have no remote camera");
        return DCAMERA_BAD_VALUE;
    }

    g_cameraInput = g_cameraManager->CreateCameraInput(g_cameraInfo);
    std::shared_ptr<DemoDCameraInputCallback> inputCallback = std::make_shared<DemoDCameraInputCallback>();
    ((sptr<CameraInput> &)g_cameraInput)->SetErrorCallback(inputCallback);
    ((sptr<CameraInput> &)g_cameraInput)->SetFocusCallback(inputCallback);
    return DCAMERA_OK;
}

static void InitCaptureInfo()
{
    g_photoInfo = std::make_shared<DCameraCaptureInfo>();
    g_photoInfo->width_ = PHOTO_WIDTH;
    g_photoInfo->height_ = PHOTO_HEIGTH;
    g_photoInfo->format_ = PHOTO_FORMAT;

    g_previewInfo = std::make_shared<DCameraCaptureInfo>();
    g_previewInfo->width_ = PREVIEW_WIDTH;
    g_previewInfo->height_ = PREVIEW_HEIGTH;
    g_previewInfo->format_ = PREVIEW_FORMAT;

    g_videoInfo = std::make_shared<DCameraCaptureInfo>();
    g_videoInfo->width_ = VIDEO_WIDTH;
    g_videoInfo->height_ = VIDEO_HEIGTH;
    g_videoInfo->format_ = VIDEO_FORMAT;
}

static void InitPhotoOutput()
{
    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %d, height = %d, format = %d",
        g_photoInfo->width_, g_photoInfo->height_, g_photoInfo->format_);
    sptr<Surface> photoSurface = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> photoListener = new DemoDCameraPhotoSurfaceListener(photoSurface);
    photoSurface->SetDefaultWidthAndHeight(g_photoInfo->width_, g_photoInfo->height_);
    photoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(g_photoInfo->format_));
    photoSurface->RegisterConsumerListener(photoListener);
    g_photoOutput = g_cameraManager->CreatePhotoOutput(photoSurface);
    ((sptr<PhotoOutput> &)g_photoOutput)->SetCallback(std::make_shared<DemoDCameraPhotoCallback>());
}

static void InitPreviewOutput()
{
    DHLOGI("Distributed Camera Demo: Create PreviewOutput, width = %d, height = %d, format = %d",
        g_previewInfo->width_, g_previewInfo->height_, g_previewInfo->format_);
    sptr<Surface> previewSurface = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> previewListener = new DemoDCameraPreviewSurfaceListener(previewSurface);
    previewSurface->SetDefaultWidthAndHeight(g_previewInfo->width_, g_previewInfo->height_);
    previewSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(g_previewInfo->format_));
    previewSurface->RegisterConsumerListener(previewListener);
    g_previewOutput = g_cameraManager->CreatePreviewOutput(previewSurface);
    ((sptr<PreviewOutput> &)g_previewOutput)->SetCallback(std::make_shared<DemoDCameraPreviewCallback>());
}

static void InitVideoOutput()
{
    DHLOGI("Distributed Camera Demo: Create VideoOutput, width = %d, height = %d, format = %d",
        g_videoInfo->width_, g_videoInfo->height_, g_videoInfo->format_);
    sptr<Surface> videoSurface = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> videoListener = new DemoDCameraVideoSurfaceListener(videoSurface);
    videoSurface->SetDefaultWidthAndHeight(g_videoInfo->width_, g_videoInfo->height_);
    videoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(g_videoInfo->format_));
    videoSurface->RegisterConsumerListener(videoListener);
    g_videoOutput = g_cameraManager->CreateVideoOutput(videoSurface);
    ((sptr<VideoOutput> &)g_videoOutput)->SetCallback(std::make_shared<DemoDCameraVideoCallback>());
}

static void ConfigCaptureSession()
{
    g_captureSession->BeginConfig();
    g_captureSession->AddInput(g_cameraInput);
    g_captureSession->AddOutput(g_previewOutput);
    g_captureSession->AddOutput(g_videoOutput);
    g_captureSession->AddOutput(g_photoOutput);
    g_captureSession->CommitConfig();

    std::vector<VideoStabilizationMode> stabilizationModes = g_captureSession->GetSupportedStabilizationMode();
    for (auto mode : stabilizationModes) {
        DHLOGI("Distributed Camera Demo: video stabilization mode %d", mode);
    }
    g_captureSession->SetVideoStabilizationMode(stabilizationModes.back());
}

static void ConfigFocusAndExposure()
{
    ((sptr<CameraInput> &)g_cameraInput)->LockForControl();
    camera_focus_mode_enum_t focusMode = OHOS_CAMERA_FOCUS_MODE_AUTO;
    camera_exposure_mode_enum_t exposureMode = OHOS_CAMERA_EXPOSURE_MODE_AUTO;
    int32_t exposureValue = 0;
    std::vector<int32_t> biasRange = ((sptr<CameraInput> &)g_cameraInput)->GetExposureBiasRange();
    if (!biasRange.empty()) {
        DHLOGI("Distributed Camera Demo: get %d exposure compensation range", biasRange.size());
        exposureValue = biasRange[0];
    }
    ((sptr<CameraInput> &)g_cameraInput)->SetFocusMode(focusMode);
    ((sptr<CameraInput> &)g_cameraInput)->SetExposureMode(exposureMode);
    ((sptr<CameraInput> &)g_cameraInput)->SetExposureBias(exposureValue);
    ((sptr<CameraInput> &)g_cameraInput)->UnlockForControl();
}

static std::shared_ptr<PhotoCaptureSetting> ConfigPhotoCaptureSetting()
{
    std::shared_ptr<PhotoCaptureSetting> photoCaptureSettings = std::make_shared<PhotoCaptureSetting>();
    // Rotation
    PhotoCaptureSetting::RotationConfig rotation = PhotoCaptureSetting::RotationConfig::Rotation_0;
    photoCaptureSettings->SetRotation(rotation);
    // QualityLevel
    PhotoCaptureSetting::QualityLevel quality = PhotoCaptureSetting::QualityLevel::HIGH_QUALITY;
    photoCaptureSettings->SetQuality(quality);
    // Location
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->latitude = LATITUDE;
    location->longitude = LONGITUDE;
    location->altitude = ALTITUDE;
    photoCaptureSettings->SetLocation(location);
    return photoCaptureSettings;
}

int main()
{
    DHLOGI("========== Distributed Camera Demo Start ==========");
    int32_t ret = InitCameraStandard();
    if (ret != DCAMERA_OK) {
        std::cout << "have no remote camera" << std::endl;
        return 0;
    }

    InitCaptureInfo();
    InitPhotoOutput();
    InitPreviewOutput();
    InitVideoOutput();
    ConfigCaptureSession();

    g_captureSession->Start();
    sleep(SLEEP_FIVE_SECOND);

    ((sptr<VideoOutput> &)g_videoOutput)->Start();
    sleep(SLEEP_FIVE_SECOND);

    ConfigFocusAndExposure();
    sleep(SLEEP_FIVE_SECOND);

    ((sptr<PhotoOutput> &)g_photoOutput)->Capture(ConfigPhotoCaptureSetting());
    sleep(SLEEP_TWENTY_SECOND);

    ((sptr<VideoOutput> &)g_videoOutput)->Stop();
    sleep(SLEEP_FIVE_SECOND);

    g_captureSession->Stop();
    g_captureSession->Release();
    g_cameraInput->Release();
    DHLOGI("========== Distributed Camera Demo End ==========");
    return 0;
}