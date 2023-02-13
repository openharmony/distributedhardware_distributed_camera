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

#define private public
#include "dcamera_source_callback.h"
#undef private

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
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
    std::shared_ptr<RegisterCallback> callback = nullptr;
    sourceCallback_->PushRegCallback(reqId, callback);
    int32_t ret = sourceCallback_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
    sourceCallback_->PopRegCallback(reqId);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
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
    int32_t ret = sourceCallback_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
    EXPECT_EQ(DCAMERA_NOT_FOUND, ret);
}

}
}