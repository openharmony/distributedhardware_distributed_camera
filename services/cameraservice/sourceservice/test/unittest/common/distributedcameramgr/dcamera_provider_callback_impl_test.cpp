/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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
#include "dcamera_provider_callback_impl.h"
#undef private

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraProviderCallbackImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<IDCameraProviderCallback> testProviderCallback_;
    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;

private:
    static void SetStreamInfos();
    static void SetCaptureInfos();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "0xFFFF";
const std::string TEST_PARAM = "0xFFFF";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_STREAMID = 2;
const int32_t SLEEP_TIME = 200000;
std::vector<DCStreamInfo> g_streamInfosSnap;
std::vector<DCCaptureInfo> g_captureInfoSnap;
std::vector<DCameraSettings> g_cameraSettingSnap;
std::vector<int> g_streamIdSnap;
}

void DCameraProviderCallbackImplTest::SetUpTestCase(void)
{
}

void DCameraProviderCallbackImplTest::SetStreamInfos()
{
    DCStreamInfo streamInfo1;
    streamInfo1.streamId_ = 1;
    streamInfo1.width_ = TEST_WIDTH;
    streamInfo1.height_ = TEST_HEIGTH;
    streamInfo1.stride_ = 1;
    streamInfo1.format_ = 1;
    streamInfo1.dataspace_ = 1;
    streamInfo1.encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo1.type_ = SNAPSHOT_FRAME;

    DCStreamInfo streamInfo2;
    streamInfo2.streamId_ = TEST_STREAMID;
    streamInfo2.width_ = TEST_WIDTH;
    streamInfo2.height_ = TEST_HEIGTH;
    streamInfo2.stride_ = 1;
    streamInfo2.format_ = 1;
    streamInfo2.dataspace_ = 1;
    streamInfo2.encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo2.type_ = SNAPSHOT_FRAME;
    g_streamInfosSnap.push_back(streamInfo1);
    g_streamInfosSnap.push_back(streamInfo2);
}

void DCameraProviderCallbackImplTest::SetCaptureInfos()
{
    DCCaptureInfo captureInfo1;
    captureInfo1.streamIds_.push_back(1);
    captureInfo1.width_ = TEST_WIDTH;
    captureInfo1.height_ = TEST_HEIGTH;
    captureInfo1.stride_ = 1;
    captureInfo1.format_ = 1;
    captureInfo1.dataspace_ = 1;
    captureInfo1.encodeType_ = ENCODE_TYPE_H265;
    captureInfo1.type_ = CONTINUOUS_FRAME;

    DCCaptureInfo captureInfo2;
    captureInfo2.streamIds_.push_back(1);
    captureInfo2.width_ = TEST_WIDTH;
    captureInfo2.height_ = TEST_HEIGTH;
    captureInfo2.stride_ = 1;
    captureInfo2.format_ = 1;
    captureInfo2.dataspace_ = 1;
    captureInfo2.encodeType_ = ENCODE_TYPE_H265;
    captureInfo2.type_ = CONTINUOUS_FRAME;
    g_captureInfoSnap.push_back(captureInfo1);
    g_captureInfoSnap.push_back(captureInfo2);

    DCameraSettings camSettings1;
    camSettings1.type_ = UPDATE_METADATA;
    camSettings1.value_ = "SettingValue";

    DCameraSettings camSettings2;
    camSettings2.type_ = ENABLE_METADATA;
    camSettings2.value_ = "SettingValue";
    g_cameraSettingSnap.push_back(camSettings1);
    g_cameraSettingSnap.push_back(camSettings2);

    g_streamIdSnap.push_back(1);
    g_streamIdSnap.push_back(TEST_STREAMID);
}

void DCameraProviderCallbackImplTest::TearDownTestCase(void)
{
}

void DCameraProviderCallbackImplTest::SetUp(void)
{
    g_streamInfosSnap.clear();
    g_captureInfoSnap.clear();
    g_cameraSettingSnap.clear();
    g_streamIdSnap.clear();

    SetStreamInfos();
    SetCaptureInfos();

    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    camDev_->InitDCameraSourceDev();
    testProviderCallback_ = std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);
}

