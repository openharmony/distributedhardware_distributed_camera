/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_sink_proxy.h"
#include "distributed_camera_sink_stub.h"
#include "distributed_hardware_log.h"
#include "mock_distributed_camera_sink_stub.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
static int32_t OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX = 0;
static int32_t OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX = 1;
static int32_t OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX = 2;
}
class DcameraSinkStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraSinkStubTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DcameraSinkStubTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraSinkStubTest::SetUp(void)
{
    DHLOGI("enter");
    uint64_t tokenId;
    int32_t numberOfPermissions = 3;
    const char *perms[numberOfPermissions];
    perms[OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX] = "ohos.permission.ENABLE_DISTRIBUTED_HARDWARE";
    perms[OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX] = OHOS_PERMISSION_DISTRIBUTED_DATASYNC;
    perms[OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX] = "ohos.permission.ACCESS_DISTRIBUTED_HARDWARE";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = numberOfPermissions,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dcamera_sink_handler",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

void DcameraSinkStubTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_stub_test_001
 * @tc.desc: Verify the InitSink function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_001");
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string params = "params000";
    int32_t ret = sinkProxy.InitSink(params, sinkCallback);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_002
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_002");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    int32_t ret = sinkProxy.ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_003
 * @tc.desc: Verify the SubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_003");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    std::string params = "params000";
    int32_t ret = sinkProxy.SubscribeLocalHardware(dhId, params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_004
 * @tc.desc: Verify the UnsubscribeLocalHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_004");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    int32_t ret = sinkProxy.UnsubscribeLocalHardware(dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_005
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_005");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    int32_t ret = sinkProxy.StopCapture(dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_006
 * @tc.desc: Verify the ChannelNeg function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_006");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    std::string channelInfo = "channelInfo000";
    int32_t ret = sinkProxy.ChannelNeg(dhId, channelInfo);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_007
 * @tc.desc: Verify the GetCameraInfo function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_007");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    std::string cameraInfo = "cameraInfo000";
    int32_t ret = sinkProxy.GetCameraInfo(dhId, cameraInfo);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_008
 * @tc.desc: Verify the OpenChannel function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_008, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_008");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    std::string openInfo = "openInfo000";
    int32_t ret = sinkProxy.OpenChannel(dhId, openInfo);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_009
 * @tc.desc: Verify the CloseChannel function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_009, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_009");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string dhId = "dhId000";
    int32_t ret = sinkProxy.CloseChannel(dhId);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS