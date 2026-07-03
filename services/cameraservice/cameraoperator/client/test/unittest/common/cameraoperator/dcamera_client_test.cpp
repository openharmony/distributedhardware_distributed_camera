/*
 * Copyright (c) 2021-2026 Huawei Device Co., Ltd.
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
#include "surface.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraClientTestStateCallback : public StateCallback {
public:
    void OnStateChanged(std::shared_ptr<DCameraEvent>& event) override
    {
        DHLOGI("DCameraClientTestStateCallback::OnStateChanged type: %{public}d, result: %{public}d",
            event->eventType_, event->eventResult_);
    }

    void OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings) override
    {
        DHLOGI("DCameraClientTestStateCallback::OnMetadataResult");
    }
};

class DCameraClientTestResultCallback : public ResultCallback {
public:
    void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer) override
    {
        DHLOGI("DCameraClientTestResultCallback::OnPhotoResult");
    }

    void OnVideoResult(std::shared_ptr<DataBuffer>& buffer) override
    {
        DHLOGI("DCameraClientTestResultCallback::OnVideoResult");
    }
};

class DCameraClientTestVideoSurfaceListener : public IBufferConsumerListener {
public:
    void OnBufferAvailable() override
    {
        DHLOGD("DCameraClientTestVideoSurfaceListener::OnBufferAvailable");
    }
};

#ifdef DCAMERA_YUV
        const int32_t TEST_WIDTH = 1920;
        const int32_t TEST_HEIGHT = 1080;
        const int32_t TEST_WIDTH_PHOTO = 1920;
        const int32_t TEST_HEIGHT_PHOTO = 1080;
        const int32_t TEST_FORMAT_VIDEO = OHOS_CAMERA_FORMAT_YCRCB_420_SP;
        const int32_t TEST_FORMAT_PHOTO = OHOS_CAMERA_FORMAT_JPEG;
#else
        const int32_t TEST_WIDTH = 640;
        const int32_t TEST_HEIGHT = 480;
        const int32_t TEST_WIDTH_PHOTO = 1280;
        const int32_t TEST_HEIGHT_PHOTO = 960;
        const int32_t TEST_FORMAT_VIDEO = OHOS_CAMERA_FORMAT_RGBA_8888;
        const int32_t TEST_FORMAT_PHOTO = OHOS_CAMERA_FORMAT_JPEG;
#endif

constexpr int32_t TEST_SLEEP_SEC = 2;
constexpr uint32_t ENTRY_CAPACITY = 30;
constexpr uint32_t DATA_CAPACITY = 2000;
constexpr double LATITUDE = 22.306;
constexpr double LONGITUDE = 52.12;
constexpr double ALTITUDE = 2.365;
constexpr uint32_t DCAMERA_TEST_MAX_METADATA_SIZE = 20;

class DCameraClientTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();
    void SetCaptureInfo(std::shared_ptr<DCameraCaptureInfo>& info);

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
    if (cameras.empty()) {
        client_ = nullptr;
    } else {
        client_ = std::make_shared<DCameraClient>(cameras[0]);
    }

    photoInfo_false_ = std::make_shared<DCameraCaptureInfo>();
    photoInfo_false_->width_ = TEST_WIDTH_PHOTO;
    photoInfo_false_->height_ = TEST_HEIGHT_PHOTO;
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
    photoInfo_true_->width_ = TEST_WIDTH_PHOTO;
    photoInfo_true_->height_ = TEST_HEIGHT_PHOTO;
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
    if (client_ != nullptr) {
        client_->captureInfosCache_.clear();
        client_->fpsRanges_.clear();
        client_->cameraMetadatas_ = std::queue<std::string>();
    }
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

void DCameraClientTest::SetCaptureInfo(std::shared_ptr<DCameraCaptureInfo>& info)
{
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
    info->width_ = TEST_WIDTH_PHOTO;
    info->height_ = TEST_HEIGHT_PHOTO;
    info->format_ = TEST_FORMAT_PHOTO;
    info->isCapture_ = true;
    info->streamType_ = SNAPSHOT_FRAME;
    info->captureSettings_ = settings;
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
    ASSERT_NE(client_, nullptr);
    auto managerCallback = std::make_shared<DCameraManagerCallback>();
    CameraStandard::CameraStatusInfo cameraStatus;
    managerCallback->OnCameraStatusChanged(cameraStatus);
    managerCallback->OnFlashlightStatusChanged("", CameraStandard::FlashStatus::FLASH_STATUS_OFF);

    auto inputCallback = std::make_shared<DCameraInputCallback>(nullptr);
    inputCallback->OnError(CameraStandard::CameraErrorCode::CONFLICT_CAMERA, 0);

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
    inputCallback->OnError(CameraStandard::CameraErrorCode::CONFLICT_CAMERA, 0);
    inputCallback->OnError(CameraStandard::CameraErrorCode::DEVICE_PREEMPTED, 0);

    sessionCallback = std::make_shared<DCameraSessionCallback>(stateCallback);
    sessionCallback->OnError(0);
    sessionCallback->OnFocusState(CameraStandard::FocusCallback::UNFOCUSED);

    int32_t invalidParam = 3;
    auto state = static_cast<CameraStandard::FocusCallback::FocusState>(invalidParam);
    sessionCallback->OnFocusState(state);

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
    ASSERT_NE(client_, nullptr);
    auto photoSurfaceListener = std::make_shared<DCameraPhotoSurfaceListener>(nullptr, nullptr);
    photoSurfaceListener->OnBufferAvailable();

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    photoSurfaceListener = std::make_shared<DCameraPhotoSurfaceListener>(nullptr, resultCallback);
    photoSurfaceListener->OnBufferAvailable();

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
    ASSERT_NE(client_, nullptr);
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
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> videoListener(new DCameraClientTestVideoSurfaceListener());
    videoSurface->RegisterConsumerListener(videoListener);
    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_004: video width: %{public}d, height: %{public}d, format: %{public}d,"
        " isCapture: %{public}d", videoInfo_true_->width_, videoInfo_true_->height_, videoInfo_true_->format_,
        videoInfo_true_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    sptr<IBufferProducer> bp = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(bp);
    int32_t mode = 0;
    ret = client_->StartCapture(captureInfos, pSurface, mode);
    EXPECT_FALSE(mode);

    ret = client_->PauseCapture();
    EXPECT_FALSE(mode);

    ret = client_->ResumeCapture();
    EXPECT_FALSE(mode);

    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::vector<int32_t> fpsRange = { ENTRY_CAPACITY, ENTRY_CAPACITY };
    metaData->addEntry(OHOS_CONTROL_FPS_RANGES, fpsRange.data(), fpsRange.size());
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    settings.push_back(setting);
    client_->GetFpsRanges();
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
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> videoListener(new DCameraClientTestVideoSurfaceListener());
    videoSurface->RegisterConsumerListener(videoListener);
    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_005: video width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", videoInfo_true_->width_, videoInfo_true_->height_,
        videoInfo_true_->format_, videoInfo_true_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_005: photo width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", photoInfo_false_->width_, photoInfo_false_->height_,
        photoInfo_false_->format_, photoInfo_false_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    captureInfos.push_back(photoInfo_false_);
    sptr<IBufferProducer> bp = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(bp);
    int32_t mode = 0;
    ret = client_->StartCapture(captureInfos, pSurface, mode);
    EXPECT_FALSE(mode);

    sleep(TEST_SLEEP_SEC);

    DHLOGI("DCameraClientTest dcamera_client_test_005: video width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", videoInfo_true_->width_, videoInfo_true_->height_,
        videoInfo_true_->format_, videoInfo_true_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_005: photo width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", photoInfo_false_->width_, photoInfo_false_->height_,
        photoInfo_false_->format_, photoInfo_false_->isCapture_);
    captureInfos.clear();
    captureInfos.push_back(videoInfo_false_);
    captureInfos.push_back(photoInfo_true_);
    ret = client_->StartCapture(captureInfos, pSurface, mode);
    EXPECT_FALSE(mode);

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
    ASSERT_NE(client_, nullptr);
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
    ASSERT_NE(client_, nullptr);
    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    uint8_t afMode = OHOS_CAMERA_FOCUS_MODE_AUTO;
    uint8_t aeMode = OHOS_CAMERA_EXPOSURE_MODE_CONTINUOUS_AUTO;
    uint8_t stabilizationMode = OHOS_CAMERA_VIDEO_STABILIZATION_OFF;
    int32_t orientation = OHOS_CAMERA_JPEG_ROTATION_0;
    metaData->addEntry(OHOS_JPEG_ORIENTATION, &orientation, sizeof(int32_t));
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
    ASSERT_NE(client_, nullptr);
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
    ASSERT_NE(client_, nullptr);
    int32_t ret = client_->CameraServiceErrorType(CameraStandard::CameraErrorCode::SUCCESS);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->CameraServiceErrorType(CameraStandard::CameraErrorCode::SERVICE_FATL_ERROR);
    EXPECT_EQ(DCAMERA_ALLOC_ERROR, ret);

    ret = client_->CameraServiceErrorType(CameraStandard::CameraErrorCode::CONFLICT_CAMERA);
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
    ASSERT_NE(client_, nullptr);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    int32_t ret = client_->CreateCaptureOutput(captureInfos);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    int32_t invalidParam = 2;
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = static_cast<DCStreamType>(invalidParam);
    captureInfos.push_back(info);
    client_->CreateCaptureOutput(captureInfos);
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
    ASSERT_NE(client_, nullptr);
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
 * @tc.name: dcamera_client_test_013
 * @tc.desc: Verify StartPhotoOutput
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_013, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_013: test startPhotoOutput");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    int32_t ret = client_->SetStateCallback(stateCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    ret = client_->SetResultCallback(resultCallback);
    EXPECT_EQ(DCAMERA_OK, ret);

    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferConsumerListener> videoListener(new DCameraClientTestVideoSurfaceListener());
    videoSurface->RegisterConsumerListener(videoListener);
    ret = client_->Init();
    EXPECT_EQ(DCAMERA_OK, ret);

    SetTokenID();
    DHLOGI("DCameraClientTest dcamera_client_test_013: video width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", videoInfo_true_->width_, videoInfo_true_->height_,
        videoInfo_true_->format_, videoInfo_true_->isCapture_);
    DHLOGI("DCameraClientTest dcamera_client_test_013: photo width: %{public}d, height: %{public}d, format: "
        "%{public}d, isCapture: %{public}d", photoInfo_false_->width_, photoInfo_false_->height_,
        photoInfo_false_->format_, photoInfo_false_->isCapture_);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(videoInfo_true_);
    captureInfos.push_back(photoInfo_false_);
    sptr<IBufferProducer> bp = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(bp);
    int32_t mode = 0;
    ret = client_->StartCapture(captureInfos, pSurface, mode);
    EXPECT_FALSE(mode);

    sleep(TEST_SLEEP_SEC);

    auto info = std::make_shared<DCameraCaptureInfo>();
    SetCaptureInfo(info);
    ret = client_->StartPhotoOutput(info);
    EXPECT_FALSE(mode);

    sleep(TEST_SLEEP_SEC);
    ret = client_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = client_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_014
 * @tc.desc: Verify StartCapture
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_014, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_014: test startCapture");
    ASSERT_NE(client_, nullptr);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    sptr<Surface> surface = nullptr;
    int32_t mode = 0;
    int32_t ret = client_->StartCapture(captureInfos, surface, mode);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_015
 * @tc.desc: Verify StartCaptureInner
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_015, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_015: test startCaptureInner");
    ASSERT_NE(client_, nullptr);
    int32_t invalidParam = 2;
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = static_cast<DCStreamType>(invalidParam);
    int32_t ret = client_->StartCaptureInner(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_016
 * @tc.desc: Verify PauseCapture ResumeCapture
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_016, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_016: test PauseCapture");
    ASSERT_NE(client_, nullptr);
    int32_t ret = client_->PauseCapture();
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = client_->ResumeCapture();
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_017
 * @tc.desc: Verify OnResult
 * @tc.type: FUNC
 * @tc.require: AR000GK6ML
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_017, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_017: test OnResult");
    std::shared_ptr<Camera::CameraMetadata> result = std::make_shared<Camera::CameraMetadata>(8, 8);
    std::shared_ptr<CaOnResultCallback> resultCallback = std::make_shared<CaOnResultCallback>(nullptr);
    EXPECT_NO_FATAL_FAILURE(resultCallback->OnResult(0, result));
}

/**
 * @tc.name: dcamera_client_test_018
 * @tc.desc: Verify DCameraClient constructor with short dhId
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_018, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_018: test constructor with short dhId");
    std::string shortDhId = "test123";
    auto client = std::make_shared<DCameraClient>(shortDhId);
    ASSERT_NE(client, nullptr);
    EXPECT_EQ(client->cameraId_, "");
}

/**
 * @tc.name: dcamera_client_test_019
 * @tc.desc: Verify UpdateSettings with nullptr setting
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_019, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_019: test UpdateSettings with nullptr setting");
    ASSERT_NE(client_, nullptr);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    settings.push_back(nullptr);
    int32_t ret = client_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_020
 * @tc.desc: Verify UpdateSettingCache queue full scenario
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_020, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_020: test UpdateSettingCache queue full");
    ASSERT_NE(client_, nullptr);
    for (int i = 0; i < DCAMERA_TEST_MAX_METADATA_SIZE + 1; i++) {
        client_->UpdateSettingCache("test_metadata_" + std::to_string(i));
    }
    EXPECT_EQ(client_->cameraMetadatas_.size(), DCAMERA_TEST_MAX_METADATA_SIZE);
}

/**
 * @tc.name: dcamera_client_test_024
 * @tc.desc: Verify ReleaseCameraInput with nullptr input
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_024, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_024: test ReleaseCameraInput nullptr");
    ASSERT_NE(client_, nullptr);
    client_->cameraInput_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->ReleaseCameraInput());
}

/**
 * @tc.name: dcamera_client_test_025
 * @tc.desc: Verify CommitCapture with nullptr info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_025, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_025: test CommitCapture nullptr info");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraClientTestStateCallback>();
    client_->SetStateCallback(stateCallback);
    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraClientTestResultCallback>();
    client_->SetResultCallback(resultCallback);
    client_->Init();

    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = SNAPSHOT_FRAME;
    info->width_ = TEST_WIDTH_PHOTO;
    info->height_ = TEST_HEIGHT_PHOTO;
    info->format_ = TEST_FORMAT_PHOTO;
    info->isCapture_ = false;
    client_->CreatePhotoOutput(info);

    client_->captureInfosCache_.clear();
    client_->captureInfosCache_.push_back(nullptr);
    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferProducer> bp = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(bp);
    int32_t ret = client_->CommitCapture(pSurface);
    EXPECT_EQ(DCAMERA_OK, ret);

    client_->UnInit();
}

/**
 * @tc.name: dcamera_client_test_026
 * @tc.desc: Verify CommitCapture with CONTINUOUS_FRAME type
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_026, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_026: test CommitCapture CONTINUOUS_FRAME");
    ASSERT_NE(client_, nullptr);
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = CONTINUOUS_FRAME;
    info->isCapture_ = true;
    client_->captureInfosCache_.push_back(info);
    sptr<IConsumerSurface> videoSurface = IConsumerSurface::Create();
    sptr<IBufferProducer> bp = videoSurface->GetProducer();
    sptr<Surface> pSurface = Surface::CreateSurfaceAsProducer(bp);
    client_->photoOutput_ = nullptr;
    client_->previewOutput_ = nullptr;
    int32_t ret = client_->Init();
    if (ret == DCAMERA_OK) {
        ret = client_->CommitCapture(pSurface);
    }
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_027
 * @tc.desc: Verify GetFpsRanges with empty metadata
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_027, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_027: test GetFpsRanges empty metadata");
    ASSERT_NE(client_, nullptr);
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = CONTINUOUS_FRAME;
    info->captureSettings_.clear();
    client_->captureInfosCache_.push_back(info);
    EXPECT_NO_FATAL_FAILURE(client_->GetFpsRanges());
}

/**
 * @tc.name: dcamera_client_test_028
 * @tc.desc: Verify GetFpsRanges with null info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_028, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_028: test GetFpsRanges null info");
    ASSERT_NE(client_, nullptr);
    client_->captureInfosCache_.push_back(nullptr);
    EXPECT_NO_FATAL_FAILURE(client_->GetFpsRanges());
}

/**
 * @tc.name: dcamera_client_test_029
 * @tc.desc: Verify StopOutput with nullptr previewOutput
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_029, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_029: test StopOutput nullptr preview");
    ASSERT_NE(client_, nullptr);
    client_->previewOutput_ = nullptr;
    client_->photoOutput_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->StopOutput());
}

/**
 * @tc.name: dcamera_client_test_030
 * @tc.desc: Verify ReleaseCaptureSession with nullptr session
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_030, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_030: test ReleaseCaptureSession nullptr");
    ASSERT_NE(client_, nullptr);
    client_->captureSession_ = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->ReleaseCaptureSession());
}

/**
 * @tc.name: dcamera_client_test_031
 * @tc.desc: Verify SetCallerInfo
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_031, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_031: test SetCallerInfo");
    ASSERT_NE(client_, nullptr);
    std::string srcDevId = "test_device_123";
    std::string deviceName = "test_device_name_123";
    EXPECT_NO_FATAL_FAILURE(client_->SetCallerInfo(srcDevId, deviceName));
    EXPECT_EQ(client_->srcDevId_, srcDevId);
}

/**
 * @tc.name: dcamera_client_test_032
 * @tc.desc: Verify StartPhotoOutput with nullptr photoOutput
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_032, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_032: test StartPhotoOutput nullptr photoOutput");
    ASSERT_NE(client_, nullptr);
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = SNAPSHOT_FRAME;
    info->isCapture_ = true;
    client_->photoOutput_ = nullptr;
    int32_t ret = client_->StartPhotoOutput(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_033
 * @tc.desc: Verify StartPhotoOutput with nullptr info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_033, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_033: test StartPhotoOutput nullptr info");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<DCameraCaptureInfo> info = nullptr;
    int32_t ret = client_->StartPhotoOutput(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_034
 * @tc.desc: Verify StartCaptureInner with nullptr info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_034, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_034: test StartCaptureInner nullptr info");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<DCameraCaptureInfo> info = nullptr;
    int32_t ret = client_->StartCaptureInner(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_035
 * @tc.desc: Verify CreatePhotoOutput with nullptr info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_035, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_035: test CreatePhotoOutput nullptr info");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<DCameraCaptureInfo> info = nullptr;
    int32_t ret = client_->CreatePhotoOutput(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_036
 * @tc.desc: Verify CreatePreviewOutput with nullptr info
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_036, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_036: test CreatePreviewOutput nullptr info");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<DCameraCaptureInfo> info = nullptr;
    int32_t ret = client_->CreatePreviewOutput(info);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_037
 * @tc.desc: Verify SetPhotoCaptureRotation with nullptr cameraMetadata
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_037, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_037: test SetPhotoCaptureRotation nullptr");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = nullptr;
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting =
        std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureRotation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_038
 * @tc.desc: Verify SetPhotoCaptureRotation with nullptr photoCaptureSetting
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_038, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_038: test SetPhotoCaptureRotation nullptr setting");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata =
        std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureRotation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_039
 * @tc.desc: Verify SetPhotoCaptureQuality with nullptr cameraMetadata
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_039, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_039: test SetPhotoCaptureQuality nullptr");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = nullptr;
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting =
        std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureQuality(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_040
 * @tc.desc: Verify SetPhotoCaptureQuality with nullptr photoCaptureSetting
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_040, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_040: test SetPhotoCaptureQuality nullptr setting");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata =
        std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureQuality(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_041
 * @tc.desc: Verify SetPhotoCaptureLocation with nullptr cameraMetadata
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_041, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_041: test SetPhotoCaptureLocation nullptr");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = nullptr;
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting =
        std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureLocation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_042
 * @tc.desc: Verify SetPhotoCaptureLocation with nullptr photoCaptureSetting
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_042, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_042: test SetPhotoCaptureLocation nullptr setting");
    ASSERT_NE(client_, nullptr);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata =
        std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting = nullptr;
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureLocation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_043
 * @tc.desc: Verify ConfigCaptureSessionInner with nullptr captureSession
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_043, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_043: test ConfigCaptureSessionInner nullptr session");
    ASSERT_NE(client_, nullptr);
    client_->captureSession_ = nullptr;
    int32_t ret = client_->ConfigCaptureSessionInner();
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_client_test_044
 * @tc.desc: Verify CreateCaptureOutput with nullptr info in vector
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_044, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_044: test CreateCaptureOutput nullptr info in vector");
    ASSERT_NE(client_, nullptr);
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    captureInfos.push_back(nullptr);
    int32_t ret = client_->CreateCaptureOutput(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_client_test_046
 * @tc.desc: Verify StopCapture with photoSurface
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_046, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_046: test StopCapture with photoSurface");
    ASSERT_NE(client_, nullptr);
    client_->photoSurface_ = IConsumerSurface::Create();
    client_->photoListener_ = nullptr;
    int32_t ret = client_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);
    client_->photoSurface_ = nullptr;
}

/**
 * @tc.name: dcamera_client_test_052
 * @tc.desc: Verify GetFpsRanges with fps count not equal to DCAMERA_FPS_SIZE
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_052, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_052: test GetFpsRanges invalid count");
    ASSERT_NE(client_, nullptr);
    auto info = std::make_shared<DCameraCaptureInfo>();
    info->streamType_ = CONTINUOUS_FRAME;
    
    auto metaData = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    int32_t fpsRange[4] = {15, 30, 60, 120};
    metaData->addEntry(OHOS_CONTROL_FPS_RANGES, fpsRange, 4);
    std::string abilityString = Camera::MetadataUtils::EncodeToString(metaData);
    
    auto setting = std::make_shared<DCameraSettings>();
    setting->type_ = UPDATE_METADATA;
    setting->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    info->captureSettings_.push_back(setting);
    
    client_->captureInfosCache_.push_back(info);
    client_->fpsRanges_.clear();
    EXPECT_NO_FATAL_FAILURE(client_->GetFpsRanges());
}

/**
 * @tc.name: dcamera_client_test_053
 * @tc.desc: Verify SetPhotoCaptureRotation with count not equal to 1
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_053, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_053: test SetPhotoCaptureRotation invalid count");
    ASSERT_NE(client_, nullptr);
    auto cameraMetadata = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    int32_t orientation[2] = {OHOS_CAMERA_JPEG_ROTATION_90, OHOS_CAMERA_JPEG_ROTATION_180};
    cameraMetadata->addEntry(OHOS_JPEG_ORIENTATION, orientation, 2);
    auto photoCaptureSetting = std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureRotation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_054
 * @tc.desc: Verify SetPhotoCaptureQuality with count not equal to 1
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_054, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_054: test SetPhotoCaptureQuality invalid count");
    ASSERT_NE(client_, nullptr);
    auto cameraMetadata = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    uint8_t quality[2] = {OHOS_CAMERA_JPEG_LEVEL_HIGH, OHOS_CAMERA_JPEG_LEVEL_MIDDLE};
    cameraMetadata->addEntry(OHOS_JPEG_QUALITY, quality, 2);
    auto photoCaptureSetting = std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureQuality(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_055
 * @tc.desc: Verify SetPhotoCaptureLocation with count not equal to 3
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_055, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_055: test SetPhotoCaptureLocation invalid count");
    ASSERT_NE(client_, nullptr);
    auto cameraMetadata = std::make_shared<Camera::CameraMetadata>(ENTRY_CAPACITY, DATA_CAPACITY);
    double gps[2] = {LATITUDE, LONGITUDE};
    cameraMetadata->addEntry(OHOS_JPEG_GPS_COORDINATES, gps, 2);
    auto photoCaptureSetting = std::make_shared<CameraStandard::PhotoCaptureSetting>();
    EXPECT_NO_FATAL_FAILURE(client_->SetPhotoCaptureLocation(cameraMetadata, photoCaptureSetting));
}

/**
 * @tc.name: dcamera_client_test_057
 * @tc.desc: Verify CameraServiceErrorType with other error types
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraClientTest, dcamera_client_test_057, TestSize.Level1)
{
    DHLOGI("DCameraClientTest dcamera_client_test_057: test CameraServiceErrorType other");
    ASSERT_NE(client_, nullptr);
    int32_t ret = client_->CameraServiceErrorType(CameraStandard::CameraErrorCode::INVALID_ARGUMENT);
    EXPECT_EQ(CameraStandard::CameraErrorCode::INVALID_ARGUMENT, ret);

    ret = client_->CameraServiceErrorType(CameraStandard::CameraErrorCode::SESSION_NOT_CONFIG);
    EXPECT_EQ(CameraStandard::CameraErrorCode::SESSION_NOT_CONFIG, ret);
}
} // namespace DistributedHardware
} // namespace OHOS