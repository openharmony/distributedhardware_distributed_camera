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
#define private public
#include "dcamera_source_service_ipc.h"
#undef private

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceServiceIpcTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraSourceServiceIpcTest::SetUpTestCase(void)
{
    DHLOGI("DCameraSourceServiceIpcTest::SetUpTestCase");
}

void DCameraSourceServiceIpcTest::TearDownTestCase(void)
{
    DHLOGI("DCameraSourceServiceIpcTest::TearDownTestCase");
}

void DCameraSourceServiceIpcTest::SetUp(void)
{
    DHLOGI("DCameraSourceServiceIpcTest::SetUp");
}

void DCameraSourceServiceIpcTest::TearDown(void)
{
    DHLOGI("DCameraSourceServiceIpcTest::TearDown");
}

/**
 * @tc.name: dcamera_source_service_ipc_test_001
 * @tc.desc: Verify Init func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceServiceIpcTest, dcamera_source_service_ipc_test_001, TestSize.Level1)
{
    DHLOGI("DCameraSourceServiceIpcTest::dcamera_source_service_ipc_test_001");
    DCameraSourceServiceIpc::GetInstance().Init();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, true);
    DCameraSourceServiceIpc::GetInstance().Init();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, true);
}

/**
 * @tc.name: dcamera_source_service_ipc_test_002
 * @tc.desc: Verify UnInit func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceServiceIpcTest, dcamera_source_service_ipc_test_002, TestSize.Level1)
{
    DHLOGI("DCameraSourceServiceIpcTest::dcamera_source_service_ipc_test_002");
    DCameraSourceServiceIpc::GetInstance().Init();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, true);
    DCameraSourceServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, false);
    DCameraSourceServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, false);
}

/**
 * @tc.name: dcamera_source_service_ipc_test_003
 * @tc.desc: Verify GetSinkRemoteCamSrv func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceServiceIpcTest, dcamera_source_service_ipc_test_003, TestSize.Level1)
{
    DHLOGI("DCameraSourceServiceIpcTest::dcamera_source_service_ipc_test_003");
    std::string deviceId = "";
    DCameraSourceServiceIpc::GetInstance().Init();
    sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(deviceId);
    EXPECT_EQ(nullptr, camSinkSrv);
    DCameraSourceServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, false);
}

/**
 * @tc.name: dcamera_source_service_ipc_test_004
 * @tc.desc: Verify DeleteSinkRemoteCamSrv func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceServiceIpcTest, dcamera_source_service_ipc_test_004, TestSize.Level1)
{
    DHLOGI("DCameraSourceServiceIpcTest::dcamera_source_service_ipc_test_004");
    std::string deviceId = "test004";
    DCameraSourceServiceIpc::GetInstance().Init();
    sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(deviceId);
    DCameraSourceServiceIpc::GetInstance().remoteSinks_.emplace(deviceId, camSinkSrv);
    DCameraSourceServiceIpc::GetInstance().DeleteSinkRemoteCamSrv(deviceId);
    deviceId = "test0004";
    DCameraSourceServiceIpc::GetInstance().DeleteSinkRemoteCamSrv(deviceId);
    camSinkSrv = nullptr;
    DCameraSourceServiceIpc::GetInstance().remoteSinks_.emplace(deviceId, camSinkSrv);
    DCameraSourceServiceIpc::GetInstance().DeleteSinkRemoteCamSrv(deviceId);
    DCameraSourceServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(nullptr, camSinkSrv);
}

/**
 * @tc.name: dcamera_source_service_ipc_test_005
 * @tc.desc: Verify GetSinkRemoteCamSrv func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceServiceIpcTest, dcamera_source_service_ipc_test_005, TestSize.Level1)
{
    DHLOGI("DCameraSourceServiceIpcTest::dcamera_source_service_ipc_test_005");
    std::string deviceId = "";
    DCameraSourceServiceIpc::GetInstance().Init();
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    wptr<IRemoteObject> remote = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    DCameraSourceServiceIpc::GetInstance().OnSinkRemoteCamSrvDied(remote);
    DCameraSourceServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(DCameraSourceServiceIpc::GetInstance().isInit_, false);
}
}
}