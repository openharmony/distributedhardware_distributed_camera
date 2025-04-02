/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstring>
#include <gtest/gtest.h>

#include "dcamera_collaboration_mock.h"
#include "dcamera_allconnect_manager_test.h"
#include "dlfcn_mock.h"

using namespace testing;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr int32_t VALUABLE_SESSION_ID = 10;
constexpr int32_t UNVALUABLE_SESSION_ID = -1;
constexpr const char* DL_HANDLE = "dlhandle success";
const std::string PEER_NETWORK_ID = "peerNetworkId";
const std::string DH_ID = "dhId";
} // namespace

bool DCameraAllConnectManagerTest::InitAllConnectManagerMockEnv()
{
    auto dlfcnInstance = DlfcnMock::GetOrCreateInstance();
    if (!dlfcnInstance) {
        return false;
    }
    EXPECT_CALL(*dlfcnInstance, DlopenMock(_, _)).WillOnce(Return((void *)DL_HANDLE));
    EXPECT_CALL(*dlfcnInstance, DlsymMock(_, _))
        .WillOnce(Return((void *)DCameraCollaborationMock::DCameraCollaborationExport));
    DCameraAllConnectManager::GetInstance().InitDCameraAllConnectManager();
    return true;
}

bool DCameraAllConnectManagerTest::UnInitAllConnectManagerMockEnv()
{
    auto dlfcnInstance = DlfcnMock::GetOrCreateInstance();
    if (!dlfcnInstance) {
        return false;
    }
    if (!DCameraAllConnectManager::IsInited()) {
        DlfcnMock::ReleaseInstance();
        return false;
    }

    EXPECT_CALL(*dlfcnInstance, DlcloseMock(_)).Times(AtLeast(1));
    DCameraAllConnectManager::GetInstance().UnInitDCameraAllConnectManager();
    DlfcnMock::ReleaseInstance();
    return true;
}

void DCameraAllConnectManagerTest::SetUp()
{}

void DCameraAllConnectManagerTest::TearDown()
{}

void DCameraAllConnectManagerTest::SetUpTestCase(void)
{
    DCameraAllConnectManagerTest::InitAllConnectManagerMockEnv();
}

void DCameraAllConnectManagerTest::TearDownTestCase(void)
{
    DCameraAllConnectManagerTest::UnInitAllConnectManagerMockEnv();
}

HWTEST_F(DCameraAllConnectManagerTest, PublishServiceState_001, testing::ext::TestSize.Level1)
{
    auto ret = DCameraAllConnectManager::GetInstance().PublishServiceState(PEER_NETWORK_ID,
        DH_ID, DCameraCollaborationBussinessStatus::SCM_CONNECTING);
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_OK);
}

HWTEST_F(DCameraAllConnectManagerTest, PublishServiceState_002, testing::ext::TestSize.Level2)
{
    std::string emptyPeerNetworkId;
    auto ret = DCameraAllConnectManager::GetInstance().PublishServiceState(emptyPeerNetworkId,
        DH_ID, DCameraCollaborationBussinessStatus::SCM_CONNECTING);
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_ERR_PUBLISH_STATE);
}

HWTEST_F(DCameraAllConnectManagerTest, ApplyAdvancedResource_001, testing::ext::TestSize.Level1)
{
    DCameraCollaborationResourceRequestInfoSets resultCallback;
    auto ret = DCameraAllConnectManager::GetInstance().ApplyAdvancedResource(PEER_NETWORK_ID, &resultCallback);
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_OK);
}

HWTEST_F(DCameraAllConnectManagerTest, ApplyAdvancedResource_002, testing::ext::TestSize.Level2)
{
    auto ret = DCameraAllConnectManager::GetInstance().ApplyAdvancedResource(PEER_NETWORK_ID, nullptr);
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT);

    std::string emptyPeerNetworkId;
    DCameraCollaborationResourceRequestInfoSets resultCallback;
    ret = DCameraAllConnectManager::GetInstance().ApplyAdvancedResource(emptyPeerNetworkId, &resultCallback);
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT);
}

HWTEST_F(DCameraAllConnectManagerTest, RegisterLifecycleCallback_001, testing::ext::TestSize.Level1)
{
    auto ret = DCameraAllConnectManager::GetInstance().RegisterLifecycleCallback();
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_OK);
    ret = DCameraAllConnectManager::GetInstance().UnRegisterLifecycleCallback();
    EXPECT_EQ(ret, DistributedCameraErrno::DCAMERA_OK);
}

HWTEST_F(DCameraAllConnectManagerTest, BuildResourceRequest_001, testing::ext::TestSize.Level1)
{
    auto ret = DCameraAllConnectManager::GetInstance().BuildResourceRequest();
    EXPECT_TRUE(ret != nullptr);
}

HWTEST_F(DCameraAllConnectManagerTest, SinkSideScenario_001, testing::ext::TestSize.Level1)
{
    DCameraAllConnectManager::SetSinkNetWorkId(PEER_NETWORK_ID, VALUABLE_SESSION_ID);
    auto targetNetworkId = DCameraAllConnectManager::GetSinkDevIdBySocket(VALUABLE_SESSION_ID);
    DCameraAllConnectManager::RemoveSinkNetworkId(VALUABLE_SESSION_ID);
    EXPECT_STREQ(targetNetworkId.c_str(), PEER_NETWORK_ID.c_str());
}

HWTEST_F(DCameraAllConnectManagerTest, SinkSideScenario_002, testing::ext::TestSize.Level2)
{
    DCameraAllConnectManager::SetSinkNetWorkId(std::string(), UNVALUABLE_SESSION_ID);
    DCameraAllConnectManager::SetSinkNetWorkId(PEER_NETWORK_ID, UNVALUABLE_SESSION_ID);
    auto targetNetworkId = DCameraAllConnectManager::GetSinkDevIdBySocket(UNVALUABLE_SESSION_ID);
    DCameraAllConnectManager::RemoveSinkNetworkId(VALUABLE_SESSION_ID);
    EXPECT_TRUE(targetNetworkId.empty());
}

HWTEST_F(DCameraAllConnectManagerTest, SourceSideScenario_001, testing::ext::TestSize.Level1)
{
    DCameraAllConnectManager::SetSourceNetworkId(PEER_NETWORK_ID, VALUABLE_SESSION_ID);
    auto targetSessionId = DCameraAllConnectManager::GetSourceSocketByNetworkId(PEER_NETWORK_ID);
    DCameraAllConnectManager::RemoveSourceNetworkId(VALUABLE_SESSION_ID);
    EXPECT_EQ(targetSessionId, VALUABLE_SESSION_ID);
}

HWTEST_F(DCameraAllConnectManagerTest, SourceSideScenario_002, testing::ext::TestSize.Level2)
{
    DCameraAllConnectManager::SetSourceNetworkId(std::string(), UNVALUABLE_SESSION_ID);
    DCameraAllConnectManager::SetSourceNetworkId(PEER_NETWORK_ID, UNVALUABLE_SESSION_ID);
    auto targetSessionId = DCameraAllConnectManager::GetSourceSocketByNetworkId(PEER_NETWORK_ID);
    DCameraAllConnectManager::RemoveSourceNetworkId(UNVALUABLE_SESSION_ID);
    EXPECT_EQ(targetSessionId, UNVALUABLE_SESSION_ID);
}
} // namespace DistributedHardware
} // OHOS
