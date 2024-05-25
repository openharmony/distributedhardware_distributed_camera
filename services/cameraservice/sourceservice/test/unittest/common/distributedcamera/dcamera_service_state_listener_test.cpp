/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "dcamera_service_state_listener.h"
#include "dcamera_source_callback.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraServiceStateListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ICameraStateListener> testListener_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_REQID = "bb536a637105409e904d4da83790a4a7";
std::string TEST_ATTRS = "";
}

void DcameraServiceStateListenerTest::SetUpTestCase(void)
{
    DHLOGI("DcameraServiceStateListenerTest::SetUpTestCase");
}

void DcameraServiceStateListenerTest::TearDownTestCase(void)
{
    DHLOGI("DcameraServiceStateListenerTest::TearDownTestCase");
}

void DcameraServiceStateListenerTest::SetUp(void)
{
    DHLOGI("DcameraServiceStateListenerTest::SetUp");
    testListener_ = std::make_shared<DCameraServiceStateListener>();
}

void DcameraServiceStateListenerTest::TearDown(void)
{
    DHLOGI("DcameraServiceStateListenerTest::TearDown");
    testListener_ = nullptr;
}

/**
 * @tc.name: dcamera_service_state_listener_test_001
 * @tc.desc: Verify the OnRegisterNotify OnUnregisterNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraServiceStateListenerTest, dcamera_service_state_listener_test_001, TestSize.Level1)
{
    DHLOGI("DcameraServiceStateListenerTest::dcamera_service_state_listener_test_001");
    EXPECT_EQ(false, testListener_ == nullptr);

    int32_t status = 0;
    int32_t ret = testListener_->OnRegisterNotify(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = testListener_->OnUnregisterNotify(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_service_state_listener_test_002
 * @tc.desc: Verify the OnRegisterNotify OnUnregisterNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraServiceStateListenerTest, dcamera_service_state_listener_test_002, TestSize.Level1)
{
    DHLOGI("DcameraServiceStateListenerTest::dcamera_service_state_listener_test_002");
    EXPECT_EQ(false, testListener_ == nullptr);

    sptr<IDCameraSourceCallback> callback(new DCameraSourceCallback());
    testListener_->SetCallback(callback);

    int32_t status = 0;
    int32_t ret = testListener_->OnRegisterNotify(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, ret);

    status = 1;
    ret =  testListener_->OnUnregisterNotify(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, TEST_REQID, status, TEST_ATTRS);
    EXPECT_EQ(DCAMERA_OK, ret);

    callback = nullptr;
}

/**
 * @tc.name: dcamera_service_state_listener_test_003
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraServiceStateListenerTest, dcamera_service_state_listener_test_003, TestSize.Level1)
{
    DHLOGI("DcameraServiceStateListenerTest::dcamera_service_state_listener_test_003");
    EXPECT_EQ(false, testListener_ == nullptr);

    int32_t status = 0;
    int32_t ret = testListener_->OnHardwareStateChanged(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_service_state_listener_test_004
 * @tc.desc: Verify the OnDataSyncTrigger function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraServiceStateListenerTest, dcamera_service_state_listener_test_004, TestSize.Level1)
{
    DHLOGI("DcameraServiceStateListenerTest::dcamera_service_state_listener_test_004");
    EXPECT_EQ(false, testListener_ == nullptr);

    int32_t ret = testListener_->OnDataSyncTrigger(TEST_DEVICE_ID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS