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

#define private public
#include "dcamera_channel_sink_impl.h"
#undef private

#include "dcamera_softbus_adapter.h"
#include "dcamera_sink_output.h"
#include "dcamera_sink_output_channel_listener.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "icamera_channel.h"
#include "session_bus_center.h"

#include "mock_camera_operator.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
std::string g_operatorStr = "";
class DCameraChannelSinkImplTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkOutput> output_;
    std::shared_ptr<DCameraChannelSinkImpl> channel_;
    std::shared_ptr<ICameraChannelListener> listener_;
    std::shared_ptr<ICameraOperator> operator_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_SLEEP_SEC = 200000;
}
void DCameraChannelSinkImplTest::SetUpTestCase(void)
{
}

void DCameraChannelSinkImplTest::TearDownTestCase(void)
{
}

void DCameraChannelSinkImplTest::SetUp(void)
{
    channel_ = std::make_shared<DCameraChannelSinkImpl>();
}

void DCameraChannelSinkImplTest::TearDown(void)
{
    output_ = nullptr;
    listener_ = nullptr;
    operator_ = nullptr;
    channel_ = nullptr;
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_001
 * @tc.desc: Verify the OpenSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test001";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_002
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test002";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    ret = channel_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_003
 * @tc.desc: Verify the ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test003";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);

    int32_t ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_004
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test004";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    usleep(TEST_SLEEP_SEC);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    ret = channel_->SendData(dataBuffer);
    usleep(TEST_SLEEP_SEC);
    channel_->CloseSession();
    ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_006
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    int32_t ret = channel_->CloseSession();
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_007
 * @tc.desc: Verify the ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    int32_t ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_008
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_008, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t ret = channel_->SendData(dataBuffer);
    usleep(TEST_SLEEP_SEC);
    EXPECT_EQ(DCAMERA_BAD_OPERATE, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_009
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_009, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test009";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;

    listener_ = nullptr;
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_0010
 * @tc.desc: Verify the OnSessionState function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_010, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);
    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    int32_t state = 1;
    int32_t eventType = 1;
    int32_t eventReason = 1;
    std::string detail = "test010";
    listener_->OnSessionState(state, "");
    listener_->OnSessionError(eventType, eventReason, detail);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    buffers.push_back(dataBuffer);
    listener_->OnDataReceived(buffers);
    int32_t ret = channel_->ReleaseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_channel_sink_impl_test_011
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraChannelSinkImplTest, dcamera_channel_sink_impl_test_011, TestSize.Level1)
{
    EXPECT_NE(nullptr, channel_);
    std::vector<DCameraIndex> camIndexs;
    DCameraIndex index;
    index.devId_ = TEST_DEVICE_ID;
    index.dhId_ = TEST_CAMERA_DH_ID_0;
    camIndexs.push_back(index);
    std::string sessionFlag = "test001";
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_JPEG;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);
    channel_->softbusSession_ = nullptr;
    int32_t ret = channel_->CreateSession(camIndexs, sessionFlag, sessionMode, listener_);
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}