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
#include "dcamera_source_dev.h"
#undef private

#include "accesstoken_kit.h"
#include "dcamera_source_state.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "dcamera_source_dev.h"
#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_controller.h"
#include "mock_dcamera_source_input.h"
#include "mock_dcamera_source_state_listener.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceDevTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();

    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<ICameraStateListener> stateListener_;
    std::shared_ptr<ICameraController> controller_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "";
const std::string TEST_VER = "1.0";
const std::string TEST_SINK_ATTRS = "";
const std::string TEST_SRC_ATTRS = "";
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

void DCameraSourceDevTest::SetTokenID()
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
        .processName = "dcamera_source_dev_test",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
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
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
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
    EnableParam param;
    param.sinkVersion = TEST_VER;
    param.sinkAttrs = TEST_SINK_ATTRS;
    param.sourceAttrs = TEST_SRC_ATTRS;
    param.sourceVersion = TEST_VER;
    int32_t ret = camDev_->RegisterDistributedHardware(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, param);
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
    DCameraSourceEvent event(DCAMERA_EVENT_NOFIFY, camEvent);
    camDev_->InitDCameraSourceDev();
    camDev_->GetVersion();
    int32_t ret = camDev_->GetStateInfo();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_013
 * @tc.desc: Verify source dev Register.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_013, TestSize.Level1)
{
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    std::string reqId = TEST_REQID;
    std::string sinkParams = TEST_VER;
    std::string srcParams = TEST_VER;
    std::shared_ptr<DCameraRegistParam> param = std::make_shared<DCameraRegistParam>(devId, dhId, reqId, sinkParams,
        srcParams);
    int32_t ret = camDev_->Register(param);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_014
 * @tc.desc: Verify source dev UnRegister.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_014, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    std::string reqId = TEST_REQID;
    std::string sinkParams = TEST_VER;
    std::string srcParams = TEST_VER;
    std::shared_ptr<DCameraRegistParam> param = std::make_shared<DCameraRegistParam>(devId, dhId, reqId, sinkParams,
        srcParams);
    int32_t ret = camDev_->UnRegister(param);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_015
 * @tc.desc: Verify source dev OpenCamera.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_015, TestSize.Level1)
{
    SetTokenID();
    std::vector<DCameraIndex> indexs;
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    int32_t ret = camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    ret = camDev_->OpenCamera();
    ret = camDev_->CloseCamera();
    EXPECT_EQ(DCAMERA_OK, ret);
}

HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_015_1, TestSize.Level1)
{
    SetTokenID();
    std::vector<DCameraIndex> indexs;
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceControllerRetErr>();
    int32_t ret = camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    ret = camDev_->OpenCamera();
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
    ret = camDev_->CloseCamera();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_016
 * @tc.desc: Verify source dev ConfigStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_016, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
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
    int32_t ret = camDev_->ConfigStreams(streamInfos);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_017
 * @tc.desc: Verify source dev ReleaseStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_017, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
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
    int32_t ret = camDev_->ConfigStreams(streamInfos);
    std::vector<int> streamIds;
    int32_t streamId = 1;
    streamIds.push_back(streamId);
    bool isAllRelease = true;
    ret = camDev_->ReleaseStreams(streamIds, isAllRelease);
    isAllRelease = false;
    ret = camDev_->ReleaseStreams(streamIds, isAllRelease);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_018
 * @tc.desc: Verify source dev ReleaseAllStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_018, TestSize.Level1)
{
    camDev_->InitDCameraSourceDev();
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    int32_t ret = camDev_->ReleaseAllStreams();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_019
 * @tc.desc: Verify source dev StartCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_019, TestSize.Level1)
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
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    int32_t ret = camDev_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);
    camDev_->HitraceAndHisyseventImpl(captureInfos);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_020
 * @tc.desc: Verify source dev StopAllCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_020, TestSize.Level1)
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
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    int32_t ret = camDev_->StartCapture(captureInfos);
    std::vector<int> streamIds;
    int32_t streamId = 1;
    streamIds.push_back(streamId);
    bool isAllStop = true;
    ret = camDev_->StopCapture(streamIds, isAllStop);
    EXPECT_EQ(DCAMERA_OK, ret);
    ret = camDev_->StopAllCapture();
    EXPECT_EQ(DCAMERA_OK, ret);
}

HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_020_1, TestSize.Level1)
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
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    auto savedInput = camDev_->input_;
    camDev_->input_ = std::make_shared<MockDCameraSourceInput>();
    camDev_->input_->Init();
    int32_t ret = camDev_->StartCapture(captureInfos);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
    std::vector<int> streamIds;
    int32_t streamId = 1;
    streamIds.push_back(streamId);
    bool isAllStop = true;
    ret = camDev_->StopCapture(streamIds, isAllStop);
    EXPECT_EQ(DCAMERA_OK, ret);
    ret = camDev_->StopAllCapture();
    EXPECT_EQ(DCAMERA_OK, ret);
    camDev_->input_ = savedInput;
}

