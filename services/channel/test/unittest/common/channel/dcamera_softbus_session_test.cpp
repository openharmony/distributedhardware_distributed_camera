/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "dcamera_softbus_session.h"
#undef private

#include "data_buffer.h"
#include "dcamera_softbus_adapter.h"
#include "dcamera_sink_output.h"
#include "dcamera_sink_output_channel_listener.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "icamera_channel.h"
#include "mock_camera_operator.h"
#include "session_bus_center.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSoftbusSessionTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkOutput> output_;
    std::shared_ptr<DCameraSoftbusSession> softbusSession_;
    std::shared_ptr<ICameraChannelListener> listener_;
    std::shared_ptr<ICameraOperator> operator_;
};

namespace {
const std::string TEST_MYDEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_PEERDEVICE_ID = "bb536a637105409e904d4da83790a4a9";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_SLEEP_SEC = 200000;
}

void DCameraSoftbusSessionTest::SetUpTestCase(void)
{
}

void DCameraSoftbusSessionTest::TearDownTestCase(void)
{
}

void DCameraSoftbusSessionTest::SetUp(void)
{
    std::string myDhId = "dhId";
    std::string myDevId = TEST_MYDEVICE_ID;
    std::string mySessionName = "testmysession";
    std::string peerSessionName = "testpeersession";
    std::string peerDevId = TEST_PEERDEVICE_ID;
    DCameraSessionMode sessionMode = DCAMERA_SESSION_MODE_VIDEO;
    operator_ = std::make_shared<MockCameraOperator>();
    output_ = std::make_shared<DCameraSinkOutput>(TEST_CAMERA_DH_ID_0, operator_);

    listener_ = std::make_shared<DCameraSinkOutputChannelListener>(CONTINUOUS_FRAME, output_);

    softbusSession_ = std::make_shared<DCameraSoftbusSession>(myDhId, myDevId, mySessionName, peerDevId,
        peerSessionName, listener_, sessionMode);
}

void DCameraSoftbusSessionTest::TearDown(void)
{
    output_ = nullptr;
    listener_ = nullptr;
    operator_ = nullptr;
    softbusSession_ = nullptr;
}

