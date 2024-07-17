/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dcamera_sink_callback_proxy.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_proxy.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraSinkCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraSinkCallbackProxyTest::SetUpTestCase(void)
{
    DHLOGI("DcameraSinkCallbackProxyTest::SetUpTestCase");
}

void DcameraSinkCallbackProxyTest::TearDownTestCase(void)
{
    DHLOGI("DcameraSinkCallbackProxyTest::TearDownTestCase");
}

void DcameraSinkCallbackProxyTest::SetUp(void)
{
    DHLOGI("DcameraSinkCallbackProxyTest::SetUp");
}

void DcameraSinkCallbackProxyTest::TearDown(void)
{
    DHLOGI("DcameraSinkCallbackProxyTest::TearDown");
}

/**
 * @tc.name: dcamera_sink_callback_proxy_test_001
 * @tc.desc: Verify the OnNotifyResourceInfo function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkCallbackProxyTest, dcamera_sink_callback_proxy_test_001, TestSize.Level1)
{
    DHLOGI("DcameraSinkCallbackProxyTest::dcamera_sink_callback_proxy_test_001");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    sptr<DCameraSinkCallbackProxy> callbackProxy(new DCameraSinkCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);
}
} // namespace DistributedHardware
} // namespace OHOS