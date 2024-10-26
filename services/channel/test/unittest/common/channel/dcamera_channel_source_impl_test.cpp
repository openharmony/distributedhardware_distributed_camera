/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#include "dcamera_channel_source_impl.h"
#include "dcamera_softbus_adapter.h"
#include "dcamera_source_event.h"
#include "dcamera_source_input.h"
#include "dcamera_source_input_channel_listener.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "data_buffer.h"
#include "icamera_channel.h"
#include "session_bus_center.h"

#include "mock_dcamera_source_dev.h"
#include "mock_dcamera_source_state_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
std::string g_regisStateStr = "";
std::string g_openStateStr = "";
std::string g_captureStateStr = "";
class DCameraChannelSourceImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceInput> input_;
    std::shared_ptr<DCameraSourceDev> camDev_;
    std::shared_ptr<DCameraChannelSourceImpl> channel_;
    std::shared_ptr<ICameraChannelListener> listener_;
    std::shared_ptr<ICameraStateListener> stateListener_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_SLEEP_SEC = 200000;
}
void DCameraChannelSourceImplTest::SetUpTestCase(void)
{
}

void DCameraChannelSourceImplTest::TearDownTestCase(void)
{
}

void DCameraChannelSourceImplTest::SetUp(void)
{
    channel_ = std::make_shared<DCameraChannelSourceImpl>();
}

void DCameraChannelSourceImplTest::TearDown(void)
{
    usleep(TEST_SLEEP_SEC);
    input_ = nullptr;
    camDev_ = nullptr;
    listener_ = nullptr;
    stateListener_ = nullptr;
    channel_ = nullptr;
}

/**
 * @tc.name: dcamera_channel_source_impl_test_001
 * @tc.desc: Verify the OpenSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test001";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);

    input_ = std::make_shared<DCameraSourceInput>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);

    listener_ = std::make_shared<DCameraSourceInputChannelListener>(input_, CONTINUOUS_FRAME);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_002
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test002";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);

    input_ = std::make_shared<DCameraSourceInput>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);

    listener_ = std::make_shared<DCameraSourceInputChannelListener>(input_, CONTINUOUS_FRAME);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    ret = channel_->CloseSession();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_003
 * @tc.desc: Verify the ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test003";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    stateListener_ = std::make_shared<MockDCameraSourceStateListener>();
    camDev_ = std::make_shared<MockDCameraSourceDev>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, stateListener_);

    input_ = std::make_shared<DCameraSourceInput>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, camDev_);

    listener_ = std::make_shared<DCameraSourceInputChannelListener>(input_, CONTINUOUS_FRAME);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    ret = channel_->SendData(dataBuffer);
    channel_->CloseSession();
    ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_006
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    int32_t ret = channel_->CloseSession();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_007
 * @tc.desc: Verify the ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    int32_t ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_008
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_008, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t ret = channel_->SendData(dataBuffer);
    usleep(TEST_SLEEP_SEC);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_channel_source_impl_test_009
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSourceImplTest, dcamera_channel_source_impl_test_009, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test004";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    listener_ = nullptr;
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
}
}