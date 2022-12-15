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

#include <securec.h>
#define private public
#include "dcamera_softbus_adapter.h"
#undef private

#include "data_buffer.h"
#include "dcamera_softbus_session.h"
#include "dcamera_hisysevent_adapter.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "session_bus_center.h"
#include "session.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSoftbusAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
}

void DCameraSoftbusAdapterTest::SetUpTestCase(void)
{
}

void DCameraSoftbusAdapterTest::TearDownTestCase(void)
{
}

void DCameraSoftbusAdapterTest::SetUp(void)
{
}

void DCameraSoftbusAdapterTest::TearDown(void)
{
}

/**
 * @tc.name: dcamera_softbus_adapter_test_001
 * @tc.desc: Verify the CreateSoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_001, TestSize.Level1)
{
    std::string sessionName = "sourcetest01";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_002
 * @tc.desc: Verify the DestroySoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_002, TestSize.Level1)
{
    std::string sessionName = "sourcetest02";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_003
 * @tc.desc: Verify the DestroySoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_003, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_004
 * @tc.desc: Verify the DestroySoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */

HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_004, TestSize.Level1)
{
    std::string sessionName = "sourcetest04";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest04";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 1;
    ret = DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_005
 * @tc.desc: Verify the SendSofbusBytes function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_005, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t sessionId = 2;
    ret = DCameraSoftbusAdapter::GetInstance().SendSofbusBytes(sessionId, dataBuffer);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_006
 * @tc.desc: Verify the SendSofbusStream function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_006, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t sessionId = 2;
    ret = DCameraSoftbusAdapter::GetInstance().SendSofbusStream(sessionId, dataBuffer);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_007
 * @tc.desc: Verify the GetLocalNetworkId function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_007, TestSize.Level1)
{
    std::string devId = TEST_DEVICE_ID;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().GetLocalNetworkId(devId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_008
 * @tc.desc: Verify the OnSourceSessionOpened function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_008, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSessions_.emplace(peerDevId + mySessName, session);
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_009
 * @tc.desc: Verify the OnSourceSessionClosed function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_009, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);

    DCameraSoftbusAdapter::GetInstance().OnSourceSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_010
 * @tc.desc: Verify the OnSourceBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_010, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().OnSourceBytesReceived(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().OnSourceSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_011
 * @tc.desc: Verify the OnSourceBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_011, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().OnSourceMessageReceived(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().OnSourceSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_012
 * @tc.desc: Verify the OnSourceBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_012, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    std::string buff01 = "testbuffer01";
    StreamData test01;
    test01.buf = const_cast<char *>(buff01.c_str());
    test01.bufLen = buff01.size();
    StreamData test02;
    std::string buff02 = "testbuffer01";
    test02.buf = const_cast<char *>(buff02.c_str());
    test02.bufLen = buff02.size();
    StreamFrameInfo param01;
    StreamData *data = &test01;
    StreamData *ext = &test02;
    StreamFrameInfo *param = &param01;
    DCameraSoftbusAdapter::GetInstance().OnSourceStreamReceived(sessionId, data, ext, param);
    data = nullptr;
    DCameraSoftbusAdapter::GetInstance().OnSourceStreamReceived(sessionId, data, ext, param);
    StreamData test03;
    test01.buf = const_cast<char *>(buff01.c_str());
    test01.bufLen = 0;
    data = &test03;
    DCameraSoftbusAdapter::GetInstance().OnSourceStreamReceived(sessionId, data, ext, param);
    DCameraSoftbusAdapter::GetInstance().OnSourceSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_012
 * @tc.desc: Verify the OnSinkSessionOpened function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_013, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.emplace(peerDevId + mySessName, session);
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}
/**
 * @tc.name: dcamera_softbus_adapter_test_014
 * @tc.desc: Verify the OnSinkSessionClosed function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_014, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);

    DCameraSoftbusAdapter::GetInstance().OnSinkSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_015
 * @tc.desc: Verify the OnSinkBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_015, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().OnSinkBytesReceived(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().OnSinkSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_016
 * @tc.desc: Verify the OnSinkMessageReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_016, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().OnSinkMessageReceived(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().OnSinkSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_017
 * @tc.desc: Verify the OnSinkStreamReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_017, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    std::string buff01 = "testbuffer01";
    StreamData test01;
    test01.buf = const_cast<char *>(buff01.c_str());
    test01.bufLen = buff01.size();
    StreamData test02;
    std::string buff02 = "testbuffer01";
    test02.buf = const_cast<char *>(buff02.c_str());
    test02.bufLen = buff02.size();
    StreamFrameInfo param01;
    const StreamData *data = &test01;
    const StreamData *ext = &test02;
    const StreamFrameInfo *param = &param01;
    DCameraSoftbusAdapter::GetInstance().OnSinkStreamReceived(sessionId, data, ext, param);
    DCameraSoftbusAdapter::GetInstance().OnSinkSessionClosed(sessionId);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_018
 * @tc.desc: Verify the DCameraSoftbusSourceGetSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_018, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_019
 * @tc.desc: Verify the DCameraSoftbusSinkGetSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_019, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSinkGetSession(sessionId, session);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_020
 * @tc.desc: Verify the DCameraSoftbusGetSessionById function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_020, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 2;
    int32_t result = 0;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusGetSessionById(sessionId, session);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_021
 * @tc.desc: Verify the OpenSoftbusSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_021, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 1;
    std::string peerDevId = TEST_DEVICE_ID;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_022
 * @tc.desc: Verify the OpenSoftbusSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_022, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    int32_t sessionMode = 2;
    std::string peerDevId = TEST_DEVICE_ID;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_023
 * @tc.desc: Verify the DCameraSoftbusGetSessionById function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_023, TestSize.Level1)
{
    std::string sessionName = "sourcetest013";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = -1;
    int32_t result = 0;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().OnSinkSessionOpened(sessionId, result);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusGetSessionById(sessionId, session);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_024
 * @tc.desc: Verify the CreateSoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_024, TestSize.Level1)
{
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string sessionName = "sourcetest024";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    EXPECT_EQ(DCAMERA_MEMORY_OPT_ERROR, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_025
 * @tc.desc: Verify the DestroySoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_025, TestSize.Level1)
{
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string sessionName = "sourcetest025";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    sessionName = "sourcetest0025";
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    sessionName = "sourcetest025";
    DCameraSoftbusAdapter::GetInstance().sessionTotal_[sessionName] = 0;
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_026
 * @tc.desc: Verify the DestroySoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_026, TestSize.Level1)
{
    std::string sessionName = "sourcetest026";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest026";
    std::string peerSessName = "sinktest0026";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    sessionMode = 1;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    sessionMode = 2;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    sessionMode = 3;
    ret = DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_027
 * @tc.desc: Verify the DCameraSoftbusSourceGetSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_027, TestSize.Level1)
{
    std::string sessionName = "sourcetest027";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftbusSessionServer(sessionName, role);
    std::string mySessName = "sourcetest027";
    std::string peerSessName = "sinktest0027";
    int32_t sessionMode = 0;
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    int32_t sessionId = 27;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    int32_t result = 0;
    ret = DCameraSoftbusAdapter::GetInstance().OnSourceSessionOpened(sessionId, result);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_MEMORY_OPT_ERROR, ret);
    mySessName = "sourcetest0027";
    DCameraSoftbusAdapter::GetInstance().OpenSoftbusSession(mySessName, peerSessName, sessionMode, peerDevId);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_MEMORY_OPT_ERROR, ret);
    sessionId = 2;
    DCameraSoftbusAdapter::GetInstance().sourceSessions_.clear();
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_028
 * @tc.desc: Verify the OnSourceBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_028, TestSize.Level1)
{
    int32_t sessionId = 2;
    const void *data = "testdata";
    uint32_t dataLen = 0;
    std::string sessionName = "sourcetest028";
    DCameraSoftbusAdapter::GetInstance().OnSourceBytesReceived(sessionId, data, dataLen);
    dataLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().OnSourceBytesReceived(sessionId, data, dataLen);
    dataLen = 8;
    data = nullptr;
    DCameraSoftbusAdapter::GetInstance().OnSourceBytesReceived(sessionId, data, dataLen);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_029
 * @tc.desc: Verify the OnSinkBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_029, TestSize.Level1)
{
    int32_t sessionId = 2;
    const void *data = "testdata";
    uint32_t dataLen = 0;
    std::string sessionName = "sourcetest029";
    DCameraSoftbusAdapter::GetInstance().OnSinkBytesReceived(sessionId, data, dataLen);
    dataLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().OnSinkBytesReceived(sessionId, data, dataLen);
    data = nullptr;
    dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().OnSinkBytesReceived(sessionId, data, dataLen);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_030
 * @tc.desc: Verify the OnSinkBytesReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_030, TestSize.Level1)
{
    std::string sessionName = "sourcetest030";
    int32_t sessionId = 2;
    std::string buff01 = "testbuffer030";
    StreamData test02;
    std::string buff02 = "testbuffer030";
    test02.buf = const_cast<char *>(buff02.c_str());
    test02.bufLen = buff02.size();
    StreamFrameInfo param01;
    StreamData *data = nullptr;
    StreamData *ext = &test02;
    StreamFrameInfo *param = &param01;
    DCameraSoftbusAdapter::GetInstance().OnSinkStreamReceived(sessionId, data, ext, param);
    data = &test02;
    data->bufLen = 0;
    DCameraSoftbusAdapter::GetInstance().OnSinkStreamReceived(sessionId, data, ext, param);
    data->bufLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().OnSinkStreamReceived(sessionId, data, ext, param);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}