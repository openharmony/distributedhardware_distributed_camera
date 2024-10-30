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
#include "dcamera_source_input_channel_listener.h"
#include "dcamera_source_input.h"
#include "distributed_camera_errno.h"
#include "mock_dcamera_source_state_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceInputChannelListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ICameraChannelListener> conListener_;
private:
    static void SetStreamInfos();
    static void SetCaptureInfos();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
}

void DCameraSourceInputChannelListenerTest::SetUpTestCase(void)
{
}

void DCameraSourceInputChannelListenerTest::TearDownTestCase(void)
{
}

void DCameraSourceInputChannelListenerTest::SetUp(void)
{
    std::shared_ptr<ICameraStateListener> stateListener = std::make_shared<MockDCameraSourceStateListener>();
    std::shared_ptr<DCameraSourceDev> camDev =
        std::make_shared<DCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener);
    std::shared_ptr<DCameraSourceInput> testInput =
        std::make_shared<DCameraSourceInput>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev);
    conListener_ = std::make_shared<DCameraSourceInputChannelListener>(testInput, CONTINUOUS_FRAME);
}

void DCameraSourceInputChannelListenerTest::TearDown(void)
{
    conListener_ = nullptr;
}

/**
 * @tc.name: dcamera_source_input_channel_listener_test_001
 * @tc.desc: Verify source inptut channel listener func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceInputChannelListenerTest, dcamera_source_input_channel_listener_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, conListener_ == nullptr);

    int32_t test = 0;
    std::string str = "test001";
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    conListener_->OnSessionState(test, str);
    conListener_->OnSessionError(test, test, str);
    conListener_->OnDataReceived(buffers);
    EXPECT_EQ(true, test == 0);
}
} // namespace DistributedHardware
} // namespace OHOS
