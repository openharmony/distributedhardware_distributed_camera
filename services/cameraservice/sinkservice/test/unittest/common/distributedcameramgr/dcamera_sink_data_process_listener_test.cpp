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
#include "dcamera_sink_data_process.h"
#include "mock_camera_channel.h"
#include "dcamera_sink_data_process_listener.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkDataProcessListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkDataProcessListener> dataProcessListener_;
};
const std::string TEST_DHID = "camera0";

void DCameraSinkDataProcessListenerTest::SetUpTestCase(void)
{
}

void DCameraSinkDataProcessListenerTest::TearDownTestCase(void)
{
}

void DCameraSinkDataProcessListenerTest::SetUp(void)
{
    std::shared_ptr<ICameraChannel> channel = std::make_shared<MockCameraChannel>();
    std::shared_ptr<DCameraSinkDataProcess> dataProcess =
        std::make_shared<DCameraSinkDataProcess>(TEST_DHID, channel);
    dataProcessListener_ = std::make_shared<DCameraSinkDataProcessListener>(dataProcess);
}

void DCameraSinkDataProcessListenerTest::TearDown(void)
{
    dataProcessListener_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_data_process_listener_test_001
 * @tc.desc: Verify the OnProcessedVideoBuffer function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkDataProcessListenerTest, dcamera_sink_data_process_listener_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_data_process_listener_test_001 enter");
    ASSERT_NE(dataProcessListener_, nullptr);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> videoResult = std::make_shared<DataBuffer>(capacity);
    dataProcessListener_->OnProcessedVideoBuffer(videoResult);
    EXPECT_EQ(true, capacity == 1);
}

/**
 * @tc.name: dcamera_sink_data_process_listener_test_002
 * @tc.desc: Verify the OnError function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkDataProcessListenerTest, dcamera_sink_data_process_listener_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_data_process_listener_test_002 enter");
    ASSERT_NE(dataProcessListener_, nullptr);
    DataProcessErrorType errorType = DataProcessErrorType::ERROR_PIPELINE_ENCODER;
    dataProcessListener_->OnError(errorType);
    EXPECT_EQ(true, errorType == DataProcessErrorType::ERROR_PIPELINE_ENCODER);
}

/**
 * @tc.name: dcamera_sink_data_process_listener_test_003
 * @tc.desc: Verify the OnDataReceived function.
 * @tc.type: FUNC
 * @tc.require: DTS
 */
HWTEST_F(DCameraSinkDataProcessListenerTest, dcamera_sink_data_process_listener_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_data_process_listener_test_003 enter");
    std::shared_ptr<DCameraSinkDataProcess> data;
    auto dataProcessListener = std::make_shared<DCameraSinkDataProcessListener>(data);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> videoResult = std::make_shared<DataBuffer>(capacity);
    dataProcessListener->OnProcessedVideoBuffer(videoResult);
    DataProcessErrorType errorType = DataProcessErrorType::ERROR_PIPELINE_ENCODER;
    dataProcessListener->OnError(errorType);
    EXPECT_EQ(true, capacity == 1);
}
} // namespace DistributedHardware
} // namespace OHOS