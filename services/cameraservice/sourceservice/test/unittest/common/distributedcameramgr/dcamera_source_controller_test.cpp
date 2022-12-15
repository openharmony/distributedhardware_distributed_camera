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

#include <gtest/gtest.h>
#define private public
#include "dcamera_source_state.h"
#undef private

#include "dcamera_source_controller.h"
#include "dcamera_utils_tools.h"
#include "icamera_state_listener.h"
#include "dcamera_source_controller_channel_listener.h"
#include "distributed_camera_errno.h"
#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<EventBus> eventBus_;
    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
    std::shared_ptr<DCameraSourceStateMachine> stateMachine_;
    std::shared_ptr<ICameraController> controller_;
    std::vector<DCameraIndex> indexs_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_FORMAT = 4;
const int32_t TEST_DATASPACE = 8;
const int32_t TEST_ISCAPTURE = 0;
const int32_t TEST_SLEEP_SEC = 200000;
}

void DCameraSourceControllerTest::SetUpTestCase(void)
{
}

void DCameraSourceControllerTest::TearDownTestCase(void)
{
}

void DCameraSourceControllerTest::SetUp(void)
{
    eventBus_ = std::make_shared<EventBus>("SrcController");
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    stateMachine_ = std::make_shared<DCameraSourceStateMachine>(camDev_);
    controller_ = std::make_shared<DCameraSourceController>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateMachine_,
        eventBus_);
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs_.push_back(index);
}

void DCameraSourceControllerTest::TearDown(void)
{
    usleep(TEST_SLEEP_SEC);
    eventBus_ = nullptr;
    stateMachine_ = nullptr;
    camDev_ = nullptr;
    stateListener_ = nullptr;
    controller_ = nullptr;
}

/**
 * @tc.name: dcamera_source_controller_test_001
 * @tc.desc: Verify source controller Init.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_001, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_OK);
}
/**
 * @tc.name: dcamera_source_controller_test_002
 * @tc.desc: Verify source controller UnInit.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_002, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_OK);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}
/**
 * @tc.name: dcamera_source_controller_test_003
 * @tc.desc: Verify source controller StartCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    std::shared_ptr<DCameraCaptureInfo> capture = std::make_shared<DCameraCaptureInfo>();
    capture->width_ = TEST_WIDTH;
    capture->height_ = TEST_HEIGTH;
    capture->format_ = TEST_FORMAT;
    capture->dataspace_ = TEST_DATASPACE;
    capture->isCapture_ = TEST_ISCAPTURE;
    capture->encodeType_ = DCEncodeType::ENCODE_TYPE_H264;
    capture->streamType_ = DCStreamType::SNAPSHOT_FRAME;
    captureInfos.push_back(capture);

    controller_->Init(indexs_);
    int32_t ret = controller_->StartCapture(captureInfos);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}
/**
 * @tc.name: dcamera_source_controller_test_004
 * @tc.desc: Verify source controller StartCapture and StopCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_004, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    std::shared_ptr<DCameraCaptureInfo> capture = std::make_shared<DCameraCaptureInfo>();
    capture->width_ = TEST_WIDTH;
    capture->height_ = TEST_HEIGTH;
    capture->format_ = TEST_FORMAT;
    capture->dataspace_ = TEST_DATASPACE;
    capture->isCapture_ = TEST_ISCAPTURE;
    capture->encodeType_ = DCEncodeType::ENCODE_TYPE_H264;
    capture->streamType_ = DCStreamType::SNAPSHOT_FRAME;
    captureInfos.push_back(capture);
    controller_->Init(indexs_);
    int32_t ret = controller_->StartCapture(captureInfos);

    ret = controller_->StopCapture();
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_005
 * @tc.desc: Verify source controller ChannelNeg.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_005, TestSize.Level1)
{
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    int32_t ret = GetLocalDeviceNetworkId(chanInfo->sourceDevId_);
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    DCameraChannelDetail snapShotChInfo(SNAP_SHOT_SESSION_FLAG, SNAPSHOT_FRAME);
    chanInfo->detail_.push_back(continueChInfo);
    chanInfo->detail_.push_back(snapShotChInfo);
    controller_->Init(indexs_);
    ret = controller_->ChannelNeg(chanInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_006
 * @tc.desc: Verify source controller UpdateSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_006, TestSize.Level1)
{
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
    setting->type_ = DCSettingsType::DISABLE_METADATA;
    setting->value_ = "UpdateSettingsTest";
    settings.push_back(setting);

    controller_->Init(indexs_);
    int32_t ret = controller_->UpdateSettings(settings);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_007
 * @tc.desc: Verify source controller GetCameraInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_007, TestSize.Level1)
{
    controller_->Init(indexs_);
    std::shared_ptr<DCameraInfo> camInfo = std::make_shared<DCameraInfo>();
    camInfo->state_ = 1;
    int32_t ret = controller_->GetCameraInfo(camInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_008
 * @tc.desc: Verify source controller OpenChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_008, TestSize.Level1)
{
    controller_->Init(indexs_);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    int32_t ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    ret = controller_->OpenChannel(openInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_009
 * @tc.desc: Verify source controller OpenChannel and CloseChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_009, TestSize.Level1)
{
    controller_->Init(indexs_);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    int32_t ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    controller_->OpenChannel(openInfo);
    ret = controller_->CloseChannel();
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_0010
 * @tc.desc: Verify source controller OnDataReceived.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_0010, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    std::shared_ptr<DCameraSourceController> controller = std::make_shared<DCameraSourceController>(TEST_DEVICE_ID,
        TEST_CAMERA_DH_ID_0, stateMachine_, eventBus_);
    std::shared_ptr<ICameraChannelListener> listener_ =
        std::make_shared<DCameraSourceControllerChannelListener>(controller);
    int32_t state = 0;
    listener_->OnSessionState(state);
    int32_t eventType = 1;
    int32_t eventReason = 1;
    std::string detail = "OnSessionErrorTest";
    listener_->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    listener_->OnDataReceived(buffers);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}
}
}