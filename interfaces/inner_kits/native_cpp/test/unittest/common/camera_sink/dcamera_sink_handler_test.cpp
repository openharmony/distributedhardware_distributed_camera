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
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "idistributed_camera_sink.h"
#include "isystem_ability_load_callback.h"
#include "refbase.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();
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
}

void DCameraSinkHandlerTest::TearDown(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerTest::SetTokenID()
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
    SetTokenID();
    int32_t ret = DCameraSinkHandler::GetInstance().InitSink(params);
    EXPECT_EQ(DCAMERA_OK, ret);

    int32_t systemAbilityId = 4804;
    DCameraSinkHandler::GetInstance().FinishStartSA(params);
    DCameraSinkHandler::GetInstance().FinishStartSAFailed(systemAbilityId);
    ret = DCameraSinkHandler::GetInstance().ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
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
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    sptr<DCameraSinkLoadCallback> loadCallback(new DCameraSinkLoadCallback(params));
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
    SetTokenID();
    int32_t systemAbilityId = 4804;
    sptr<DCameraSinkLoadCallback> loadCallback(new DCameraSinkLoadCallback(params));
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);

    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);
    int32_t ret = DCameraSinkHandler::GetInstance().ReleaseSink();
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}