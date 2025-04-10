/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "dcamera_feeding_smoother.h"
#undef private
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_utils_tools.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

class FeedableDataImpl : public IFeedableData {
public:
    int64_t GetTimeStamp() override
    {
        return 1;
    }
};

class FeedableDataProducerImpl : public IFeedableDataProducer {
public:
    void OnSmoothFinished(const std::shared_ptr<IFeedableData> &data) override
    {
        if (data) {
            data->GetTimeStamp();
        }
        return;
    }
};

class DCameraFeedingSmootherImpl : public IFeedingSmoother {
public:
    ~DCameraFeedingSmootherImpl() override = default;

    void PrepareSmooth() override
    {
    }

    void InitBaseline(const int64_t timeStampBaseline, const int64_t clockBaseline) override
    {
    }

    void InitTimeStatistician() override
    {
        dCameraStatistician_ = std::make_shared<DCameraTimeStatistician>();
    }

    int32_t NotifySmoothFinished(const std::shared_ptr<IFeedableData> &data) override
    {
        return listener_->OnSmoothFinished(data);
    }

private:
    constexpr static uint8_t DYNAMIC_BALANCE_THRE = 3;
    constexpr static int32_t SMOOTH_BUFFER_TIME_US = 20000;
    constexpr static uint32_t AVER_INTERVAL_DIFF_THRE_US = 2000;
    constexpr static uint32_t FEED_ONCE_DIFF_THRE_US = 10000;

    std::shared_ptr<DCameraTimeStatistician> dCameraStatistician_ = nullptr;
};

const int32_t SLEEP_TIME = 1000;
const int64_t FRAME_INTERVAL = 40000;
const uint8_t TWOFOLD = 2;
class DCameraFeedingSmotherTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraFeedingSmotherTest::SetUpTestCase(void)
{
}

void DCameraFeedingSmotherTest::TearDownTestCase(void)
{
}

void DCameraFeedingSmotherTest::SetUp(void)
{
}

void DCameraFeedingSmotherTest::TearDown(void)
{
}

/**
 * @tc.name: dcamera_feeding_smoother_test_001
 * @tc.desc: Verify StartSmooth func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_001");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    smoother->state_ = SMOOTH_STOP;
    int32_t ret = smoother->StartSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
    ret = smoother->StartSmooth();
    EXPECT_EQ(SMOOTH_IS_STARTED, ret);
    ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_002
 * @tc.desc: Verify StopSmooth func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_002");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    smoother->state_ = SMOOTH_STOP;
    int32_t ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_IS_STOPED, ret);
    smoother->StartSmooth();
    std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME));
    ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_003
 * @tc.desc: Verify LooperSmooth func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_003");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    int32_t ret = smoother->StartSmooth();
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> data = std::make_shared<DataBuffer>(capacity);
    smoother->PushData(data);
    std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME));
    ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_004
 * @tc.desc: Verify SmoothFeeding func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_004");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    int32_t ret = smoother->StartSmooth();
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> data = std::make_shared<DataBuffer>(capacity);
    data->frameInfo_.pts = FRAME_INTERVAL;
    smoother->PushData(data);
    std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME));
    smoother->PushData(data);
    smoother->SetProcessDynamicBalanceState(true);
    smoother->smoothCon_.notify_one();
    std::this_thread::sleep_for(std::chrono::microseconds(SLEEP_TIME));
    smoother->InitTimeStatistician();
    smoother->PushData(data);
    smoother->SetProcessDynamicBalanceState(false);
    smoother->smoothCon_.notify_one();
    ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_005
 * @tc.desc: Verify CheckIsProcessInDynamicBalance func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_005");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    smoother->CheckIsProcessInDynamicBalance();
    smoother->SetProcessDynamicBalanceState(false);
    smoother->InitTimeStatistician();
    smoother->state_ = SMOOTH_START;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> data = std::make_shared<DataBuffer>(capacity);
    for (uint8_t i = 1; i <= DCameraFeedingSmoother::DYNAMIC_BALANCE_THRE; i++) {
        data->frameInfo_.pts = i * FRAME_INTERVAL;
        data->frameInfo_.index = i;
        smoother->PushData(data);
        smoother->CheckIsProcessInDynamicBalanceOnce();
        std::this_thread::sleep_for(std::chrono::microseconds(FRAME_INTERVAL));
    }
    smoother->CheckIsProcessInDynamicBalance();
    smoother->SetDynamicBalanceThre(1);
    smoother->CheckIsProcessInDynamicBalance();
    smoother->state_ = SMOOTH_STOP;
    int32_t ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_IS_STOPED, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_006
 * @tc.desc: Verify AdjustSleepTime func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_006");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    smoother->delta_ = FRAME_INTERVAL;
    smoother->sleep_ = FRAME_INTERVAL;
    smoother->AdjustSleepTime(FRAME_INTERVAL);
    smoother->delta_ = -FRAME_INTERVAL;
    smoother->AdjustSleepTime(FRAME_INTERVAL);
    int32_t ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_IS_STOPED, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_007
 * @tc.desc: Verify SyncClock func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_007");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    int64_t timeStamp = TWOFOLD * FRAME_INTERVAL;
    int64_t clock = TWOFOLD * FRAME_INTERVAL;
    smoother->SyncClock(timeStamp, FRAME_INTERVAL, FRAME_INTERVAL);
    smoother->SyncClock(FRAME_INTERVAL, FRAME_INTERVAL, clock);
    int32_t ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_IS_STOPED, ret);
}

/**
 * @tc.name: dcamera_feeding_smoother_test_008
 * @tc.desc: Verify SyncClock func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraFeedingSmotherTest, dcamera_feeding_smoother_test_008, TestSize.Level1)
{
    DHLOGI("dcamera_feeding_smoother_test_008");
    std::unique_ptr<IFeedingSmoother> smoother = std::make_unique<DCameraFeedingSmoother>();
    int32_t ret = smoother->StartSmooth();
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> data = std::make_shared<DataBuffer>(capacity);
    smoother->state_ = SmoothState::SMOOTH_STOP;
    smoother->PushData(data);
    smoother->state_ = SmoothState::SMOOTH_START;
    smoother->PushData(data);
    smoother->InitTimeStatistician();
    smoother->CheckIsProcessInDynamicBalanceOnce();
    std::shared_ptr<IFeedableDataProducer> producer = std::make_shared<FeedableDataProducerImpl>();
    std::shared_ptr<FeedingSmootherListener> listener = std::make_shared<FeedingSmootherListener>(producer);
    smoother->RegisterListener(listener);
    std::shared_ptr<IFeedableData> iFeedableData = std::make_shared<FeedableDataImpl>();
    std::unique_ptr<IFeedingSmoother> smootherImpl = std::make_unique<DCameraFeedingSmootherImpl>();
    smootherImpl->RegisterListener(listener);
    smootherImpl->NotifySmoothFinished(iFeedableData);
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    ret = smootherImpl->StopSmooth();
    ret = smoother->StopSmooth();
    EXPECT_EQ(SMOOTH_SUCCESS, ret);
}
} // namespace DistributedHardware
} // namespace OHOS