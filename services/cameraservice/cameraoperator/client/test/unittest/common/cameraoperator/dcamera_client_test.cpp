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

#include <gtest/gtest.h>

#define private public
#include "dcamera_client.h"
#undef private

#include "accesstoken_kit.h"
#include "anonymous_string.h"
#include "camera_metadata_operator.h"
#include "camera_util.h"
#include "dcamera_handler.h"
#include "dcamera_input_callback.h"
#include "dcamera_manager_callback.h"
#include "dcamera_photo_callback.h"
#include "dcamera_preview_callback.h"
#include "dcamera_session_callback.h"
#include "dcamera_utils_tools.h"
#include "dcamera_video_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraClientTestStateCallback : public StateCallback {
public:
    void OnStateChanged(std::shared_ptr<DCameraEvent>& event)
    {
        DHLOGI("DCameraClientTestStateCallback::OnStateChanged type: %d, result: %d",
               event->eventType_, event->eventResult_);
    }

    void OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        DHLOGI("DCameraClientTestStateCallback::OnMetadataResult");
    }
};

class DCameraClientTestResultCallback : public ResultCallback {
public:
    void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer)
    {
        DHLOGI("DCameraClientTestResultCallback::OnPhotoResult");
    }

    void OnVideoResult(std::shared_ptr<DataBuffer>& buffer)
    {
        DHLOGI("DCameraClientTestResultCallback::OnVideoResult");
    }
};

#ifdef DCAMERA_YUV
        const int32_t TEST_WIDTH = 1920;
        const int32_t TEST_HEIGHT = 1080;
        const int32_t TEST_FORMAT_VIDEO = OHOS_CAMERA_FORMAT_YCRCB_420_SP;
        const int32_t TEST_FORMAT_PHOTO = OHOS_CAMERA_FORMAT_JPEG;
#else
        const int32_t TEST_WIDTH = 640;
        const int32_t TEST_HEIGHT = 480;
        const int32_t TEST_FORMAT_VIDEO = OHOS_CAMERA_FORMAT_RGBA_8888;
        const int32_t TEST_FORMAT_PHOTO = OHOS_CAMERA_FORMAT_RGBA_8888;
#endif

constexpr int32_t TEST_SLEEP_SEC = 2;
constexpr uint32_t ENTRY_CAPACITY = 30;
constexpr uint32_t DATA_CAPACITY = 2000;
constexpr double LATITUDE = 22.306;
constexpr double LONGITUDE = 52.12;
constexpr double ALTITUDE = 2.365;

class DCameraClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();

    std::shared_ptr<DCameraClient> client_;
    std::shared_ptr<DCameraCaptureInfo> photoInfo_false_;
    std::shared_ptr<DCameraCaptureInfo> videoInfo_false_;
    std::shared_ptr<DCameraCaptureInfo> photoInfo_true_;
    std::shared_ptr<DCameraCaptureInfo> videoInfo_true_;
};

void DCameraClientTest::SetUpTestCase(void)
{
    DHLOGI("DCameraClientTest::SetUpTestCase");
}

void DCameraClientTest::TearDownTestCase(void)
{
    DHLOGI("DCameraClientTest::TearDownTestCase");
}

void DCameraClientTest::SetUp(void)
{
    DHLOGI("DCameraClientTest::SetUp");
    DCameraHandler::GetInstance().Initialize();
    std::vector<std::string> cameras = DCameraHandler::GetInstance().GetCameras();
    client_ = std::make_shared<DCameraClient>(cameras[0]);

    photoInfo_false_ = std::make_shared<DCameraCaptureInfo>();
    photoInfo_false_->width_ = TEST_WIDTH;
    photoInfo_false_->height_ = TEST_HEIGHT;
    photoInfo_false_->format_ = TEST_FORMAT_PHOTO;
    photoInfo_false_->isCapture_ = false;
    photoInfo_false_->streamType_ = SNAPSHOT_FRAME;

    videoInfo_false_ = std::make_shared<DCameraCaptureInfo>();
    videoInfo_false_->width_ = TEST_WIDTH;
    videoInfo_false_->height_ = TEST_HEIGHT;
    videoInfo_false_->format_ = TEST_FORMAT_VIDEO;
    videoInfo_false_->isCapture_ = false;
    videoInfo_false_->streamType_ = CONTINUOUS_FRAME;

    photoInfo_true_ = std::make_shared<DCameraCaptureInfo>();
    photoInfo_true_->width_ = TEST_WIDTH;
    photoInfo_true_->height_ = TEST_HEIGHT;
    photoInfo_true_->format_ = TEST_FORMAT_PHOTO;
    photoInfo_true_->isCapture_ = true;
    photoInfo_true_->streamType_ = SNAPSHOT_FRAME;

    videoInfo_true_ = std::make_shared<DCameraCaptureInfo>();
    videoInfo_true_->width_ = TEST_WIDTH;
    videoInfo_true_->height_ = TEST_HEIGHT;
    videoInfo_true_->format_ = TEST_FORMAT_VIDEO;
    videoInfo_true_->isCapture_ = true;
    videoInfo_true_->streamType_ = CONTINUOUS_FRAME;
}

void DCameraClientTest::TearDown(void)
{
    DHLOGI("DCameraClientTest::TearDown");
    client_ = nullptr;
    photoInfo_false_ = nullptr;
    videoInfo_false_ = nullptr;
    photoInfo_true_ = nullptr;
    videoInfo_true_ = nullptr;
}

void DCameraClientTest::SetTokenID()
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

