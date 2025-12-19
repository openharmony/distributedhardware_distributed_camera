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
#include "dcamera_sink_handler.h"
#undef private

#include "anonymous_string.h"
#include "accesstoken_kit.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"
#include "softbus_common.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_sink_handler_ipc.h"
#include "dcamera_sink_load_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "iaccess_listener.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "idistributed_camera_sink.h"
#include "isystem_ability_load_callback.h"
#include "refbase.h"

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

class DCameraSinkHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
}
void DCameraSinkHandlerTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerTest::SetUp(void)
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

void DCameraSinkHandlerTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_handler_test_001
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_001, TestSize.Level1)
{
    std::string params = "test001";
    int32_t ret = DCameraSinkHandler::GetInstance().InitSink(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_002
 * @tc.desc: Verify the ReleaseSink function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_002, TestSize.Level1)
{
    std::string params = "test002";
    int32_t ret = DCameraSinkHandler::GetInstance().InitSink(params);
    EXPECT_EQ(DCAMERA_OK, ret);

    int32_t systemAbilityId = 4804;
    DCameraSinkHandler::GetInstance().FinishStartSA(params);
    DCameraSinkHandler::GetInstance().FinishStartSAFailed(systemAbilityId);
    ret = DCameraSinkHandler::GetInstance().ReleaseSink();
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_003
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_003, TestSize.Level1)
{
    std::string param = "test003";
    std::string dhId = TEST_CAMERA_DH_ID_0;

    int32_t ret = DCameraSinkHandler::GetInstance().SubscribeLocalHardware(dhId, param);
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_004
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_004, TestSize.Level1)
{
    std::string dhId = TEST_CAMERA_DH_ID_0;

    int32_t ret = DCameraSinkHandler::GetInstance().UnsubscribeLocalHardware(dhId);
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_005
 * @tc.desc: Verify the OnLoadSystemAbilitySuccess function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_005, TestSize.Level1)
{
    std::string params = "test005";
    int32_t systemAbilityId = 4804;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    sptr<DCameraSinkLoadCallback> loadCallback(new DCameraSinkLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

    remoteObject = nullptr;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    int32_t ret = DCameraSinkHandler::GetInstance().InitSink(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_006
 * @tc.desc: Verify the OnLoadSystemAbilityFail function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_006, TestSize.Level1)
{
    std::string params = "test006";
    int32_t systemAbilityId = 4804;
    sptr<DCameraSinkLoadCallback> loadCallback(new DCameraSinkLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);

    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);
    int32_t ret = DCameraSinkHandler::GetInstance().ReleaseSink();
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_008
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_008, TestSize.Level1)
{
    std::shared_ptr<PrivacyResourcesListener> listener;
    int32_t ret = DCameraSinkHandler::GetInstance().RegisterPrivacyResources(listener);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_009
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_009, TestSize.Level1)
{
    std::string networkId = "test009";
    int32_t ret = DCameraSinkHandler::GetInstance().PauseDistributedHardware(networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = DCameraSinkHandler::GetInstance().ResumeDistributedHardware(networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = DCameraSinkHandler::GetInstance().StopDistributedHardware(networkId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_010
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_010, TestSize.Level1)
{
    std::string params = "test010";
    int32_t systemAbilityId = 4804;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    sptr<DCameraSinkLoadCallback> loadCallback(new DCameraSinkLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

    remoteObject = nullptr;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    int32_t ret = DCameraSinkHandler::GetInstance().InitSink(params);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = DCameraSinkHandler::GetInstance().PauseDistributedHardware(params);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = DCameraSinkHandler::GetInstance().ResumeDistributedHardware(params);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = DCameraSinkHandler::GetInstance().StopDistributedHardware(params);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_sink_handler_test_011
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_011, TestSize.Level1)
{
    sptr<IAccessListener> listenerNull = nullptr;
    int32_t timeOut = 0;
    std::string pkgName = "pkgName";
    int32_t ret = DCameraSinkHandler::GetInstance().SetAccessListener(listenerNull, timeOut, pkgName);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    sptr<IAccessListener> listener(new TestAccessListener());
    std::string pkgNameNull = "";
    ret = DCameraSinkHandler::GetInstance().SetAccessListener(listener, timeOut, pkgNameNull);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    ret = DCameraSinkHandler::GetInstance().SetAccessListener(listener, timeOut, pkgName);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: dcamera_sink_handler_test_012
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_012, TestSize.Level1)
{
    std::string pkgName = "pkgName";
    std::string pkgNameNull = "";
    int32_t ret = DCameraSinkHandler::GetInstance().RemoveAccessListener(pkgNameNull);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    ret = DCameraSinkHandler::GetInstance().RemoveAccessListener(pkgName);
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_sink_handler_test_013
 * @tc.desc: Verify the function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerTest, dcamera_sink_handler_test_013, TestSize.Level1)
{
    std::string requestId = "requestId";
    std::string requestIdNull = "";
    bool granted = true;
    int32_t ret = DCameraSinkHandler::GetInstance().SetAuthorizationResult(requestIdNull, granted);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    ret = DCameraSinkHandler::GetInstance().SetAuthorizationResult(requestId, granted);
    EXPECT_EQ(ret, DCAMERA_OK);
}
}
}