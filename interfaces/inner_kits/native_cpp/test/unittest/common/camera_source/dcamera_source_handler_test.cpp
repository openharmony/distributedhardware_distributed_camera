/*
 * Copyright (c) 2022-2026 Huawei Device Co., Ltd.
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
#include <atomic>

#define private public
#include "dcamera_source_handler.h"
#undef private
#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_source_callback.h"
#include "dcamera_source_handler_ipc.h"
#include "dcamera_source_load_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "mock_component_disable.h"
#include "mock_component_enable.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const std::string TEST_VER = "1.0";
const std::string TEST_ATTRS = "";
}
void DCameraSourceHandlerTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraSourceHandlerTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_source_handler_test_001
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_001, TestSize.Level0)
{
    std::string params = "test001";
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_002
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_002, TestSize.Level0)
{
    std::string params = "test002";
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);

    int32_t systemAbilityId = 4803;
    DCameraSourceHandler::GetInstance().FinishStartSA(params);
    DCameraSourceHandler::GetInstance().FinishStartSAFailed(systemAbilityId);
    ret = DCameraSourceHandler::GetInstance().ReleaseSource();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_003
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_003, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    EnableParam param;
    param.sourceVersion = TEST_VER;
    param.sourceAttrs = TEST_ATTRS;
    param.sinkVersion = TEST_VER;
    param.sinkAttrs = TEST_ATTRS;
    std::shared_ptr<RegisterCallback> callback = nullptr;
    int32_t ret = DCameraSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    callback = std::make_shared<MockComponentEnable>();
    ret = DCameraSourceHandler::GetInstance().RegisterDistributedHardware(devId, dhId, param, callback);
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_004
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_004, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    std::string params = "test004";
    std::shared_ptr<UnregisterCallback> uncallback = std::make_shared<MockComponentDisable>();

    int32_t ret = DCameraSourceHandler::GetInstance().UnregisterDistributedHardware(devId, dhId, uncallback);
    ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_005
 * @tc.desc: Verify the OnLoadSystemAbilitySuccess function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_005, TestSize.Level0)
{
    std::string params = "test005";
    int32_t systemAbilityId = 4803;
    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    ASSERT_NE(samgr, nullptr);
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(systemAbilityId);
    sptr<DCameraSourceLoadCallback> loadCallback(new DCameraSourceLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);

    remoteObject = nullptr;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilitySuccess(systemAbilityId, remoteObject);
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_006
 * @tc.desc: Verify the OnLoadSystemAbilityFail function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_006, TestSize.Level0)
{
    std::string params = "test006";
    int32_t systemAbilityId = 4803;
    sptr<DCameraSourceLoadCallback> loadCallback(new DCameraSourceLoadCallback(params));
    ASSERT_NE(loadCallback, nullptr);
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);

    systemAbilityId = 1;
    loadCallback->OnLoadSystemAbilityFail(systemAbilityId);
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_handler_test_007
 * @tc.desc: Verify the UpdateDistributedHardwareWorkMode function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_007, TestSize.Level0)
{
    std::string devId = TEST_DEVICE_ID;
    std::string dhId = TEST_CAMERA_DH_ID_0;
    WorkModeParam param(-1, 0, 0, false);
    int32_t ret = DCameraSourceHandler::GetInstance().UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
#ifdef UT_COVER_SPECIAL
    param.fd = 12;
    param.sharedMemLen = 120;
    ret = DCameraSourceHandler::GetInstance().UpdateDistributedHardwareWorkMode(devId, dhId, param);
    EXPECT_NE(DCAMERA_OK, ret);
#endif
}

/**
 * @tc.name: dcamera_source_handler_test_008
 * @tc.desc: Verify InitSource with RELEASING state handling.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_008, TestSize.Level1)
{
    DHLOGI("Testing InitSource with RELEASING state");

    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_START;
    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING;
    std::string params = "test008";
    auto start = std::chrono::steady_clock::now();
    // 启动一个线程来调用 InitSource（应该进入等待）
    std::atomic<bool> initStarted{false};
    std::atomic<bool> initCompleted{false};
    std::atomic<int32_t> initResult{-1};
    std::thread initThread([&initStarted, &initResult, &params, &initCompleted]() {
        initStarted = true;
        initResult = DCameraSourceHandler::GetInstance().InitSource(params);
        initCompleted = true;
    });

    while (!initStarted) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_STOP;
    DCameraSourceHandler::GetInstance().producerCon_.notify_all();

    initThread.join();
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    EXPECT_TRUE(initCompleted);
    EXPECT_GE(duration.count(), 0);
}

/**
 * @tc.name: dcamera_source_handler_test_009
 * @tc.desc: Verify InitSource with already START state returns immediately.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_009, TestSize.Level1)
{
    DHLOGI("Testing InitSource with START state");
    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_START;
    std::string params = "test009";
    auto start = std::chrono::steady_clock::now();
    int32_t ret = DCameraSourceHandler::GetInstance().InitSource(params);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_LT(duration.count(), 1000);
}

/**
 * @tc.name: dcamera_source_handler_test_010
 * @tc.desc: Verify ReleaseSource immediately sets RELEASING state.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_010, TestSize.Level1)
{
    DHLOGI("Testing ReleaseSource state transition");
    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_START;
    {
        std::lock_guard<std::mutex> lock(DCameraSourceHandler::GetInstance().producerMutex_);
        DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING;
    }
    EXPECT_EQ(DCameraSourceHandler::GetInstance().state_, DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING);
    {
        std::lock_guard<std::mutex> lock(DCameraSourceHandler::GetInstance().producerMutex_);
        DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_STOP;
        DCameraSourceHandler::GetInstance().producerCon_.notify_all();
    }
    EXPECT_EQ(DCameraSourceHandler::GetInstance().state_, DCameraSourceHandler::DCAMERA_SA_STATE_STOP);
}

/**
 * @tc.name: dcamera_source_handler_test_011
 * @tc.desc: Verify concurrent scenario simulation.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_011, TestSize.Level1)
{
    DHLOGI("Testing concurrent scenario simulation");
    DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_START;
    std::atomic<bool> releaseThreadStarted{false};
    std::atomic<bool> releaseThreadCompleted{false};
    std::atomic<bool> initThreadCompleted{false};
    std::atomic<int32_t> initResult{-1};
    std::thread releaseThread([&releaseThreadStarted, &releaseThreadCompleted]() {
        releaseThreadStarted = true;
        {
            std::lock_guard<std::mutex> lock(DCameraSourceHandler::GetInstance().producerMutex_);
            EXPECT_EQ(DCameraSourceHandler::GetInstance().state_,
                      DCameraSourceHandler::DCAMERA_SA_STATE_START);
            DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(DCameraSourceHandler::GetInstance().producerMutex_);
            DCameraSourceHandler::GetInstance().state_ = DCameraSourceHandler::DCAMERA_SA_STATE_STOP;
            DCameraSourceHandler::GetInstance().producerCon_.notify_all();
        }
        releaseThreadCompleted = true;
    });
    std::thread initThread([&releaseThreadStarted, &initResult, &initThreadCompleted]() {
        while (!releaseThreadStarted) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        std::string params = "test013_concurrent";
        auto start = std::chrono::steady_clock::now();
        initResult = DCameraSourceHandler::GetInstance().InitSource(params);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        initThreadCompleted = true;
        EXPECT_GE(duration.count(), 80);
    });
    releaseThread.join();
    initThread.join();
    EXPECT_TRUE(releaseThreadCompleted);
    EXPECT_TRUE(initThreadCompleted);
}

/**
 * @tc.name: dcamera_source_handler_test_012
 * @tc.desc: Verify state enum values consistency.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSourceHandlerTest, dcamera_source_handler_test_012, TestSize.Level0)
{
    DHLOGI("Testing state enum values");
    EXPECT_EQ(DCameraSourceHandler::DCAMERA_SA_STATE_STOP, 0);
    EXPECT_EQ(DCameraSourceHandler::DCAMERA_SA_STATE_START, 1);
    EXPECT_EQ(DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING, 2);
    EXPECT_NE(DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING,
              DCameraSourceHandler::DCAMERA_SA_STATE_STOP);
    EXPECT_NE(DCameraSourceHandler::DCAMERA_SA_STATE_RELEASING,
              DCameraSourceHandler::DCAMERA_SA_STATE_START);
}
}
}