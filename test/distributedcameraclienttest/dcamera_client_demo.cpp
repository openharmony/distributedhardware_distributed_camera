/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

namespace OHOS {
namespace DistributedHardware {
constexpr double LOCATION_LATITUDE = 22.306;
constexpr double LOCATION_LONGITUDE = 52.12;
constexpr double LOCATION_ALTITUDE = 2.365;
#ifdef DCAMERA_COMMON
    constexpr int32_t PHOTO_WIDTH = 1280;
    constexpr int32_t PHOTO_HEIGTH = 960;
#else
    constexpr int32_t PHOTO_WIDTH = 1920;
    constexpr int32_t PHOTO_HEIGTH = 1080;
#endif
constexpr int32_t PREVIEW_WIDTH = 640;
constexpr int32_t PREVIEW_HEIGTH = 480;
constexpr int32_t VIDEO_WIDTH = 640;
constexpr int32_t VIDEO_HEIGTH = 480;

static sptr<CameraDevice> g_cameraInfo = nullptr;
static sptr<CameraManager> g_cameraManager = nullptr;
static sptr<CaptureInput> g_cameraInput = nullptr;
static sptr<CaptureOutput> g_photoOutput = nullptr;
static sptr<CaptureOutput> g_previewOutput = nullptr;
static sptr<CaptureOutput> g_videoOutput = nullptr;
static sptr<CaptureSession> g_captureSession = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_photoInfo = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_previewInfo = nullptr;
static std::shared_ptr<DCameraCaptureInfo> g_videoInfo = nullptr;

#ifdef DCAMERA_COMMON
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
#else
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
#endif

int32_t InitCameraStandard(CameraPosition position)
{
    g_cameraManager = CameraManager::GetInstance();
    g_cameraManager->SetCallback(std::make_shared<DemoDCameraManagerCallback>());

    int rv = g_cameraManager->CreateCaptureSession(&g_captureSession);
    if (rv != DCAMERA_OK) {
        DHLOGE("InitCameraStandard create captureSession failed, rv: %{public}d", rv);
        return rv;
    }
    std::shared_ptr<DemoDCameraSessionCallback> sessionCallback = std::make_shared<DemoDCameraSessionCallback>();
    g_captureSession->SetCallback(sessionCallback);
    g_captureSession->SetFocusCallback(sessionCallback);

    std::vector<sptr<CameraDevice>> cameraObjList = g_cameraManager->GetSupportedCameras();
    for (auto info : cameraObjList) {
        DHLOGI("Camera: %{public}s, position: %{public}d, camera type: %{public}d, connection type: %{public}d",
            GetAnonyString(info->GetID()).c_str(),
            info->GetPosition(), info->GetCameraType(), info->GetConnectionType());
        // CAMERA_POSITION_BACK or CAMERA_POSITION_FRONT
        if ((info->GetPosition() == position) &&
            (info->GetConnectionType() == ConnectionType::CAMERA_CONNECTION_REMOTE)) {
            g_cameraInfo = info;
            break;
        }
    }

    if (g_cameraInfo == nullptr) {
        DHLOGE("Distributed Camera Demo: have no remote camera");
        return DCAMERA_BAD_VALUE;
    }

    rv = g_cameraManager->CreateCameraInput(g_cameraInfo, &((sptr<CameraInput> &)g_cameraInput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitCameraStandard create cameraInput failed, rv: %{public}d", rv);
        return rv;
    }
    int32_t ret = ((sptr<CameraInput> &)g_cameraInput)->Open();
    if (ret != DCAMERA_OK) {
        DHLOGE("InitCameraStandard g_cameraInput Open failed, ret: %{public}d", ret);
        return ret;
    }
    std::shared_ptr<DemoDCameraInputCallback> inputCallback = std::make_shared<DemoDCameraInputCallback>();
    ((sptr<CameraInput> &)g_cameraInput)->SetErrorCallback(inputCallback);
    return DCAMERA_OK;
}

void InitCaptureInfo(int32_t width, int32_t height)
{
    g_photoInfo = std::make_shared<DCameraCaptureInfo>();
    g_photoInfo->width_ = PHOTO_WIDTH;
    g_photoInfo->height_ = PHOTO_HEIGTH;
    g_photoInfo->format_ = PHOTO_FORMAT;

    g_previewInfo = std::make_shared<DCameraCaptureInfo>();
    if (width == 0 && height == 0) {
        g_previewInfo->width_ = PREVIEW_WIDTH;
        g_previewInfo->height_ = PREVIEW_HEIGTH;
    } else {
        g_previewInfo->width_ = width;
        g_previewInfo->height_ = height;
    }
    g_previewInfo->format_ = PREVIEW_FORMAT;

    g_videoInfo = std::make_shared<DCameraCaptureInfo>();
    g_videoInfo->width_ = VIDEO_WIDTH;
    g_videoInfo->height_ = VIDEO_HEIGTH;
    g_videoInfo->format_ = VIDEO_FORMAT;
}

CameraFormat ConvertToCameraFormat(int32_t format)
{
    CameraFormat ret = CameraFormat::CAMERA_FORMAT_INVALID;
    DCameraFormat df = static_cast<DCameraFormat>(format);
    switch (df) {
        case DCameraFormat::OHOS_CAMERA_FORMAT_RGBA_8888:
            ret = CameraFormat::CAMERA_FORMAT_RGBA_8888;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_YCBCR_420_888:
            ret = CameraFormat::CAMERA_FORMAT_YCBCR_420_888;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_YCRCB_420_SP:
            ret = CameraFormat::CAMERA_FORMAT_YUV_420_SP;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_JPEG:
            ret = CameraFormat::CAMERA_FORMAT_JPEG;
            break;
        default:
            break;
    }
    return ret;
}

void InitPhotoOutput(void)
{
    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %{public}d, height = %{public}d, format = %{public}d",
        g_photoInfo->width_, g_photoInfo->height_, g_photoInfo->format_);
    sptr<IConsumerSurface> photoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> photoListener =
        OHOS::sptr<IBufferConsumerListener>(new DemoDCameraPhotoSurfaceListener(photoSurface));
    photoSurface->RegisterConsumerListener(photoListener);
    CameraFormat photoFormat = ConvertToCameraFormat(g_photoInfo->format_);
    Size photoSize = {g_photoInfo->width_, g_photoInfo->height_};
    Profile photoProfile(photoFormat, photoSize);
    sptr<IBufferProducer> photoProducer = photoSurface->GetProducer();
    int rv = g_cameraManager->CreatePhotoOutput(photoProfile, photoProducer, &((sptr<PhotoOutput> &)g_photoOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create photoOutput failed, rv: %{public}d", rv);
        return;
    }
    ((sptr<PhotoOutput> &)g_photoOutput)->SetCallback(std::make_shared<DemoDCameraPhotoCallback>());
}

void InitPreviewOutput(void)
{
    DHLOGI("Distributed Camera Demo: Create PreviewOutput, width = %{public}d, height = %{public}d, format = "
        "%{public}d", g_previewInfo->width_, g_previewInfo->height_, g_previewInfo->format_);
    sptr<IConsumerSurface> previewSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> previewListener =
        OHOS::sptr<IBufferConsumerListener>(new DemoDCameraPreviewSurfaceListener(previewSurface));
    previewSurface->RegisterConsumerListener(previewListener);
    CameraFormat previewFormat = ConvertToCameraFormat(g_previewInfo->format_);
    Size previewSize = {g_previewInfo->width_, g_previewInfo->height_};
    Profile previewProfile(previewFormat, previewSize);
    sptr<IBufferProducer> previewProducer = previewSurface->GetProducer();
    sptr<Surface> previewProducerSurface = Surface::CreateSurfaceAsProducer(previewProducer);
    int rv = g_cameraManager->CreatePreviewOutput(
        previewProfile, previewProducerSurface, &((sptr<PreviewOutput> &)g_previewOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create previewOutput failed, rv: %{public}d", rv);
        return;
    }
    ((sptr<PreviewOutput> &)g_previewOutput)->SetCallback(std::make_shared<DemoDCameraPreviewCallback>());
}

void InitVideoOutput(void)
{
    DHLOGI("Distributed Camera Demo: Create VideoOutput, width = %{public}d, height = %{public}d, format = %{public}d",
        g_videoInfo->width_, g_videoInfo->height_, g_videoInfo->format_);
    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> videoListener =
        OHOS::sptr<IBufferConsumerListener>(new DemoDCameraVideoSurfaceListener(videoSurface));
    videoSurface->RegisterConsumerListener(videoListener);
    CameraFormat videoFormat = ConvertToCameraFormat(g_videoInfo->format_);
    Size videoSize = {g_videoInfo->width_, g_videoInfo->height_};
    std::vector<int32_t> framerates = {};
    VideoProfile videoSettings(videoFormat, videoSize, framerates);
    sptr<IBufferProducer> videoProducer = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(videoProducer);
    int rv = g_cameraManager->CreateVideoOutput(videoSettings, pSurface, &((sptr<VideoOutput> &)g_videoOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create videoOutput failed, rv: %{public}d", rv);
        return;
    }
    ((sptr<VideoOutput> &)g_videoOutput)->SetCallback(std::make_shared<DemoDCameraVideoCallback>());
}

void ConfigCaptureSession(void)
{
    g_captureSession->BeginConfig();
    g_captureSession->AddInput(g_cameraInput);
    g_captureSession->AddOutput(g_previewOutput);
    g_captureSession->AddOutput(g_videoOutput);
    g_captureSession->AddOutput(g_photoOutput);
    g_captureSession->CommitConfig();

    std::vector<VideoStabilizationMode> stabilizationModes;
    int32_t rv = g_captureSession->GetSupportedStabilizationMode(stabilizationModes);
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession get supported stabilization mode failed, rv: %{public}d", rv);
        return;
    }
    if (!stabilizationModes.empty()) {
        for (auto mode : stabilizationModes) {
            DHLOGI("Distributed Camera Demo: video stabilization mode %{public}d", mode);
        }
        g_captureSession->SetVideoStabilizationMode(stabilizationModes.back());
    }
    g_captureSession->Start();
}

void ConfigFocusFlashAndExposure(bool isVideo)
{
    g_captureSession->LockForControl();
    FocusMode focusMode = FOCUS_MODE_CONTINUOUS_AUTO;
    ExposureMode exposureMode = EXPOSURE_MODE_AUTO;
    float exposureValue = 0;
    std::vector<float> biasRange;
    int32_t rv = g_captureSession->GetExposureBiasRange(biasRange);
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigFocusAndExposure get exposure bias range failed, rv: %{public}d", rv);
        return;
    }
    if (!biasRange.empty()) {
        DHLOGI("Distributed Camera Demo: get %{public}d exposure compensation range", biasRange.size());
        exposureValue = biasRange[0];
    }
    FlashMode flash = FLASH_MODE_OPEN;
    if (isVideo) {
        flash = FLASH_MODE_ALWAYS_OPEN;
    }
    g_captureSession->SetFlashMode(flash);
    g_captureSession->SetFocusMode(focusMode);
    g_captureSession->SetExposureMode(exposureMode);
    g_captureSession->SetExposureBias(exposureValue);
    g_captureSession->UnlockForControl();
}

std::shared_ptr<PhotoCaptureSetting> ConfigPhotoCaptureSetting()
{
    std::shared_ptr<PhotoCaptureSetting> photoCaptureSettings = std::make_shared<PhotoCaptureSetting>();
    // Rotation
    PhotoCaptureSetting::RotationConfig rotation = PhotoCaptureSetting::RotationConfig::Rotation_0;
    photoCaptureSettings->SetRotation(rotation);
    // QualityLevel
    PhotoCaptureSetting::QualityLevel quality = PhotoCaptureSetting::QualityLevel::QUALITY_LEVEL_HIGH;
    photoCaptureSettings->SetQuality(quality);
    // Location
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->latitude = LOCATION_LATITUDE;
    location->longitude = LOCATION_LONGITUDE;
    location->altitude = LOCATION_ALTITUDE;
    photoCaptureSettings->SetLocation(location);
    return photoCaptureSettings;
}

void ReleaseResource(void)
{
    if (g_previewOutput != nullptr) {
        ((sptr<CameraStandard::PreviewOutput> &)g_previewOutput)->Stop();
        g_previewOutput->Release();
        g_previewOutput = nullptr;
    }
    if (g_photoOutput != nullptr) {
        g_photoOutput->Release();
        g_photoOutput = nullptr;
    }
    if (g_videoOutput != nullptr) {
        g_videoOutput->Release();
        g_videoOutput = nullptr;
    }
    if (g_cameraInput != nullptr) {
        g_cameraInput->Close();
        g_cameraInput->Release();
        g_cameraInput = nullptr;
    }
    if (g_captureSession != nullptr) {
        g_captureSession->Stop();
        g_captureSession->Release();
        g_captureSession = nullptr;
    }
    if (g_cameraManager != nullptr) {
        g_cameraManager->SetCallback(nullptr);
    }
    g_cameraInfo = nullptr;
    g_cameraManager = nullptr;
}

int32_t Capture()
{
    int32_t ret = ((sptr<PhotoOutput> &)g_photoOutput)->Capture(ConfigPhotoCaptureSetting());
    if (ret != DCAMERA_OK) {
        DHLOGE("main g_photoOutput Capture failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t Video()
{
    int32_t ret = ((sptr<VideoOutput> &)g_videoOutput)->Start();
    if (ret != DCAMERA_OK) {
        DHLOGE("main VideoOutput Start failed, ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t GetPreviewProfiles(std::vector<CameraStandard::Size> &previewResolution)
{
    sptr<CameraOutputCapability> capability = g_cameraManager->GetSupportedOutputCapability(g_cameraInfo);
    if (capability == nullptr) {
        DHLOGI("get supported capability is null");
        return DCAMERA_BAD_VALUE;
    }
    std::vector<CameraStandard::Profile> previewProfiles = capability->GetPreviewProfiles();
    DHLOGI("size: %{public}d", previewProfiles.size());
    for (auto& profile : previewProfiles) {
        CameraStandard::Size picSize = profile.GetSize();
        DHLOGI("width: %{public}d, height: %{public}d", picSize.width, picSize.height);
        if (IsValid(picSize)) {
            previewResolution.push_back(picSize);
        }
    }
    return DCAMERA_OK;
}

bool IsValid(const CameraStandard::Size& size)
{
    return (size.width >= RESOLUTION_MIN_WIDTH) && (size.height >= RESOLUTION_MIN_HEIGHT) &&
        (size.width <= RESOLUTION_MAX_WIDTH_CONTINUOUS) && (size.height <= RESOLUTION_MAX_HEIGHT_CONTINUOUS);
}
} // namespace DistributedHardware
} // namespace OHOS
