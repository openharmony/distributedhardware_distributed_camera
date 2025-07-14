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
#include "dcamera_sink_handler_ipc.h"
#undef private

#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "iremote_broker.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkHandlerIpcTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraSinkHandlerIpcTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerIpcTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerIpcTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraSinkHandlerIpcTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_handler_ipc_test_001
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerIpcTest, dcamera_sink_handler_ipc_test_001, TestSize.Level1)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);

    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_sink_handler_ipc_test_002
 * @tc.desc: Verify the UnInit function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerIpcTest, dcamera_sink_handler_ipc_test_002, TestSize.Level1)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);

    DCameraSinkHandlerIpc::GetInstance().UnInit();
    EXPECT_EQ(false, DCameraSinkHandlerIpc::GetInstance().isInit_);

    DCameraSinkHandlerIpc::GetInstance().UnInit();
    EXPECT_EQ(false, DCameraSinkHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_sink_handler_ipc_test_003
 * @tc.desc: Verify the GetSinkLocalCamSrv function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerIpcTest, dcamera_sink_handler_ipc_test_003, TestSize.Level1)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);

    DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_sink_handler_ipc_test_004
 * @tc.desc: Verify the DeleteSinkLocalCamSrv function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerIpcTest, dcamera_sink_handler_ipc_test_004, TestSize.Level1)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);

    DCameraSinkHandlerIpc::GetInstance().DeleteSinkLocalCamSrv();
    EXPECT_EQ(nullptr, DCameraSinkHandlerIpc::GetInstance().localSink_);
}

/**
 * @tc.name: dcamera_sink_handler_ipc_test_005
 * @tc.desc: Verify the OnSinkLocalCamSrvDied function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkHandlerIpcTest, dcamera_sink_handler_ipc_test_005, TestSize.Level1)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    EXPECT_EQ(true, DCameraSinkHandlerIpc::GetInstance().isInit_);
    int32_t systemAbilityId = 4804;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    wptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    DCameraSinkHandlerIpc::GetInstance().OnSinkLocalCamSrvDied(remoteObject);
    EXPECT_EQ(nullptr, DCameraSinkHandlerIpc::GetInstance().localSink_);
}
}
}