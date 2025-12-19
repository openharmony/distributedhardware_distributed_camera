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

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

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
}

void DCameraSourceControllerTest::TearDownTestCase(void)
{
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
 * @tc.name: dcamera_source_controller_test_025
 * @tc.desc: Verify source controller DCameraNotify when eventResult is DCAMERA_EVENT_DEVICE_IN_USE
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_025, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = 1;
    events->eventResult_ = DCAMERA_EVENT_DEVICE_IN_USE;
    events->eventContent_ = "device in use test";
    controller_->devId_ = TEST_DEVICE_ID;
    controller_->dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = controller_->DCameraNotify(events);
    EXPECT_EQ(ret, DCAMERA_BAD_OPERATE);
    controller_->UnInit();
}

/**
 * @tc.name: dcamera_source_controller_test_026
 * @tc.desc: Verify source controller DCameraNotify with camHdiProvider when eventResult is DCAMERA_EVENT_DEVICE_IN_USE
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSourceControllerTest, dcamera_source_controller_test_026, TestSize.Level1)
{
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = 1;
    events->eventResult_ = DCAMERA_EVENT_DEVICE_IN_USE;
    events->eventContent_ = "device in use test with provider";
    controller_->devId_ = TEST_DEVICE_ID;
    controller_->dhId_ = TEST_CAMERA_DH_ID_0;
    controller_->camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    int32_t ret = controller_->DCameraNotify(events);
    EXPECT_TRUE(ret == DCAMERA_OK || ret == DCAMERA_BAD_OPERATE);
    controller_->UnInit();
}

}
}