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

#include "data_buffer.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DataBufferTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    size_t capacity_ = 0;
    std::shared_ptr<DataBuffer> dataBuffer_ = nullptr;
};

void DataBufferTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DataBufferTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DataBufferTest::SetUp(void)
{
    DHLOGI("enter");
    size_t capacity = 1;
    dataBuffer_ = std::make_shared<DataBuffer>(capacity);
}

void DataBufferTest::TearDown(void)
{
    DHLOGI("enter");
    dataBuffer_ = nullptr;
}

/**
 * @tc.name: SetRange_001
 * @tc.desc: Verify the SetRange function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DataBufferTest, SetRange_001, TestSize.Level1)
{
    size_t offset = 0;
    size_t size = 0;
    ASSERT_NE(dataBuffer_, nullptr);
    int32_t ret = dataBuffer_->SetRange(offset, size);
    EXPECT_EQ(DCAMERA_OK, ret);
    offset = dataBuffer_->Offset() + 2;
    size = dataBuffer_->Size();
    ret = dataBuffer_->SetRange(offset, size);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: FindInt32_001
 * @tc.desc: Verify the FindInt32 function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DataBufferTest, FindInt32_001, TestSize.Level1)
{
    string name = "test";
    int32_t value = 1;
    ASSERT_NE(dataBuffer_, nullptr);
    dataBuffer_->SetInt32(name, value);
    bool ret = dataBuffer_->FindInt32(name, value);
    EXPECT_EQ(true, ret);
    name = "test1";
    ret = dataBuffer_->FindInt32(name, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: FindInt64_001
 * @tc.desc: Verify the FindInt64 function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DataBufferTest, FindInt62_001, TestSize.Level1)
{
    string name = "test";
    int64_t value = 1;
    ASSERT_NE(dataBuffer_, nullptr);
    dataBuffer_->SetInt64(name, value);
    bool ret = dataBuffer_->FindInt64(name, value);
    EXPECT_EQ(true, ret);
    name = "test1";
    ret = dataBuffer_->FindInt64(name, value);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: FindString_001
 * @tc.desc: Verify the FindString function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DataBufferTest, FindString_001, TestSize.Level1)
{
    string name = "test";
    string value = "test";
    ASSERT_NE(dataBuffer_, nullptr);
    dataBuffer_->SetString(name, value);
    bool ret = dataBuffer_->FindString(name, value);
    EXPECT_EQ(true, ret);
    name = "test1";
    ret = dataBuffer_->FindString(name, value);
    EXPECT_EQ(false, ret);
}
} // namespace DistributedHardware
} // namespace OHOS