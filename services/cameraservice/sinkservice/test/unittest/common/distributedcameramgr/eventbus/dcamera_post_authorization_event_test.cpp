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

#include "dcamera_post_authorization_event.h"
#include "dcamera_capture_info_cmd.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraPostAuthorizationEventTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraCaptureInfo> videoInfo;
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfos;
    std::shared_ptr<EventSender> testsender_;
    std::shared_ptr<DCameraPostAuthorizationEvent> testDCameraPostAuthorizationEvent_;
};

namespace {
    constexpr int32_t TEST_WIDTH = 640;
    constexpr int32_t TEST_HEIGHT = 480;
    constexpr int32_t TEST_FORMAT_VIDEO = OHOS_CAMERA_FORMAT_RGBA_8888;
}

void DCameraPostAuthorizationEventTest::SetUpTestCase(void)
{
        DHLOGI("DCameraPostAuthorizationEventTest SetUpTestCase");
}

void DCameraPostAuthorizationEventTest::TearDownTestCase(void)
{
        DHLOGI("DCameraPostAuthorizationEventTest TearDownTestCase");
}

void DCameraPostAuthorizationEventTest::SetUp(void)
{
    DHLOGI("DCameraPostAuthorizationEventTest SetUp");
    videoInfo = std::make_shared<DCameraCaptureInfo>();
    videoInfo->width_ = TEST_WIDTH;
    videoInfo->height_ = TEST_HEIGHT;
    videoInfo->format_ = TEST_FORMAT_VIDEO;
    videoInfo->isCapture_ = true;
    videoInfo->streamType_ = CONTINUOUS_FRAME;
    captureInfos.push_back(videoInfo);
    testsender_ = std::make_shared<EventSender>();
    testDCameraPostAuthorizationEvent_ = std::make_shared<DCameraPostAuthorizationEvent>(*testsender_, captureInfos);
}

void DCameraPostAuthorizationEventTest::TearDown(void)
{
    DHLOGI("DCameraPostAuthorizationEventTest SetUp");
    testsender_ = nullptr;
    testDCameraPostAuthorizationEvent_ = nullptr;
}

/**
 * @tc.name: dcamera_post_authorization_event_test_001
 * @tc.desc: Verify DCameraPostAuthorizationEvent init param correct
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPostAuthorizationEventTest, dcamera_post_authorization_event_test_001, TestSize.Level1)
{
    std::vector<std::shared_ptr<DCameraCaptureInfo>> info = testDCameraPostAuthorizationEvent_->GetParam();
    EXPECT_EQ(info.size(), captureInfos.size());
}
} // namespace DistributedHardware
} // namespace OHOS
