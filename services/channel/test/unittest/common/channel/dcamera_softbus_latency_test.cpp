/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "dcamera_softbus_latency.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "session_bus_center.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSoftbusLatencyTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
}

void DCameraSoftbusLatencyTest::SetUpTestCase(void)
{
}

void DCameraSoftbusLatencyTest::TearDownTestCase(void)
{
}

void DCameraSoftbusLatencyTest::SetUp(void)
{
}

void DCameraSoftbusLatencyTest::TearDown(void)
{
}

/**
 * @tc.name: dcamera_softbus_latency_test_001
 * @tc.desc: Verify the StartSoftbusTimeSync function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusLatencyTest, dcamera_softbus_latency_test_001, TestSize.Level1)
{
    int32_t ret = DCameraSoftbusLatency::GetInstance().StartSoftbusTimeSync(TEST_DEVICE_ID);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_latency_test_002
 * @tc.desc: Verify the StopSoftbusTimeSync function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusLatencyTest, dcamera_softbus_latency_test_002, TestSize.Level1)
{
    int32_t ret = DCameraSoftbusLatency::GetInstance().StartSoftbusTimeSync(TEST_DEVICE_ID);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = DCameraSoftbusLatency::GetInstance().StopSoftbusTimeSync(TEST_DEVICE_ID);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_latency_test_003
 * @tc.desc: Verify the SetTimeSyncInfo function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusLatencyTest, dcamera_softbus_latency_test_003, TestSize.Level1)
{
    int32_t microsecond = 0;
    DCameraSoftbusLatency::GetInstance().SetTimeSyncInfo(microsecond, TEST_DEVICE_ID);
    int32_t ret = DCameraSoftbusLatency::GetInstance().GetTimeSyncInfo(TEST_DEVICE_ID);
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}