/**
 * @tc.name: dcamera_softbus_session_test_001
 * @tc.desc: Verify the CloseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_001, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->sessionId_ = 2;
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}


/**
 * @tc.name: dcamera_softbus_session_test_002
 * @tc.desc: Verify the OnSessionOpend function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_002, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    int32_t sessionId = 1;
    std::string sessionName = "dcamera_softbus_session_test_002";
    std::string pkgName = "ohos.dhardware.dcamera";
    std::string networkId = "abcdefg123";
    int32_t ret = softbusSession_->OnSessionOpened(sessionId, "");
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_003
 * @tc.desc: Verify the OnDataReceived function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_003, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::string mySessionName = "testmysession";
    auto runner = AppExecFwk::EventRunner::Create(mySessionName);
    softbusSession_->eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    int32_t ret = softbusSession_->OnDataReceived(buffer);
    usleep(TEST_SLEEP_SEC);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_004
 * @tc.desc: Verify the DealRecvData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_004, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    softbusSession_->mode_ = DCAMERA_SESSION_MODE_VIDEO;
    softbusSession_->DealRecvData(buffer);
    softbusSession_->mode_ = DCAMERA_SESSION_MODE_JPEG;
    softbusSession_->DealRecvData(buffer);
    usleep(TEST_SLEEP_SEC);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_005
 * @tc.desc: Verify the PackRecvData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_005, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    size_t offset = 0;
    size_t size = 22;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    buffer->SetRange(offset, size);
    softbusSession_->PackRecvData(buffer);
    size = 2;
    softbusSession_->PackRecvData(buffer);
    usleep(TEST_SLEEP_SEC);
    buffer = nullptr;
    softbusSession_->PackRecvData(buffer);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_006
 * @tc.desc: Verify the AssembleNoFrag function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_006, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    DCameraSoftbusSession::SessionDataHeader headerPara;
    headerPara.dataLen = 1;
    headerPara.totalLen = 2;
    softbusSession_->AssembleNoFrag(buffer, headerPara);
    headerPara.totalLen = 1;
    softbusSession_->AssembleNoFrag(buffer, headerPara);
    buffer = nullptr;
    softbusSession_->AssembleNoFrag(buffer, headerPara);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_007
 * @tc.desc: Verify the AssembleFrag function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_007, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    DCameraSoftbusSession::SessionDataHeader headerPara;
    headerPara.dataLen = 1;
    headerPara.totalLen = 2;
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_START;
    softbusSession_->AssembleFrag(buffer, headerPara);
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_MID;
    softbusSession_->AssembleFrag(buffer, headerPara);
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_END;
    softbusSession_->AssembleFrag(buffer, headerPara);
    softbusSession_->isWaiting_ = true;
    headerPara.seqNum = 1;
    softbusSession_->nowSeq_ = 1;
    headerPara.subSeq = 1;
    softbusSession_->nowSubSeq_ = 1;
    headerPara.totalLen = 10;
    softbusSession_->AssembleFrag(buffer, headerPara);
    buffer = nullptr;
    softbusSession_->AssembleFrag(buffer, headerPara);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_008
 * @tc.desc: Verify the CheckUnPackBuffer function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_008, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    DCameraSoftbusSession::SessionDataHeader headerPara;
    headerPara.dataLen = 1;
    headerPara.totalLen = 2;
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_START;
    softbusSession_->isWaiting_ = false;
    int32_t ret = softbusSession_->CheckUnPackBuffer(headerPara);
    softbusSession_->isWaiting_ = true;
    softbusSession_->nowSeq_ = 1;
    headerPara.seqNum = 2;
    ret = softbusSession_->CheckUnPackBuffer(headerPara);
    softbusSession_->nowSubSeq_ = 0;
    headerPara.seqNum = 1;
    headerPara.subSeq = 1;
    ret = softbusSession_->CheckUnPackBuffer(headerPara);
    softbusSession_->offset_ = 5;
    softbusSession_->totalLen_ = 2;
    ret = softbusSession_->CheckUnPackBuffer(headerPara);
    softbusSession_->offset_ = 0;
    ret = softbusSession_->CheckUnPackBuffer(headerPara);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_009
 * @tc.desc: Verify the GetFragDataLen function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_009, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    DCameraSoftbusSession::SessionDataHeader headerPara;
    headerPara.dataLen = 1;
    headerPara.totalLen = 2;
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_START;
    uint8_t ptr = 9;
    uint8_t *ptrPacket = &ptr;
    softbusSession_->GetFragDataLen(ptrPacket, headerPara);
    softbusSession_->U16Get(ptrPacket);
    softbusSession_->U32Get(ptrPacket);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_010
 * @tc.desc: Verify the SendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_010, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    DCameraSessionMode mode = DCAMERA_SESSION_MODE_VIDEO;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    int32_t ret = softbusSession_->SendData(mode, buffer);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
    mode = DCAMERA_SESSION_MODE_JPEG;
    ret = softbusSession_->SendData(mode, buffer);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
    mode = static_cast<DCameraSessionMode>(-1);
    ret = softbusSession_->SendData(mode, buffer);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
    softbusSession_->sendFuncMap_.clear();
    ret = softbusSession_->SendData(mode, buffer);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_011
 * @tc.desc: Verify the UnPackSendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_011, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t offset = 1;
    size_t size = DCameraSoftbusSession::BINARY_DATA_PACKET_MAX_LEN - 10;
    size_t capacity = size;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    buffer->SetRange(offset, size);
    int32_t ret = softbusSession_->UnPackSendData(buffer, softbusSession_->sendFuncMap_[DCAMERA_SESSION_MODE_VIDEO]);
    size = DCameraSoftbusSession::BINARY_DATA_PACKET_MAX_LEN + 1;
    buffer->SetRange(offset, size);
    ret = softbusSession_->UnPackSendData(buffer, softbusSession_->sendFuncMap_[DCAMERA_SESSION_MODE_VIDEO]);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_012
 * @tc.desc: Verify the UnPackSendData function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_012, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    DCameraSoftbusSession::SessionDataHeader headerPara;
    headerPara.dataLen = 1;
    headerPara.totalLen = 2;
    headerPara.fragFlag = DCameraSoftbusSession::FRAG_START;
    uint8_t ptr = 9;
    uint32_t len = sizeof(uint8_t) * DCameraSoftbusSession::HEADER_UINT8_NUM;
    uint8_t *header = &ptr;
    softbusSession_->MakeFragDataHeader(headerPara, header, len);
    len = sizeof(uint8_t) * DCameraSoftbusSession::HEADER_UINT8_NUM + sizeof(uint16_t) *
        DCameraSoftbusSession::HEADER_UINT16_NUM + sizeof(uint32_t) * DCameraSoftbusSession::HEADER_UINT32_NUM + 1;
    softbusSession_->MakeFragDataHeader(headerPara, header, len);
    int32_t ret = softbusSession_->CloseSession();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_013
 * @tc.desc: Verify the SendBytes function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_013, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t ret = softbusSession_->SendBytes(buffer);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_CLOSED;
    ret = softbusSession_->SendBytes(buffer);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_014
 * @tc.desc: Verify the SendStream function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_014, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t ret = softbusSession_->SendStream(buffer);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_CLOSED;
    ret = softbusSession_->SendStream(buffer);
    usleep(TEST_SLEEP_SEC);
    EXPECT_EQ(DCAMERA_WRONG_STATE, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_015
 * @tc.desc: Verify the SendStream function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_015, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    int32_t sessionId = 1;
    int32_t ret = softbusSession_->OnSessionClose(sessionId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_016
 * @tc.desc: Verify the OnSessionOpened function when isConflict_ is true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_016, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
    int32_t sessionId = 1;
    std::string networkId = "test_network_id";
    int32_t ret = softbusSession_->OnSessionOpened(sessionId, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(sessionId, softbusSession_->sessionId_);
    EXPECT_EQ(DCAMERA_SOFTBUS_STATE_OPENED, softbusSession_->state_);
}

/**
 * @tc.name: dcamera_softbus_session_test_017
 * @tc.desc: Verify the OnSessionClose function when isConflict_ is true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_017, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
    int32_t sessionId = 1;
    int32_t ret = softbusSession_->OnSessionClose(sessionId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(-1, softbusSession_->sessionId_);
    EXPECT_EQ(DCAMERA_SOFTBUS_STATE_CLOSED, softbusSession_->state_);
}

/**
 * @tc.name: dcamera_softbus_session_test_018
 * @tc.desc: Verify the ReleaseSession function.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_018, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    int32_t testSessionId = 5;
    softbusSession_->sessionId_ = testSessionId;
    softbusSession_->ReleaseSession();
    EXPECT_NE(nullptr, softbusSession_);
}

/**
 * @tc.name: dcamera_softbus_session_test_019
 * @tc.desc: Verify the SetConflict function - set to true.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_019, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    EXPECT_FALSE(softbusSession_->isConflict_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
}

/**
 * @tc.name: dcamera_softbus_session_test_020
 * @tc.desc: Verify the SetConflict function - set to false.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_020, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
    softbusSession_->SetConflict(false);
    EXPECT_FALSE(softbusSession_->isConflict_);
}

/**
 * @tc.name: dcamera_softbus_session_test_021
 * @tc.desc: Verify the SetConflict function - affects OnSessionOpened behavior.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_021, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
    int32_t sessionId = 1;
    std::string networkId = "test_network_id";
    int32_t ret = softbusSession_->OnSessionOpened(sessionId, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(sessionId, softbusSession_->sessionId_);
    EXPECT_EQ(DCAMERA_SOFTBUS_STATE_OPENED, softbusSession_->state_);
    softbusSession_->SetConflict(false);
    EXPECT_FALSE(softbusSession_->isConflict_);
    ret = softbusSession_->OnSessionOpened(sessionId, networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_022
 * @tc.desc: Verify the SetConflict function - affects OnSessionClose behavior.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_022, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->SetConflict(true);
    EXPECT_TRUE(softbusSession_->isConflict_);
    int32_t sessionId = 1;
    int32_t ret = softbusSession_->OnSessionClose(sessionId);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(-1, softbusSession_->sessionId_);
    EXPECT_EQ(DCAMERA_SOFTBUS_STATE_CLOSED, softbusSession_->state_);
    softbusSession_->SetConflict(false);
    EXPECT_FALSE(softbusSession_->isConflict_);
    ret = softbusSession_->OnSessionClose(sessionId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_023
 * @tc.desc: Verify the NotifyError function with normal parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_023, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = 1;
    int32_t eventReason = 2;
    std::string detail = "Test error detail";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_024
 * @tc.desc: Verify the NotifyError function with empty detail string.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_024, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = 1;
    int32_t eventReason = 2;
    std::string detail = "";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_025
 * @tc.desc: Verify the NotifyError function with long detail string.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_025, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = 1;
    int32_t eventReason = 2;
    std::string detail = "This is a very long error detail string that contains many characters "
        "and should test how the NotifyError function handles longer strings without issues "
        "and still properly marshals and sends the data correctly.";
    
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_026
 * @tc.desc: Verify the NotifyError function with negative parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_026, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = -1;
    int32_t eventReason = -1;
    std::string detail = "Test error with negative values";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_027
 * @tc.desc: Verify the NotifyError function with special characters in detail.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_027, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = 1;
    int32_t eventReason = 2;
    std::string detail = "Test error with special characters: !@#$%^&*()_+-={}[]|\\:;\"'<>,.?/";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_028
 * @tc.desc: Verify the NotifyError function with zero parameters.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_028, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = 0;
    int32_t eventReason = 0;
    std::string detail = "Test error with zero values";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_softbus_session_test_029
 * @tc.desc: Verify the NotifyError function with maximum integer values.
 * @tc.type: FUNC
 * @tc.require:
 */
HWTEST_F(DCameraSoftbusSessionTest, dcamera_softbus_session_test_029, TestSize.Level1)
{
    EXPECT_NE(nullptr, softbusSession_);
    softbusSession_->state_ = DCAMERA_SOFTBUS_STATE_OPENED;
    int32_t eventType = INT_MAX;
    int32_t eventReason = INT_MAX;
    std::string detail = "Test error with maximum integer values";
    int32_t ret = softbusSession_->NotifyError(eventType, eventReason, detail);
    EXPECT_EQ(DCAMERA_OK, ret);
}

}
}