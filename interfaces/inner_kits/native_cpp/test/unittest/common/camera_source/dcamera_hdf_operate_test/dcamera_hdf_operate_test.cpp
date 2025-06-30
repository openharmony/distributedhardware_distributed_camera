/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dcamera_hdf_operate.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "mock_hdfoperate_device_manager.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraHdfOperateTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
private:
    sptr<MockDeviceManager> deviceManager_;
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
    deviceManager_ = MockDeviceManager::GetOrCreateInstance();
}

void DCameraHdfOperateTest::TearDown(void)
{
    DHLOGI("DCameraHdfOperateTest::TearDown");
    MockDeviceManager::ReleaseInstance();
    deviceManager_ = nullptr;
}

class MockHdfDeathCallback : public HdfDeathCallback {
public:
    virtual ~MockHdfDeathCallback() {}
    bool IsCalled()
    {
        return isCalled_;
    }
protected:
    void OnHdfHostDied()
    {
        isCalled_ = true;
    }
private:
    bool isCalled_ = false;
};

/**
 * @tc.name: LoadDcameraHDFImpl_001
 * @tc.desc: Verify LoadDcameraHDFImpl func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDcameraHDFImpl_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDcameraHDFImpl_001");
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl(nullptr);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: LoadDcameraHDFImpl_002
 * @tc.desc: Verify LoadDcameraHDFImpl func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDcameraHDFImpl_002, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDaudioHDFImpl_002");
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(DCAMERA_OK));
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl(nullptr);
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: UnLoadDcameraHDFImpl_001
 * @tc.desc: Verify UnLoadDcameraHDFImpl func
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnLoadDcameraHDFImpl_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnLoadDcameraHDFImpl_001");
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDcameraHDFImpl();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: WaitLoadCameraService_001
 * @tc.desc: Verify WaitLoadCameraService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, WaitLoadCameraService_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::WaitLoadCameraService_001");
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadCameraService();
    EXPECT_EQ(DCAMERA_OK, ret);
    DCameraHdfOperate::GetInstance().cameraServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    ret = DCameraHdfOperate::GetInstance().WaitLoadCameraService();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: WaitLoadProviderService_001
 * @tc.desc: Verify WaitLoadProviderService func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, WaitLoadProviderService_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::WaitLoadProviderService_001");
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    int32_t ret = DCameraHdfOperate::GetInstance().WaitLoadProviderService();
    EXPECT_EQ(DCAMERA_OK, ret);
    DCameraHdfOperate::GetInstance().providerServStatus_.store(OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_STOP);
    ret = DCameraHdfOperate::GetInstance().WaitLoadProviderService();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: LoadDevice_001
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDevice_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDevice_001");
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly(testing::Return(DCAMERA_BAD_OPERATE));
    ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: LoadDevice_002
 * @tc.desc: Verify LoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, LoadDevice_002, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::LoadDevice_002");
    bool isFirstTime = true;
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly([&]()->int32_t {
        if (isFirstTime) {
            isFirstTime = false;
            return HDF_SUCCESS;
        } else {
            return HDF_ERR_DEVICE_BUSY;
        }
    });
    DCameraHdfOperate::GetInstance().cameraServStatus_ = OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START;
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
    isFirstTime = true;
    EXPECT_CALL(*deviceManager_, LoadDevice(_)).WillRepeatedly([&]()->int32_t {
        if (isFirstTime) {
            isFirstTime = false;
            return HDF_SUCCESS;
        } else {
            return DCAMERA_BAD_OPERATE;
        }
    });
    ret = DCameraHdfOperate::GetInstance().LoadDevice();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: UnLoadDevice_001
 * @tc.desc: Verify UnLoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnloadDevice_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnloadDevice_001");
    EXPECT_CALL(*deviceManager_, UnloadDevice(_)).WillRepeatedly(testing::Return(HDF_ERR_DEVICE_BUSY));
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDevice();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: UnLoadDevice_002
 * @tc.desc: Verify UnLoadDevice func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnloadDevice_002, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnloadDevice_002");
    auto devmgr = DCameraHdfOperate::GetInstance().devmgr_;
    DCameraHdfOperate::GetInstance().devmgr_ = nullptr;
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDevice();
    DCameraHdfOperate::GetInstance().devmgr_ = devmgr;
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: UnRegisterHdfListener_001
 * @tc.desc: Verify UnRegisterHdfListener func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, UnRegisterHdfListener_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::UnRegisterHdfListener_001");
    auto camHdiProvider = DCameraHdfOperate::GetInstance().camHdiProvider_;
    DCameraHdfOperate::GetInstance().camHdiProvider_ = nullptr;
    int32_t ret = DCameraHdfOperate::GetInstance().UnRegisterHdfListener();
    DCameraHdfOperate::GetInstance().camHdiProvider_ = camHdiProvider;
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: OnHdfHostDied_001
 * @tc.desc: Verify OnHdfHostDied func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, OnHdfHostDied_001, TestSize.Level1)
{
    auto hdfDeathCallback = std::make_shared<MockHdfDeathCallback>();
    DCameraHdfOperate::GetInstance().hdfDeathCallback_ = nullptr;
    DCameraHdfOperate::GetInstance().OnHdfHostDied();
    EXPECT_EQ(hdfDeathCallback->IsCalled(), false);
    DCameraHdfOperate::GetInstance().hdfDeathCallback_ = hdfDeathCallback;
    DCameraHdfOperate::GetInstance().OnHdfHostDied();
    EXPECT_EQ(hdfDeathCallback->IsCalled(), true);
}

/**
 * @tc.name: AddHdfDeathBind_001
 * @tc.desc: Verify AddHdfDeathBind func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, AddHdfDeathBind_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::AddHdfDeathBind_001");
    auto camHdiProvider = DCameraHdfOperate::GetInstance().camHdiProvider_;
    DCameraHdfOperate::GetInstance().camHdiProvider_ = nullptr;
    int32_t ret = DCameraHdfOperate::GetInstance().AddHdfDeathBind();
    DCameraHdfOperate::GetInstance().camHdiProvider_ = camHdiProvider;
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: RemoveHdfDeathBind_001
 * @tc.desc: Verify RemoveHdfDeathBind func.
 * @tc.type: FUNC
 * @tc.require: AR000GHSJM
 */
HWTEST_F(DCameraHdfOperateTest, RemoveHdfDeathBind_001, TestSize.Level1)
{
    DHLOGI("DCameraHdfOperateTest::RemoveHdfDeathBind_001");
    auto camHdiProvider = DCameraHdfOperate::GetInstance().camHdiProvider_;
    DCameraHdfOperate::GetInstance().camHdiProvider_ = nullptr;
    int32_t ret = DCameraHdfOperate::GetInstance().RemoveHdfDeathBind();
    DCameraHdfOperate::GetInstance().camHdiProvider_ = camHdiProvider;
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
