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

#include "dcamera_source_callback_proxy.h"
#include "dcamera_source_callback.h"
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
    ASSERT_NE(samgr, nullptr);
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
    ASSERT_NE(samgr, nullptr);
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
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    int32_t status = 0;
    int32_t ret = callbackProxy->OnNotifyRegResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, status);

    ret = callbackProxy->OnNotifyUnregResult(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, status);
}

/**
 * @tc.name: dcamera_source_callback_proxy_test_004
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, dcamera_source_callback_proxy_test_004, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::dcamera_source_callback_proxy_test_004");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    int32_t status = 0;
    const std::string maxdirstr(257, '1');
    const std::string testemptystr = "";
    const std::string teststr = "TESTSTR";
    std::string maxdirstr1(50 * 1024 * 1024 + 1, '1');
    std::string emptystr = "";

    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        testemptystr, testemptystr, testemptystr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        maxdirstr, testemptystr, testemptystr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        teststr, testemptystr, testemptystr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        teststr, maxdirstr, testemptystr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        teststr, teststr, testemptystr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        teststr, teststr, maxdirstr, status, emptystr));
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnNotifyRegResult(
        teststr, teststr, teststr, status, maxdirstr1));
}

/**
 * @tc.name: CheckParams_001
 * @tc.desc: Verify the CheckParams function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, CheckParams_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::CheckParams_001");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));

    EXPECT_EQ(false, callbackProxy->CheckParams("", "dhId", 0));
    EXPECT_EQ(false, callbackProxy->CheckParams("devId", "", 0));

    std::string devId(257, '1');
    EXPECT_EQ(false, callbackProxy->CheckParams(devId, "dhId", 0));

    std::string dhId(257, '1');
    EXPECT_EQ(false, callbackProxy->CheckParams("devId", dhId, 0));

    EXPECT_EQ(false, callbackProxy->CheckParams("devId", "dhId", -1));
    EXPECT_EQ(true, callbackProxy->CheckParams("devId", "dhId", 0));
}

/**
 * @tc.name: OnDataSyncTrigger_001
 * @tc.desc: Verify the OnDataSyncTrigger function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, OnDataSyncTrigger_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::OnDataSyncTrigger_001");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));

    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnDataSyncTrigger(""));
    std::string devId(257, '1');
    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnDataSyncTrigger(devId));
}

/**
 * @tc.name: OnDataSyncTrigger_002
 * @tc.desc: Verify the OnDataSyncTrigger function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, OnDataSyncTrigger_002, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::OnDataSyncTrigger_002");

    sptr<DCameraSourceCallback> remoteObject = sptr<DCameraSourceCallback>(new DCameraSourceCallback());
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));

    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnDataSyncTrigger(TEST_DEVICE_ID));
}

/**
 * @tc.name: OnHardwareStateChanged_001
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, OnHardwareStateChanged_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::OnHardwareStateChanged_001");
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));

    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnHardwareStateChanged("", "", -1));
}

/**
 * @tc.name: OnHardwareStateChanged_002
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceCallbackProxyTest, OnHardwareStateChanged_002, TestSize.Level1)
{
    DHLOGI("DcameraSourceCallbackProxyTest::OnHardwareStateChanged_002");

    sptr<DCameraSourceCallback> remoteObject = sptr<DCameraSourceCallback>(new DCameraSourceCallback());
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));

    EXPECT_EQ(DCAMERA_BAD_VALUE, callbackProxy->OnHardwareStateChanged(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, 0));
}
} // namespace DistributedHardware
} // namespace OHOS