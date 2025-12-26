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
#include "dcamera_source_input.h"
#undef private
#include "distributed_camera_errno.h"
#include "mock_dcamera_source_state_listener.h"
#include "dcamera_source_input_channel_listener.h"
#include "metadata_utils.h"
#include "dcamera_utils_tools.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceInputTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceInput> testInput_;
    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
private:
    static void SetStreamInfos();
    static void SetCaptureInfos();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_STREAMID = 2;
const int32_t TEST_SLEEP_SEC = 200000;
std::vector<std::shared_ptr<DCStreamInfo>> g_streamInfos;
std::vector<std::shared_ptr<DCCaptureInfo>> g_captureInfos;
std::vector<std::shared_ptr<DCameraSettings>> g_cameraSettings;
std::vector<int> g_streamIds;
std::vector<DCameraIndex> g_camIndexs;
}

void DCameraSourceInputTest::SetUpTestCase(void)
{
}

void DCameraSourceInputTest::SetStreamInfos()
{
    DCameraIndex camIndex1(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0);
    g_camIndexs.push_back(camIndex1);
    std::shared_ptr<DCStreamInfo> streamInfo1 = std::make_shared<DCStreamInfo>();
    streamInfo1->streamId_ = 1;
    streamInfo1->width_ = TEST_WIDTH;
    streamInfo1->height_ = TEST_HEIGTH;
    streamInfo1->stride_ = 1;
    streamInfo1->format_ = 1;
    streamInfo1->dataspace_ = 1;
    streamInfo1->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo1->type_ = SNAPSHOT_FRAME;

    std::shared_ptr<DCStreamInfo> streamInfo2 = std::make_shared<DCStreamInfo>();
    streamInfo2->streamId_ = TEST_STREAMID;
    streamInfo2->width_ = TEST_WIDTH;
    streamInfo2->height_ = TEST_HEIGTH;
    streamInfo2->stride_ = 1;
    streamInfo2->format_ = 1;
    streamInfo2->dataspace_ = 1;
    streamInfo2->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo2->type_ = CONTINUOUS_FRAME;
    g_streamInfos.push_back(streamInfo1);
    g_streamInfos.push_back(streamInfo2);
}

void DCameraSourceInputTest::SetCaptureInfos()
{
    std::shared_ptr<DCCaptureInfo> captureInfo1 = std::make_shared<DCCaptureInfo>();
    captureInfo1->streamIds_.push_back(1);
    captureInfo1->width_ = TEST_WIDTH;
    captureInfo1->height_ = TEST_HEIGTH;
    captureInfo1->stride_ = 1;
    captureInfo1->format_ = 1;
    captureInfo1->dataspace_ = 1;
    captureInfo1->encodeType_ = ENCODE_TYPE_H265;
    captureInfo1->type_ = CONTINUOUS_FRAME;

    std::shared_ptr<DCCaptureInfo> captureInfo2 = std::make_shared<DCCaptureInfo>();
    captureInfo2->streamIds_.push_back(1);
    captureInfo2->width_ = TEST_WIDTH;
    captureInfo2->height_ = TEST_HEIGTH;
    captureInfo2->stride_ = 1;
    captureInfo2->format_ = 1;
    captureInfo2->dataspace_ = 1;
    captureInfo2->encodeType_ = ENCODE_TYPE_H265;
    captureInfo2->type_ = CONTINUOUS_FRAME;
    g_captureInfos.push_back(captureInfo1);
    g_captureInfos.push_back(captureInfo2);

    std::shared_ptr<DCameraSettings> camSettings1 = std::make_shared<DCameraSettings>();
    camSettings1->type_ = UPDATE_METADATA;
    camSettings1->value_ = "SettingValue";

    std::shared_ptr<DCameraSettings> camSettings2 = std::make_shared<DCameraSettings>();
    camSettings2->type_ = ENABLE_METADATA;
    camSettings2->value_ = "SettingValue";
    g_cameraSettings.push_back(camSettings1);
    g_cameraSettings.push_back(camSettings2);

    g_streamIds.push_back(1);
    g_streamIds.push_back(TEST_STREAMID);
}

void DCameraSourceInputTest::TearDownTestCase(void)
{
}

void DCameraSourceInputTest::SetUp(void)
{
    g_streamInfos.clear();
    g_captureInfos.clear();
    g_cameraSettings.clear();
    g_streamIds.clear();
    g_camIndexs.clear();

    SetStreamInfos();
    SetCaptureInfos();

    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    testInput_ = std::make_shared<DCameraSourceInput>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);
}

void DCameraSourceInputTest::TearDown(void)
{
    usleep(TEST_SLEEP_SEC);
    camDev_ = nullptr;
    stateListener_ = nullptr;
    testInput_ = nullptr;
}

