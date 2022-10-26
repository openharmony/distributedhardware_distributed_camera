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

#include "buffer_handle_utils.h"
#include "dcamera_buffer_handle.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"


using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraBufferHandleTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraBufferHandleTest::SetUpTestCase(void)
{
    DHLOGI("DcameraBufferHandleTest::SetUpTestCase");
}

void DcameraBufferHandleTest::TearDownTestCase(void)
{
    DHLOGI("DcameraBufferHandleTest::TearDownTestCase");
}

void DcameraBufferHandleTest::SetUp(void)
{
    DHLOGI("DcameraBufferHandleTest::SetUp");
}

void DcameraBufferHandleTest::TearDown(void)
{
    DHLOGI("DcameraBufferHandleTest::TearDown");
}

/**
 * @tc.name: DCameraMemoryMap_001
 * @tc.desc: Verify the DCameraMemoryMap function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraBufferHandleTest, DCameraMemoryMap_001, TestSize.Level1)
{
    BufferHandle *handle = new BufferHandle();
    void* viraddr = DCameraMemoryMap(handle);
    EXPECT_EQ(nullptr, viraddr);
}

/**
 * @tc.name: DCameraMemoryUnmap_001
 * @tc.desc: Verify the DCameraMemoryUnmap function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraBufferHandleTest, DCameraMemoryUnmap_001, TestSize.Level1)
{
    BufferHandle *handle = new BufferHandle();
    DCameraMemoryUnmap(handle);
    EXPECT_EQ(nullptr, handle->virAddr);
}
} // namespace DistributedHardware
} // namespace OHOS