/**
 * @tc.name: dcamera_client_test_001
 * @tc.desc: Verify SetStateCallback
 * @tc.type: FUNC
 * @tc.require: AR000GK6MN
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_001, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_001: test set state callback");
    auto managerCallback = std::make_shared<DCameraManagerCallback>();
    CameraStandard::CameraStatusInfo cameraStatus;
    managerCallback->OnCameraStatusChanged(cameraStatus);
    managerCallback->OnFlashlightStatusChanged("", CameraStandard::FlashStatus::FLASH_STATUS_OFF);

    auto inputCallback = std::make_shared<DCameraInputCallback>(nullptr);
    inputCallback->OnError(CameraStandard::CamServiceError::CAMERA_DEVICE_BUSY, 0);

    auto sessionCallback = std::make_shared<DCameraSessionCallback>(nullptr);
    sessionCallback->OnError(0);
    sessionCallback->OnFocusState(CameraStandard::FocusCallback::UNFOCUSED);

    auto photoCallback = std::make_shared<DCameraPhotoCallback>(nullptr);
    photoCallback->OnCaptureStarted(0);
    photoCallback->OnCaptureEnded(0, 0);
    photoCallback->OnFrameShutter(0, 0);
    photoCallback->OnCaptureError(0, 0);

    auto previewCallback = std::make_shared<DCameraPreviewCallback>(nullptr);
    previewCallback->OnFrameStarted();
    previewCallback->OnFrameEnded(0);
    previewCallback->OnError(0);

    auto videoCallback = std::make_shared<DCameraVideoCallback>(nullptr);
    videoCallback->OnFrameStarted();
    videoCallback->OnFrameEnded(0);
    videoCallback->OnError(0);

    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    inputCallback = std::make_shared<DCameraInputCallback>(stateCallback);
    inputCallback->OnError(CameraStandard::CamServiceError::CAMERA_DEVICE_BUSY, 0);
    inputCallback->OnError(CameraStandard::CamServiceError::CAMERA_DEVICE_PREEMPTED, 0);

    sessionCallback = std::make_shared<DCameraSessionCallback>(stateCallback);
    sessionCallback->OnError(0);
    sessionCallback->OnFocusState(CameraStandard::FocusCallback::UNFOCUSED);

    photoCallback = std::make_shared<DCameraPhotoCallback>(stateCallback);
    photoCallback->OnCaptureError(0, 0);

    previewCallback = std::make_shared<DCameraPreviewCallback>(stateCallback);
    previewCallback->OnError(0);

    videoCallback = std::make_shared<DCameraVideoCallback>(stateCallback);
    videoCallback->OnError(0);

    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    stateCallback = nullptr;
    ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_002
 * @tc.desc: Verify SetResultCallback
 * @tc.type: FUNC
 * @tc.require: AR000GK6MN
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_002, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_002: test set result callback");
    auto photoSurfaceListener = std::make_shared<DCameraPhotoSurfaceListener>(nullptr, nullptr);
    photoSurfaceListener->OnBufferAvailable();

    auto videoSurfaceListener = std::make_shared<DCameraVideoSurfaceListener>(nullptr, nullptr);
    videoSurfaceListener->OnBufferAvailable();

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    photoSurfaceListener = std::make_shared<DCameraPhotoSurfaceListener>(nullptr, resultCallback);
    photoSurfaceListener->OnBufferAvailable();

    videoSurfaceListener = std::make_shared<DCameraVideoSurfaceListener>(nullptr, resultCallback);
    videoSurfaceListener->OnBufferAvailable();

    int32_t ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    resultCallback = nullptr;
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_003
 * @tc.desc: Verify Init and UnInit
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_003, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_003: test init and release client");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_004
 * @tc.desc: Verify StartCapture and StopCapture
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_004, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_004: test startCapture and stopCapture");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_004: video width: %d, height: %d, format: %d, isCapture: %d",
        videoInfo_true_->width_, videoInfo_true_->height_, videoInfo_true_->format_, videoInfo_true_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    ret = client_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);

    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    settings.push_back(setting);
    ret = client_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);

    sleep(TEST_SLEEP_SEC);

    ret = client_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_005
 * @tc.desc: Verify StartCapture and StopCapture
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_005, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_005: test startCapture and stopCapture");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_005: video width: %d, height: %d, format: %d, isCapture: %d",
        videoInfo_true_->width_, videoInfo_true_->height_, videoInfo_true_->format_, videoInfo_true_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_005: photo width: %d, height: %d, format: %d, isCapture: %d",
        photoInfo_false_->width_, photoInfo_false_->height_, photoInfo_false_->format_, photoInfo_false_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    captureInfos.push_back(photoInfo_false_);
    ret = client_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);

    sleep(TEST_SLEEP_SEC);

    DHLOGI("DCameraClientTest dcamera_client_test_005: video width: %d, height: %d, format: %d, isCapture: %d",
        videoInfo_false_->width_, videoInfo_false_->height_, videoInfo_false_->format_, videoInfo_false_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_005: photo width: %d, height: %d, format: %d, isCapture: %d",
        photoInfo_true_->width_, photoInfo_true_->height_, photoInfo_true_->format_, photoInfo_true_->isCapture_);
    captureInfos.clear();
    captureInfos.push_back(videoInfo_false_);
    captureInfos.push_back(photoInfo_true_);
    ret = client_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);

    sleep(TEST_SLEEP_SEC);
    ret = client_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_006
 * @tc.desc: Verify UpdateSettings
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_006, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_006: test updateSettings");
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = ENABLE_METADATA;
    setting->value_ = "";
    settings.push_back(setting);
    int32_t ret = client_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_007
 * @tc.desc: Verify UpdateSettings
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_007, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_007: test updateSettings");
    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    uint8_t afMode = OHOS_CAMERA_FOCUS_MODE_AUTO;
    uint8_t aeMode = OHOS_CAMERA_EXPOSURE_MODE_CONTINUOUS_AUTO;
    uint8_t stabilizationMode = OHOS_CAMERA_VIDEO_STABILIZATION_OFF;
    metaData->addEntry(OHOS_CONTROL_FOCUS_MODE, &afMode, sizeof(afMode));
    metaData->addEntry(OHOS_CONTROL_EXPOSURE_MODE, &aeMode, sizeof(aeMode));
    metaData->addEntry(OHOS_CONTROL_VIDEO_STABILIZATION_MODE, &stabilizationMode, sizeof(stabilizationMode));
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);

    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    settings.push_back(setting);
    int32_t ret = client_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_008
 * @tc.desc: Verify UpdateSettings
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_008, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_008: test updateSettings");
    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    settings.push_back(setting);
    int32_t ret = client_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_009
 * @tc.desc: Verify CameraServiceErrorType
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_009, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_009: test cameraServiceErrorType");
    int32_t ret = client_->CameraServiceErrorType(CameraStandard::CamServiceError::CAMERA_OK);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->CameraServiceErrorType(CameraStandard::CamServiceError::CAMERA_ALLOC_ERROR);
    EXPECT_EQ(DCAMERA_ALLOC_ERROR, ret);

    ret = client_->CameraServiceErrorType(CameraStandard::CamServiceError::CAMERA_DEVICE_BUSY);
    EXPECT_EQ(DCAMERA_DEVICE_BUSY, ret);
}

/**
 * @tc.name: dcamera_client_test_010
 * @tc.desc: Verify CreateCaptureOutput
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_010, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_010: test createCaptureOutput");
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    int32_t ret = client_->CreateCaptureOutput(captureInfos);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_011
 * @tc.desc: Verify ConvertToCameraFormat
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_011, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_011: test convertToCameraFormat");
    CameraStandard::CameraFormat ret = client_->ConvertToCameraFormat(OHOS_CAMERA_FORMAT_INVALID);
    EXPECT_EQ(CameraStandard::CameraFormat::CAMERA_FORMAT_INVALID, ret);

    ret = client_->ConvertToCameraFormat(OHOS_CAMERA_FORMAT_RGBA_8888);
    EXPECT_EQ(CameraStandard::CameraFormat::CAMERA_FORMAT_RGBA_8888, ret);

    ret = client_->ConvertToCameraFormat(OHOS_CAMERA_FORMAT_YCBCR_420_888);
    EXPECT_EQ(CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_420_888, ret);

    ret = client_->ConvertToCameraFormat(OHOS_CAMERA_FORMAT_YCRCB_420_SP);
    EXPECT_EQ(CameraStandard::CameraFormat::CAMERA_FORMAT_YUV_420_SP, ret);

    ret = client_->ConvertToCameraFormat(OHOS_CAMERA_FORMAT_JPEG);
    EXPECT_EQ(CameraStandard::CameraFormat::CAMERA_FORMAT_JPEG, ret);
}

/**
 * @tc.name: dcamera_client_test_012
 * @tc.desc: Verify StartPhotoOutput and StartVideoOutput
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_012, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_012: test startOutput");
    auto info = std::make_shared<DCameraCaptureInfo>();
    int32_t ret = client_->StartPhotoOutput(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = client_->StartVideoOutput();
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_013
 * @tc.desc: Verify StartPhotoOutput
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_013, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_013: test startPhotoOutput");
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_013: video width: %d, height: %d, format: %d, isCapture: %d",
        videoInfo_true_->width_, videoInfo_true_->height_, videoInfo_true_->format_, videoInfo_true_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_013: photo width: %d, height: %d, format: %d, isCapture: %d",
        photoInfo_false_->width_, photoInfo_false_->height_, photoInfo_false_->format_, photoInfo_false_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    captureInfos.push_back(photoInfo_false_);
    ret = client_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);

    sleep(TEST_SLEEP_SEC);

    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    int32_t orientation = OHOS_CAMERA_JPEG_ROTATION_180;
    metaData->addEntry(OHOS_JPEG_ORIENTATION, &orientation, sizeof(orientation));
    uint8_t quality = OHOS_CAMERA_JPEG_LEVEL_HIGH;
    metaData->addEntry(OHOS_JPEG_QUALITY, &quality, sizeof(quality));
    std::vector<double> gps;
    gps.push_back(LATITUDE);
    gps.push_back(LONGITUDE);
    gps.push_back(ALTITUDE);
    metaData->addEntry(OHOS_JPEG_GPS_COORDINATES, gps.data(), gps.size());
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    settings.push_back(setting);

    auto info = std::make_shared<DCameraCaptureInfo>();
    info->width_ = TEST_WIDTH;
    info->height_ = TEST_HEIGHT;
    info->format_ = TEST_FORMAT_PHOTO;
    info->isCapture_ = true;
    info->streamType_ = SNAPSHOT_FRAME;
    info->captureSettings_ = settings;
    ret = client_->StartPhotoOutput(info);
    EXPECT_EQ(DCAMERA_OK, ret);

    sleep(TEST_SLEEP_SEC);
    ret = client_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS