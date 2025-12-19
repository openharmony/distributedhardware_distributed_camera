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

#include "dcamera_handler.h"
#define private public
#include "distributed_camera_sink_service.h"
#undef private
#include "dcamera_sink_callback.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "iaccess_listener.h"
#include "idistributed_camera_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class TestAccessListener : public IAccessListener {
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    void OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info, const DHType dhType,
        const std::string &pkgName)
    {
        (void)requestId;
        (void)info;
        (void)dhType;
        (void)pkgName;
    }
};

class DistributedCameraSinkServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

const int32_t TEST_TWENTY_MS = 20000;
std::string g_dhId;
std::string g_networkId = "08647073e02e7a78f09473aa122ff57fc81c00";
std::string g_testParams = "TestParams";
std::string g_testCameraInfo = "";
std::string g_testChannelInfoContinue = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId",
    "Detail": [{"DataSessionFlag": "dataContinue", "StreamType": 0}]}
})";
std::string g_testOpenInfoService = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";
std::shared_ptr<DistributedCameraSinkService> sinkService_;

void DistributedCameraSinkServiceTest::SetUpTestCase(void)
{
    DHLOGI("enter");
    sinkService_ = std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    DCameraHandler::GetInstance().Initialize();
    g_dhId = DCameraHandler::GetInstance().GetCameras().front();
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    sinkService_->InitSink(g_testParams, sinkCallback);
}

void DistributedCameraSinkServiceTest::TearDownTestCase(void)
{
    DHLOGI("enter");
    sinkService_->ReleaseSink();
    usleep(TEST_TWENTY_MS);
    sinkService_ = nullptr;
}

void DistributedCameraSinkServiceTest::SetUp(void)
{
    DHLOGI("enter");
}

void DistributedCameraSinkServiceTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_service_test_001
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_001");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->SubscribeLocalHardware(g_dhId, g_testParams);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_002
 * @tc.desc: Verify the UnSubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_002");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->UnsubscribeLocalHardware(g_dhId);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_003
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_003");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->StopCapture(g_dhId);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_004
 * @tc.desc: Verify the ChannelNeg function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_004");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->ChannelNeg(g_dhId, g_testChannelInfoContinue);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_005
 * @tc.desc: Verify the GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_005");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->GetCameraInfo(g_dhId, g_testCameraInfo);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_006
 * @tc.desc: Verify the OpenChannel and CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_006");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->OpenChannel(g_dhId, g_testOpenInfoService);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = sinkService_->ChannelNeg(g_dhId, g_testChannelInfoContinue);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = sinkService_->CloseChannel(g_dhId);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_007
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_007");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string str(u"");
    args.push_back(str);
    int ret = sinkService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_008
 * @tc.desc: Verify the SubscribeLocalHardware function can't find id.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_008, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_008");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->SubscribeLocalHardware("", g_testParams);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_009
 * @tc.desc: Verify the UnsubscribeLocalHardware function can't find id.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_009, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_009");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->UnsubscribeLocalHardware("");
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_010
 * @tc.desc: Verify the ChannelNeg function can't find id.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_010, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_010");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->ChannelNeg("", g_testChannelInfoContinue);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_011
 * @tc.desc: Verify the GetCameraInfo function can't find id.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_011, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_011");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->GetCameraInfo("", g_testCameraInfo);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_012
 * @tc.desc: Verify the OpenChannel and CloseChannel function can't find id.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_012, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_012");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->OpenChannel("", g_testOpenInfoService);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    ret = sinkService_->CloseChannel("");
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_013
 * @tc.desc: Verify the Dump function with large arg.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_013, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_013");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t fd = 1;
    std::vector<std::u16string> args(sinkService_->DUMP_MAX_SIZE, u"");
    std::u16string str(u"");
    args.push_back(str);
    int ret = sinkService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_014
 * @tc.desc: Verify the StopCapture function can't find id.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_014, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_014");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->StopCapture("");
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_015
 * @tc.desc: Verify the PauseDistributedHardware function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_015, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_015");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->PauseDistributedHardware(g_networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_016
 * @tc.desc: Verify the ResumeDistributedHardware function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_016, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_016");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->ResumeDistributedHardware(g_networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_017
 * @tc.desc: Verify the StopDistributedHardware function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_017, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_017");
    EXPECT_EQ(sinkService_ == nullptr, false);

    int32_t ret = sinkService_->StopDistributedHardware(g_networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_018
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_018, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_018");
    EXPECT_EQ(sinkService_ == nullptr, false);

    sptr<IAccessListener> listenerNull = nullptr;
    int32_t timeOut = 0;
    std::string pkgName = "pkgName";
    sptr<IAccessListener> listener(new TestAccessListener());
    std::string pkgNameNull = "";
    int32_t ret = sinkService_->SetAccessListener(listenerNull, timeOut, pkgName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkService_->SetAccessListener(listener, timeOut, pkgNameNull);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkService_->SetAccessListener(listener, timeOut, pkgName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_019
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_019, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_019");
    EXPECT_EQ(sinkService_ == nullptr, false);

    std::string pkgName = "pkgName";
    std::string pkgNameNull = "";
    int32_t ret = sinkService_->RemoveAccessListener(pkgNameNull);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkService_->RemoveAccessListener(pkgName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_020
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_020, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_test_020");
    EXPECT_EQ(sinkService_ == nullptr, false);

    std::string requestId = "requestId";
    std::string requestIdNull = "";
    bool granted = true;
    int32_t ret = sinkService_->SetAuthorizationResult(requestIdNull, granted);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkService_->SetAuthorizationResult(requestId, granted);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS