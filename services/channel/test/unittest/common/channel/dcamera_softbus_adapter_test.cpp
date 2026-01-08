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
#include <thread>
#include <chrono>

#include <securec.h>
#include <chrono>
#include <thread>
#define private public
#include "dcamera_softbus_adapter.h"
#include "dcamera_utils_tools.h"
#undef private

#include "data_buffer.h"
#include "dcamera_access_listener_mock.h"
#include "dcamera_allconnect_manager_test.h"
#include "dcamera_channel_listener_mock.h"
#include "dcamera_softbus_session.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "iaccess_listener.h"
#include "session_bus_center.h"

using namespace testing::ext;
using ::testing::_;
using ::testing::An;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::AtLeast;
using ::testing::InSequence;
using ::testing::SetArgReferee;

namespace OHOS {
namespace DistributedHardware {
static bool g_sinkConnect;

bool ManageSelectChannel::GetSinkConnect()
{
    return g_sinkConnect;
}

static sptr<IAccessListener> g_accessListener;

sptr<IAccessListener> DCameraAccessConfigManager::GetAccessListener()
{
    return g_accessListener;
}

static std::string g_pkgName;

std::string DCameraAccessConfigManager::GetAccessPkgName()
{
    return g_pkgName;
}
class TestAccessListener : public IAccessListener {
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    void OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info, const DHType dhType,
        const std::string &pkgName)
    {
        (void)requestId;
        (void)info;
        (void)dhType;
        (void)pkgName;
    }
};

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
constexpr int32_t UNIQUE_SOCKED_ID = 0;
}

void DCameraSoftbusAdapterTest::SetUpTestCase(void)
{
    DCameraAllConnectManagerTest::InitAllConnectManagerMockEnv();
}

