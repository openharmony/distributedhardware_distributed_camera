/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include <memory>
#define private public
#include "dcamera_source_state.h"
#undef private

#include "dcamera_source_capture_state.h"
#include "dcamera_source_config_stream_state.h"
#include "dcamera_source_init_state.h"
#include "dcamera_source_opened_state.h"
#include "dcamera_source_regist_state.h"
#include "dcamera_source_state_factory.h"
#include "dcamera_source_state_machine.h"
#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
std::string g_regisStateStr = "";
std::string g_openStateStr = "";
std::string g_captureStateStr = "";
class DCameraSourceStateMachineTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
    std::shared_ptr<DCameraSourceStateMachine> stateMachine_;

private:
    static void SetStreamInfos();
    static void SetCaptureInfos();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "0xFFFF";
const std::string TEST_SINK_PARAM = "0xFFFF";
const std::string TEST_SRC_PARAM = "0xFFFF";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_STREAMID = 2;
}

std::vector<std::shared_ptr<DCStreamInfo>> g_streamInfosSnap;
std::vector<std::shared_ptr<DCCaptureInfo>> g_captureInfoSnap;
std::vector<std::shared_ptr<DCameraSettings>> g_cameraSettingSnap;
std::vector<int> g_streamIdSnap;
std::shared_ptr<DCameraEvent> g_camEvent;
std::shared_ptr<DCameraRegistParam> g_registParam;
DCameraIndex g_camIndex;

void DCameraSourceStateMachineTest::SetUpTestCase(void)
{
}

void DCameraSourceStateMachineTest::SetStreamInfos()
{
    g_camIndex.devId_ = TEST_DEVICE_ID;
    g_camIndex.dhId_ = TEST_CAMERA_DH_ID_0;
    g_camEvent = std::make_shared<DCameraEvent>();
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
    streamInfo2->type_ = SNAPSHOT_FRAME;
    g_streamInfosSnap.push_back(streamInfo1);
    g_streamInfosSnap.push_back(streamInfo2);
}

void DCameraSourceStateMachineTest::SetCaptureInfos()
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
    g_captureInfoSnap.push_back(captureInfo1);
    g_captureInfoSnap.push_back(captureInfo2);

    std::shared_ptr<DCameraSettings> camSettings1 = std::make_shared<DCameraSettings>();
    camSettings1->type_ = UPDATE_METADATA;
    camSettings1->value_ = "SettingValue";

    std::shared_ptr<DCameraSettings> camSettings2 = std::make_shared<DCameraSettings>();
    camSettings2->type_ = ENABLE_METADATA;
    camSettings2->value_ = "SettingValue";
    g_cameraSettingSnap.push_back(camSettings1);
    g_cameraSettingSnap.push_back(camSettings2);

    g_streamIdSnap.push_back(1);
    g_streamIdSnap.push_back(TEST_STREAMID);

    g_registParam = std::make_shared<DCameraRegistParam>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID,
        TEST_SINK_PARAM, TEST_SRC_PARAM);
}

void DCameraSourceStateMachineTest::TearDownTestCase(void)
{
}

void DCameraSourceStateMachineTest::SetUp(void)
{
    g_streamInfosSnap.clear();
    g_captureInfoSnap.clear();
    g_cameraSettingSnap.clear();
    g_streamIdSnap.clear();
    g_camEvent = nullptr;
    g_registParam = nullptr;
    g_camIndex = {}; // Reset struct to default
    g_regisStateStr = "";
    g_openStateStr = "";
    g_captureStateStr = "";

    SetStreamInfos();
    SetCaptureInfos();

    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    stateMachine_ = std::make_shared<DCameraSourceStateMachine>(camDev_);
}

void DCameraSourceStateMachineTest::TearDown(void)
{
    stateMachine_ = nullptr;
    camDev_ = nullptr;
    stateListener_ = nullptr;
}

/**
 * @tc.name: dcamera_source_state_machine_test_001
 * @tc.desc: Verify source init state.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_001, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_002
 * @tc.desc: Verify source regist state.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_002, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_OPEN, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event9(DCAMERA_EVENT_NOFIFY, g_camEvent);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_OPEN, event2);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event9);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_003
 * @tc.desc: Verify source open state.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MD
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_003, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_OPEN, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event4(DCAMERA_EVENT_CONFIG_STREAMS, g_streamInfosSnap);
    DCameraSourceEvent event5(DCAMERA_EVENT_RELEASE_STREAMS, g_streamIdSnap);
    DCameraSourceEvent event8(DCAMERA_EVENT_UPDATE_SETTINGS, g_cameraSettingSnap);
    DCameraSourceEvent event9(DCAMERA_EVENT_NOFIFY, g_camEvent);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_OPEN, event2);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CONFIG_STREAMS, event4);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_RELEASE_STREAMS, event5);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event9);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UPDATE_SETTINGS, event8);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_004
 * @tc.desc: Verify source config Stream state.
 * @tc.type: FUNC
 * @tc.require: AR000GK6ME
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_004, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_OPEN, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event4(DCAMERA_EVENT_CONFIG_STREAMS, g_streamInfosSnap);
    DCameraSourceEvent event5(DCAMERA_EVENT_RELEASE_STREAMS, g_streamIdSnap);
    DCameraSourceEvent event6(DCAMERA_EVENT_START_CAPTURE, g_captureInfoSnap);
    DCameraSourceEvent event7(DCAMERA_EVENT_STOP_CAPTURE, g_streamIdSnap);
    DCameraSourceEvent event8(DCAMERA_EVENT_UPDATE_SETTINGS, g_cameraSettingSnap);
    DCameraSourceEvent event9(DCAMERA_EVENT_NOFIFY, g_camEvent);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_OPEN, event2);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CONFIG_STREAMS, event4);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_START_CAPTURE, event6);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event9);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UPDATE_SETTINGS, event8);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_STOP_CAPTURE, event7);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_RELEASE_STREAMS, event5);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_005
 * @tc.desc: Verify source capture state.
 * @tc.type: FUNC
 * @tc.require: AR000GK6ME
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_005, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_OPEN, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event6(DCAMERA_EVENT_START_CAPTURE, g_captureInfoSnap);
    DCameraSourceEvent event7(DCAMERA_EVENT_STOP_CAPTURE, g_streamIdSnap);
    DCameraSourceEvent event8(DCAMERA_EVENT_UPDATE_SETTINGS, g_cameraSettingSnap);
    DCameraSourceEvent event9(DCAMERA_EVENT_NOFIFY, g_camEvent);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_OPEN, event2);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_START_CAPTURE, event6);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event9);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UPDATE_SETTINGS, event8);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_STOP_CAPTURE, event7);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_006
 * @tc.desc: Verify source state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_006, TestSize.Level1)
{
    int32_t ret = DCAMERA_OK;
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);
    stateMachine_ ->UpdateState(static_cast<DCameraStateType>(-1));
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_camIndex);
    ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_007
 * @tc.desc: Verify source regist state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_007, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_camIndex);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_camIndex);
    DCameraSourceEvent event2(DCAMERA_EVENT_NOFIFY, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_GET_FULLCAPS, g_camIndex);
    DCameraSourceEvent event4(static_cast<DCAMERA_EVENT>(-1), g_camIndex);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event2);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_GET_FULLCAPS, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(static_cast<DCAMERA_EVENT>(-1), event4);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_008
 * @tc.desc: Verify source regist state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_008, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_NOFIFY, g_camEvent);
    DCameraSourceEvent event3(DCAMERA_EVENT_OPEN, g_camIndex);

    g_regisStateStr = "test008";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event2);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_OPEN, event3);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_009
 * @tc.desc: Verify source open state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_009, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_camIndex);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_camIndex);
    DCameraSourceEvent event2(DCAMERA_EVENT_CONFIG_STREAMS, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_RELEASE_STREAMS, g_streamIdSnap);
    DCameraSourceEvent event4(static_cast<DCAMERA_EVENT>(-1), g_camIndex);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CONFIG_STREAMS, event2);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_RELEASE_STREAMS, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(static_cast<DCAMERA_EVENT>(-1), event4);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_010
 * @tc.desc: Verify source open state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_010, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_CONFIG_STREAMS, g_streamInfosSnap);
    DCameraSourceEvent event3(DCAMERA_EVENT_UPDATE_SETTINGS, g_cameraSettingSnap);
    DCameraSourceEvent event4(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event5(DCAMERA_EVENT_NOFIFY, g_camEvent);

    g_openStateStr = "test010";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CONFIG_STREAMS, event2);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UPDATE_SETTINGS, event3);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event4);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event4);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_011
 * @tc.desc: Verify source open state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_011, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_openStateStr = "test011";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_012
 * @tc.desc: Verify source open state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_012, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_openStateStr = "test012";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_013
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_013, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event4(static_cast<DCAMERA_EVENT>(-1), g_camIndex);
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);
    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_REGIST, event0);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = stateMachine_ ->Execute(static_cast<DCAMERA_EVENT>(-1), event4);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_014
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_014, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);
    DCameraSourceEvent event2(DCAMERA_EVENT_OPEN, g_camIndex);
    DCameraSourceEvent event3(DCAMERA_EVENT_CLOSE, g_camIndex);
    DCameraSourceEvent event4(DCAMERA_EVENT_START_CAPTURE, g_captureInfoSnap);
    DCameraSourceEvent event5(DCAMERA_EVENT_STOP_CAPTURE, g_streamIdSnap);
    DCameraSourceEvent event8(DCAMERA_EVENT_UPDATE_SETTINGS, g_cameraSettingSnap);
    DCameraSourceEvent event9(DCAMERA_EVENT_NOFIFY, g_camEvent);

    g_captureStateStr = "test014";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event3);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_START_CAPTURE, event4);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_STOP_CAPTURE, event5);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_UPDATE_SETTINGS, event8);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = stateMachine_ ->Execute(DCAMERA_EVENT_NOFIFY, event9);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_015
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_015, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_captureStateStr = "test015";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_016
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_016, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_captureStateStr = "test016";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_017
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_017, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_captureStateStr = "test017";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_018
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_018, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_UNREGIST, g_registParam);

    g_captureStateStr = "test018";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_UNREGIST, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_019
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_019, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_CLOSE, g_camIndex);

    g_captureStateStr = "test019";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_state_machine_test_020
 * @tc.desc: Verify source capture state machine.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MC
 */
HWTEST_F(DCameraSourceStateMachineTest, dcamera_source_state_machine_test_020, TestSize.Level1)
{
    DCameraSourceEvent event1(DCAMERA_EVENT_CLOSE, g_camIndex);

    g_captureStateStr = "test020";
    stateMachine_ ->UpdateState(DCAMERA_STATE_INIT);
    stateMachine_ ->UpdateState(DCAMERA_STATE_REGIST);
    stateMachine_ ->UpdateState(DCAMERA_STATE_OPENED);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CONFIG_STREAM);
    stateMachine_ ->UpdateState(DCAMERA_STATE_CAPTURE);

    int32_t ret = stateMachine_ ->Execute(DCAMERA_EVENT_CLOSE, event1);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
