/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dcamera_video_output_event.h"
#include "data_buffer.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraVideoOutputEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DataBuffer> dataBuffer_ = nullptr;
    std::shared_ptr<EventSender> testsender_ = nullptr;
    std::shared_ptr<DCameraVideoOutputEvent> testDCameraVideoOutputEvent_ = nullptr;
};

namespace {
    constexpr size_t TEST_CAPACITY = 1;
}

void DCameraVideoOutputEventTest::SetUpTestCase(void)
{
        DHLOGI("DCameraVideoOutputEventTest SetUpTestCase");
}

void DCameraVideoOutputEventTest::TearDownTestCase(void)
{
        DHLOGI("DCameraVideoOutputEventTest TearDownTestCase");
}

void DCameraVideoOutputEventTest::SetUp(void)
{
    DHLOGI("DCameraVideoOutputEventTest SetUp");
    dataBuffer_ = std::make_shared<DataBuffer>(TEST_CAPACITY);
    testsender_ = std::make_shared<EventSender>();
    testDCameraVideoOutputEvent_ = std::make_shared<DCameraVideoOutputEvent>(*testsender_, dataBuffer_);
}

void DCameraVideoOutputEventTest::TearDown(void)
{
    DHLOGI("DCameraVideoOutputEventTest SetUp");
    testsender_ = nullptr;
    testDCameraVideoOutputEvent_ = nullptr;
}

/**
 * @tc.name: dcamera_video_output_event_test_001
 * @tc.desc: Verify DCameraVideoOutputEvent init param correct
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraVideoOutputEventTest, dcamera_video_output_event_test_001, TestSize.Level1)
{
    std::shared_ptr<DataBuffer> buffer = testDCameraVideoOutputEvent_->GetParam();
    EXPECT_EQ(buffer->Capacity(), TEST_CAPACITY);
}
} // namespace DistributedHardware
} // namespace OHOS
