/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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
#include "dcamera_sink_callback.h"
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

    std::string GenerateString(size_t length)
    {
        return std::string(length, 'a');
    }
};

namespace {
const size_t DID_MAX_SIZE = 256;
}

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
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    sptr<DCameraSinkCallbackProxy> callbackProxy(new DCameraSinkCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);
}

/**
 * @tc.name: dcamera_sink_callback_proxy_test_002
 * @tc.desc: Verify the OnNotifyResourceInfo function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkCallbackProxyTest, dcamera_sink_callback_proxy_test_002, TestSize.Level1)
{
    DHLOGI("DcameraSinkCallbackProxyTest::dcamera_sink_callback_proxy_test_002");
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>(new DCameraSinkCallback());
    sptr<DCameraSinkCallbackProxy> callbackProxy(new DCameraSinkCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    int32_t ret = DCAMERA_BAD_VALUE;
    ResourceEventType type = ResourceEventType::EVENT_TYPE_QUERY_RESOURCE;
    std::string subtype("");
    std::string newworkId("");
    bool isSensitive = false;
    bool isSameAccout = false;
    ret = callbackProxy->OnNotifyResourceInfo(type, subtype, newworkId, isSensitive, isSameAccout);
    EXPECT_EQ(DCAMERA_OK, ret);
    remoteObject = nullptr;
    callbackProxy = nullptr;
}

/**
 * @tc.name: dcamera_sink_callback_proxy_test_003
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkCallbackProxyTest, dcamera_sink_callback_proxy_test_003, TestSize.Level1)
{
    DHLOGI("DcameraSinkCallbackProxyTest::dcamera_sink_callback_proxy_test_003");
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>(new DCameraSinkCallback());
    sptr<DCameraSinkCallbackProxy> callbackProxy(new DCameraSinkCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    int32_t ret = DCAMERA_BAD_VALUE;
    std::string devId = "test_dev_id";
    std::string dhId = "test_dh_id";
    int32_t status = 1;
    ret = callbackProxy->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_NE(DCAMERA_OK, ret);
    remoteObject = nullptr;
    callbackProxy = nullptr;
}

/**
 * @tc.name: dcamera_sink_callback_proxy_test_004
 * @tc.desc: Verify the CheckParams function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkCallbackProxyTest, dcamera_sink_callback_proxy_test_004, TestSize.Level1)
{
    DHLOGI("DcameraSinkCallbackProxyTest::dcamera_sink_callback_proxy_test_004");
    sptr<IRemoteObject> remoteObject = sptr<IRemoteObject>(new DCameraSinkCallback());
    sptr<DCameraSinkCallbackProxy> callbackProxy(new DCameraSinkCallbackProxy(remoteObject));
    EXPECT_EQ(false, callbackProxy == nullptr);

    std::string devId = "";
    std::string dhId = "valid_dh_id";
    int32_t status = 0;
    bool result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_FALSE(result);

    devId = GenerateString(DID_MAX_SIZE + 1);
    result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_FALSE(result);

    devId = "valid_dev_id";
    dhId = "";
    result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_FALSE(result);

    dhId = GenerateString(DID_MAX_SIZE + 1);
    result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_FALSE(result);

    dhId = "valid_dh_id";
    status = -1;
    result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_FALSE(result);

    status = 0;
    result = callbackProxy->CheckParams(devId, dhId, status);
    EXPECT_TRUE(result);
    remoteObject = nullptr;
    callbackProxy = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS