/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "access_token.h"
#include "accesstoken_kit.h"
#include "hap_token_info.h"
#include "ipc_skeleton.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace OHOS;
using namespace OHOS::Camera;
using namespace OHOS::CameraStandard;
using namespace OHOS::DistributedHardware;

constexpr double LATITUDE = 22.306;
constexpr double LONGITUDE = 52.12;
constexpr double ALTITUDE = 2.365;
constexpr int32_t PHOTO_WIDTH = 1280;
constexpr int32_t PHOTO_HEIGTH = 960;
constexpr int32_t PREVIEW_WIDTH = 640;
constexpr int32_t PREVIEW_HEIGTH = 480;
constexpr int32_t VIDEO_WIDTH = 640;
constexpr int32_t VIDEO_HEIGTH = 480;
constexpr int32_t SLEEP_FIVE_SECOND = 5;
constexpr int32_t SLEEP_TWENTY_SECOND = 20;

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

#ifdef DCAMERA_YUV
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
#else
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
#endif

static int32_t InitCameraStandard()
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
        // OHOS_CAMERA_POSITION_FRONT or OHOS_CAMERA_POSITION_BACK
        if ((info->GetPosition() == CameraPosition::CAMERA_POSITION_FRONT) &&
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

static CameraFormat ConvertToCameraFormat(int32_t format)
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

static void InitPhotoOutput()
{
    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %{public}d, height = %{public}d, format = %{public}d",
        g_photoInfo->width_, g_photoInfo->height_, g_photoInfo->format_);
    sptr<IConsumerSurface> photoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> photoListener(new DemoDCameraPhotoSurfaceListener(photoSurface));
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

static void InitPreviewOutput()
{
    DHLOGI("Distributed Camera Demo: Create PreviewOutput, width = %{public}d, height = %{public}d, format = "
        "%{public}d", g_previewInfo->width_, g_previewInfo->height_, g_previewInfo->format_);
    sptr<IConsumerSurface> previewSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> previewListener(new DemoDCameraPreviewSurfaceListener(previewSurface));
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

static void InitVideoOutput()
{
    DHLOGI("Distributed Camera Demo: Create VideoOutput, width = %{public}d, height = %{public}d, format = %{public}d",
        g_videoInfo->width_, g_videoInfo->height_, g_videoInfo->format_);
    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> videoListener(new DemoDCameraVideoSurfaceListener(videoSurface));
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

static void ConfigCaptureSession()
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
}

static void ConfigFocusAndExposure()
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
        DHLOGI("Distributed Camera Demo: get %{public}zu exposure compensation range", biasRange.size());
        exposureValue = biasRange[0];
    }
    g_captureSession->SetFocusMode(focusMode);
    g_captureSession->SetExposureMode(exposureMode);
    g_captureSession->SetExposureBias(exposureValue);
    g_captureSession->UnlockForControl();
}

static std::shared_ptr<PhotoCaptureSetting> ConfigPhotoCaptureSetting()
{
    std::shared_ptr<PhotoCaptureSetting> photoCaptureSettings = std::make_shared<PhotoCaptureSetting>();
    // Rotation
    PhotoCaptureSetting::RotationConfig rotation = PhotoCaptureSetting::RotationConfig::Rotation_0;
    photoCaptureSettings->SetRotation(rotation);
    // QualityLevel
    PhotoCaptureSetting::QualityLevel quality = PhotoCaptureSetting::QualityLevel::QUALITY_LEVEL_HIGH;
    photoCaptureSettings->SetQuality(quality);
    // Location
    auto location = std::make_shared<Location>();
    location->latitude = LATITUDE;
    location->longitude = LONGITUDE;
    location->altitude = ALTITUDE;
    photoCaptureSettings->SetLocation(location);
    return photoCaptureSettings;
}

static void SetPermissions()
{
    uint64_t tokenId;
    const char *perms[2];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.CAMERA";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dcamera_client_demo",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

int main()
{
    DHLOGI("========== Distributed Camera Demo Start ==========");
    SetPermissions();
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

    if (((sptr<VideoOutput> &)g_videoOutput)->Start() != DCAMERA_OK) {
        DHLOGE("main g_videoOutput Start failed");
    }
    sleep(SLEEP_FIVE_SECOND);

    ConfigFocusAndExposure();
    sleep(SLEEP_FIVE_SECOND);

    ((sptr<PhotoOutput> &)g_photoOutput)->Capture(ConfigPhotoCaptureSetting());
    sleep(SLEEP_TWENTY_SECOND);

    if (((sptr<VideoOutput> &)g_videoOutput)->Stop() != DCAMERA_OK) {
        DHLOGE("main g_videoOutput Stop failed");
    }
    sleep(SLEEP_FIVE_SECOND);

    g_captureSession->Stop();
    if (g_cameraInput->Close() != DCAMERA_OK) {
        DHLOGE("main g_cameraInput Close failed");
    }
    g_captureSession->Release();
    if (g_cameraInput->Release() != DCAMERA_OK) {
        DHLOGE("main g_cameraInput Close failed");
    }
    DHLOGI("========== Distributed Camera Demo End ==========");
    return 0;
}