void DCameraProviderCallbackImplTest::TearDown(void)
{
    usleep(SLEEP_TIME);
    testProviderCallback_ = nullptr;
    camDev_ = nullptr;
    stateListener_ = nullptr;
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_001
 * @tc.desc: Verify OpenSession CloseSession func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_001, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase;
    int32_t ret = testProviderCallback_->OpenSession(dhBase);
    EXPECT_EQ(FAILED, ret);

    ret = testProviderCallback_->CloseSession(dhBase);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_002
 * @tc.desc: Verify OpenSession CloseSession func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_002, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);
    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    int32_t ret = testProviderCallback_->OpenSession(dhBase);
    EXPECT_EQ(SUCCESS, ret);

    ret = testProviderCallback_->CloseSession(dhBase);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_003
 * @tc.desc: Verify ConfigureStreams ReleaseStreams func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_003, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase;
    std::vector<DCStreamInfo> streamInfos;
    int32_t ret = testProviderCallback_->ConfigureStreams(dhBase, streamInfos);
    EXPECT_EQ(FAILED, ret);

    std::vector<int> streamIds;
    ret = testProviderCallback_->ReleaseStreams(dhBase, streamIds);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_004
 * @tc.desc: Verify ConfigureStreams ReleaseStreams func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_004, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    int32_t ret = testProviderCallback_->ConfigureStreams(dhBase, g_streamInfosSnap);
    EXPECT_EQ(SUCCESS, ret);

    ret = testProviderCallback_->ReleaseStreams(dhBase, g_streamIdSnap);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_005
 * @tc.desc: Verify StartCapture StopCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_005, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase;
    std::vector<DCCaptureInfo> captureInfos;
    int32_t ret = testProviderCallback_->StartCapture(dhBase, g_captureInfoSnap);
    EXPECT_EQ(FAILED, ret);

    std::vector<int> streamIds;
    ret = testProviderCallback_->StopCapture(dhBase, streamIds);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_006
 * @tc.desc: Verify StartCapture StopCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_006, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    int32_t ret = testProviderCallback_->StartCapture(dhBase, g_captureInfoSnap);
    EXPECT_EQ(SUCCESS, ret);

    ret = testProviderCallback_->StopCapture(dhBase, g_streamIdSnap);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_007
 * @tc.desc: Verify UpdateSettings func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_007, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase;
    int32_t ret = testProviderCallback_->UpdateSettings(dhBase, g_cameraSettingSnap);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_008
 * @tc.desc: Verify UpdateSettings func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_008, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    int32_t ret = testProviderCallback_->OpenSession(dhBase);
    EXPECT_EQ(SUCCESS, ret);

    ret = testProviderCallback_->UpdateSettings(dhBase, g_cameraSettingSnap);
    EXPECT_EQ(SUCCESS, ret);

    ret = testProviderCallback_->CloseSession(dhBase);
    EXPECT_EQ(SUCCESS, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_009
 * @tc.desc: Verify OpenSession CloseSession func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_009, TestSize.Level0)
{
    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    std::shared_ptr<DCameraSourceDev> camDev = nullptr;
    std::shared_ptr<DCameraProviderCallbackImpl> testProviderCallback =
        std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev);
    int32_t ret = testProviderCallback->OpenSession(dhBase);
    EXPECT_EQ(FAILED, ret);

    ret = testProviderCallback->CloseSession(dhBase);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_010
 * @tc.desc: Verify UpdateSettings func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_010, TestSize.Level0)
{
    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    std::shared_ptr<DCameraSourceDev> camDev = nullptr;
    std::shared_ptr<DCameraProviderCallbackImpl> testProviderCallback =
        std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev);
    int32_t ret = testProviderCallback->UpdateSettings(dhBase, g_cameraSettingSnap);

    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_011
 * @tc.desc: Verify ConfigureStreams ReleaseStreams func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_011, TestSize.Level0)
{
    EXPECT_EQ(false, testProviderCallback_ == nullptr);

    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    std::shared_ptr<DCameraSourceDev> camDev = nullptr;
    std::shared_ptr<DCameraProviderCallbackImpl> testProviderCallback =
        std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev);
    std::vector<DCStreamInfo> streamInfos;
    DCStreamInfo streamInfo11;
    streamInfo11.streamId_ = 1;
    streamInfo11.width_ = TEST_WIDTH;
    streamInfo11.height_ = TEST_HEIGTH;
    streamInfo11.stride_ = 1;
    streamInfo11.format_ = 1;
    streamInfo11.dataspace_ = 1;
    streamInfo11.encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo11.type_ = SNAPSHOT_FRAME;
    streamInfos.push_back(streamInfo11);
    int32_t ret = testProviderCallback->ConfigureStreams(dhBase, streamInfos);
    EXPECT_EQ(FAILED, ret);

    std::vector<int> streamIds;
    ret = testProviderCallback->ReleaseStreams(dhBase, streamIds);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_012
 * @tc.desc: Verify CheckStreamInfo func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_012, TestSize.Level1)
{
    std::shared_ptr<DCameraProviderCallbackImpl> testProviderCallback =
        std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);
    DCStreamInfo stream;
    stream.streamId_ = 1;
    stream.width_ = TEST_WIDTH;
    stream.height_ = TEST_HEIGTH;
    stream.stride_ = 1;
    stream.format_ = 1;
    stream.dataspace_ = 1;
    stream.encodeType_ = ENCODE_TYPE_JPEG;
    stream.type_ = SNAPSHOT_FRAME;
    int32_t ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(true, ret);
    stream.streamId_ = -1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.streamId_ = 1;
    stream.height_ = -1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.streamId_ = 1;
    stream.height_ = TEST_HEIGTH;
    stream.width_ = -1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.height_ = testProviderCallback->RESOLUTION_MAX_WIDTH;
    stream.width_ = testProviderCallback->RESOLUTION_MAX_WIDTH + 1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.width_ = TEST_WIDTH;
    stream.height_ = TEST_HEIGTH;
    stream.stride_ = -1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.stride_ = 1;
    stream.dataspace_ = -1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
    stream.format_ = -1;
    stream.dataspace_ = 1;
    ret = testProviderCallback->CheckStreamInfo(stream);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_013
 * @tc.desc: Verify StartCapture StopCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_013, TestSize.Level0)
{
    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    std::vector<DCCaptureInfo> captureInfos;
    DCCaptureInfo captureInfo13;
    captureInfo13.streamIds_.push_back(1);
    captureInfo13.width_ = TEST_WIDTH;
    captureInfo13.height_ = TEST_HEIGTH;
    captureInfo13.stride_ = 1;
    captureInfo13.format_ = 1;
    captureInfo13.dataspace_ = 1;
    captureInfo13.encodeType_ = ENCODE_TYPE_H265;
    captureInfo13.type_ = CONTINUOUS_FRAME;
    std::shared_ptr<DCameraSourceDev> camDev = nullptr;
    std::shared_ptr<DCameraProviderCallbackImpl> testProviderCallback =
        std::make_shared<DCameraProviderCallbackImpl>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev);
    int32_t ret = testProviderCallback->StartCapture(dhBase, captureInfos);
    EXPECT_EQ(FAILED, ret);

    std::vector<int> streamIds;
    ret = testProviderCallback->StopCapture(dhBase, streamIds);
    EXPECT_EQ(FAILED, ret);
}

/**
 * @tc.name: dcamera_provider_callback_impl_test_014
 * @tc.desc: Verify NotifyEvent func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraProviderCallbackImplTest, dcamera_provider_callback_impl_test_014, TestSize.Level0)
{
    DHBase dhBase{TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0};
    DCameraHDFEvent event;
    event.type_ = DCameraEventType::DCAMERE_GETFULLCAP;
    int32_t ret = testProviderCallback_->NotifyEvent(dhBase, event);
    EXPECT_EQ(SUCCESS, ret);
    event.type_ = DCameraEventType::DCAMERA_MESSAGE;
    ret = testProviderCallback_->NotifyEvent(dhBase, event);
    EXPECT_EQ(SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
