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

#define private public
#include "dcamera_source_callback.h"
#undef private

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
namespace {
constexpr uint32_t DID_MAX_SIZE = 256;
constexpr uint32_t ERR_IPC_CODE = 99999;
constexpr uint32_t PARAM_MAX_SIZE = 50 * 1024 * 1024;
}
class DCameraSourceCallbackTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSourceCallback> sourceCallback_;
};

class RegisterCallbackTest : public RegisterCallback {
public:
    RegisterCallbackTest() = default;
    virtual ~RegisterCallbackTest() = default;

    int32_t OnRegisterResult(const std::string &uuid, const std::string &dhId, int32_t status,
        const std::string &data)
    {
        return 0;
    }
};

class MockDistributedHardwareStateListener : public DistributedHardwareStateListener {
public:
    MockDistributedHardwareStateListener() = default;
    virtual ~MockDistributedHardwareStateListener() = default;

    void OnStateChanged(const std::string &uuid, const std::string &dhId, const BusinessState state) override
    {
        DHLOGI("Handle state changed event");
    }
};

class MockDataSyncTriggerListener : public DataSyncTriggerListener {
public:
    MockDataSyncTriggerListener() = default;
    virtual ~MockDataSyncTriggerListener() = default;

    void OnDataSyncTrigger(const std::string &uuid) override
    {
        DHLOGI("Handle data sync event");
    }
};

class UnregisterCallbackTest : public UnregisterCallback {
public:
    UnregisterCallbackTest() = default;
    virtual ~UnregisterCallbackTest() = default;

    int32_t OnUnregisterResult(const std::string &uuid, const std::string &dhId, int32_t status,
        const std::string &data)
    {
        return 0;
    }
};

void DCameraSourceCallbackTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceCallbackTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceCallbackTest::SetUp(void)
{
    DHLOGI("enter");
    sourceCallback_ = std::make_shared<DCameraSourceCallback>();
}

void DCameraSourceCallbackTest::TearDown(void)
{
    DHLOGI("enter");
    sourceCallback_ = nullptr;
}

/**
 * @tc.name: dcamera_source_callback_test_001
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_001, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    std::string reqId = "reqIdReg";
    int32_t status = 0;
    std::string data = "data";
    std::shared_ptr<RegisterCallback> callback = std::make_shared<RegisterCallbackTest>();
    sourceCallback_->PushRegCallback(reqId, callback);
    int32_t ret = sourceCallback_->OnNotifyRegResult(devId, dhId, reqId, status, data);
    sourceCallback_->PopRegCallback(reqId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_002
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_002, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    std::string reqId = "reqId";
    int32_t status = 0;
    std::string data = "data";
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->OnNotifyRegResult(devId, dhId, reqId, status, data);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_003
 * @tc.desc: Verify the OnNotifyUnregResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_003, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    std::string reqId = "reqId";
    int32_t status = 0;
    std::string data = "data";
    std::shared_ptr<UnregisterCallback> callback = std::make_shared<UnregisterCallbackTest>();
    sourceCallback_->PushUnregCallback(reqId, callback);
    int32_t ret = sourceCallback_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
    sourceCallback_->PopUnregCallback(reqId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_004
 * @tc.desc: Verify the OnNotifyUnregResult function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_004, TestSize.Level1)
{
    const std::string devId = "devId";
    const std::string dhId = "dhId";
    std::string reqId = "reqId";
    int32_t status = 0;
    std::string data = "data";
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_005
 * @tc.desc: Verify the CheckParams function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_005, TestSize.Level1)
{
    std::string devId = "";
    std::string dhId = "dhId";
    std::string reqId = "reqId";
    std::string result = "result";
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    devId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    devId = "devId";
    dhId = "";
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    dhId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    dhId = "dhId";
    reqId = "";
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    reqId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
    reqId ="reqId";
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(true, ret);
    result = std::string(PARAM_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->CheckParams(devId, dhId, reqId, result);
    EXPECT_EQ(false, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_006
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_006, TestSize.Level1)
{
    uint32_t code = ERR_IPC_CODE;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;
    std::string descriptor = "desc";
    EXPECT_EQ(true, data.WriteInterfaceToken(to_utf16(descriptor)));
    std::string devId = "devId";
    std::string dhId = "dhId";
    std::string reqId = "reqId";
    data.WriteString(devId);
    data.WriteString(dhId);
    data.WriteString(reqId);
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(DCAMERA_OK, ret);
    code = DCameraSourceCallback::NOTIFY_REG_RESULT;
    ret = sourceCallback_->OnRemoteRequest(code, data, reply, option);
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_007
 * @tc.desc: Verify the NotifyRegResultInner function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_007, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string descriptor = "desc";
    EXPECT_EQ(true, data.WriteInterfaceToken(to_utf16(descriptor)));
    std::string devId = "devId";
    std::string dhId = "dhId";
    std::string reqId = "reqId";
    int32_t status = 1;
    std::string result = "success";
    data.WriteString(devId);
    data.WriteString(dhId);
    data.WriteString(reqId);
    data.WriteInt32(status);
    data.WriteString(result);
    int32_t ret = sourceCallback_->NotifyRegResultInner(data, reply);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_008
 * @tc.desc: Verify the NotifyUnregResultInner function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_008, TestSize.Level1)
{
    MessageParcel data;
    MessageParcel reply;
    std::string descriptor = "desc";
    EXPECT_EQ(true, data.WriteInterfaceToken(to_utf16(descriptor)));
    std::string devId = "devId";
    std::string dhId = "dhId";
    std::string reqId = "reqId";
    int32_t status = 1;
    std::string result = "success";
    data.WriteString(devId);
    data.WriteString(dhId);
    data.WriteString(reqId);
    data.WriteInt32(status);
    data.WriteString(result);
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->NotifyUnregResultInner(data, reply);
    sourceCallback_->RegisterStateListener(nullptr);
    sourceCallback_->UnRegisterStateListener();
    sourceCallback_->RegisterTriggerListener(nullptr);
    sourceCallback_->UnRegisterTriggerListener();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_009
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_009, TestSize.Level1)
{
    std::string devId = "devId";
    std::string dhId = "dhId";
    int32_t status = 1;
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    std::shared_ptr<DistributedHardwareStateListener> listener =
        std::make_shared<MockDistributedHardwareStateListener>();
    sourceCallback_->RegisterStateListener(listener);
    devId = "";
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    devId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    devId = "devId";
    dhId = "";
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    dhId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    dhId = "dhId";
    status = -1;
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    status = 1;
    ret = sourceCallback_->OnHardwareStateChanged(devId, dhId, status);
    sourceCallback_->UnRegisterStateListener();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_callback_test_010
 * @tc.desc: Verify the OnDataSyncTrigger function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceCallbackTest, dcamera_source_callback_test_010, TestSize.Level1)
{
    std::string devId = "";
    ASSERT_NE(sourceCallback_, nullptr);
    int32_t ret = sourceCallback_->OnDataSyncTrigger(devId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    devId += std::string(DID_MAX_SIZE + 1, 'a');
    ret = sourceCallback_->OnDataSyncTrigger(devId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    devId = "devId";
    ret = sourceCallback_->OnDataSyncTrigger(devId);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    std::shared_ptr<DataSyncTriggerListener> listener = std::make_shared<MockDataSyncTriggerListener>();
    sourceCallback_->RegisterTriggerListener(listener);
    ret = sourceCallback_->OnDataSyncTrigger(devId);
    sourceCallback_->UnRegisterTriggerListener();
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}