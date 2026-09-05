/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include "dcamera_ipc_interface_code.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_sink_proxy.h"
#include "distributed_camera_sink_stub.h"
#include "distributed_hardware_log.h"
#include "iaccess_listener.h"
#include "mock_distributed_camera_sink_stub.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
static int32_t OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX = 0;
static int32_t OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX = 1;
static int32_t OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX = 2;
}
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
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
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
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
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
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
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
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
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

/**
 * @tc.name: dcamera_sink_stub_test_010
 * @tc.desc: Verify the PauseDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_010, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_010");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string networkId = "test10";
    int32_t ret = sinkProxy.PauseDistributedHardware(networkId);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_011
 * @tc.desc: Verify the ResumeDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_011, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_011");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string networkId = "test11";
    int32_t ret = sinkProxy.ResumeDistributedHardware(networkId);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_012
 * @tc.desc: Verify the StopDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_012, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_012");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string networkId = "test12";
    int32_t ret = sinkProxy.StopDistributedHardware(networkId);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_013
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_013, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_013");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    DistributedCameraSinkProxy sinkProxyNull(nullptr);
    sptr<IAccessListener> listenerNull = nullptr;
    int32_t timeOut = 0;
    std::string pkgName = "pkgName";
    sptr<IAccessListener> listener(new TestAccessListener());
    std::string pkgNameNull = "";
    int32_t ret = sinkProxyNull.SetAccessListener(listener, timeOut, pkgName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkProxy.SetAccessListener(listenerNull, timeOut, pkgName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkProxy.SetAccessListener(listener, timeOut, pkgNameNull);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = sinkProxy.SetAccessListener(listener, timeOut, pkgName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_014
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_014, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_014");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    DistributedCameraSinkProxy sinkProxyNull(nullptr);
    std::string pkgName = "pkgName";
    std::string pkgNameNull = "";
    int32_t ret = sinkProxyNull.RemoveAccessListener(pkgName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_015
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_015, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_015");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    DistributedCameraSinkProxy sinkProxyNull(nullptr);
    std::string requestId = "pkgName";
    bool granted = true;
    int32_t ret = sinkProxyNull.SetAuthorizationResult(requestId, granted);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_016
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_016, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_016");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(DCAMERA_OK, sinkStubPtr->SetAccessListenerInner(data, reply));
}

/**
 * @tc.name: dcamera_sink_stub_test_017
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_017, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_017");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(DCAMERA_OK, sinkStubPtr->RemoveAccessListenerInner(data, reply));
}

/**
 * @tc.name: dcamera_sink_stub_test_018
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_018, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_018");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(DCAMERA_OK, sinkStubPtr->SetAuthorizationResultInner(data, reply));
}

/**
 * @tc.name: dcamera_sink_stub_test_019
 * @tc.desc: Verify ConfigDistributedHardware via proxy with valid params.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_019, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_019");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string devId = "devId_test";
    std::string dhId = "camera_0";
    std::string key = "enable_init_params";
    std::string value = R"({"tokenId": 12345})";
    int32_t ret = sinkProxy.ConfigDistributedHardware(devId, dhId, key, value);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_020
 * @tc.desc: Verify ConfigDistributedHardware via proxy with empty dhId returns DCAMERA_BAD_VALUE.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_020, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_020");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string devId = "devId_test";
    std::string dhId = "";
    std::string key = "enable_init_params";
    std::string value = R"({"tokenId": 12345})";
    int32_t ret = sinkProxy.ConfigDistributedHardware(devId, dhId, key, value);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_021
 * @tc.desc: Verify ConfigDistributedHardware via proxy with empty key returns DCAMERA_BAD_VALUE.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_021, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_021");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string devId = "devId_test";
    std::string dhId = "camera_0";
    std::string key = "";
    std::string value = R"({"tokenId": 12345})";
    int32_t ret = sinkProxy.ConfigDistributedHardware(devId, dhId, key, value);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_022
 * @tc.desc: Verify ConfigDistributedHardware via proxy with empty value returns DCAMERA_BAD_VALUE.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_022, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_022");
    sptr<IRemoteObject> sinkStubPtr(new MockDistributedCameraSinkStub());
    DistributedCameraSinkProxy sinkProxy(sinkStubPtr);
    std::string devId = "devId_test";
    std::string dhId = "camera_0";
    std::string key = "enable_init_params";
    std::string value = "";
    int32_t ret = sinkProxy.ConfigDistributedHardware(devId, dhId, key, value);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_stub_test_023
 * @tc.desc: Verify ConfigDistributedHardwareInner directly with valid params.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_023, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_023");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    data.WriteString("devId_test");
    data.WriteString("camera_0");
    data.WriteString("enable_init_params");
    data.WriteString(R"({"tokenId": 12345})");
    EXPECT_EQ(DCAMERA_OK, sinkStubPtr->ConfigDistributedHardwareInner(data, reply));
    EXPECT_EQ(DCAMERA_OK, reply.ReadInt32());
}

/**
 * @tc.name: dcamera_sink_stub_test_024
 * @tc.desc: Verify ConfigDistributedHardwareInner with empty data still returns DCAMERA_OK (read empty strings).
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_024, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_024");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    EXPECT_EQ(DCAMERA_OK, sinkStubPtr->ConfigDistributedHardwareInner(data, reply));
}

/**
 * @tc.name: dcamera_sink_stub_test_025
 * @tc.desc: Verify OnRemoteRequest routes CONFIG_DISTRIBUTED_HARDWARE code.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkStubTest, dcamera_sink_stub_test_025, TestSize.Level1)
{
    DHLOGI("dcamera_sink_stub_test_025");
    sptr<DistributedCameraSinkStub> sinkStubPtr(new MockDistributedCameraSinkStub());
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    data.WriteString("devId_test");
    data.WriteString("camera_0");
    data.WriteString("enable_init_params");
    data.WriteString(R"({"tokenId": 12345})");
    uint32_t code = static_cast<uint32_t>(IDCameraSinkInterfaceCode::CONFIG_DISTRIBUTED_HARDWARE);
    sinkStubPtr->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DCAMERA_OK, reply.ReadInt32());
}
} // namespace DistributedHardware
} // namespace OHOS