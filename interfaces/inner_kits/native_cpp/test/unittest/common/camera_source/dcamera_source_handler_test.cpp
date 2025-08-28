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
#include "dcamera_source_handler.h"
#undef private
#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_source_callback.h"
#include "dcamera_source_handler_ipc.h"
#include "dcamera_source_load_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "mock_component_disable.h"
#include "mock_component_enable.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_VER = "1.0";
const std::string TEST_ATTRS = "";
}
void DCameraSourceHandlerTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_source_handler_test_001
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_001, TestSize.Level0)
{
    std::string params = "test001";
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_002
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_002, TestSize.Level0)
{
    std::string params = "test002";
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);

    int32_t systemAbilityId = 4803;
    DCameraSourceHandler::GetInstance().FinishStartSA(params);
    DCameraSourceHandler::GetInstance().FinishStartSAFailed(systemAbilityId);
    ret = DCameraSourceHandler::GetInstance().ReleaseSource();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_003
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_003, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    EnableParam param;
    param.sourceVersion = TEST_VER;
    param.sourceAttrs = TEST_ATTRS;
    param.sinkVersion = TEST_VER;
    param.sinkAttrs = TEST_ATTRS;
    std::shared_ptr<RegisterCallback> callback = nullptr;
    int32_t ret = DCameraSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    callback = std::make_shared<MockComponentEnable>();
    ret = DCameraSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_004
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_004, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    std::string params = "test004";
    std::shared_ptr<UnregisterCallback> uncallback = std::make_shared<MockComponentDisable>();

    int32_t ret = DCameraSourceHandler::GetInstance().UnregisterDistributedHardware(devId, dhId, uncallback);
    ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_005
 * @tc.desc: Verify the OnLoadSystemAbilitySuccess function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_005, TestSize.Level0)
{
    std::string params = "test005";
    int32_t systemAbilityId = 4803;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    sptr<DCameraSourceLoadCallback> loadCallback(new DCameraSourceLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

    remoteObject = nullptr;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_006
 * @tc.desc: Verify the OnLoadSystemAbilityFail function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_006, TestSize.Level0)
{
    std::string params = "test006";
    int32_t systemAbilityId = 4803;
    sptr<DCameraSourceLoadCallback> loadCallback(new DCameraSourceLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);

    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_007
 * @tc.desc: Verify the UpdateDistributedHardwareWorkMode function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_007, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    WorkModeParam param(-1, 0, 0, false);
    int32_t ret = DCameraSourceHandler::GetInstance().UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    param.fd = 12;
    param.sharedMemLen = 120;
    ret = DCameraSourceHandler::GetInstance().UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_NE(DCAMERA_OK, ret);
}
}
}