/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "distributed_camera_source_service.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSourceServiceTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DistributedCameraSourceService> testSrcService_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "bb536a637105409e904d4da83790a4a7";
}

void DistributedCameraSourceServiceTest::SetUpTestCase(void)
{
    DHLOGI("DistributedCameraSourceServiceTest::SetUpTestCase");
}

void DistributedCameraSourceServiceTest::TearDownTestCase(void)
{
    DHLOGI("DistributedCameraSourceServiceTest::TearDownTestCase");
}

void DistributedCameraSourceServiceTest::SetUp(void)
{
    DHLOGI("DistributedCameraSourceServiceTest::SetUp");
    testSrcService_ = std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
}

void DistributedCameraSourceServiceTest::TearDown(void)
{
    DHLOGI("DistributedCameraSourceServiceTest::TearDown");
    testSrcService_ = nullptr;
}

/**
 * @tc.name: dcamera_source_service_test_001
 * @tc.desc: Verify the RegisterDistributedHardware UnregisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_001, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_001");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    EnableParam param;
    int32_t ret = testSrcService_->RegisterDistributedHardware(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, param);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = testSrcService_->UnregisterDistributedHardware(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_service_test_002
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_002, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_002");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    std::string events;
    int32_t ret = testSrcService_->DCameraNotify(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, events);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_service_test_003
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_003, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_003");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t fd = 0;
    std::vector<std::u16string> args;
    std::u16string str(u"");
    args.push_back(str);
    int ret = testSrcService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS