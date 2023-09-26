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

#include "dcamera_frame_trigger_event.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraFrameTriggerEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::string TEST_PARAM = "testing_param";
    std::shared_ptr<EventSender> testsender_ = nullptr;
    std::shared_ptr<DCameraFrameTriggerEvent> testDCameraFrameTriggerEvent_ = nullptr;
};

void DCameraFrameTriggerEventTest::SetUpTestCase(void)
{
        DHLOGI("DCameraFrameTriggerEventTest SetUpTestCase");
}

void DCameraFrameTriggerEventTest::TearDownTestCase(void)
{
        DHLOGI("DCameraFrameTriggerEventTest TearDownTestCase");
}

void DCameraFrameTriggerEventTest::SetUp(void)
{
    DHLOGI("DCameraFrameTriggerEventTest SetUp");
    testsender_ = std::make_shared<EventSender>();
    testDCameraFrameTriggerEvent_ = std::make_shared<DCameraFrameTriggerEvent>(*testsender_, TEST_PARAM);
}

void DCameraFrameTriggerEventTest::TearDown(void)
{
    DHLOGI("DCameraFrameTriggerEventTest SetUp");
    testsender_ = nullptr;
    testDCameraFrameTriggerEvent_ = nullptr;
}

/**
 * @tc.name: dcamera_frame_trigger_event_test_001
 * @tc.desc: Verify DCameraFrameTriggerEvent init param correct
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraFrameTriggerEventTest, dcamera_frame_trigger_event_test_001, TestSize.Level1)
{
    std::string param = testDCameraFrameTriggerEvent_->GetParam();
    EXPECT_EQ(param, TEST_PARAM);
}
} // namespace DistributedHardware
} // namespace OHOS
