/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <chrono>
#include <gtest/gtest.h>
#include <memory>
#include <thread>

#define private public
#include "dcamera_sink_controller.h"
#include "dcamera_utils_tools.h"
#undef private

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "mock_camera_channel.h"
#include "mock_camera_operator.h"
#include "mock_dcamera_sink_output.h"
#include "dcamera_sink_callback.h"
#include "dcamera_handler.h"
#include "dcamera_metadata_setting_cmd.h"
#include "dcamera_sink_access_control.h"
#include "dcamera_sink_dev.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
std::string g_channelStr = "";
std::string g_outputStr = "";
std::string g_operatorStr = "";

static std::shared_ptr<AppExecFwk::EventRunner> g_eventRunner = nullptr;
class DCameraSinkControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();

    std::shared_ptr<DCameraSinkController> controller_;
    std::shared_ptr<ICameraSinkAccessControl> accessControl_;
};
std::string g_testDeviceIdController;

const int32_t TEST_TWENTY_MS = 20000;
const int32_t TEST_FIVE_S = 5;
const std::string SESSION_FLAG_CONTINUE = "dataContinue";
const std::string SESSION_FLAG_SNAPSHOT = "dataSnapshot";
const std::string TEST_DEVICE_ID_EMPTY = "";

const std::string TEST_CAPTURE_INFO_CMD_JSON = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1,
        "DataSpace": 1, "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

const std::string TEST_CHANNEL_INFO_CMD_CONTINUE_JSON = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId",
    "Detail": [{"DataSessionFlag": "dataContinue", "StreamType": 0}]}
})";

const std::string TEST_EVENT_CMD_JSON = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "STATE_NOTIFY",
    "Value": {"EventType": 1, "EventResult": 1, "EventContent": "TestContent"}
})";

const std::string TEST_INFO_CMD_JSON = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "GET_INFO",
    "Value": {"State": 1}
})";

const std::string TEST_METADATA_SETTING_CMD_JSON = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

const std::string TEST_OPEN_INFO_CMD_JSON = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId" }
})";

std::vector<DCameraIndex> g_testCamIndex;

void DCameraSinkControllerTest::SetUpTestCase(void)
{
    GetLocalDeviceNetworkId(g_testDeviceIdController);
    DCameraHandler::GetInstance().Initialize();
    std::vector<std::string> cameras = DCameraHandler::GetInstance().GetCameras();
    if (!cameras.empty()) {
        g_testCamIndex.push_back(DCameraIndex(g_testDeviceIdController, cameras[0]));
    }
    g_eventRunner = AppExecFwk::EventRunner::Create("DCSinkControllerTestRunner");
    ASSERT_NE(g_eventRunner, nullptr);
}

void DCameraSinkControllerTest::TearDownTestCase(void)
{
    g_eventRunner = nullptr;
}

void DCameraSinkControllerTest::SetUp(void)
{
    g_channelStr = "";
    g_outputStr = "";
    g_operatorStr = "";

    accessControl_ = std::make_shared<DCameraSinkAccessControl>();
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    controller_ = std::make_shared<DCameraSinkController>(accessControl_, sinkCallback);

    std::vector<std::string> cameras = DCameraHandler::GetInstance().GetCameras();
    controller_->channel_ = std::make_shared<MockCameraChannel>();
    controller_->operator_ = std::make_shared<MockCameraOperator>();
    if (!cameras.empty()) {
        controller_->output_ = std::make_shared<MockDCameraSinkOutput>(cameras[0], controller_->operator_);
        controller_->dhId_ = cameras[0];
    }
    controller_->srcDevId_ = g_testDeviceIdController;

    controller_->sinkCotrEventHandler_ =
        std::make_shared<DCameraSinkController::DCameraSinkContrEventHandler>(g_eventRunner, controller_);
}

void DCameraSinkControllerTest::TearDown(void)
{
    if (controller_ && controller_->operator_) {
        auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
        if (mockOperator && mockOperator->asyncOperationState.load() != 0) {
            mockOperator->ResetAsyncState();
            controller_->StopCapture();
            std::unique_lock<std::mutex> lock(mockOperator->mtx_);
            mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
                [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->stopCaptureState; });
        }
    }
    accessControl_ = nullptr;
    controller_ = nullptr;
}

void DCameraSinkControllerTest::SetTokenID()
{
    uint64_t tokenId;
    int32_t numberOfPermissions = 2;
    const char *perms[numberOfPermissions];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.CAMERA";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = numberOfPermissions,
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
 * @tc.name: dcamera_sink_controller_test_001
 * @tc.desc: Verify the Init and UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_001, TestSize.Level1)
{
    controller_->isInit_ = true;
    int32_t ret = controller_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(controller_->isInit_);
}

/**
 * @tc.name: dcamera_sink_controller_test_002
 * @tc.desc: Verify the GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_002, TestSize.Level1)
{
    DCameraInfoCmd cmd;
    cmd.value_ = std::make_shared<DCameraInfo>();
    int32_t ret = controller_->GetCameraInfo(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCAMERA_CHANNEL_STATE_DISCONNECTED, cmd.value_->state_);
}

/**
 * @tc.name: dcamera_sink_controller_test_003
 * @tc.desc: Verify the ChannelNeg function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_003, TestSize.Level1)
{
    DCameraChannelInfoCmd cmd;
    cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_CONTINUE_JSON);
    int32_t ret = controller_->ChannelNeg(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_004
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_004, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);
    int32_t mode = 0;
    int32_t ret = controller_->StartCapture(cmd.value_, mode);
    EXPECT_EQ(DCAMERA_OK, ret);

    // Wait for StartCapture to complete successfully
    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->commitCaptureState; });
        ASSERT_TRUE(waitResult);
    }
}

/**
 * @tc.name: dcamera_sink_controller_test_005
 * @tc.desc: Verify the UpdateSettings function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_005, TestSize.Level1)
{
    SetTokenID();
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    // Start and wait
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);
    controller_->StartCapture(cmd.value_, 0);
    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->commitCaptureState; });
        ASSERT_TRUE(waitResult);
    }
    
    // Then test UpdateSettings
    DCameraMetadataSettingCmd cmdMetadata;
    cmdMetadata.Unmarshal(TEST_METADATA_SETTING_CMD_JSON);
    int32_t ret = controller_->UpdateSettings(cmdMetadata.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_006
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_006, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);

    // Start and wait
    controller_->StartCapture(cmd.value_, 0);
    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->commitCaptureState; });
        ASSERT_TRUE(waitResult);
    }
    ASSERT_EQ(mockOperator->asyncOperationState.load(), mockOperator->commitCaptureState);

    // Stop and wait
    mockOperator->ResetAsyncState();
    controller_->StopCapture();
    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->stopCaptureState; });
        ASSERT_TRUE(waitResult);
    }
    ASSERT_EQ(mockOperator->asyncOperationState.load(), mockOperator->stopCaptureState);
}

/**
 * @tc.name: dcamera_sink_controller_test_007
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_007, TestSize.Level1)
{
    controller_->srcDevId_ = TEST_DEVICE_ID_EMPTY;
    DCameraEventCmd cmd;
    cmd.Unmarshal(TEST_EVENT_CMD_JSON);
    int32_t ret = controller_->DCameraNotify(cmd.value_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_008
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_008, TestSize.Level1)
{
    DCameraEventCmd cmd;
    cmd.Unmarshal(TEST_EVENT_CMD_JSON);
    // This test may fail if SA is not available. It's not a unit test of the controller logic itself.
    EXPECT_NE(DCAMERA_OK, controller_->DCameraNotify(cmd.value_));
}

/**
 * @tc.name: dcamera_sink_controller_test_009
 * @tc.desc: Verify the OnSessionState and GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_009, TestSize.Level1)
{
    controller_->OnSessionState(DCAMERA_CHANNEL_STATE_CONNECTING, "");
    DCameraInfoCmd cmd;
    cmd.value_ = std::make_shared<DCameraInfo>();
    int32_t ret = controller_->GetCameraInfo(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCAMERA_CHANNEL_STATE_CONNECTING, cmd.value_->state_);
}

/**
 * @tc.name: dcamera_sink_controller_test_010
 * @tc.desc: Verify the OnSessionState function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_010, TestSize.Level1)
{
    controller_->OnSessionState(DCAMERA_CHANNEL_STATE_CONNECTED, "");
    DCameraInfoCmd cmd;
    cmd.value_ = std::make_shared<DCameraInfo>();
    int32_t ret = controller_->GetCameraInfo(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCAMERA_CHANNEL_STATE_CONNECTED, cmd.value_->state_);
}

/**
 * @tc.name: dcamera_sink_controller_test_011
 * @tc.desc: Verify the OnSessionState function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_011, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    controller_->OnSessionState(DCAMERA_CHANNEL_STATE_DISCONNECTED, "");

    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    SUCCEED();
}

/**
 * @tc.name: dcamera_sink_controller_test_012
 * @tc.desc: Verify the OpenChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_012, TestSize.Level1)
{
    SetTokenID();
    DCameraOpenInfoCmd cmd;
    cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON);
    int32_t ret = controller_->OpenChannel(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_013
 * @tc.desc: Verify the OpenChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_013, TestSize.Level1)
{
    controller_->OnSessionState(DCAMERA_CHANNEL_STATE_CONNECTED, "");

    DCameraInfoCmd cmd;
    cmd.value_ = std::make_shared<DCameraInfo>();
    int32_t ret = controller_->GetCameraInfo(cmd.value_);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCAMERA_CHANNEL_STATE_CONNECTED, cmd.value_->state_);

    DCameraOpenInfoCmd cmdOpenInfo;
    cmdOpenInfo.Unmarshal(TEST_OPEN_INFO_CMD_JSON);
    ret = controller_->OpenChannel(cmdOpenInfo.value_);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_014
 * @tc.desc: Verify the CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_014, TestSize.Level1)
{
    int32_t ret = controller_->CloseChannel();
    EXPECT_EQ(DCAMERA_OK, ret);
    controller_->isPageStatus_.store(true);
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    EXPECT_EQ(true, controller_->isPageStatus_.load());
    controller_->sinkCallback_ = sinkCallback;
    ret = controller_->CloseChannel();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_015
 * @tc.desc: Verify the UpdateSettings function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_015, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    SetTokenID();
    
    // Start and wait for success first
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);
    controller_->StartCapture(cmd.value_, 0);
    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->commitCaptureState; });
        ASSERT_TRUE(waitResult);
    }

    // Then test the UpdateSettings failure case
    DCameraMetadataSettingCmd cmdMetadata;
    cmdMetadata.Unmarshal(TEST_METADATA_SETTING_CMD_JSON);
    g_operatorStr = "test015";
    int32_t ret = controller_->UpdateSettings(cmdMetadata.value_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_016
 * @tc.desc: Verify the OpenChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_016, TestSize.Level1)
{
    SetTokenID();
    DCameraOpenInfoCmd cmd;
    cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON);
    g_channelStr = "test016";
    int32_t ret = controller_->OpenChannel(cmd.value_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_017
 * @tc.desc: Verify the CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_017, TestSize.Level1)
{
    g_channelStr = "test017";
    int32_t ret = controller_->CloseChannel();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_018
 * @tc.desc: Verify the CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_018, TestSize.Level1)
{
    g_outputStr = "test018";
    int32_t ret = controller_->CloseChannel();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_019
 * @tc.desc: Verify the Init and UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_019, TestSize.Level1)
{
    g_outputStr = "test019";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    int32_t ret = controller_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_020
 * @tc.desc: Verify the Init and UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_020, TestSize.Level1)
{
    g_operatorStr = "test020";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    int32_t ret = controller_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_021
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_021, TestSize.Level1)
{
    g_outputStr = "test021";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);

    int32_t ret = controller_->StartCapture(cmd.value_, 0);
    EXPECT_EQ(DCAMERA_OK, ret);

    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->stopCaptureState; });
    }
}

/**
 * @tc.name: dcamera_sink_controller_test_022
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_022, TestSize.Level1)
{
    g_operatorStr = "test_prepare_fail";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);

    int32_t ret = controller_->StartCapture(cmd.value_, 0);
    EXPECT_EQ(DCAMERA_OK, ret);

    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->stopCaptureState; });
    }
}

/**
 * @tc.name: dcamera_sink_controller_test_023
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_023, TestSize.Level1)
{
    g_operatorStr = "test_prepare_fail";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);

    int32_t ret = controller_->StartCapture(cmd.value_, 0);
    EXPECT_EQ(DCAMERA_OK, ret);

    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->stopCaptureState; });
    }
}

/**
 * @tc.name: dcamera_sink_controller_test_024
 * @tc.desc: Verify the PullUpPage function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_024, TestSize.Level1)
{
    g_operatorStr = "test024";
    controller_->isSensitive_ = false;
    EXPECT_EQ(false, controller_->isSensitive_);
    usleep(TEST_TWENTY_MS);
}

/**
 * @tc.name: dcamera_sink_controller_test_025
 * @tc.desc: Verify the PullUpPage function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_025, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    DCameraCaptureInfoCmd cmd;
    cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON);
    auto infos = std::make_shared<std::vector<std::shared_ptr<DCameraCaptureInfo>>>(cmd.value_);

    controller_->PostAuthorization(*infos);

    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState.load() != 0; });
        ASSERT_TRUE(waitResult) << "wait PostAuthorization timeout";
    }

    EXPECT_EQ(mockOperator->asyncOperationState.load(), mockOperator->commitCaptureState);
}
/**
 * @tc.name: dcamera_sink_controller_test_026
 * @tc.desc: Verify the Init and UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_026, TestSize.Level1)
{
    g_outputStr = "test026";
    auto outputMock = std::make_shared<MockDCameraSinkOutput>("camera_1", nullptr);
    EXPECT_NE(DCAMERA_OK, outputMock->Init());
    usleep(TEST_TWENTY_MS);
}

/**
 * @tc.name: dcamera_sink_controller_test_027
 * @tc.desc: Verify the Init and UnInit function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_027, TestSize.Level1)
{
    g_operatorStr = "test027";
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();

    int32_t ret = controller_->UnInit();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_controller_test_028
 * @tc.desc: Verify HandleReceivedData function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_028, TestSize.Level1)
{
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->HandleReceivedData(dataBuffer));
}

/**
 * @tc.name: dcamera_sink_controller_test_029
 * @tc.desc: Verify PauseDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_029, TestSize.Level1)
{
    std::string netId = "";
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->PauseDistributedHardware(netId));
    netId = "netId";
    EXPECT_EQ(DCAMERA_OK, controller_->PauseDistributedHardware(netId));
    g_operatorStr = "test029";
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->PauseDistributedHardware(netId));
    g_operatorStr = "";
    controller_->operator_ = nullptr;
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->PauseDistributedHardware(netId));
}

/**
 * @tc.name: dcamera_sink_controller_test_030
 * @tc.desc: Verify ResumeDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_030, TestSize.Level1)
{
    std::string netId = "";
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->ResumeDistributedHardware(netId));
    netId = "netId";
    EXPECT_EQ(DCAMERA_OK, controller_->ResumeDistributedHardware(netId));
    g_operatorStr = "test030";
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->ResumeDistributedHardware(netId));
    g_operatorStr = "";
    controller_->operator_ = nullptr;
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->ResumeDistributedHardware(netId));
}

/**
 * @tc.name: dcamera_sink_controller_test_031
 * @tc.desc: Verify StopDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_031, TestSize.Level1)
{
    std::string netId = "";
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->StopDistributedHardware(netId));
    netId = "netId";
    EXPECT_NE(DCAMERA_OK, controller_->StopDistributedHardware(netId));
}

/**
 * @tc.name: dcamera_sink_controller_test_032
 * @tc.desc: Verify CheckDeviceSecurityLevel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_032, TestSize.Level1)
{
    std::string srcNetId = "";
    std::string dstNetId = "";
    EXPECT_NE(true, controller_->CheckDeviceSecurityLevel(srcNetId, dstNetId));

    srcNetId = "abc";
    dstNetId = "";
    EXPECT_NE(true, controller_->CheckDeviceSecurityLevel(srcNetId, dstNetId));

    srcNetId = "";
    dstNetId = "abc";
    EXPECT_NE(true, controller_->CheckDeviceSecurityLevel(srcNetId, dstNetId));

    srcNetId = "abc";
    dstNetId = "abc";
    EXPECT_NE(true, controller_->CheckDeviceSecurityLevel(srcNetId, dstNetId));
}

/**
 * @tc.name: dcamera_sink_controller_test_034
 * @tc.desc: Verify GetUdidByNetworkId function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_034, TestSize.Level1)
{
    std::string netId = "";
    EXPECT_EQ("", controller_->GetUdidByNetworkId(netId));
    netId = "netId";
    EXPECT_EQ("", controller_->GetUdidByNetworkId(netId));
}
/**
 * @tc.name: dcamera_sink_controller_test_035
 * @tc.desc: Verify HandleReceivedData handles invalid buffers and unknown commands gracefully.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_035, TestSize.Level1)
{
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->HandleReceivedData(dataBuffer));

    cJSON *metaJson = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson, "Command", "an_unknown_command");
    char* jsonStr = cJSON_PrintUnformatted(metaJson);
    dataBuffer = std::make_shared<DataBuffer>(strlen(jsonStr) + 1);
    memcpy_s(dataBuffer->Data(), dataBuffer->Capacity(), jsonStr, strlen(jsonStr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, controller_->HandleReceivedData(dataBuffer));
    free(jsonStr);
    cJSON_Delete(metaJson);
}
/**
 * @tc.name: dcamera_sink_controller_test_036
 * @tc.desc: Verify the asynchronous process of HandleReceivedData with a valid CAPTURE command.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_036, TestSize.Level1)
{
    auto mockOperator = std::static_pointer_cast<MockCameraOperator>(controller_->operator_);
    mockOperator->ResetAsyncState();
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(TEST_CAPTURE_INFO_CMD_JSON.length() + 1);
    memcpy_s(dataBuffer->Data(), dataBuffer->Capacity(),
             TEST_CAPTURE_INFO_CMD_JSON.c_str(), TEST_CAPTURE_INFO_CMD_JSON.length());

    int32_t ret = controller_->HandleReceivedData(dataBuffer);
    EXPECT_EQ(DCAMERA_OK, ret);

    {
        std::unique_lock<std::mutex> lock(mockOperator->mtx_);
        bool waitResult = mockOperator->cv_.wait_for(lock, std::chrono::seconds(TEST_FIVE_S),
            [&mockOperator] { return mockOperator->asyncOperationState == mockOperator->commitCaptureState; });
        ASSERT_TRUE(waitResult);
    }
}
/**
 * @tc.name: dcamera_sink_controller_test_037
 * @tc.desc: Verify function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerTest, dcamera_sink_controller_test_037, TestSize.Level1)
{
    EXPECT_TRUE(controller_->CheckAclRight());
    controller_->userId_ = 100;
    EXPECT_FALSE(controller_->CheckAclRight());
}
} // namespace DistributedHardware
} // namespace OHOS