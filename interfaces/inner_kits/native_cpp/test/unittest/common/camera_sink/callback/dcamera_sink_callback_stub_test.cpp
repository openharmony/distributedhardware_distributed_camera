/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "dcamera_sink_callback_stub.h"
#include "dcamera_sink_callback.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkCallbackStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkCallbackStub> sinkCallbackStub_;
};

void DCameraSinkCallbackStubTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkCallbackStubTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkCallbackStubTest::SetUp(void)
{
    DHLOGI("enter");
    sinkCallbackStub_ = std::make_shared<DCameraSinkCallback>();
}

void DCameraSinkCallbackStubTest::TearDown(void)
{
    DHLOGI("enter");
    sinkCallbackStub_ = nullptr;
}

/**
 * @tc.name: OnRemoteRequest_001
 * @tc.desc: Verify the OnNotifyRegResult function.
 * @tc.type: FUNC
 * @tc.require: issue

 int32_t DCameraSinkCallbackStub::OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply,
    MessageOption &option)
 */
HWTEST_F(DCameraSinkCallbackStubTest, OnRemoteRequest_001, TestSize.Level1)
{
    DHLOGI("DCameraSinkCallbackStubTest OnRemoteRequest_001.");
    EXPECT_EQ(false, sinkCallbackStub_ == nullptr);

    int32_t ret = DCAMERA_OK;
    uint32_t code = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(DCameraSinkCallbackStub::GetDescriptor());
    ret = sinkCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DCAMERA_OK, ret);

    data.WriteInterfaceToken(u"");
    ret = sinkCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(ERR_INVALID_DATA, ret);
}

/**
 * @tc.name: OnRemoteRequest_002
 * @tc.desc: Verify the OnHardwareStateChanged function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkCallbackStubTest, OnRemoteRequest_002, TestSize.Level1)
{
    DHLOGI("DCameraSinkCallbackStubTest OnRemoteRequest_002.");
    EXPECT_EQ(false, sinkCallbackStub_ == nullptr);

    int32_t ret = DCAMERA_OK;
    uint32_t code = 1;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(DCameraSinkCallbackStub::GetDescriptor());
    ret = sinkCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: OnRemoteRequest_003
 * @tc.desc: Verify the OnRemoteRequest function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkCallbackStubTest, OnRemoteRequest_003, TestSize.Level1)
{
    DHLOGI("DCameraSinkCallbackStubTest OnRemoteRequest_003.");
    EXPECT_EQ(false, sinkCallbackStub_ == nullptr);

    int32_t ret = DCAMERA_OK;
    int32_t UNKNOW_TRANS_ERR = 305;
    uint32_t code = 2;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    data.WriteInterfaceToken(DCameraSinkCallbackStub::GetDescriptor());
    ret = sinkCallbackStub_->OnRemoteRequest(code, data, reply, option);
    EXPECT_EQ(UNKNOW_TRANS_ERR, ret);
}
}
}