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

#include "access_token.h"
#include "accesstoken_kit.h"
#include "hap_token_info.h"
#include "ipc_skeleton.h"
#include "token_setproc.h"

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
    constexpr int32_t PHOTO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t PREVIEW_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
    constexpr int32_t VIDEO_FORMAT = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
#endif

static int32_t InitCameraStandard()
{
    g_cameraManager = CameraManager::GetInstance();
    g_cameraManager->SetCallback(std::make_shared<DemoDCameraManagerCallback>());

    int rv = g_cameraManager->CreateCaptureSession(&g_captureSession);
    if (rv != DCAMERA_OK) {
        DHLOGE("InitCameraStandard create captureSession failed, rv: %d", rv);
        return rv;
    }
    g_captureSession->SetCallback(std::make_shared<DemoDCameraSessionCallback>());

    std::vector<sptr<CameraDevice>> cameraObjList = g_cameraManager->GetSupportedCameras();
    for (auto info : cameraObjList) {
        DHLOGI("Camera: %s, position: %d, camera type: %d, connection type: %d", GetAnonyString(info->GetID()).c_str(),
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
        DHLOGE("InitCameraStandard create cameraInput failed, rv: %d", rv);
        return rv;
    }
    int32_t ret = ((sptr<CameraInput> &)g_cameraInput)->Open();
    if (ret != DCAMERA_OK) {
        DHLOGE("InitCameraStandard g_cameraInput Open failed, ret: %d", ret);
        return ret;
    }
    std::shared_ptr<DemoDCameraInputCallback> inputCallback = std::make_shared<DemoDCameraInputCallback>();
    ((sptr<CameraInput> &)g_cameraInput)->SetErrorCallback(inputCallback);
    g_captureSession->SetFocusCallback(std::make_shared<DemoDCameraSessionCallback>());
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
    DHLOGI("Distributed Camera Demo: Create PhotoOutput, width = %d, height = %d, format = %d",
        g_photoInfo->width_, g_photoInfo->height_, g_photoInfo->format_);
    sptr<Surface> photoSurface = Surface::CreateSurfaceAsConsumer();
    sptr<IBufferConsumerListener> photoListener = new DemoDCameraPhotoSurfaceListener(photoSurface);
    photoSurface->SetDefaultWidthAndHeight(g_photoInfo->width_, g_photoInfo->height_);
    photoSurface->SetUserData(CAMERA_SURFACE_FORMAT, std::to_string(g_photoInfo->format_));
    photoSurface->RegisterConsumerListener(photoListener);
    CameraFormat photoFormat = ConvertToCameraFormat(g_photoInfo->format_);
    Size photoSize = {g_photoInfo->width_, g_photoInfo->height_};
    Profile photoProfile(photoFormat, photoSize);
    int rv = g_cameraManager->CreatePhotoOutput(photoProfile, photoSurface, &((sptr<PhotoOutput> &)g_photoOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create photoOutput failed, rv: %d", rv);
        return;
    }
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
    CameraFormat previewFormat = ConvertToCameraFormat(g_previewInfo->format_);
    Size previewSize = {g_previewInfo->width_, g_previewInfo->height_};
    Profile previewProfile(previewFormat, previewSize);
    int rv = g_cameraManager->CreatePreviewOutput(
        previewProfile, previewSurface, &((sptr<PreviewOutput> &)g_previewOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create previewOutput failed, rv: %d", rv);
        return;
    }
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
    CameraFormat videoFormat = ConvertToCameraFormat(g_videoInfo->format_);
    Size videoSize = {g_videoInfo->width_, g_videoInfo->height_};
    std::vector<int32_t> framerates = {};
    VideoProfile videoSettings(videoFormat, videoSize, framerates);
    int rv = g_cameraManager->CreateVideoOutput(videoSettings, videoSurface, &((sptr<VideoOutput> &)g_videoOutput));
    if (rv != DCAMERA_OK) {
        DHLOGE("InitPhotoOutput create videoOutput failed, rv: %d", rv);
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
        DHLOGE("ConfigCaptureSession get supported stabilization mode failed, rv: %d", rv);
        return;
    }
    for (auto mode : stabilizationModes) {
        DHLOGI("Distributed Camera Demo: video stabilization mode %d", mode);
    }
    g_captureSession->SetVideoStabilizationMode(stabilizationModes.back());
}

static void ConfigFocusAndExposure()
{
    g_captureSession->LockForControl();
    FocusMode focusMode = FOCUS_MODE_CONTINUOUS_AUTO;
    ExposureMode exposureMode = EXPOSURE_MODE_AUTO;
    int32_t exposureValue = 0;
    std::vector<int32_t> biasRange;
    int32_t rv = g_captureSession->GetExposureBiasRange(biasRange);
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigFocusAndExposure get exposure bias range failed, rv: %d", rv);
        return;
    }
    if (!biasRange.empty()) {
        DHLOGI("Distributed Camera Demo: get %d exposure compensation range", biasRange.size());
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
    std::unique_ptr<Location> location = std::make_unique<Location>();
    location->latitude = LATITUDE;
    location->longitude = LONGITUDE;
    location->altitude = ALTITUDE;
    photoCaptureSettings->SetLocation(location);
    return photoCaptureSettings;
}

static std::string permissionName = "ohos.permission.CAMERA";
static OHOS::Security::AccessToken::HapInfoParams g_infoManagerTestInfoParms = {
    .userID = 1,
    .bundleName = permissionName,
    .instIndex = 0,
    .appIDDesc = "testtesttesttest"
};

static OHOS::Security::AccessToken::PermissionDef g_infoManagerTestPermDef1 = {
    .permissionName = "ohos.permission.CAMERA",
    .bundleName = "ohos.permission.CAMERA",
    .grantMode = 1,
    .availableLevel = OHOS::Security::AccessToken::ATokenAplEnum::APL_NORMAL,
    .label = "label",
    .labelId = 1,
    .description = "dcamera client test",
    .descriptionId = 1
};

static OHOS::Security::AccessToken::PermissionStateFull g_infoManagerTestState1 = {
    .permissionName = "ohos.permission.CAMERA",
    .isGeneral = true,
    .resDeviceID = {"local"},
    .grantStatus = {OHOS::Security::AccessToken::PermissionState::PERMISSION_GRANTED},
    .grantFlags = {1}
};

static OHOS::Security::AccessToken::HapPolicyParams g_infoManagerTestPolicyPrams = {
    .apl = OHOS::Security::AccessToken::ATokenAplEnum::APL_NORMAL,
    .domain = "test.domain",
    .permList = {g_infoManagerTestPermDef1},
    .permStateList = {g_infoManagerTestState1}
};

int main()
{
    /* Grant the permission so that create camera test can be success */
    int32_t rc = -1;
    OHOS::Security::AccessToken::AccessTokenIDEx tokenIdEx = {0};
    tokenIdEx = OHOS::Security::AccessToken::AccessTokenKit::AllocHapToken(
        g_infoManagerTestInfoParms,
        g_infoManagerTestPolicyPrams);
    if (tokenIdEx.tokenIdExStruct.tokenID == 0) {
        unsigned int tokenIdOld = 0;
        DHLOGI("Alloc TokenID failure, cleaning the old token ID \n");
        tokenIdOld = OHOS::Security::AccessToken::AccessTokenKit::GetHapTokenID(
            1, permissionName, 0);
        if (tokenIdOld == 0) {
            DHLOGI("Unable to get the Old Token ID, need to reflash the board");
            return 0;
        }
        rc = OHOS::Security::AccessToken::AccessTokenKit::DeleteToken(tokenIdOld);
        if (rc != 0) {
            DHLOGI("Unable to delete the Old Token ID, need to reflash the board");
            return 0;
        }

        /* Retry the token allocation again */
        tokenIdEx = OHOS::Security::AccessToken::AccessTokenKit::AllocHapToken(
            g_infoManagerTestInfoParms,
            g_infoManagerTestPolicyPrams);
        if (tokenIdEx.tokenIdExStruct.tokenID == 0) {
            DHLOGI("Alloc TokenID failure, need to reflash the board \n");
            return 0;
        }
    }

    (void)SetSelfTokenID(tokenIdEx.tokenIdExStruct.tokenID);

    rc = Security::AccessToken::AccessTokenKit::GrantPermission(
        tokenIdEx.tokenIdExStruct.tokenID,
        permissionName, OHOS::Security::AccessToken::PERMISSION_USER_FIXED);
    if (rc != 0) {
        DHLOGI("GrantPermission() failed");
        (void)OHOS::Security::AccessToken::AccessTokenKit::DeleteToken(
            tokenIdEx.tokenIdExStruct.tokenID);
        return 0;
    } else {
        DHLOGI("GrantPermission() success");
    }

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