void DCameraSoftbusAdapterTest::TearDownTestCase(void)
{
    DCameraAllConnectManagerTest::UnInitAllConnectManagerMockEnv();
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
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    g_sinkConnect = false;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
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
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    g_sinkConnect = false;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    g_sinkConnect = false;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(mySessName, role,
        sessionMode, peerDevId, peerSessName);
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
    std::string mySessName = "sourcetest04";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    g_sinkConnect = false;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(mySessName, role,
        sessionMode, peerDevId, peerSessName);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t sessionId = 2;
    ret = DCameraSoftbusAdapter::GetInstance().SendSofbusStream(sessionId, dataBuffer);
    dataBuffer->SetInt64(TIME_STAMP_US, 1);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);

    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().SourceOnMessage(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
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
    DCameraSoftbusAdapter::GetInstance().SourceOnStream(sessionId, data, ext, param);
    data = nullptr;
    DCameraSoftbusAdapter::GetInstance().SourceOnStream(sessionId, data, ext, param);
    StreamData test03;
    test01.buf = const_cast<char *>(buff01.c_str());
    test01.bufLen = 0;
    data = &test03;
    DCameraSoftbusAdapter::GetInstance().SourceOnStream(sessionId, data, ext, param);
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.emplace(peerDevId + mySessName, session);
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    ManageSelectChannel::GetInstance().SetSinkConnect(true);
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);

    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);
    const void *data = "testdata";
    uint32_t dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().SinkOnMessage(sessionId, data, dataLen);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);
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
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(sessionId, data, ext, param);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
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
    std::string mySessName = "sourcetest013";
    std::string peerSessName = "sinktest012";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = -1;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, info);

    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
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
    std::string mySessName = "sourcetest027";
    std::string peerSessName = "sinktest0027";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 27;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
    mySessName = "sourcetest0027";
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
    sessionId = 2;
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
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionId, data, dataLen);
    dataLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionId, data, dataLen);
    dataLen = 8;
    data = nullptr;
    DCameraSoftbusAdapter::GetInstance().SourceOnBytes(sessionId, data, dataLen);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(sessionId);
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
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, data, dataLen);
    dataLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, data, dataLen);
    data = nullptr;
    dataLen = 8;
    DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, data, dataLen);
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
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(sessionId, data, ext, param);
    data = &test02;
    data->bufLen = 0;
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(sessionId, data, ext, param);
    data->bufLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    DCameraSoftbusAdapter::GetInstance().SinkOnStream(sessionId, data, ext, param);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_032
 * @tc.desc: Verify the CreateSoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_032, TestSize.Level1)
{
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    std::string buff01 = R"({
        "type": 0,
        "index": 1,
        "pts": 1,
        "encodeT": 1,
        "sendT": 1,
        "ver": "test",
    })";
    StreamData test01;
    test01.buf = const_cast<char *>(buff01.c_str());
    test01.bufLen = buff01.size();
    StreamData *data = &test01;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleSourceStreamExt(dataBuffer, data);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    test01.bufLen = 0;
    ret = DCameraSoftbusAdapter::GetInstance().HandleSourceStreamExt(dataBuffer, data);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    test01.bufLen = DCAMERA_MAX_RECV_EXT_LEN + 1;
    ret = DCameraSoftbusAdapter::GetInstance().HandleSourceStreamExt(dataBuffer, data);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_033
 * @tc.desc: Verify the DCameraSoftbusSourceGetSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_033, TestSize.Level1)
{
    std::string sessionName = "sourcetest033";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest033";
    std::string peerSessName = "sourcetest033";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    ManageSelectChannel::GetInstance().SetSrcConnect(true);
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 27;
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
    mySessName = "sourcetest0027";
    ret = DCameraSoftbusAdapter::GetInstance().DCameraSoftbusSourceGetSession(sessionId, session);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
    sessionId = 2;
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);

    int32_t socket = 0;
    DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(socket, session);

    session = nullptr;
    DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(socket, session);
    std::shared_ptr<DataBuffer> buffer = nullptr;
    StreamData *ext = nullptr;
    ret = DCameraSoftbusAdapter::GetInstance().HandleSourceStreamExt(buffer, ext);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_034
 * @tc.desc: Verify the SendSofbusStream function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_034, TestSize.Level1)
{
    std::string sessionName = "sourcetest034";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest034";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    int32_t sessionId = 1;
    dataBuffer->SetInt64(TIME_STAMP_US, 1);
    dataBuffer->SetInt64(FRAME_TYPE, 1);
    dataBuffer->SetInt64(INDEX, 1);
    dataBuffer->SetInt64(START_ENCODE_TIME_US, 0);
    dataBuffer->SetInt64(FINISH_ENCODE_TIME_US, 1);
    ret = DCameraSoftbusAdapter::GetInstance().SendSofbusStream(sessionId, dataBuffer);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_035, TestSize.Level1)
{
    DCameraSoftbusAdapter::GetInstance().CloseSessionWithNetWorkId("");
    std::string sessionName = "sourcetest035";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest035";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    DCameraSoftbusAdapter::GetInstance().CloseSessionWithNetWorkId("testNetworkId");
    EXPECT_NE(ret, DCAMERA_BAD_VALUE);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_036, TestSize.Level1)
{
    std::string myDevId = "abcde";
    std::string peerSessName = "sink_control";
    std::string peerDevId = TEST_DEVICE_ID;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);

    DCameraSoftbusAdapter::GetInstance().CloseSessionWithNetWorkId(peerDevId);
    EXPECT_EQ(ret, UNIQUE_SOCKED_ID);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_037, TestSize.Level1)
{
    std::string myDevId = "abcde";
    std::string sessionName = "sourcetest037";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string peerSessName = "sink_control";
    std::string dhId = "dhId";

    auto listener = std::make_shared<DCameraChannelListenerMock>();
    auto session = std::make_shared<DCameraSoftbusSession>(
        dhId, myDevId, sessionName, peerDevId, peerSessName, listener, DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    EXPECT_CALL(*listener, OnSessionState(_, _)).Times(AtLeast(1));

    DCameraSoftbusAdapter::GetInstance().RecordSourceSocketSession(UNIQUE_SOCKED_ID, session);
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    auto ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(UNIQUE_SOCKED_ID, info);

    DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(UNIQUE_SOCKED_ID);
    DCameraAllConnectManager::RemoveSourceNetworkId(UNIQUE_SOCKED_ID);
    EXPECT_EQ(ret, DCAMERA_OK);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_038, TestSize.Level1)
{
    std::string mySessionName = "control";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNEL_ROLE::DCAMERA_CHANNLE_ROLE_SINK;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    std::string myDevId = "abcde";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string peerSessionName = peerDevId + "_control";
    std::string dhId = "dhId";
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.clear();
    ManageSelectChannel::GetInstance().SetSinkConnect(false);
    g_sinkConnect = false;
    DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(
        mySessionName, role, sessionMode, peerDevId, peerSessionName);
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    EXPECT_CALL(*listener, OnSessionState(_, _)).Times(AtLeast(1));
    auto session = std::make_shared<DCameraSoftbusSession>(
        dhId, myDevId, mySessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    DCameraSoftbusAdapter::GetInstance().sinkSessions_[mySessionName] = session;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessionName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    auto ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(UNIQUE_SOCKED_ID, info);

    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(UNIQUE_SOCKED_ID, SHUTDOWN_REASON_UNEXPECTED);
    DCameraSoftbusAdapter::GetInstance().CloseSoftbusSession(UNIQUE_SOCKED_ID);
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.erase(UNIQUE_SOCKED_ID);
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.erase(mySessionName);
    EXPECT_EQ(ret, DCAMERA_OK);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_039, TestSize.Level1)
{
    std::string networkId = "testNetworkId";
    bool isInvalid = false;
    int32_t typeVal = DCameraSoftbusAdapter::GetInstance().CheckOsType(networkId, isInvalid);
    typeVal = 50;
    std::string jsonStr = "{\"OS_TYPE\": 50}";
    std::string key = "OS_TYPE";
    int32_t result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, typeVal);

    jsonStr = "invalid_json";
    key = "typeVal";
    result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "{\"test\": 80}";
    result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "{\"test\": \"typeVal\"}";
    result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "";
    result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);

    jsonStr = "null";
    key = "typeVal";
    result = DCameraSoftbusAdapter::GetInstance().ParseValueFromCjson(jsonStr, key);
    EXPECT_EQ(result, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_041
 * @tc.desc: Verify the SinkOnBind function updates trustSessionId_ when session name contains "_control".
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_041, TestSize.Level1)
{
    std::string sessionName = "control_test_session";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    ManageSelectChannel::GetInstance().SetSinkConnect(true);
    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        DCameraSoftbusAdapter::GetInstance().mySessionNameMapV2_[peerSessionName] = sessionName;
    } else {
        DCameraSoftbusAdapter::GetInstance().peerDevIdMySessionNameMap_[peerSessionName] = sessionName;
    }
    DCameraSoftbusAdapter::GetInstance().sinkSessions_[sessionName] = session;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessionName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_BYTES,
    };
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.insert(socketId);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(socketId, info);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(socketId, DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_);
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.erase(sessionName);
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.erase(socketId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        DCameraSoftbusAdapter::GetInstance().mySessionNameMapV2_.erase(peerSessionName);
    } else {
        DCameraSoftbusAdapter::GetInstance().peerDevIdMySessionNameMap_.erase(peerSessionName);
    }
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_042
 * @tc.desc: Verify the SinkOnBind function when HandleConflictSession fails.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_042, TestSize.Level1)
{
    std::string sessionName = "conflict_test_session";
    std::string peerSessionName = "dh_video_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = 456;
    ManageSelectChannel::GetInstance().SetSinkConnect(false);
    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        DCameraSoftbusAdapter::GetInstance().mySessionNameMapV2_[peerSessionName] = sessionName;
    } else {
        DCameraSoftbusAdapter::GetInstance().peerDevIdMySessionNameMap_[peerDevId + "_" + sessionName] = sessionName;
    }
    DCameraSoftbusAdapter::GetInstance().sinkSessions_[sessionName] = session;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessionName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.insert(socketId);
    int32_t ret = DCameraSoftbusAdapter::GetInstance().SinkOnBind(socketId, info);
    EXPECT_TRUE(DCAMERA_DEVICE_BUSY == ret || DCAMERA_NOT_FOUND == ret);
    DCameraSoftbusAdapter::GetInstance().sinkSessions_.erase(sessionName);
    DCameraSoftbusAdapter::GetInstance().mySocketSet_.erase(socketId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        DCameraSoftbusAdapter::GetInstance().mySessionNameMapV2_.erase(peerSessionName);
    } else {
        DCameraSoftbusAdapter::GetInstance().peerDevIdMySessionNameMap_.erase(peerDevId
            + std::string("_") + sessionName);
    }
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_043
 * @tc.desc: Verify HandleConflictSession allows control session when no conflict exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_043, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_, socketId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_044
 * @tc.desc: Verify HandleConflictSession allows data continue session when no conflict exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_044, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_dataContinue_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 124;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_, socketId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_045
 * @tc.desc: Verify HandleConflictSession allows data snapshot session when no conflict exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_045, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_dataSnapshot_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 125;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_JPEG);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_, socketId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_047
 * @tc.desc: Verify HandleConflictSession rejects data continue session when conflict exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_047, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_dataContinue_1";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 127;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = 888;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t originalSessionId = DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_DEVICE_BUSY, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_, originalSessionId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_048
 * @tc.desc: Verify HandleConflictSession rejects data snapshot session when conflict exists
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_048, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_dataSnapshot_1";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 128;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_JPEG);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = 777;
    int32_t originalSessionId = DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_DEVICE_BUSY, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_, originalSessionId);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_049
 * @tc.desc: Verify HandleConflictSession allows different session types coexist
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_049, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_dataSnapshot_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 129;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_JPEG);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = 666;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = 555;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_, socketId);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_, 666);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_, 555);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_050
 * @tc.desc: Verify HandleConflictSession handles unknown session type
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_050, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_unknown_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 130;
    std::string networkId = "networkId";
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = -1;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().HandleConflictSession(socketId, session, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_, -1);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_, -1);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_, -1);
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_051
 * @tc.desc: Verify the SinkOnShutDown function properly removes session from sinkSocketSessionMap_.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_051, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_video_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    session->SetSessionId(socketId);
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socketId] = session;
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socketId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_052
 * @tc.desc: Verify the SinkOnShutDown function handles case when session is not found.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_052, TestSize.Level1)
{
    int32_t socketId = 999;
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());

    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socketId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_053
 * @tc.desc: Verify the SinkOnShutDown function handles control session correctly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_053, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL);
    session->SetSessionId(socketId);
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socketId] = session;
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId)->second, session);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socketId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());
}