/**
 * @tc.name: dcamera_source_input_test_001
 * @tc.desc: Verify source inptut Init.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_002
 * @tc.desc: Verify source inptut UnInit.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_003
 * @tc.desc: Verify source inptut ConfigStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DCStreamInfo>> streamInfos;
    rc = testInput_->ConfigStreams(streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_004
 * @tc.desc: Verify source inptut ReleaseStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    bool isAllRelease = true;
    rc = testInput_->ReleaseStreams(g_streamIds, isAllRelease);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_005
 * @tc.desc: Verify source inptut ReleaseAllStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->ReleaseAllStreams();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_006
 * @tc.desc: Verify source inptut StartCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->StartCapture(g_captureInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_007
 * @tc.desc: Verify source inptut StopCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->StartCapture(g_captureInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    bool isAllStop = true;
    rc = testInput_->StopCapture(g_streamIds, isAllStop);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_008
 * @tc.desc: Verify source inptut StopAllCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->StartCapture(g_captureInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->StopAllCapture();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_009
 * @tc.desc: Verify source inptut OpenChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_009, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = camDev_->InitDCameraSourceDev();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->OpenChannel(g_camIndexs);
    EXPECT_NE(rc, DCAMERA_OK);
    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_010
 * @tc.desc: Verify source inptut CloseChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_010, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = camDev_->InitDCameraSourceDev();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->OpenChannel(g_camIndexs);
    EXPECT_NE(rc, DCAMERA_OK);
    rc = testInput_->CloseChannel();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_011
 * @tc.desc: Verify source inptut UpdateSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_011, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UpdateSettings(g_cameraSettings);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_012
 * @tc.desc: Verify source inptut OnDataReceived.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_012, TestSize.Level1)
{
    int32_t rc = testInput_->Init();
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(buffer);
    testInput_->OnDataReceived(DCStreamType::SNAPSHOT_FRAME, buffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_013
 * @tc.desc: Verify source inptut EstablishSnapshotFrameSession.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_013, TestSize.Level1)
{
    int32_t rc = testInput_->Init();
    rc = camDev_->InitDCameraSourceDev();
    EXPECT_EQ(rc, DCAMERA_OK);

    testInput_->FinshFrameAsyncTrace(DCStreamType::CONTINUOUS_FRAME);
    testInput_->FinshFrameAsyncTrace(DCStreamType::SNAPSHOT_FRAME);
    int32_t state = 0;
    testInput_->OnSessionState(DCStreamType::CONTINUOUS_FRAME, state);
    state = 1;
    testInput_->OnSessionState(DCStreamType::CONTINUOUS_FRAME, state);
    state = 2;
    testInput_->OnSessionState(DCStreamType::CONTINUOUS_FRAME, state);
    rc = testInput_->EstablishSnapshotFrameSession(g_camIndexs);
    EXPECT_NE(rc, DCAMERA_OK);

    rc = testInput_->EstablishContinuousFrameSession(g_camIndexs);
    EXPECT_NE(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_014
 * @tc.desc: Verify source inptut EstablishSnapshotFrameSession.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_014, TestSize.Level1)
{
    auto testInputListener =
        std::make_shared<DCameraSourceInputChannelListener>(testInput_, DCStreamType::CONTINUOUS_FRAME);
    int32_t state = 0;
    std::string networkId = "networkId";
    testInputListener->OnSessionState(state, networkId);
    int32_t eventType = 0;
    int32_t eventReason = 1;
    std::string detail = "detail";
    testInputListener->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    size_t capacity = 0;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    testInputListener->OnDataReceived(buffers);
    EXPECT_EQ(true, capacity == 0);
}

/**
 * @tc.name: dcamera_source_input_test_015
 * @tc.desc: Verify source inptut UpdateWorkMode.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_015, TestSize.Level1)
{
    EXPECT_EQ(false, testInput_ == nullptr);

    int32_t rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    WorkModeParam param(12, 120, 0, false);
    rc = testInput_->UpdateWorkMode(param);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UnInit();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_input_test_016
 * @tc.desc: Verify source inptut UpdateSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputTest, dcamera_source_input_test_016, TestSize.Level1)
{
    auto metaData = std::make_shared<OHOS::Camera::CameraMetadata>(100, 200);
    std::vector<std::shared_ptr<DCameraSettings>> settingVectors;
    std::string settinStr = Camera::MetadataUtils::EncodeToString(metaData);
    std::shared_ptr<DCameraSettings> dCameraSettings = std::make_shared<DCameraSettings>();
    dCameraSettings->type_ = UPDATE_METADATA;
    dCameraSettings->value_ = Base64Encode(reinterpret_cast<const unsigned char *>(settinStr.c_str()),
        settinStr.length());
    settingVectors.push_back(dCameraSettings);

    int32_t rc = testInput_->UpdateSettings(settingVectors);
    EXPECT_EQ(rc, DCAMERA_WRONG_STATE);

    rc = testInput_->Init();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->StartCapture(g_captureInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testInput_->UpdateSettings(settingVectors);
    EXPECT_EQ(rc, DCAMERA_OK);
}

} // namespace DistributedHardware
} // namespace OHOS
