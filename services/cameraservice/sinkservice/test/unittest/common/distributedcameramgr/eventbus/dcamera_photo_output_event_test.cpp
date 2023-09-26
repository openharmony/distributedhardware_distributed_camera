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

#include "dcamera_photo_output_event.h"
#include "data_buffer.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraPhotoOutputEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DataBuffer> dataBuffer_ = nullptr;
    std::shared_ptr<EventSender> testsender_ = nullptr;
    std::shared_ptr<DCameraPhotoOutputEvent> testDCameraPhotoOutputEvent_ = nullptr;
};

namespace {
    constexpr size_t TEST_CAPACITY = 1;
}

void DCameraPhotoOutputEventTest::SetUpTestCase(void)
{
        DHLOGI("DCameraPhotoOutputEventTest SetUpTestCase");
}

void DCameraPhotoOutputEventTest::TearDownTestCase(void)
{
        DHLOGI("DCameraPhotoOutputEventTest TearDownTestCase");
}

void DCameraPhotoOutputEventTest::SetUp(void)
{
    DHLOGI("DCameraPhotoOutputEventTest SetUp");
    dataBuffer_ = std::make_shared<DataBuffer>(TEST_CAPACITY);
    testsender_ = std::make_shared<EventSender>();
    testDCameraPhotoOutputEvent_ = std::make_shared<DCameraPhotoOutputEvent>(*testsender_, dataBuffer_);
}

void DCameraPhotoOutputEventTest::TearDown(void)
{
    DHLOGI("DCameraPhotoOutputEventTest SetUp");
    testsender_ = nullptr;
    testDCameraPhotoOutputEvent_ = nullptr;
}

/**
 * @tc.name: dcamera_photo_output_event_test_001
 * @tc.desc: Verify DCameraPhotoOutputEvent init param correct
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPhotoOutputEventTest, dcamera_photo_output_event_test_001, TestSize.Level1)
{
    std::shared_ptr<DataBuffer> buffer = testDCameraPhotoOutputEvent_->GetParam();
    EXPECT_EQ(buffer->Capacity(), TEST_CAPACITY);
}
} // namespace DistributedHardware
} // namespace OHOS
