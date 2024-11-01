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

#define private public
#include "dcamera_hdf_operate.h"
#undef private

#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraHdfOperateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraHdfOperateTest::SetUpTestCase(void)
{
    DHLOGI("DCameraHdfOperateTest::SetUpTestCase");
}

void DCameraHdfOperateTest::TearDownTestCase(void)
{
    DHLOGI("DCameraHdfOperateTest::TearDownTestCase");
}

void DCameraHdfOperateTest::SetUp(void)
{
    DHLOGI("DCameraHdfOperateTest::SetUp");
}

void DCameraHdfOperateTest::TearDown(void)
{
    DHLOGI("DCameraHdfOperateTest::TearDown");
}

/**
 * @tc.name: dcamera_hdf_operate_test_001
 * @tc.desc: Verify LoadDcameraHDFImpl func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraHdfOperateTest, dcamera_hdf_operate_test_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::dcamera_hdf_operate_test_001");
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_hdf_operate_test_002
 * @tc.desc: Verify UnLoadDcameraHDFImpl func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraHdfOperateTest, dcamera_hdf_operate_test_002, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::dcamera_hdf_operate_test_002");
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDcameraHDFImpl();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_hdf_operate_test_003
 * @tc.desc: Verify LoadDcameraHDFImpl func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraHdfOperateTest, dcamera_hdf_operate_test_003, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::dcamera_hdf_operate_test_003");
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_hdf_operate_test_004
 * @tc.desc: Verify UnLoadDcameraHDFImpl func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraHdfOperateTest, dcamera_hdf_operate_test_004, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::dcamera_hdf_operate_test_004");
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadCameraService();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_hdf_operate_test_005
 * @tc.desc: Verify WaitLoadProviderService func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraHdfOperateTest, dcamera_hdf_operate_test_005, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::dcamera_hdf_operate_test_005");
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadProviderService();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
