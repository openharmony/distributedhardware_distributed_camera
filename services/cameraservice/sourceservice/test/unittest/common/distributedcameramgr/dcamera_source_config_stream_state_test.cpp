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
#include "dcamera_source_config_stream_state.h"
#include "dcamera_source_opened_state.h"
#include "dcamera_source_capture_state.h"
#undef private

#include "dcamera_source_state.h"
#include "dcamera_source_regist_state.h"
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
class DCameraSourceConfigStreamStateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
    std::shared_ptr<DCameraSourceStateMachine> stateMachine_;
    std::shared_ptr<DCameraRegistParam> g_registParam;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "0xFFFF";
const std::string TEST_SINK_PARAM = "0xFFFF";
const std::string TEST_SRC_PARAM = "0xFFFF";
}

void DCameraSourceConfigStreamStateTest::SetUpTestCase(void)
{
}

void DCameraSourceConfigStreamStateTest::TearDownTestCase(void)
{
}

void DCameraSourceConfigStreamStateTest::SetUp(void)
{
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    stateMachine_ = std::make_shared<DCameraSourceStateMachine>(camDev_);
    g_registParam = std::make_shared<DCameraRegistParam>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID,
        TEST_SINK_PARAM, TEST_SRC_PARAM);
}

void DCameraSourceConfigStreamStateTest::TearDown(void)
{
    stateMachine_ = nullptr;
    camDev_ = nullptr;
    stateListener_ = nullptr;
    g_registParam = nullptr;
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_001
 * @tc.desc: Verify source DoUnregisterTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_001, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    std::shared_ptr<DCameraSourceConfigStreamState> configstate =
        std::make_shared<DCameraSourceConfigStreamState>(stateMachine_);
    int32_t ret = configstate->DoUnregisterTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_002
 * @tc.desc: Verify source DoUpdateSettingsTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_002, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    std::shared_ptr<DCameraSourceConfigStreamState> configstate =
        std::make_shared<DCameraSourceConfigStreamState>(stateMachine_);
    int32_t ret = configstate->DoUpdateSettingsTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_003
 * @tc.desc: Verify source DoEventNofityTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_003, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_REGIST, g_registParam);
    std::shared_ptr<DCameraSourceConfigStreamState> configstate =
        std::make_shared<DCameraSourceConfigStreamState>(stateMachine_);
    int32_t ret = configstate->DoEventNofityTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_004
 * @tc.desc: Verify source DoUnregisterTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_004, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceOpenedState> openState = std::make_shared<DCameraSourceOpenedState>(stateMachine_);
    int32_t ret = openState->DoUnregisterTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_005
 * @tc.desc: Verify source DoUpdateSettingsTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_005, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceOpenedState> openState = std::make_shared<DCameraSourceOpenedState>(stateMachine_);
    int32_t ret = openState->DoUpdateSettingsTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_006
 * @tc.desc: Verify source DoEventNofityTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_006, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceOpenedState> openState = std::make_shared<DCameraSourceOpenedState>(stateMachine_);
    int32_t ret = openState->DoEventNofityTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_007
 * @tc.desc: Verify source DoUnregisterTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_007, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceCaptureState> captureState =
        std::make_shared<DCameraSourceCaptureState>(stateMachine_);
    int32_t ret = captureState->DoUnregisterTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_008
 * @tc.desc: Verify source DoStartCaptureTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_008, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceCaptureState> captureState =
        std::make_shared<DCameraSourceCaptureState>(stateMachine_);
    int32_t ret = captureState->DoStartCaptureTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_009
 * @tc.desc: Verify source DoStopCaptureTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_009, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceCaptureState> captureState =
        std::make_shared<DCameraSourceCaptureState>(stateMachine_);
    int32_t ret = captureState->DoStopCaptureTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_010
 * @tc.desc: Verify source DoUpdateSettingsTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_010, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceCaptureState> captureState =
        std::make_shared<DCameraSourceCaptureState>(stateMachine_);
    int32_t ret = captureState->DoUpdateSettingsTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_config_stream_state_test_011
 * @tc.desc: Verify source DoEventNofityTask state.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSourceConfigStreamStateTest, dcamera_source_config_stream_state_test_011, TestSize.Level1)
{
    DCameraSourceEvent event0(DCAMERA_EVENT_OPEN, g_registParam);
    std::shared_ptr<DCameraSourceCaptureState> captureState =
        std::make_shared<DCameraSourceCaptureState>(stateMachine_);
    int32_t ret = captureState->DoEventNofityTask(camDev_, event0);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
