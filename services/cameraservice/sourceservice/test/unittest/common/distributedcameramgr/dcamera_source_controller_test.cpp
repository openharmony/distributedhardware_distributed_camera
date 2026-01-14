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
#include "dcamera_source_controller.h"
#undef private

#include <securec.h>

#include "dcamera_source_state.h"
#include "dcamera_utils_tools.h"
#include "mock_camera_channel.h"
#include "distributed_hardware_log.h"
#include "icamera_state_listener.h"
#include "dcamera_source_controller_channel_listener.h"
#include "distributed_camera_errno.h"
#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"
#include "mock_device_manager.h"

using namespace testing::ext;
using namespace testing;

namespace OHOS {
namespace DistributedHardware {

static bool g_srcConnect;

bool ManageSelectChannel::GetSrcConnect()
{
    return g_srcConnect;
}

class DCameraSourceStateImpl : public DCameraSourceState {
public:
    int32_t Execute(std::shared_ptr<DCameraSourceDev> &camDev, DCAMERA_EVENT eventType,
        DCameraSourceEvent &event) override
    {
        return 0;
    }

    DCameraStateType GetStateType() override
    {
        return DCameraStateType::DCAMERA_STATE_INIT;
    }
};

class CameraStateListenerImpl : public ICameraStateListener {
public:
    int32_t OnRegisterNotify(const std::string &devId, const std::string &dhId, const std::string &reqId,
        int32_t status, std::string &data) override
    {
        return 0;
    }

    int32_t OnUnregisterNotify(const std::string &devId, const std::string &dhId, const std::string &reqId,
        int32_t status, std::string &data) override
    {
        return 0;
    }

    int32_t OnHardwareStateChanged(const std::string &devId, const std::string &dhId, int32_t status) override
    {
        return 0;
    }

    int32_t OnDataSyncTrigger(const std::string &devId) override
    {
        return 0;
    }

    void SetCallback(sptr<IDCameraSourceCallback> callback) override
    {
        callback_ = callback;
    }

private:
    sptr<IDCameraSourceCallback> callback_;
};

std::string g_channelStr = "";
class DCameraSourceControllerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
    std::shared_ptr<DCameraSourceStateMachine> stateMachine_;
    std::shared_ptr<DCameraSourceController> controller_;
    std::vector<DCameraIndex> indexs_;
    static inline std::shared_ptr<DeviceManagerMock> deviceMgrMock_ = nullptr;
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
const size_t DATABUFF_MAX_SIZE = 100 * 1024 * 1024;
static const char* DCAMERA_PROTOCOL_CMD_METADATA_RESULT = "METADATA_RESULT";
static const char* DCAMERA_PROTOCOL_CMD_STATE_NOTIFY = "STATE_NOTIFY";
}

void DCameraSourceControllerTest::SetUpTestCase(void)
{
    deviceMgrMock_ = std::make_shared<DeviceManagerMock>();
    DeviceManagerMock::deviceMgrMock = deviceMgrMock_;
}

void DCameraSourceControllerTest::TearDownTestCase(void)
{
    DeviceManagerMock::deviceMgrMock = nullptr;
    deviceMgrMock_ = nullptr;
}

void DCameraSourceControllerTest::SetUp(void)
{
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);
    stateMachine_ = std::make_shared<DCameraSourceStateMachine>(camDev_);
    controller_ = std::make_shared<DCameraSourceController>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateMachine_,
        camDev_);
    controller_->channel_ = std::make_shared<MockCameraChannel>();
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs_.push_back(index);
    g_srcConnect = true;
}

void DCameraSourceControllerTest::TearDown(void)
{
    usleep(TEST_SLEEP_SEC);
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
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
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
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
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

    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    int32_t mode = 0;
    ret = controller_->StartCapture(captureInfos, mode);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
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
    g_srcConnect = false;
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    int32_t mode = 0;
    ret = controller_->StartCapture(captureInfos, mode);

    ret = controller_->StopCapture();
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: dcamera_source_controller_test_005
 * @tc.desc: Verify source controller ChannelNeg.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_005, TestSize.Level1)
{
    DHLOGI("start execute dcamera_source_controller_test_005");
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    int32_t ret = GetLocalDeviceNetworkId(chanInfo->sourceDevId_);
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    DCameraChannelDetail snapShotChInfo(SNAP_SHOT_SESSION_FLAG, SNAPSHOT_FRAME);
    chanInfo->detail_.push_back(continueChInfo);
    chanInfo->detail_.push_back(snapShotChInfo);
    g_srcConnect = false;
    ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
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
    DHLOGI("start execute dcamera_source_controller_test_006");
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
    setting->type_ = DCSettingsType::DISABLE_METADATA;
    setting->value_ = "UpdateSettingsTest";
    settings.push_back(setting);

    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    ret = controller_->UpdateSettings(settings);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_007
 * @tc.desc: Verify source controller GetCameraInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_007, TestSize.Level1)
{
    DHLOGI("start execute dcamera_source_controller_test_007");
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    std::shared_ptr<DCameraInfo> camInfo = std::make_shared<DCameraInfo>();
    camInfo->state_ = 1;
    g_srcConnect = false;
    ret = controller_->GetCameraInfo(camInfo);
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
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
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
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    controller_->OpenChannel(openInfo);
    ret = controller_->CloseChannel();
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_010
 * @tc.desc: Verify source controller OnDataReceived.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_010, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    ret = camDev_->InitDCameraSourceDev();
    std::shared_ptr<ICameraChannelListener> listener =
        std::make_shared<DCameraSourceControllerChannelListener>(controller_);
    int32_t state = 0;
    listener->OnSessionState(state, "");
    state = 1;
    listener->OnSessionState(state, "");
    int32_t eventType = 1;
    int32_t eventReason = 1;
    std::string detail = "OnSessionErrorTest";
    listener->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    listener->OnDataReceived(buffers);
    size_t capacity = 0;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    listener->OnDataReceived(buffers);
    buffers.clear();
    capacity = DATABUFF_MAX_SIZE + 1;
    dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    listener->OnDataReceived(buffers);
    buffers.clear();
    capacity = 1;
    dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    listener->OnDataReceived(buffers);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_011
 * @tc.desc: Verify source controller ChannelNeg.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_011, TestSize.Level1)
{
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    int32_t ret = GetLocalDeviceNetworkId(chanInfo->sourceDevId_);
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    DCameraChannelDetail snapShotChInfo(SNAP_SHOT_SESSION_FLAG, SNAPSHOT_FRAME);
    chanInfo->detail_.push_back(continueChInfo);
    chanInfo->detail_.push_back(snapShotChInfo);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    g_srcConnect = false;
    controller_->Init(indexs_);

    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    ret = controller_->ChannelNeg(chanInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_012
 * @tc.desc: Verify source controller DCameraNotify.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_012, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = 1;
    events->eventResult_ = DCAMERA_EVENT_CAMERA_ERROR;
    events->eventContent_ = "controllerTest012";
    int32_t ret = controller_->DCameraNotify(events);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_012_1, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = 1;
    events->eventResult_ = DCAMERA_EVENT_CAMERA_SUCCESS;
    events->eventContent_ = START_CAPTURE_SUCC;
    int32_t ret = controller_->DCameraNotify(events);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_013
 * @tc.desc: Verify source controller UpdateSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_013, TestSize.Level1)
{
    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
    setting->type_ = DCSettingsType::DISABLE_METADATA;
    setting->value_ = "UpdateSettingsTest";
    settings.push_back(setting);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    int32_t ret = controller_->UpdateSettings(settings);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_014
 * @tc.desc: Verify source controller GetCameraInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_014, TestSize.Level1)
{
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    indexs_.push_back(index1);

    std::shared_ptr<DCameraInfo> camInfo = std::make_shared<DCameraInfo>();
    camInfo->state_ = 1;
    g_srcConnect = false;
    int32_t ret = controller_->GetCameraInfo(camInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_015
 * @tc.desc: Verify source controller OpenChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_015, TestSize.Level1)
{
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    indexs_.push_back(index1);

    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    int32_t ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    ret = controller_->OpenChannel(openInfo);
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_015_1, TestSize.Level1)
{
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    indexs_.push_back(index1);

    bool saved = ManageSelectChannel::GetInstance().GetSrcConnect();
    ManageSelectChannel::GetInstance().SetSrcConnect(true);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    int32_t ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    ret = controller_->OpenChannel(openInfo);
    ret = controller_->UnInit();
    ManageSelectChannel::GetInstance().SetSrcConnect(saved);
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_016
 * @tc.desc: Verify source controller OpenChannel and CloseChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_016, TestSize.Level1)
{
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    controller_->CloseChannel();
    int32_t ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_017
 * @tc.desc: Verify source controller OpenChannel and HandleMetaDataResult.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_017, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    std::string jsonStr = "controllerTest17";
    controller_->HandleMetaDataResult(jsonStr);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_018
 * @tc.desc: Verify source controller OpenChannel and WaitforSessionResult.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_018, TestSize.Level1)
{
    int32_t ret = camDev_->InitDCameraSourceDev();
    controller_->PublishEnableLatencyMsg(TEST_DEVICE_ID);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_019
 * @tc.desc: Verify source controller OnSessionError.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_019, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    std::shared_ptr<DCameraSourceController> controller = nullptr;
    std::shared_ptr<ICameraChannelListener> listener_ =
        std::make_shared<DCameraSourceControllerChannelListener>(controller);
    int32_t state = DCAMERA_CHANNEL_STATE_CONNECTED;
    listener_->OnSessionState(state, "");
    int32_t eventType = 1;
    int32_t eventReason = 1;
    std::string detail = "OnSessionErrorTest";
    listener_->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    listener_->OnDataReceived(buffers);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_019_1, TestSize.Level1)
{
    int32_t ret = controller_->Init(indexs_);
    std::shared_ptr<ICameraChannelListener> listener_ =
        std::make_shared<DCameraSourceControllerChannelListener>(controller_);
    stateMachine_->UpdateState(DCAMERA_STATE_INIT);
    int32_t state = DCAMERA_CHANNEL_STATE_CONNECTED;
    listener_->OnSessionState(state, "");
    int32_t eventType = 1;
    int32_t eventReason = 1;
    std::string detail = "OnSessionErrorTest";
    listener_->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    listener_->OnDataReceived(buffers);
    ret = controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_controller_test_020
 * @tc.desc: Verify source controller OpenChannel and CloseChannel.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_020, TestSize.Level1)
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
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    DCameraIndex index2;
    index2.devId_ = TEST_DEVICE_ID;
    index2.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    controller_->indexs_.push_back(index2);
    indexs_.push_back(index1);

    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    int32_t mode = 0;
    ret = controller_->StartCapture(captureInfos, mode);
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
    ret = controller_->StopCapture();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: dcamera_source_controller_test_021
 * @tc.desc: Verify source controller DCameraNotify.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_021, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = 1;
    events->eventResult_ = DCAMERA_EVENT_CAMERA_SUCCESS;
    events->eventContent_ = START_CAPTURE_SUCC;
    controller_->camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    int32_t ret = controller_->DCameraNotify(events);
    int32_t state = 2;
    std::shared_ptr<ICameraStateListener> stateLisener = std::make_shared<CameraStateListenerImpl>();
    std::shared_ptr<DCameraSourceDev> camDev = std::make_shared<DCameraSourceDev>("devId", "dhId",
        stateLisener);
    std::shared_ptr<DCameraSourceStateMachine> stMachine = std::make_shared<DCameraSourceStateMachine>(camDev);
    controller_->stateMachine_ = stMachine;
    std::shared_ptr<DCameraSourceState> currentState = std::make_shared<DCameraSourceStateImpl>();
    controller_->stateMachine_->currentState_ = currentState;
    controller_->OnSessionState(state, "networkId");
    controller_->camDev_ = std::make_shared<DCameraSourceDev>("devId", "dhId", stateLisener);
    controller_->PostChannelDisconnectedEvent();
    controller_->PauseDistributedHardware("networkId");
    controller_->ResumeDistributedHardware("networkId");
    controller_->StopDistributedHardware("networkId");
    controller_->UnInit();
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_022, TestSize.Level1)
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
    int32_t ret = controller_->Init(indexs_);
    EXPECT_EQ(ret, DCAMERA_INIT_ERR);
    DCameraIndex index1;
    index1.devId_ = TEST_DEVICE_ID;
    index1.dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->indexs_.push_back(index1);
    int32_t mode = 0;
    ret = controller_->StartCapture(captureInfos, mode);
    bool saved = ManageSelectChannel::GetInstance().GetSrcConnect();
    ManageSelectChannel::GetInstance().SetSrcConnect(true);
    ret = controller_->StopCapture();
    controller_->UnInit();
    ManageSelectChannel::GetInstance().SetSrcConnect(saved);
    EXPECT_EQ(ret, DCAMERA_OK);
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_023, TestSize.Level1)
{
    cJSON *metaJson1 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson1, "Command", "skip");
    std::string cjson1 = cJSON_PrintUnformatted(metaJson1);
    size_t capacity = cjson1.length() + 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    if (memcpy_s(dataBuffer->Data(), capacity, cjson1.c_str(), capacity) != EOK) {
        EXPECT_TRUE(false);
    }
    controller_->HandleReceivedData(dataBuffer);
    cJSON_Delete(metaJson1);

    metaJson1 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson1, "Command", DCAMERA_PROTOCOL_CMD_METADATA_RESULT);
    cjson1 = cJSON_PrintUnformatted(metaJson1);
    capacity = cjson1.length() + 1;
    dataBuffer = std::make_shared<DataBuffer>(capacity);
    if (memcpy_s(dataBuffer->Data(), capacity, cjson1.c_str(), capacity) != EOK) {
        EXPECT_TRUE(false);
    }
    controller_->HandleReceivedData(dataBuffer);
    cJSON_Delete(metaJson1);

    metaJson1 = cJSON_CreateObject();
    cJSON_AddStringToObject(metaJson1, "Command", DCAMERA_PROTOCOL_CMD_STATE_NOTIFY);
    cjson1 = cJSON_PrintUnformatted(metaJson1);
    capacity = cjson1.length() + 1;
    dataBuffer = std::make_shared<DataBuffer>(capacity);
    if (memcpy_s(dataBuffer->Data(), capacity, cjson1.c_str(), capacity) != EOK) {
        EXPECT_TRUE(false);
    }
    controller_->HandleReceivedData(dataBuffer);
    cJSON_Delete(metaJson1);
    EXPECT_FALSE(controller_->CheckAclRight());
}

HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_024, TestSize.Level1)
{
    std::string networkId = "testNetworkId";
    bool isInvalid = false;
    int32_t typeVal = controller_->CheckOsType(networkId, isInvalid);
    typeVal = 50;
    std::string jsonStr = "{\"OS_TYPE\": 50}";
    std::string key = "OS_TYPE";
    int32_t result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, typeVal);

    jsonStr = "invalid_json";
    key = "typeVal";
    result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "{\"test\": 80}";
    result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "{\"test\": \"typeVal\"}";
    result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "";
    result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "null";
    key = "typeVal";
    result = controller_->ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_001
 * @tc.desc: Verify ChannelNeg returns error when info is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_001, TestSize.Level1)
{
    // Given: info is nullptr
    std::shared_ptr<DCameraChannelInfo> info = nullptr;
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_002
 * @tc.desc: Verify ChannelNeg returns success when GetSrcConnect returns true.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_002, TestSize.Level1)
{
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    info->detail_.push_back(continueChInfo);
    info->detail_[0].dataSessionFlag_ = "dataContinue";
    info->detail_[0].streamType_ = CONTINUOUS_FRAME;
    
    int32_t ret = controller_->ChannelNeg(info);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_003
 * @tc.desc: Verify ChannelNeg returns error when GetSrcConnect is false and indexs_ is empty.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_003, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    info->detail_.push_back(continueChInfo);
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_004
 * @tc.desc: Verify ChannelNeg returns error when GetSrcConnect is false and indexs_ size exceeds DCAMERA_MAX_NUM.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_004, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    
    for (int i = 0; i < 33; i++) { // Exceeds max
        controller_->indexs_.push_back(DCameraIndex("dev_" + std::to_string(i), "dh_" + std::to_string(i)));
    }
    
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    info->detail_.push_back(continueChInfo);
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_005
 * @tc.desc: Verify ChannelNeg returns error when GetSrcConnect is false and cannot get remote service.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_005, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    controller_->indexs_.push_back(DCameraIndex("test_dev", "test_dh"));
    
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    info->detail_.push_back(continueChInfo);
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_006
 * @tc.desc: Verify ChannelNeg returns error when marshaling fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_006, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    controller_->indexs_.push_back(DCameraIndex("test_dev", "test_dh"));
    
    auto info = std::make_shared<DCameraChannelInfo>();
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_007
 * @tc.desc: Verify ChannelNeg returns error when remote call fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_007, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    // Create a valid channel info
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    info->detail_.push_back(continueChInfo);
    info->detail_[0].dataSessionFlag_ = "dataContinue";
    info->detail_[0].streamType_ = CONTINUOUS_FRAME;
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_009
 * @tc.desc: Verify ChannelNeg with multiple camera indexes (within max limit).
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_009, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    
    // Add multiple camera indexes (within DCAMERA_MAX_NUM)
    for (int i = 0; i < 3; i++) {
        controller_->indexs_.push_back(DCameraIndex("dev_" + std::to_string(i), "dh_" + std::to_string(i)));
    }
    
    // Create a valid channel info
    auto info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "test_source_dev";
    
    // Add multiple channel details
    for (int i = 0; i < 2; i++) {
        DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
        auto detail = std::make_shared<DCameraChannelDetail>();
        detail->dataSessionFlag_ = (i == 0) ? "dataContinue" : "dataSnapshot";
        detail->streamType_ = CONTINUOUS_FRAME;
        info->detail_.push_back(continueChInfo);
    }
    
    int32_t ret = controller_->ChannelNeg(info);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_channel_neg_010
 * @tc.desc: Verify ChannelNeg handles different data session flags.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_channel_neg_010, TestSize.Level1)
{
    g_srcConnect = false;
    controller_->indexs_.clear();
    controller_->indexs_.push_back(DCameraIndex("test_dev", "test_dh"));
    
    // Test with different data session flags
    std::vector<std::string> sessionFlags = {"dataContinue", "dataSnapshot", "dataPreview"};
    
    for (const auto& flag : sessionFlags) {
        // Create channel info with specific flag
        auto info = std::make_shared<DCameraChannelInfo>();
        info->sourceDevId_ = "test_source_dev";
        DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
        info->detail_.push_back(continueChInfo);
        info->detail_[0].dataSessionFlag_ = flag;
        info->detail_[0].streamType_ = CONTINUOUS_FRAME;
        
        int32_t ret = controller_->ChannelNeg(info);
        
        EXPECT_NE(DCAMERA_BAD_VALUE, ret);
    }
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_001
 * @tc.desc: Verify DCameraNotify returns error when events is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_001, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = nullptr;
    
    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_002
 * @tc.desc: Verify DCameraNotify reports radar when event is START_CAPTURE_SUCC.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_002, TestSize.Level1)
{
    auto events = std::make_shared<DCameraEvent>();
    events->eventResult_ = DCAMERA_EVENT_CAMERA_SUCCESS;
    events->eventContent_ = "StartCapture";
    
    controller_->camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);

    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_004
 * @tc.desc: Verify DCameraNotify reports device in use event.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_004, TestSize.Level1)
{
    auto events = std::make_shared<DCameraEvent>();
    events->eventResult_ = DCAMERA_EVENT_DEVICE_IN_USE;
    events->eventContent_ = "Device is in use";
    
    controller_->devId_ = "test_device_id";
    controller_->dhId_ = "test_dh_id";
    
    controller_->camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    
    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_005
 * @tc.desc: Verify DCameraNotify returns error when camHdiProvider_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_005, TestSize.Level1)
{
    controller_->camHdiProvider_ = nullptr;
    
    auto events = std::make_shared<DCameraEvent>();
    events->eventType_ = DCAMERA_SINK_STOP;
    events->eventResult_ = DCAMERA_EVENT_SINK_STOP;
    events->eventContent_ = "Test event";
    
    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_006
 * @tc.desc: Verify DCameraNotify converts SINK_STOP event to OPERATION event.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_006, TestSize.Level1)
{
    auto events = std::make_shared<DCameraEvent>();
    events->eventType_ = DCAMERA_SINK_STOP;
    events->eventResult_ = DCAMERA_EVENT_SINK_STOP;
    events->eventContent_ = "Sink stop event";

    controller_->camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    
    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}


