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

#include "dcamera_source_callback_proxy.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_proxy.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraSourceCallbackProxyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "bb536a637105409e904d4da83790a4a7";
std::string TEST_ATTRS = "";
}

void DcameraSourceCallbackProxyTest::SetUpTestCase(void)
{
    DHLOGI("DcameraSourceCallbackProxyTest::SetUpTestCase");
}

void DcameraSourceCallbackProxyTest::TearDownTestCase(void)
{
    DHLOGI("DcameraSourceCallbackProxyTest::TearDownTestCase");
}

void DcameraSourceCallbackProxyTest::SetUp(void)
{
    DHLOGI("DcameraSourceCallbackProxyTest::SetUp");
}

void DcameraSourceCallbackProxyTest::TearDown(void)
{
    DHLOGI("DcameraSourceCallbackProxyTest::TearDown");
}

/**
 * @tc.name: dcamera_source_callback_proxy_test_001
 * @tc.desc: Verify the OnNotifyRegResult OnNotifyUnregResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, dcamera_source_callback_proxy_test_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::dcamera_source_callback_proxy_test_001");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    const std::string devId = "";
    int32_t status = 0;
    int32_t ret = callbackProxy->OnNotifyRegResult(devId, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = callbackProxy->OnNotifyUnregResult(devId, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_callback_proxy_test_002
 * @tc.desc: Verify the OnNotifyRegResult OnNotifyUnregResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, dcamera_source_callback_proxy_test_002, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::dcamera_source_callback_proxy_test_002");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    const std::string reqId = "";
    int32_t status = 0;
    int32_t ret = callbackProxy->OnNotifyRegResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, reqId, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = callbackProxy->OnNotifyUnregResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, reqId, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_callback_proxy_test_003
 * @tc.desc: Verify the OnNotifyRegResult OnNotifyUnregResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, dcamera_source_callback_proxy_test_003, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::dcamera_source_callback_proxy_test_003");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    int32_t status = 0;
    int32_t ret = callbackProxy->OnNotifyRegResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, status);

    ret = callbackProxy->OnNotifyUnregResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, status);
}
} // namespace DistributedHardware
} // namespace OHOS