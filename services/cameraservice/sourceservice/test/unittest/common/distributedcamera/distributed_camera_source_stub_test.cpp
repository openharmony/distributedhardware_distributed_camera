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

#include "accesstoken_kit.h"
#include "ashmem.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "if_system_ability_manager.h"
#include "dcamera_source_callback.h"
#include "distributed_camera_source_proxy.h"
#include "distributed_camera_source_stub.h"
#include "distributed_hardware_log.h"
#include "mock_distributed_camera_source_stub.h"


using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
static int32_t OHOS_PERMISSION_ENABLE_DISTRIBUTED_HARDWARE_INDEX = 0;
static int32_t OHOS_PERMISSION_DISTRIBUTED_DATASYNC_INDEX = 1;
static int32_t OHOS_PERMISSION_ACCESS_DISTRIBUTED_HARDWARE_INDEX = 2;
}
class DcameraSourceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraSourceStubTest::SetUpTestCase(void)
{
    DHLOGI("DcameraSourceStubTest::SetUpTestCase");
}

void DcameraSourceStubTest::TearDownTestCase(void)
{
    DHLOGI("DcameraSourceStubTest::TearDownTestCase");
}

void DcameraSourceStubTest::SetUp(void)
{
    DHLOGI("DcameraSourceStubTest::SetUp");
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

void DcameraSourceStubTest::TearDown(void)
{
    DHLOGI("DcameraSourceStubTest::TearDown");
}

/**
 * @tc.name: dcamera_source_stub_test_001
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_001");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string params = "params000";
    sptr<IDCameraSourceCallback> callback(new DCameraSourceCallback());
    int32_t ret = sourceProxy.InitSource(params, callback);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_002
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_002, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_002");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    int32_t ret = sourceProxy.ReleaseSource();
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_003
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_003, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_003");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    EnableParam param;
    param.sourceVersion = "1";
    param.sourceAttrs = "sourceattrs";
    param.sinkVersion = "1";
    param.sinkAttrs = "sinkattrs";
    int32_t ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_004
 * @tc.desc: Verify the UnregisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_004, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_004");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    int32_t ret = sourceProxy.UnregisterDistributedHardware(devId, dhId, reqId);
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_005
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_005, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_005");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string events = "events000";
    int32_t ret = sourceProxy.DCameraNotify(devId, dhId, events);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_006
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_006, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_006");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "";
    std::string dhId = "dhId000";
    std::string events = "events000";
    int32_t ret = sourceProxy.DCameraNotify(devId, dhId, events);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId000";
    events = "";
    ret = sourceProxy.DCameraNotify(devId, dhId, events);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "";
    ret = sourceProxy.DCameraNotify(devId, dhId, events);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_007
 * @tc.desc: Verify the UnregisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_007, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_007");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    int32_t ret = sourceProxy.UnregisterDistributedHardware(devId, dhId, reqId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId000";
    reqId = "";
    ret = sourceProxy.UnregisterDistributedHardware(devId, dhId, reqId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "";
    ret = sourceProxy.UnregisterDistributedHardware(devId, dhId, reqId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_008
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_008, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_008");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    EnableParam param;
    param.sourceVersion = "1";
    param.sourceAttrs = "sourceattrs";
    param.sinkVersion = "1";
    param.sinkAttrs = "sinkattrs";
    int32_t ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId000";
    reqId = "";
    ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "";
    ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "dhId000";
    reqId = "reqId000";
    param.sinkVersion = "";
    ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    param.sinkVersion = "1";
    param.sinkAttrs = "";
    ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_009
 * @tc.desc: Verify the UpdateDistributedHardwareWorkMode function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_009, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_009");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "";
    std::string dhId = "dhId000";
    WorkModeParam param(-1, 0, 0, false);

    int32_t ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId000";
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "";
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "dhId000";
    param.sharedMemLen = 120;
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    param.fd = 12;
    param.sharedMemLen = 0;
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_010
 * @tc.desc: Verify the UpdateDistributedHardwareWorkMode function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_010, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_009");
    sptr<IRemoteObject> sourceStubPtr(new MockDistributedCameraSourceStub());
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "";
    std::string dhId = "dhId000";
    WorkModeParam param(-1, 0, 0, false);

    int32_t ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId000";
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "";
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    dhId = "dhId000";
    param.sharedMemLen = 120;
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    uint32_t memLen = sizeof(static_cast<uint32_t>(120));
    std::string memName = "testMemory";
    auto syncSharedMem = OHOS::Ashmem::CreateAshmem(memName.c_str(), memLen);
    param.fd = syncSharedMem->GetAshmemFd();
    param.sharedMemLen = 120;
    ret = sourceProxy.UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_OK, ret);
    syncSharedMem->CloseAshmem();
}
} // namespace DistributedHardware
} // namespace OHOS