/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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
#include <new>

#define private public
#include "dcamera_source_handler_ipc.h"
#undef private

#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "iremote_broker.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceHandlerIpcTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraSourceHandlerIpcTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerIpcTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerIpcTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerIpcTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_source_handler_ipc_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerIpcTest, dcamera_source_handler_ipc_test_001, TestSize.Level1)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);

    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_source_handler_ipc_test_002
 * @tc.desc: Verify the UnInit function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerIpcTest, dcamera_source_handler_ipc_test_002, TestSize.Level1)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);

    DCameraSourceHandlerIpc::GetInstance().UnInit();
    EXPECT_EQ(false, DCameraSourceHandlerIpc::GetInstance().isInit_);

    DCameraSourceHandlerIpc::GetInstance().UnInit();
    EXPECT_EQ(false, DCameraSourceHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_source_handler_ipc_test_003
 * @tc.desc: Verify the GetSourceLocalCamSrv function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerIpcTest, dcamera_source_handler_ipc_test_003, TestSize.Level1)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);

    DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_source_handler_ipc_test_004
 * @tc.desc: Verify the DeleteSourceLocalCamSrv function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerIpcTest, dcamera_source_handler_ipc_test_004, TestSize.Level1)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);

    DCameraSourceHandlerIpc::GetInstance().DeleteSourceLocalCamSrv();
    EXPECT_EQ(nullptr, DCameraSourceHandlerIpc::GetInstance().localSource_);
}

/**
 * @tc.name: dcamera_source_handler_ipc_test_005
 * @tc.desc: Verify the OnSourceLocalCamSrvDied function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerIpcTest, dcamera_source_handler_ipc_test_005, TestSize.Level1)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSourceHandlerIpc::GetInstance().isInit_);
    int32_t systemAbilityId = 4803;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    wptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    DCameraSourceHandlerIpc::GetInstance().OnSourceLocalCamSrvDied(remoteObject);
    EXPECT_EQ(nullptr, DCameraSourceHandlerIpc::GetInstance().localSource_);
}
}
}