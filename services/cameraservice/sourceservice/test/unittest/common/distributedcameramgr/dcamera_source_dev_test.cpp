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

#include "dcamera_source_dev.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceDevTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "";
const std::string TEST_VER = "1.0";
const std::string TEST_ATTRS = "";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_SLEEP_SEC = 200000;
std::string TEST_EVENT_CMD_JSON = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "STATE_NOTIFY",
    "Value": {"EventType": 1, "EventResult": 1, "EventContent": "TestContent"}
})";
}

void DCameraSourceDevTest::SetUpTestCase(void)
{
}

void DCameraSourceDevTest::TearDownTestCase(void)
{
}

void DCameraSourceDevTest::SetUp(void)
{
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
}

void DCameraSourceDevTest::TearDown(void)
{
    usleep(TEST_SLEEP_SEC);
    camDev_ = nullptr;
    stateListener_ = nullptr;
}

/**
 * @tc.name: dcamera_source_dev_test_001
 * @tc.desc: Verify source dev InitDCameraSourceDev.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_001, TestSize.Level1)
{
    int32_t ret = camDev_->InitDCameraSourceDev();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_002
 * @tc.desc: Verify source dev RegisterDistributedHardware.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_002, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->RegisterDistributedHardware(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID,
        TEST_VER, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_003
 * @tc.desc: Verify source dev UnRegisterDistributedHardware.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_003, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->UnRegisterDistributedHardware(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_004
 * @tc.desc: Verify source dev DCameraNotify.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_004, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->DCameraNotify(TEST_EVENT_CMD_JSON);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_005
 * @tc.desc: Verify source dev OpenSession.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_005, TestSize.Level1)
{
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->OpenSession(index);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_006
 * @tc.desc: Verify source dev CloseSession.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_006, TestSize.Level1)
{
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->CloseSession(index);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_007
 * @tc.desc: Verify source dev ConfigCameraStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_007, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCStreamInfo>> streamInfos;
    std::shared_ptr<DCStreamInfo> streamInfo = std::make_shared<DCStreamInfo>();
    streamInfo->streamId_ = 1;
    streamInfo->width_ = TEST_WIDTH;
    streamInfo->height_ = TEST_HEIGTH;
    streamInfo->stride_ = 1;
    streamInfo->format_ = 1;
    streamInfo->dataspace_ = 1;
    streamInfo->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo->type_ = SNAPSHOT_FRAME;
    streamInfos.push_back(streamInfo);
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->ConfigCameraStreams(streamInfos);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_008
 * @tc.desc: Verify source dev ReleaseCameraStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_008, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCStreamInfo>> streamInfos;
    std::vector<int> streamIds;
    int streamId = 1;
    streamIds.push_back(streamId);
    std::shared_ptr<DCStreamInfo> streamInfo = std::make_shared<DCStreamInfo>();
    streamInfo->streamId_ = 1;
    streamInfo->width_ = TEST_WIDTH;
    streamInfo->height_ = TEST_HEIGTH;
    streamInfo->stride_ = 1;
    streamInfo->format_ = 1;
    streamInfo->dataspace_ = 1;
    streamInfo->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo->type_ = SNAPSHOT_FRAME;
    streamInfos.push_back(streamInfo);
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->ConfigCameraStreams(streamInfos);
    ret = camDev_->ReleaseCameraStreams(streamIds);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_009
 * @tc.desc: Verify source dev StartCameraCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_009, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCCaptureInfo>> captureInfos;
    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    captureInfo->streamIds_.push_back(1);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    captureInfos.push_back(captureInfo);
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->StartCameraCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_010
 * @tc.desc: Verify source dev StopCameraCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_010, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCCaptureInfo>> captureInfos;
    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    int streamId = 1;
    captureInfo->streamIds_.push_back(streamId);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    captureInfos.push_back(captureInfo);
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->StartCameraCapture(captureInfos);
    std::vector<int> streamIds;
    streamIds.push_back(streamId);
    ret = camDev_->StopCameraCapture(streamIds);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_011
 * @tc.desc: Verify source dev UpdateCameraSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_011, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCCaptureInfo>> captureInfos;
    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    int streamId = 1;
    captureInfo->streamIds_.push_back(streamId);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    captureInfos.push_back(captureInfo);
    camDev_->InitDCameraSourceDev();
    int32_t ret = camDev_->StartCameraCapture(captureInfos);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
    setting->type_ = DCSettingsType::DISABLE_METADATA;
    setting->value_ = "UpdateSettingsTest";
    settings.push_back(setting);
    ret = camDev_->UpdateCameraSettings(settings);
    std::vector<int> streamIds;
    streamIds.push_back(streamId);
    camDev_->StopCameraCapture(streamIds);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_012
 * @tc.desc: Verify source dev GetStateInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_012, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
    DCameraSourceEvent event(*camDev_, DCAMERA_EVENT_NOFIFY, camEvent);
    camDev_->InitDCameraSourceDev();
    camDev_->OnEvent(event);
    camDev_->GetVersion();
    int32_t ret = camDev_->GetStateInfo();
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}