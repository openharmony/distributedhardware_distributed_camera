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

#include "abstract_data_process.h"
#include "distributed_camera_errno.h"
#include "encode_data_process.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class AbstractDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<AbstractDataProcess> testAbstractDataProcess_;
};

void AbstractDataProcessTest::SetUpTestCase(void)
{
}

void AbstractDataProcessTest::TearDownTestCase(void)
{
}

void AbstractDataProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSink> sinkPipeline = std::make_shared<DCameraPipelineSink>();
    testAbstractDataProcess_ = std::make_shared<EncodeDataProcess>(sinkPipeline);
}

void AbstractDataProcessTest::TearDown(void)
{
}

/**
 * @tc.name: abstract_data_process_test_001
 * @tc.desc: Verify SetNextNode.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(AbstractDataProcessTest, abstract_data_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testAbstractDataProcess_ == nullptr);

    std::shared_ptr<AbstractDataProcess> nextDataProcess = nullptr;
    int32_t ret = testAbstractDataProcess_->SetNextNode(nextDataProcess);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
}
} // namespace DistributedHardware
} // namespace OHOS