/**
 * @tc.name: DCameraSoftbusAdapterTest_054
 * @tc.desc: Verify the SinkOnShutDown function handles non-control session correctly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_054, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_video_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    session->SetSessionId(socketId);
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socketId] = session;
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId)->second, session);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socketId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_055, TestSize.Level1)
{
    std::string sessionName = "test_session";
    std::string peerSessionName = "dh_video_0";
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "testDevId";
    std::string myDhId = "camera_0";
    int32_t socketId = 123;
    auto listener = std::make_shared<DCameraChannelListenerMock>();
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>(
        myDhId, myDevId, sessionName, peerDevId, peerSessionName, listener,
        DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO);
    session->SetSessionId(socketId);
    DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_[socketId] = session;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.controlSessionId_ = -1;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataContinueSessionId_ = 123;
    DCameraSoftbusAdapter::GetInstance().trustSessionId_.dataSnapshotSessionId_ = 123;
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(socketId, ShutdownReason::SHUTDOWN_REASON_LOCAL);
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.find(socketId),
              DCameraSoftbusAdapter::GetInstance().sinkSocketSessionMap_.end());
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_056, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string requestId = "0";
    std::string requestIdDiff = "1";
    int32_t timeOutMs = 1000;
    DCameraSoftbusAdapter::GetInstance().StartAuthorizationTimer(requestId, timeOutMs);
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, true);
    std::this_thread::sleep_for(std::chrono::milliseconds(timeOutMs));
    DCameraSoftbusAdapter::GetInstance().StartAuthorizationTimer(requestIdDiff, timeOutMs);
    std::this_thread::sleep_for(std::chrono::milliseconds(timeOutMs * 2));
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().authTimerCancelFlags_[requestId], false);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_057, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string requestId = "2";
    DCameraSoftbusAdapter::GetInstance().CancelAuthorizationTimer(requestId);
    DCameraSoftbusAdapter::GetInstance().authTimerCancelFlags_[requestId] = false;
    EXPECT_EQ(DCameraSoftbusAdapter::GetInstance().authTimerCancelFlags_[requestId], false);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_058, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string requestId = "3";
    std::string networkId = "networkId";
    DCameraSoftbusAdapter::GetInstance().HandleAuthorizationTimeout(requestId);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = "";
    DCameraSoftbusAdapter::GetInstance().HandleAuthorizationTimeout(requestId);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = networkId;
    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DCameraSoftbusAdapter::GetInstance().HandleAuthorizationTimeout(requestId);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = networkId;
    DCameraAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    DCameraSoftbusAdapter::GetInstance().HandleAuthorizationTimeout(requestId);
    EXPECT_EQ(DCameraAccessConfigManager::GetInstance().authorizationResults_[networkId], false);
}

HWTEST_F(DCameraSoftbusAdapterTest, DCameraSoftbusAdapterTest_059, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string requestId = "4";
    std::string networkId = "networkId";
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, false);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = "";
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, false);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = networkId;
    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, false);
    DCameraSoftbusAdapter::GetInstance().pendingAuthRequests_[requestId] = networkId;
    DCameraAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, false);
    EXPECT_EQ(DCameraAccessConfigManager::GetInstance().authorizationResults_[networkId], false);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_060
 * @tc.desc: Verify the ReplaceSuffix function when suffix matches exactly.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_060, TestSize.Level1)
{
    std::string mySessNmRep = "session_suffix";
    const std::string suffix = "_suffix";
    const std::string replacement = "_new";
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ("session_new", mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_061
 * @tc.desc: Verify the ReplaceSuffix function when suffix doesn't match.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_061, TestSize.Level1)
{
    std::string mySessNmRep = "session_test";
    const std::string suffix = "_suffix";
    const std::string replacement = "_new";
    std::string original = mySessNmRep;
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ(original, mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_062
 * @tc.desc: Verify the ReplaceSuffix function when string is shorter than suffix.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_062, TestSize.Level1)
{
    std::string mySessNmRep = "short";
    const std::string suffix = "_longsuffix";
    const std::string replacement = "_new";
    std::string original = mySessNmRep;
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ(original, mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_063
 * @tc.desc: Verify the ReplaceSuffix function when suffix is empty.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_063, TestSize.Level1)
{
    std::string mySessNmRep = "session_test";
    const std::string suffix = "";
    const std::string replacement = "_new";
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ("session_test_new", mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_064
 * @tc.desc: Verify the ReplaceSuffix function when replacement is empty.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_064, TestSize.Level1)
{
    std::string mySessNmRep = "session_suffix";
    const std::string suffix = "_suffix";
    const std::string replacement = "";
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ("session", mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_065
 * @tc.desc: Verify the ReplaceSuffix function when string exactly equals suffix.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_065, TestSize.Level1)
{
    std::string mySessNmRep = "_suffix";
    const std::string suffix = "_suffix";
    const std::string replacement = "_new";
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ("_new", mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_066
 * @tc.desc: Verify the ReplaceSuffix function with partial suffix match.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_066, TestSize.Level1)
{
    std::string mySessNmRep = "session_suffixx";
    const std::string suffix = "_suffix";
    const std::string replacement = "_new";
    std::string original = mySessNmRep;
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ(original, mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_067
 * @tc.desc: Verify the ReplaceSuffix function with special characters in suffix.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_067, TestSize.Level1)
{
    std::string mySessNmRep = "session_123";
    const std::string suffix = "_123";
    const std::string replacement = "_456";
    
    DCameraSoftbusAdapter::GetInstance().ReplaceSuffix(mySessNmRep, suffix, replacement);
    
    EXPECT_EQ("session_456", mySessNmRep);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_068
 * @tc.desc: Verify the CreateSoftbusSessionServer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_068, TestSize.Level1)
{
    std::string sessionName = "sourcetest01";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    g_sinkConnect = true;
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_069
 * @tc.desc: Verify the DestroySoftbusSessionServer function when sessionName does not exist.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_069, TestSize.Level1)
{
    std::string sessionName = "nonexistent_session";
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_070
 * @tc.desc: Verify the DestroySoftbusSessionServer function when sessionTotal_[sessionName] is 1.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_070, TestSize.Level1)
{
    std::string sessionName = "session_to_destroy";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    ASSERT_EQ(DCAMERA_OK, ret);
    
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_071
 * @tc.desc: Verify the DestroySoftbusSessionServer function when sessionTotal_[sessionName] > 1.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_071, TestSize.Level1)
{
    std::string sessionName = "multi_ref_session";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    ASSERT_EQ(DCAMERA_OK, ret);
    
    ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    ASSERT_EQ(DCAMERA_OK, ret);
    
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
    
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_072
 * @tc.desc: Verify the DestroySoftbusSessionServer function with empty sessionName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_072, TestSize.Level1)
{
    std::string sessionName = "";
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_073
 * @tc.desc: Verify the DestroySoftbusSessionServer function with special characters in sessionName.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_073, TestSize.Level1)
{
    std::string sessionName = "session@123#test";
    std::string peerSessionName = "dh_control_0";
    std::string peerDevId = "abcd";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_CTRL;
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreatSoftBusSinkSocketServer(sessionName, role,
        sessionMode, peerDevId, peerSessionName);
    ASSERT_EQ(DCAMERA_OK, ret);
    
    ret = DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_074
 * @tc.desc: Verify the OnSourceSessionOpened function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_074, TestSize.Level1)
{
    std::string sessionName = "sourcetest03";
    DCAMERA_CHANNEL_ROLE role = DCAMERA_CHANNLE_ROLE_SOURCE;
    std::string mySessName = "sourcetest03";
    std::string peerSessName = "sinktest02";
    DCameraSessionMode sessionMode = DCameraSessionMode::DCAMERA_SESSION_MODE_VIDEO;
    std::string peerDevId = TEST_DEVICE_ID;
    std::string myDevId = "abcde";
    std::string myDhId = "mydhid";
    int32_t ret = DCameraSoftbusAdapter::GetInstance().CreateSoftBusSourceSocketClient(myDhId, myDevId,
        peerSessName, peerDevId, sessionMode, role);
    int32_t sessionId = 2;
    PeerSocketInfo info = {
        .name = const_cast<char*>(peerSessName.c_str()),
        .pkgName = const_cast<char*>(DCAMERA_PKG_NAME.c_str()),
        .networkId = const_cast<char*>(peerDevId.c_str()),
        .dataType = TransDataType::DATA_TYPE_VIDEO_STREAM,
    };
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    ret = DCameraSoftbusAdapter::GetInstance().SourceOnBind(sessionId, info);
    EXPECT_NE(DCAMERA_OK, ret);
    DCameraSoftbusAdapter::GetInstance().DestroySoftbusSessionServer(sessionName);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_075
 * @tc.desc: Verify the SourceOnBytes function when dataLen is 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_075, TestSize.Level1)
{
    int32_t socket = 1001;
    uint32_t dataLen = 0;
    const void* data = nullptr;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
}

/**
 * @tc.name: dcamera_softbus_adapter_test_077
 * @tc.desc: Verify the SourceOnBytes function when data is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_077, TestSize.Level1)
{
    int32_t socket = 1003;
    uint32_t dataLen = 1024;
    const void* data = nullptr;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
}

/**
 * @tc.name: dcamera_softbus_adapter_test_078
 * @tc.desc: Verify the SourceOnBytes function when DCameraSoftbusSourceGetSession returns error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_078, TestSize.Level1)
{
    int32_t socket = -1;
    uint32_t dataLen = 1024;
    char buffer[1024];
    const void* data = buffer;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
}

/**
 * @tc.name: dcamera_softbus_adapter_test_079
 * @tc.desc: Verify the SourceOnBytes function when DCameraSoftbusSourceGetSession returns null session.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_079, TestSize.Level1)
{
    int32_t socket = 1005;
    uint32_t dataLen = 1024;
    char buffer[1024];
    const void* data = buffer;
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(1005, nullptr);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
}

/**
 * @tc.name: dcamera_softbus_adapter_test_080
 * @tc.desc: Verify the SourceOnBytes function when memcpy_s fails.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_080, TestSize.Level1)
{
    int32_t socket = 1006;
    uint32_t dataLen = 1024;
    char buffer[1024];
    const void* data = buffer;

    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(1006, session);

    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
}

/**
 * @tc.name: dcamera_softbus_adapter_test_081
 * @tc.desc: Verify the SourceOnBytes function with normal flow.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_081, TestSize.Level1)
{
    int32_t socket = 1007;
    uint32_t dataLen = 1024;
    char buffer[1024];
    memset_s(buffer, dataLen, 'A', dataLen);
    const void* data = buffer;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(1007, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
}

/**
 * @tc.name: dcamera_softbus_adapter_test_082
 * @tc.desc: Verify the SourceOnBytes function with minimum valid dataLen (1 byte).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_082, TestSize.Level1)
{
    int32_t socket = 1008;
    uint32_t dataLen = 1;
    char buffer[1] = {'X'};
    const void* data = buffer;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(1008, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
}

/**
 * @tc.name: dcamera_softbus_adapter_test_083
 * @tc.desc: Verify the SourceOnBytes function with maximum valid dataLen (DCAMERA_MAX_RECV_DATA_LEN).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_083, TestSize.Level1)
{
    int32_t socket = 1009;
    uint32_t dataLen = DCAMERA_MAX_RECV_DATA_LEN;
    char* buffer = new char[dataLen];
    memset_s(buffer, dataLen, 'B', dataLen);
    const void* data = buffer;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(1009, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnBytes(socket, data, dataLen));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] buffer;
}

/**
 * @tc.name: dcamera_softbus_adapter_test_084
 * @tc.desc: Verify the SourceOnStream function when data is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_084, TestSize.Level1)
{
    int32_t socket = 2001;
    StreamData* data = nullptr;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, data, ext, param));
}

/**
 * @tc.name: dcamera_softbus_adapter_test_085
 * @tc.desc: Verify the SourceOnStream function when data->bufLen <= 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_085, TestSize.Level1)
{
    int32_t socket = 2002;
    StreamData streamData;
    streamData.buf = new char[100];
    streamData.bufLen = 0;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_086
 * @tc.desc: Verify the SourceOnStream function when data->bufLen > DCAMERA_MAX_RECV_DATA_LEN.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_086, TestSize.Level1)
{
    int32_t socket = 2003;
    StreamData streamData;
    streamData.buf = new char[DCAMERA_MAX_RECV_DATA_LEN + 1];
    streamData.bufLen = DCAMERA_MAX_RECV_DATA_LEN + 1;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_087
 * @tc.desc: Verify the SourceOnStream function when DCameraSoftbusSourceGetSession returns error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_087, TestSize.Level1)
{
    int32_t socket = -1;
    StreamData streamData;
    streamData.buf = new char[1024];
    streamData.bufLen = 1024;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;

    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_088
 * @tc.desc: Verify the SourceOnStream function when DCameraSoftbusSourceGetSession returns null session.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_088, TestSize.Level1)
{
    int32_t socket = 2005;
    StreamData streamData;
    streamData.buf = new char[1024];
    streamData.bufLen = 1024;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2005, nullptr);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_089
 * @tc.desc: Verify the SourceOnStream function when memcpy_s fails.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_089, TestSize.Level1)
{
    int32_t socket = 2006;
    StreamData streamData;
    streamData.buf = new char[1024];
    streamData.bufLen = 1024;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2006, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_090
 * @tc.desc: Verify the SourceOnStream function when HandleSourceStreamExt returns error.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_090, TestSize.Level1)
{
    int32_t socket = 2007;
    StreamData streamData;
    streamData.buf = new char[1024];
    streamData.bufLen = 1024;
    StreamData streamExt;
    streamExt.buf = new char[100];
    streamExt.bufLen = 100;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2007, session);

    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData,
        &streamExt, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
    delete[] reinterpret_cast<char*>(streamExt.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_091
 * @tc.desc: Verify the SourceOnStream function with normal flow (ext is nullptr).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_091, TestSize.Level1)
{
    int32_t socket = 2008;
    StreamData streamData;
    streamData.buf = new char[1024];
    memset_s(streamData.buf, 1024, 'C', 1024);
    streamData.bufLen = 1024;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2008, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_092
 * @tc.desc: Verify the SourceOnStream function with normal flow.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_092, TestSize.Level1)
{
    int32_t socket = 2009;
    StreamData streamData;
    streamData.buf = new char[1024];
    memset_s(streamData.buf, 1024, 'D', 1024);
    streamData.bufLen = 1024;
    StreamData streamExt;
    streamExt.buf = new char[50];
    memset_s(streamExt.buf, 50, 'E', 50);
    streamExt.bufLen = 50;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2009, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData,
        &streamExt, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
    delete[] reinterpret_cast<char*>(streamExt.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_093
 * @tc.desc: Verify the SourceOnStream function with minimum valid dataLen (1 byte).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_093, TestSize.Level1)
{
    int32_t socket = 2010;
    StreamData streamData;
    streamData.buf = new char[1];
    streamData.buf[0] = 'F';
    streamData.bufLen = 1;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2010, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_094
 * @tc.desc: Verify the SourceOnStream function with maximum valid dataLen (DCAMERA_MAX_RECV_DATA_LEN).
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_094, TestSize.Level1)
{
    int32_t socket = 2011;
    StreamData streamData;
    streamData.buf = new char[DCAMERA_MAX_RECV_DATA_LEN];
    memset_s(streamData.buf, DCAMERA_MAX_RECV_DATA_LEN, 'G', DCAMERA_MAX_RECV_DATA_LEN);
    streamData.bufLen = DCAMERA_MAX_RECV_DATA_LEN;
    StreamData* ext = nullptr;
    StreamFrameInfo* param = nullptr;
    
    std::shared_ptr<DCameraSoftbusSession> session = std::make_shared<DCameraSoftbusSession>();
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.emplace(2011, session);
    
    EXPECT_NO_FATAL_FAILURE(DCameraSoftbusAdapter::GetInstance().SourceOnStream(socket, &streamData, ext, param));
    DCameraSoftbusAdapter::GetInstance().sourceSocketSessionMap_.clear();
    
    delete[] reinterpret_cast<char*>(streamData.buf);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_099
 * @tc.desc: Verify the RequestAndWaitForAuthorization function when listener is nullptr.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_099, TestSize.Level1)
{
    std::string peerNetworkId = "test_network_001";

    g_accessListener = nullptr;
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().RequestAndWaitForAuthorization(peerNetworkId);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_100
 * @tc.desc: Verify the RequestAndWaitForAuthorization function when pkgName is empty.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_100, TestSize.Level1)
{
    std::string peerNetworkId = "test_network_002";
    
    sptr<IAccessListener> listener(new DCameraAccessListenerMock());
    g_accessListener = listener;
    g_pkgName = "";
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().RequestAndWaitForAuthorization(peerNetworkId);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_adapter_test_101
 * @tc.desc: Verify the RequestAndWaitForAuthorization function when pkgName is not empty and timeOut > 0.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusAdapterTest, dcamera_softbus_adapter_test_101, TestSize.Level1)
{
    std::string peerNetworkId = "test_network_003";
    
    sptr<IAccessListener> listener(new DCameraAccessListenerMock());
    g_accessListener = listener;
    g_pkgName = "com.example.camera";
    
    int32_t ret = DCameraSoftbusAdapter::GetInstance().RequestAndWaitForAuthorization(peerNetworkId);
    
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}
