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

#define private public
#define protected public
#include "distributed_camera_source_service.h"
#undef protected
#undef private

#include "dcamera_source_callback_proxy.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "xcollie/watchdog.h"

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
const int32_t TEST_SOURCE_SERVICE = 200000;
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

    int32_t fd = 1;
    std::vector<std::u16string> args;
    std::u16string str(u"");
    args.push_back(str);
    int ret = testSrcService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_OK, ret);

    for (int i = 0; i < 10242; i++) {
        args.push_back(u"");
    }
    ret = testSrcService_->Dump(fd, args);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_source_service_test_004
 * @tc.desc: Verify the OnStart function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_004, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_004");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t ret = 0;
    testSrcService_->state_ = DCameraServiceState::DCAMERA_SRV_STATE_RUNNING;
    testSrcService_->OnStart();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_service_test_005
 * @tc.desc: Verify the Init function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_005, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_005");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t ret = 0;
    testSrcService_->registerToService_ = true;
    testSrcService_->Init();
    testSrcService_->OnStop();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_service_test_006
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_006, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_006");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    std::string params = "test006";
    sptr<ISystemAbilityManager> samgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    sptr<DCameraSourceCallbackProxy> callbackProxy(new DCameraSourceCallbackProxy(remoteObject));
    testSrcService_->listener_ = std::make_shared<DCameraServiceStateListener>();
    int32_t ret = testSrcService_->InitSource(params, callbackProxy);
    EXPECT_EQ(DCAMERA_OK, ret);
    usleep(TEST_SOURCE_SERVICE);
}

/**
 * @tc.name: dcamera_source_service_test_007
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_007, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_007");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t ret = testSrcService_->ReleaseSource();
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    usleep(TEST_SOURCE_SERVICE);
}

/**
 * @tc.name: dcamera_source_service_test_008
 * @tc.desc: Verify the LoadDCameraHDF function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_008, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_008");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t ret = testSrcService_->LoadDCameraHDF();
    EXPECT_EQ(DCAMERA_OK, ret);
    usleep(TEST_SOURCE_SERVICE);
}

/**
 * @tc.name: dcamera_source_service_test_009
 * @tc.desc: Verify the UnLoadCameraHDF function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_009, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_009");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    int32_t ret = testSrcService_->UnLoadCameraHDF();
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = testSrcService_->LoadDCameraHDF();
    EXPECT_EQ(DCAMERA_OK, ret);
    ret = testSrcService_->UnLoadCameraHDF();
    EXPECT_EQ(DCAMERA_OK, ret);
    usleep(TEST_SOURCE_SERVICE);
}

/**
 * @tc.name: dcamera_source_service_test_010
 * @tc.desc: Verify the UpdateDistributedHardwareWorkMode function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DistributedCameraSourceServiceTest, dcamera_source_service_test_010, TestSize.Level1)
{
    DHLOGI("DistributedCameraSourceServiceTest::dcamera_source_service_test_010");
    EXPECT_EQ(false, testSrcService_ == nullptr);

    WorkModeParam param(12, 120, 0, false);;
    int32_t ret = testSrcService_->UpdateDistributedHardwareWorkMode(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, param);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS