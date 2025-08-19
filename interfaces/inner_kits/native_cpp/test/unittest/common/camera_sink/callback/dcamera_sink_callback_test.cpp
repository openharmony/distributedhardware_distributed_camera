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

#include "dcamera_sink_callback.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr uint32_t DID_MAX_SIZE = 256;
}
class DCameraSinkCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkCallback> sinkCallback_;
};

class PrivacyResourcesListenerTest : public PrivacyResourcesListener {
public:
    PrivacyResourcesListenerTest() = default;
    virtual ~PrivacyResourcesListenerTest() = default;

    int32_t OnPrivaceResourceMessage(const ResourceEventType &type, const std::string &subType,
        const std::string &networkId, bool &isSensitive, bool &isSameAccout)
    {
        return 0;
    }
};

class MockDistributedHardwareSinkStateListener : public DistributedHardwareSinkStateListener {
public:
    MockDistributedHardwareSinkStateListener() = default;
    virtual ~MockDistributedHardwareSinkStateListener() = default;

    void OnStateChanged(const std::string &uuid, const std::string &dhId, const BusinessSinkState state) override
    {
        DHLOGI("Handle state changed event");
    }
};

void DCameraSinkCallbackTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkCallbackTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkCallbackTest::SetUp(void)
{
    DHLOGI("enter");
    sinkCallback_ = std::make_shared<DCameraSinkCallback>();
}

void DCameraSinkCallbackTest::TearDown(void)
{
    DHLOGI("enter");
    sinkCallback_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_callback_test_001
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkCallbackTest, dcamera_sink_callback_test_001, TestSize.Level1)
{
    DHLOGI("DCameraSinkCallbackTest dcamera_sink_callback_test_001.");
    EXPECT_EQ(false, sinkCallback_ == nullptr);

    ResourceEventType type = ResourceEventType::EVENT_TYPE_QUERY_RESOURCE;
    const std::string subType = "camera";
    const std::string networkId = "networkId";
    bool isSensitive = false;
    bool isSameAccout = false;
    int32_t ret = sinkCallback_->OnNotifyResourceInfo(type, subType, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<PrivacyResourcesListenerTest> callback = std::make_shared<PrivacyResourcesListenerTest>();
    sinkCallback_->PushPrivacyResCallback(callback);
    ret = sinkCallback_->OnNotifyResourceInfo(type, subType, networkId, isSensitive, isSameAccout);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_callback_test_002
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkCallbackTest, dcamera_sink_callback_test_002, TestSize.Level1)
{
    DHLOGI("DCameraSinkCallbackTest dcamera_sink_callback_test_002.");
    ASSERT_NE(sinkCallback_, nullptr);

    std::string devId = "devId";
    std::string dhId = "dhId";
    int32_t status = 1;
    int32_t ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    std::shared_ptr<DistributedHardwareSinkStateListener> listener =
        std::make_shared<MockDistributedHardwareSinkStateListener>();
    sinkCallback_->RegisterStateListener(listener);
    devId = "";
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    devId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    devId = "devId";
    dhId = "";
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    dhId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    dhId = "dhId";
    status = -1;
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    status = 1;
    ret = sinkCallback_->OnHardwareStateChanged(devId, dhId, status);
    sinkCallback_->UnRegisterStateListener();
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}