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
#include "dcamera_sink_controller_state_callback.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkControllerStateCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkControllerStateCallback> stateCallback_;
};

void DCameraSinkControllerStateCallbackTest::SetUpTestCase(void)
{
}

void DCameraSinkControllerStateCallbackTest::TearDownTestCase(void)
{
}

void DCameraSinkControllerStateCallbackTest::SetUp(void)
{
    std::shared_ptr<ICameraSinkAccessControl> accessControl = std::make_shared<DCameraSinkAccessControl>();
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    std::shared_ptr<DCameraSinkController> controller =
        std::make_shared<DCameraSinkController>(accessControl, sinkCallback);
    stateCallback_ = std::make_shared<DCameraSinkControllerStateCallback>(controller);
}

void DCameraSinkControllerStateCallbackTest::TearDown(void)
{
    stateCallback_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_controller_state_callback_test_001
 * @tc.desc: Verify the OnStateChanged function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerStateCallbackTest, dcamera_sink_controller_state_callback_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_state_callback_test_001 enter");
    ASSERT_NE(stateCallback_, nullptr);
    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    stateCallback_->OnStateChanged(event);
    EXPECT_EQ(true, event != nullptr);
}

/**
 * @tc.name: dcamera_sink_controller_state_callback_test_002
 * @tc.desc: Verify the OnMetadataResult function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerStateCallbackTest, dcamera_sink_controller_state_callback_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_state_callback_test_002 enter");
    ASSERT_NE(stateCallback_, nullptr);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    stateCallback_->OnMetadataResult(settings);
    EXPECT_EQ(true, settings.empty());
}

/**
 * @tc.name: dcamera_sink_controller_state_callback_test_003
 * @tc.desc: Verify the OnDataReceived function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkControllerStateCallbackTest, dcamera_sink_controller_state_callback_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_controller_state_callback_test_003 enter");
    std::shared_ptr<DCameraSinkController> contl;
    auto stateCallback = std::make_shared<DCameraSinkControllerStateCallback>(contl);
    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    stateCallback->OnStateChanged(event);
    std::vector<std::shared_ptr<DCameraSettings>> settings;
    stateCallback->OnMetadataResult(settings);
    EXPECT_EQ(true, settings.empty());
}
} // namespace DistributedHardware
} // namespace OHOS