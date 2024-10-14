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

#include "property_carrier.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class PropertyCarrierTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<PropertyCarrier> testPropertyCarrier_;
};

void PropertyCarrierTest::SetUpTestCase(void)
{
}

void PropertyCarrierTest::TearDownTestCase(void)
{
}

void PropertyCarrierTest::SetUp(void)
{
    testPropertyCarrier_ = std::make_shared<PropertyCarrier>();
}

void PropertyCarrierTest::TearDown(void)
{
    testPropertyCarrier_ = nullptr;
}

/**
 * @tc.name: property_carrier_test_001
 * @tc.desc: Verify CarrySurfaceProperty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(PropertyCarrierTest, property_carrier_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testPropertyCarrier_ == nullptr);

    sptr<Surface> surface = nullptr;
    int32_t ret = testPropertyCarrier_->CarrySurfaceProperty(surface);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
}
} // namespace DistributedHardware
} // namespace OHOS