/**
 * @tc.name: dcamera_source_dev_test_021
 * @tc.desc: Verify source dev UpdateSettings.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_021, TestSize.Level1)
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
    std::vector<DCameraIndex> indexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    indexs.push_back(index);
    camDev_->InitDCameraSourceDev();
    camDev_->controller_ = std::make_shared<MockDCameraSourceController>();
    camDev_->controller_->Init(indexs);
    camDev_->input_->Init();
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
    setting->type_ = DCSettingsType::DISABLE_METADATA;
    setting->value_ = "UpdateSettingsTest";
    settings.push_back(setting);
    int32_t ret = camDev_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_OK, ret);

    auto savedInput = camDev_->input_;
    camDev_->input_ = std::make_shared<MockDCameraSourceInput>();
    ret = camDev_->UpdateSettings(settings);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
    camDev_->input_ = savedInput;
}

/**
 * @tc.name: dcamera_source_dev_test_022
 * @tc.desc: Verify source dev NotifyResult.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_022, TestSize.Level1)
{
    int32_t ret = camDev_->InitDCameraSourceDev();
    int32_t result = 0;
    DCAMERA_EVENT eventType = DCAMERA_EVENT::DCAMERA_EVENT_OPEN;
    std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
    DCameraSourceEvent event(DCAMERA_EVENT_NOFIFY, camEvent);
    camDev_->NotifyResult(eventType, event, result);
    camDev_->memberFuncMap_.clear();
    camDev_->NotifyResult(eventType, event, result);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_dev_test_023
 * @tc.desc: Verify source dev UpdateDCameraWorkMode.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, dcamera_source_dev_test_023, TestSize.Level1)
{
    WorkModeParam param(12, 120, 0, false);
    int32_t ret = camDev_->UpdateDCameraWorkMode(param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    camDev_->InitDCameraSourceDev();
    camDev_->input_->Init();
    ret = camDev_->UpdateDCameraWorkMode(param);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: SetHicollieFlag_001
 * @tc.desc: Verify source dev SetHicollieFlag.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, SetHicollieFlag_001, TestSize.Level1)
{
    camDev_->SetHicollieFlag(true);
    EXPECT_EQ(true, camDev_->GetHicollieFlag());
    camDev_->SetHicollieFlag(false);
    EXPECT_EQ(false, camDev_->GetHicollieFlag());
}

/**
 * @tc.name: SetHicollieFlag_001
 * @tc.desc: Verify source dev SetHicollieFlag.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, GetFullCaps_001, TestSize.Level1)
{
    DHLOGI("DCameraSourceDevTest GetFullCaps_001");
    EXPECT_EQ(DCAMERA_OK, camDev_->GetFullCaps());

    std::shared_ptr<DCameraSourceDev> camDev1_;
    std::shared_ptr<ICameraStateListener> stateListener1_;
    camDev1_ = std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener1_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, camDev1_->GetFullCaps());
}

HWTEST_F(DCameraSourceDevTest, OnChannelConnectedEvent_001, TestSize.Level1)
{
    std::shared_ptr<DCameraSourceDev> camDev1_;
    std::shared_ptr<ICameraStateListener> stateListener1_;
    camDev1_ = std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener1_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, camDev1_->OnChannelConnectedEvent());

    camDev1_->InitDCameraSourceDev();
    EXPECT_EQ(DCAMERA_OK, camDev1_->OnChannelConnectedEvent());
}

HWTEST_F(DCameraSourceDevTest, PostHicollieEvent_001, TestSize.Level1)
{
    std::shared_ptr<DCameraSourceDev> camDev1_;
    std::shared_ptr<ICameraStateListener> stateListener1_;
    camDev1_ = std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener1_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, camDev1_->PostHicollieEvent());

    camDev1_->InitDCameraSourceDev();
    EXPECT_EQ(DCAMERA_OK, camDev1_->PostHicollieEvent());
}

/**
 * @tc.name: processHDFEvent_001
 * @tc.desc: Verify source dev processHDFEvent.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDevTest, processHDFEvent_001, TestSize.Level1) {
    DCameraHDFEvent event;
    event.type_ = EVENT_DCAMERA_FORCE_SWITCH;
    event.result_ = 90;
    DHLOGI("DCameraSourceDevTest processHDFEvent_001");
    int32_t result = camDev_->ProcessHDFEvent(event);
    bool flag = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchFlag(TEST_DEVICE_ID);
    EXPECT_TRUE(flag);
    EXPECT_EQ(result, DCAMERA_OK);
    int32_t rotate = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchRotation(TEST_DEVICE_ID);
    EXPECT_EQ(rotate, 90);
}
} // namespace DistributedHardware
} // namespace OHOS
