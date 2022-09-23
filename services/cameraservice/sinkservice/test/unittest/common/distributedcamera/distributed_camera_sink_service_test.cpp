/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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
#include "dcamera_sink_handler.h"
#include "dcamera_sink_handler_ipc.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_camera_sink.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSinkServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static std::string g_dhId;
static sptr<IDistributedCameraSink> g_service;

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

void DistributedCameraSinkServiceTest::SetUpTestCase(void)
{
    DHLOGI("DistributedCameraSinkServiceTest::SetUpTestCase");
    DCameraHandler::GetInstance().Initialize();
    g_dhId = DCameraHandler::GetInstance().GetCameras().front();

    DCameraSinkHandler::GetInstance().InitSink(g_testParams);
    g_service = DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
}

void DistributedCameraSinkServiceTest::TearDownTestCase(void)
{
    DHLOGI("DistributedCameraSinkServiceTest::TearDownTestCase");
    g_service = nullptr;
    DCameraSinkHandler::GetInstance().ReleaseSink();
}

void DistributedCameraSinkServiceTest::SetUp(void)
{
    DHLOGI("DistributedCameraSinkServiceTest::SetUp");
}

void DistributedCameraSinkServiceTest::TearDown(void)
{
    DHLOGI("DistributedCameraSinkServiceTest::TearDown");
}

/**
 * @tc.name: dcamera_sink_service_test_001
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_001, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::SubscribeLocalHardware");
    int32_t ret = g_service->SubscribeLocalHardware(g_dhId, g_testParams);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_002
 * @tc.desc: Verify the UnSubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_002, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::UnSubscribeLocalHardware");
    int32_t ret = g_service->UnsubscribeLocalHardware(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_003
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_003, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::StopCapture");
    int32_t ret = g_service->StopCapture(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_004
 * @tc.desc: Verify the ChannelNeg function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_004, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::ChannelNeg");
    int32_t ret = g_service->ChannelNeg(g_dhId, g_testChannelInfoContinue);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_005
 * @tc.desc: Verify the GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MT
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_005, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::GetCameraInfo");
    int32_t ret = g_service->GetCameraInfo(g_dhId, g_testCameraInfo);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_service_test_006
 * @tc.desc: Verify the OpenChannel and CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DistributedCameraSinkServiceTest, dcamera_sink_service_test_006, TestSize.Level1)
{
    DHLOGI("DistributedCameraSinkServiceTest::OpenChannel and CloseChannel");
    int32_t ret = g_service->OpenChannel(g_dhId, g_testOpenInfoService);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = g_service->CloseChannel(g_dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS