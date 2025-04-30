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

#include <gtest/gtest.h>
#include "icamera_channel_listener.h"
#include "dcamera_sink_controller.h"
#include "dcamera_sink_access_control.h"
#include "dcamera_sink_callback.h"
#include "dcamera_sink_controller_channel_listener.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkControllerChannelListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkControllerChannelListener> channelListener_;
};

void DCameraSinkControllerChannelListenerTest::SetUpTestCase(void)
{
}

void DCameraSinkControllerChannelListenerTest::TearDownTestCase(void)
{
}

void DCameraSinkControllerChannelListenerTest::SetUp(void)
{
    std::shared_ptr<ICameraSinkAccessControl> accessControl = std::make_shared<DCameraSinkAccessControl>();
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    std::shared_ptr<DCameraSinkController> controller =
        std::make_shared<DCameraSinkController>(accessControl, sinkCallback);
    channelListener_ = std::make_shared<DCameraSinkControllerChannelListener>(controller);
}

void DCameraSinkControllerChannelListenerTest::TearDown(void)
{
    channelListener_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_controller_channel_listener_test_001
 * @tc.desc: Verify the OnSessionState function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerChannelListenerTest, dcamera_sink_controller_channel_listener_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_channel_listener_test_001 enter");
    ASSERT_NE(channelListener_, nullptr);
    int32_t state = 1;
    std::string networkId = "networkId";
    channelListener_->OnSessionState(state, networkId);
    EXPECT_EQ(true, state == 1);
}

/**
 * @tc.name: dcamera_sink_controller_channel_listener_test_002
 * @tc.desc: Verify the OnSessionError function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerChannelListenerTest, dcamera_sink_controller_channel_listener_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_channel_listener_test_002 enter");
    ASSERT_NE(channelListener_, nullptr);
    int32_t eventType = 0;
    int32_t eventReason = 1;
    std::string detail = "detail";
    channelListener_->OnSessionError(eventType, eventReason, detail);
    EXPECT_EQ(true, eventType == 0);
}

/**
 * @tc.name: dcamera_sink_controller_channel_listener_test_003
 * @tc.desc: Verify the OnDataReceived function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerChannelListenerTest, dcamera_sink_controller_channel_listener_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_channel_listener_test_003 enter");
    ASSERT_NE(channelListener_, nullptr);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    size_t capacity = 0;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    channelListener_->OnDataReceived(buffers);
    EXPECT_EQ(true, capacity == 0);
}

/**
 * @tc.name: dcamera_sink_controller_channel_listener_test_004
 * @tc.desc: Verify the OnDataReceived function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerChannelListenerTest, dcamera_sink_controller_channel_listener_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_channel_listener_test_004 enter");
    std::shared_ptr<DCameraSinkController> contl;
    auto channelListener = std::make_shared<DCameraSinkControllerChannelListener>(contl);
    int32_t state = 1;
    std::string networkId = "networkId";
    channelListener->OnSessionState(state, networkId);
    int32_t eventType = 0;
    int32_t eventReason = 1;
    std::string detail = "detail";
    channelListener->OnSessionError(eventType, eventReason, detail);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    size_t capacity = 0;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(dataBuffer);
    channelListener->OnDataReceived(buffers);
    EXPECT_EQ(true, capacity == 0);
}
} // namespace DistributedHardware
} // namespace OHOS