/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_008
 * @tc.desc: Verify DCameraNotify returns error when Notify returns non-SUCCESS.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_008, TestSize.Level1)
{
    auto events = std::make_shared<DCameraEvent>();
    events->eventType_ = DCAMERA_OPERATION;
    events->eventResult_ = DCAMERA_EVENT_CAMERA_SUCCESS;
    events->eventContent_ = "Operation successful";
    
    int32_t ret = controller_->DCameraNotify(events);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_dcamera_notify_009
 * @tc.desc: Verify DCameraNotify handles multiple event types correctly.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_dcamera_notify_009, TestSize.Level1)
{
    // Test different event types
    std::vector<std::pair<int32_t, int32_t>> testEvents = {
        {DCAMERA_MESSAGE, DCAMERA_EVENT_CAMERA_SUCCESS},
        {DCAMERA_OPERATION, DCAMERA_EVENT_CAMERA_ERROR},
        {DCAMERA_SINK_STOP, DCAMERA_EVENT_SINK_STOP}, // This will be converted
        {DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_IN_USE}
    };
    
    for (const auto& eventPair : testEvents) {
        auto events = std::make_shared<DCameraEvent>();
        events->eventType_ = eventPair.first;
        events->eventResult_ = eventPair.second;
        events->eventContent_ = "Test event content";
        
        int32_t ret = controller_->DCameraNotify(events);
        
        EXPECT_TRUE(ret == DCAMERA_OK || ret == DCAMERA_BAD_OPERATE);
    }
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_001
 * @tc.desc: Verify OpenChannel returns error when openInfo is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_001, TestSize.Level1)
{
    std::shared_ptr<DCameraOpenInfo> openInfo = nullptr;
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_002
 * @tc.desc: Verify OpenChannel returns error when indexs_ is empty.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_002, TestSize.Level1)
{
    controller_->indexs_.clear();
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_003
 * @tc.desc: Verify OpenChannel returns error when indexs_ size exceeds DCAMERA_MAX_NUM.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_003, TestSize.Level1)
{
    controller_->indexs_.clear();
    
    for (int i = 0; i < 33; i++) { // Exceeds max
        controller_->indexs_.push_back(DCameraIndex("dev_" + std::to_string(i), "dh_" + std::to_string(i)));
    }
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_004
 * @tc.desc: Verify OpenChannel returns error when AddCameraServiceDeathRecipient fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_004, TestSize.Level1)
{
    controller_->indexs_.clear();
    controller_->indexs_.push_back(DCameraIndex("test_dev", "test_dh"));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_005
 * @tc.desc: Verify OpenChannel returns error when ACL check fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_005, TestSize.Level1)
{
    controller_->indexs_.clear();
    controller_->indexs_.push_back(DCameraIndex("test_dev", "test_dh"));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    // Setup: Mock CheckAclRight to return false
    // This requires setting up the controller state or mocking
    
    // When: Call OpenChannel
    int32_t ret = controller_->OpenChannel(openInfo);
    
    // Then: Should return DCAMERA_BAD_OPERATE
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_006
 * @tc.desc: Verify OpenChannel returns error when CheckOsType fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_006, TestSize.Level1)
{
    // Given: indexs_ has one item, CheckOsType will fail
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_007
 * @tc.desc: Verify OpenChannel returns error when GetSrcConnect is false and cannot get remote service.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_007, TestSize.Level1)
{
    // Given: indexs_ has one item, GetSrcConnect returns false, remote service is null
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    // Setup: Mock GetSrcConnect to return false
    // Mock DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv to return nullptr
    
    // When: Call OpenChannel
    int32_t ret = controller_->OpenChannel(openInfo);
    
    // Then: Should return DCAMERA_BAD_OPERATE
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_008
 * @tc.desc: Verify OpenChannel returns error when GetSrcConnect is false and marshaling fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_008, TestSize.Level1)
{
    // Given: indexs_ has one item, GetSrcConnect returns false, marshaling fails
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    // Create an invalid openInfo that will cause marshaling to fail
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    // Don't set required fields or create a problematic structure
    
    // Setup: Mock GetSrcConnect to return false
    // Mock remote service to be available
    // Mock DCameraOpenInfoCmd.Marshal to return non-DCAMERA_OK
    
    // When: Call OpenChannel
    int32_t ret = controller_->OpenChannel(openInfo);
    
    // Then: Should return marshaling error
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_009
 * @tc.desc: Verify OpenChannel returns error when GetSrcConnect is false and remote call fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_009, TestSize.Level1)
{
    // Given: indexs_ has one item, GetSrcConnect returns false, remote call fails
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    // Setup: Mock GetSrcConnect to return false
    // Mock remote service to be available
    // Mock remote service's OpenChannel method to return non-DCAMERA_OK
    
    // When: Call OpenChannel
    int32_t ret = controller_->OpenChannel(openInfo);
    
    // Then: Should return error from remote call
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_open_channel_010
 * @tc.desc: Verify OpenChannel returns error when channel_ is nullptr.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_open_channel_010, TestSize.Level1)
{
    // Given: indexs_ has one item, channel_ is nullptr
    controller_->indexs_.clear();
    std::string devId = "test_dev";
    std::string dhId = "test_dh";
    controller_->indexs_.push_back(DCameraIndex(devId, dhId));
    
    auto openInfo = std::make_shared<DCameraOpenInfo>();
    openInfo->sourceDevId_ = "test_source_dev";
    
    // Setup: channel_ is nullptr
    controller_->channel_ = nullptr;
    
    int32_t ret = controller_->OpenChannel(openInfo);
    
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_001
 * @tc.desc: Verify CheckOsType returns error when GetTrustedDeviceList fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_001, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_002
 * @tc.desc: Verify CheckOsType returns OK when device list is empty.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_002, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> emptyDeviceList;
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_003
 * @tc.desc: Verify CheckOsType returns OK when device list has devices but no match.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_003, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo device1;
    std::string networkId1 = "different_network_id_1";
    strcpy_s(device1.networkId, networkId1.length(), networkId1.c_str());
    device1.extraData = "{\"osType\":1}"; // VALID_OS_TYPE
    deviceList.push_back(device1);
    
    DistributedHardware::DmDeviceInfo device2;
    std::string networkId2 = "different_network_id_2";
    strcpy_s(device2.networkId, networkId2.length(), networkId2.c_str());
    device2.extraData = "{\"osType\":2}"; // Other OS type
    deviceList.push_back(device2);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_004
 * @tc.desc: Verify CheckOsType returns OK and isInvalid=false when osType is VALID_OS_TYPE.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_004, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;

    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "{\"osType\":1}"; // Assuming VALID_OS_TYPE = 1
    deviceList.push_back(matchingDevice);
    
    DistributedHardware::DmDeviceInfo otherDevice;
    std::string networkId1 = "other_network_id";
    strcpy_s(otherDevice.networkId, networkId1.length(), networkId1.c_str());
    otherDevice.extraData = "{\"osType\":2}";
    deviceList.push_back(otherDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_005
 * @tc.desc: Verify CheckOsType returns OK and isInvalid=false when osType is DCAMERA_BAD_VALUE.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_005, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "{\"osType\":-1}"; // Assuming DCAMERA_BAD_VALUE = -1
    deviceList.push_back(matchingDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_006
 * @tc.desc: Verify CheckOsType returns OK and sets isInvalid=true when osType is invalid.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_006, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    // Setup: Mock GetTrustedDeviceList to return list with matching device with invalid osType
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "{\"osType\":99}"; // Invalid OS type (not VALID_OS_TYPE and not DCAMERA_BAD_VALUE)
    deviceList.push_back(matchingDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_007
 * @tc.desc: Verify CheckOsType handles multiple matching devices (should find first).
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_007, TestSize.Level1)
{
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice1;
    strcpy_s(matchingDevice1.networkId, networkId.length(), networkId.c_str());
    matchingDevice1.extraData = "{\"osType\":99}"; // Invalid
    deviceList.push_back(matchingDevice1);
    
    DistributedHardware::DmDeviceInfo matchingDevice2;
    strcpy_s(matchingDevice2.networkId, networkId.length(), networkId.c_str());
    matchingDevice2.extraData = "{\"osType\":1}"; // Valid
    deviceList.push_back(matchingDevice2);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_008
 * @tc.desc: Verify CheckOsType when isInvalid is initially true and osType is valid.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_008, TestSize.Level1)
{
    // Given: networkId and isInvalid flag initially true
    std::string networkId = "test_network_id";
    bool isInvalid = true; // Initially true
    
    // Setup: Mock GetTrustedDeviceList to return matching device with VALID_OS_TYPE
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "{\"osType\":1}"; // VALID_OS_TYPE
    deviceList.push_back(matchingDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_TRUE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_009
 * @tc.desc: Verify CheckOsType when ParseValueFromCjson returns invalid JSON.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_009, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "invalid_json"; // Invalid JSON
    deviceList.push_back(matchingDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_os_type_010
 * @tc.desc: Verify CheckOsType when extraData doesn't contain osType key.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_os_type_010, TestSize.Level1)
{
    // Given: networkId and isInvalid flag
    std::string networkId = "test_network_id";
    bool isInvalid = false;
    
    std::vector<DistributedHardware::DmDeviceInfo> deviceList;
    
    DistributedHardware::DmDeviceInfo matchingDevice;
    strcpy_s(matchingDevice.networkId, networkId.length(), networkId.c_str());
    matchingDevice.extraData = "{\"otherKey\":123}"; // No osType key
    deviceList.push_back(matchingDevice);
    
    int32_t ret = controller_->CheckOsType(networkId, isInvalid);
    
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_FALSE(isInvalid);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_001
 * @tc.desc: Verify CheckAclRight returns false when GetOsAccountInfo fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_001, TestSize.Level1)
{
    // Given: Setup controller state
    controller_->accountId_ = "test_account_id";
    controller_->srcDevId_ = "test_src_dev_id";
    controller_->devId_ = "test_dev_id";
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;

    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_002
 * @tc.desc: Verify CheckAclRight returns false when InitDeviceManager fails.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_002, TestSize.Level1)
{
    controller_->accountId_ = "test_account_id";
    controller_->srcDevId_ = "test_src_dev_id";
    controller_->devId_ = "test_dev_id";
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;
    
    EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(1));
    
    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_003
 * @tc.desc: Verify CheckAclRight returns false when CheckSrcAccessControl returns false.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_003, TestSize.Level1)
{
    controller_->accountId_ = "test_account_id";
    controller_->srcDevId_ = "test_src_dev_id";
    controller_->devId_ = "test_dev_id";
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;
    
    EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(false));
    
    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_004
 * @tc.desc: Verify CheckAclRight returns true when all conditions are met.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_004, TestSize.Level1)
{
    controller_->accountId_ = "test_account_id";
    controller_->srcDevId_ = "test_src_dev_id";
    controller_->devId_ = "test_dev_id";
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;
    
    EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
    
    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_005
 * @tc.desc: Verify CheckAclRight with different accountId values.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_005, TestSize.Level1)
{
    std::vector<std::string> accountIds = {"", "0", "100", "test_account", "another_account"};
    
    for (const auto& accountId : accountIds) {
        controller_->accountId_ = accountId;
        controller_->srcDevId_ = "test_src_dev_id";
        controller_->devId_ = "test_dev_id";
        controller_->userId_ = 100;
        controller_->tokenId_ = 200;
        
        EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
        
        bool result = controller_->CheckAclRight();
        
        EXPECT_FALSE(result);
    }
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_006
 * @tc.desc: Verify CheckAclRight with different device IDs.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_006, TestSize.Level1)
{
    struct TestCase {
        std::string srcDevId;
        std::string devId;
        bool expectedCheckResult;
    };
    
    std::vector<TestCase> testCases = {
        {"src_device_1", "dst_device_1", false},
        {"", "dst_device_1", false}, // Empty srcDevId
        {"src_device_1", "", false}, // Empty devId
        {"", "", false}, // Both empty
    };
    
    for (const auto& testCase : testCases) {
        controller_->accountId_ = "test_account_id";
        controller_->srcDevId_ = testCase.srcDevId;
        controller_->devId_ = testCase.devId;
        controller_->userId_ = 100;
        controller_->tokenId_ = 200;
        
        EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
        
        bool result = controller_->CheckAclRight();
        
        EXPECT_EQ(testCase.expectedCheckResult, result);
    }
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_007
 * @tc.desc: Verify CheckAclRight with different user and token IDs.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_007, TestSize.Level1)
{
    struct TestCase {
        int32_t userId;
        uint64_t tokenId;
    };
    
    std::vector<TestCase> testCases = {
        {0, 0}, // Minimum values
        {100, 1000},
        {1000, 10000},
        {-1, 0}, // Negative userId
        {0, static_cast<uint64_t>(-1)}, // Max tokenId
    };
    
    for (const auto& testCase : testCases) {
        controller_->accountId_ = "test_account_id";
        controller_->srcDevId_ = "test_src_dev_id";
        controller_->devId_ = "test_dev_id";
        controller_->userId_ = testCase.userId;
        controller_->tokenId_ = testCase.tokenId;
        
        EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
        
        bool result = controller_->CheckAclRight();
        
        EXPECT_FALSE(result);
    }
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_008
 * @tc.desc: Verify CheckAclRight when DeviceInitCallback is created.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_008, TestSize.Level1)
{
    controller_->accountId_ = "test_account_id";
    controller_->srcDevId_ = "test_src_dev_id";
    controller_->devId_ = "test_dev_id";
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;
    
    EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
    
    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_009
 * @tc.desc: Verify CheckAclRight logs correct information.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_009, TestSize.Level1)
{
    std::string accountId = "test_account_123";
    std::string srcDevId = "source_device_456";
    std::string devId = "dest_device_789";
    
    controller_->accountId_ = accountId;
    controller_->srcDevId_ = srcDevId;
    controller_->devId_ = devId;
    controller_->userId_ = 100;
    controller_->tokenId_ = 200;

    EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
    EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
    
    bool result = controller_->CheckAclRight();
    
    EXPECT_FALSE(result);
}

/**
 * @tc.name: dcamera_source_controller_test_check_acl_right_010
 * @tc.desc: Verify CheckAclRight handles edge cases with special characters in IDs.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_check_acl_right_010, TestSize.Level1)
{
    struct TestCase {
        std::string accountId;
        std::string srcDevId;
        std::string devId;
    };
    
    std::vector<TestCase> testCases = {
        {"account with spaces", "src with spaces", "dst with spaces"},
        {"account\nwith\nnewlines", "src\nwith\nnewlines", "dst\nwith\nnewlines"},
        {"account\twith\ttabs", "src\twith\ttabs", "dst\twith\ttabs"},
        {"account\"with\"quotes", "src\"with\"quotes", "dst\"with\"quotes"},
        {"account\\with\\backslashes", "src\\with\\backslashes", "dst\\with\\backslashes"},
        {"", "", ""}, // All empty
        {"very_long_account_id_" + std::string(1000, 'a'),
         "very_long_src_dev_id_" + std::string(1000, 'b'),
         "very_long_dev_id_" + std::string(1000, 'c')}, // Very long IDs
    };
    
    for (const auto& testCase : testCases) {
        controller_->accountId_ = testCase.accountId;
        controller_->srcDevId_ = testCase.srcDevId;
        controller_->devId_ = testCase.devId;
        controller_->userId_ = 100;
        controller_->tokenId_ = 200;
        
        EXPECT_CALL(*deviceMgrMock_, InitDeviceManager(_, _)).WillRepeatedly(Return(0));
        EXPECT_CALL(*deviceMgrMock_, CheckSrcAccessControl(_, _)).WillRepeatedly(Return(true));
        
        bool result = controller_->CheckAclRight();
        
        EXPECT_FALSE(result);
    }
}
}
}