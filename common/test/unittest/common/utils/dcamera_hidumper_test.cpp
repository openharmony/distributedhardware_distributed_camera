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

#include "dcamera_hidumper.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraHidumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraHidumperTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DcameraHidumperTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraHidumperTest::SetUp(void)
{
    DHLOGI("enter");
}

void DcameraHidumperTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: GetDumpFlag_001
 * @tc.desc: Verify the GetDumpFlag function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHidumperTest, GetDumpFlag_001, TestSize.Level1)
{
    EXPECT_EQ(false, DcameraHidumper::GetInstance().GetDumpFlag());
}

/**
 * @tc.name: SetDumpFlag_001
 * @tc.desc: Verify the StartDump function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHidumperTest, SetDumpFlag_001, TestSize.Level1)
{
    EXPECT_EQ(DCAMERA_OK, DcameraHidumper::GetInstance().StartDump());
    EXPECT_EQ(true, DcameraHidumper::GetInstance().GetDumpFlag());
}

/**
 * @tc.name: ResetDumpFlag_001
 * @tc.desc: Verify the StopDump function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHidumperTest, ResetDumpFlag_001, TestSize.Level1)
{
    EXPECT_EQ(DCAMERA_OK, DcameraHidumper::GetInstance().StopDump());
    EXPECT_EQ(false, DcameraHidumper::GetInstance().GetDumpFlag());
}
} // namespace DistributedHardware
} // namespace OHOS