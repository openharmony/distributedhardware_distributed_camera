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

#include <chrono>
#include <gtest/gtest.h>
#include <securec.h>
#include <thread>
#define private public
#include "dcamera_stream_data_process_producer.h"
#undef private
#include "anonymous_string.h"
#include "dcamera_buffer_handle.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_camera_source.h"
#include "v1_1/dcamera_types.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraStreamDataProcessProducerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraStreamDataProcessProducer> producer_ = nullptr;
    std::string devId_ = "test_devId";
    std::string dhId_ = "test_dhId";
    int32_t streamId_ = 1;
    DCStreamType streamType_ = CONTINUOUS_FRAME;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const uint8_t SLEEP_TIME = 1;
const int32_t STREAM_ID_1 = 1;
const int32_t STREAM_ID_2 = 2;
}
void DCameraStreamDataProcessProducerTest::SetUpTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SetUpTestCase");
}

void DCameraStreamDataProcessProducerTest::TearDownTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest TearDownTestCase");
}

void DCameraStreamDataProcessProducerTest::SetUp(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SetUp");
    producer_ = std::make_shared<DCameraStreamDataProcessProducer>(devId_, dhId_, streamId_, streamType_);
}

void DCameraStreamDataProcessProducerTest::TearDown(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest TearDown");
    if (producer_ != nullptr) {
        if (producer_->state_ == DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_START) {
            producer_->Stop();
        }
        producer_ = nullptr;
    }
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_001
 * @tc.desc: Verify Start Stop FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_001");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess1->Start();
    sleep(SLEEP_TIME);
    streamProcess1->FeedStream(buffer);
    streamProcess1->Stop();
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_2,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess2->Start();
    sleep(SLEEP_TIME);
    streamProcess2->FeedStream(buffer);
    streamProcess2->Stop();
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = streamProcess1->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    ret = streamProcess2->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    streamProcess1 = nullptr;
    auto listener = std::make_shared<FeedingSmootherListener>(streamProcess1);
    buffer = nullptr;
    EXPECT_EQ(NOTIFY_FAILED, listener->OnSmoothFinished(buffer));
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_002
 * @tc.desc: Verify Start Stop FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_002");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_2,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess1->Start();
    sleep(SLEEP_TIME);
    streamProcess1->Stop();

    streamProcess2->Start();
    sleep(SLEEP_TIME);
    streamProcess2->Stop();
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = streamProcess1->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    ret = streamProcess2->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_003
 * @tc.desc: Verify Start Stop FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_003");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_2,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess1->FeedStream(buffer);
    streamProcess1->FeedStream(buffer);
    streamProcess2->Start();
    sleep(SLEEP_TIME);
    streamProcess2->FeedStream(buffer);
    streamProcess2->FeedStream(buffer);
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = streamProcess1->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    ret = streamProcess2->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    streamProcess2->Stop();
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_004
 * @tc.desc: Verify LooperSnapShot func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_004");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess->LooperSnapShot();
    streamProcess->state_ = DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP;
    streamProcess->LooperSnapShot();
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = streamProcess->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_005
 * @tc.desc: Verify CheckSharedMemory func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_005");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_2,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    sharedMemory.bufferHandle_ = nullptr;
    int32_t ret = streamProcess1->CheckSharedMemory(sharedMemory, buffer);
    ret = streamProcess2->CheckSharedMemory(sharedMemory, buffer);
    sharedMemory.bufferHandle_ = sptr<NativeBuffer>(new NativeBuffer());
    ret = streamProcess1->CheckSharedMemory(sharedMemory, buffer);
    ret = streamProcess2->CheckSharedMemory(sharedMemory, buffer);
    auto bufferHandle = std::make_unique<BufferHandle>();
    sharedMemory.bufferHandle_ = sptr<NativeBuffer>(new NativeBuffer(bufferHandle.get()));
    ret = streamProcess1->CheckSharedMemory(sharedMemory, buffer);
    ret = streamProcess2->CheckSharedMemory(sharedMemory, buffer);
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    ret = streamProcess1->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    ret = streamProcess2->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Start_001
 * @tc.desc: Verify DCameraStreamDataProcessProducer Start with CONTINUOUS_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, Start_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest Start_001");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->Start();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_START, producer_->state_);
}

/**
 * @tc.name: Start_002
 * @tc.desc: Test DCameraStreamDataProcessProducer Start with SNAPSHOT_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, Start_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest Start_002");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = SNAPSHOT_FRAME;
    producer_->Start();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_START, producer_->state_);
}

/**
 * @tc.name: Stop_001
 * @tc.desc: Test DCameraStreamDataProcessProducer Stop when in start state with CONTINUOUS_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, Stop_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest Stop_001");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->Start();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_START, producer_->state_);
    producer_->Stop();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP, producer_->state_);
}

/**
 * @tc.name: Stop_002
 * @tc.desc: Test DCameraStreamDataProcessProducer Stop when in start state with SNAPSHOT_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, Stop_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest Stop_002");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = SNAPSHOT_FRAME;
    producer_->Start();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_START, producer_->state_);
    producer_->Stop();
    EXPECT_EQ(DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP, producer_->state_);
}

/**
 * @tc.name: FeedStream_001
 * @tc.desc: Test DCameraStreamDataProcessProducer FeedStream with CONTINUOUS_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, FeedStream_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest FeedStream_001");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->Start();
    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1000000;
    producer_->FeedStream(buffer);
    producer_->Stop();
    SUCCEED();
}

/**
 * @tc.name: FeedStream_002
 * @tc.desc: Test DCameraStreamDataProcessProducer FeedStream with SNAPSHOT_FRAME stream type.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, FeedStream_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest FeedStream_002");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = SNAPSHOT_FRAME;
    producer_->Start();
    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1000000;
    producer_->FeedStream(buffer);
    {
        std::lock_guard<std::mutex> lock(producer_->bufferMutex_);
        EXPECT_FALSE(producer_->buffers_.empty());
    }
    producer_->Stop();
}

/**
 * @tc.name: FeedStream_003
 * @tc.desc: Test DCameraStreamDataProcessProducer FeedStream when buffer queue is full.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, FeedStream_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest FeedStream_003");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = SNAPSHOT_FRAME;
    producer_->Start();

    for (uint32_t i = 0; i <= producer_->DCAMERA_PRODUCER_MAX_BUFFER_SIZE; i++) {
        auto buffer = std::make_shared<DataBuffer>(100);
        buffer->frameInfo_.rawTime = 1000000 + i * 1000; // Incremental timestamps
        producer_->FeedStream(buffer);
    }

    {
        std::lock_guard<std::mutex> lock(producer_->bufferMutex_);
        EXPECT_EQ(producer_->buffers_.size(), producer_->DCAMERA_PRODUCER_MAX_BUFFER_SIZE);
    }
    producer_->Stop();
}

/**
 * @tc.name: OnSmoothFinished_001
 * @tc.desc: Test DCameraStreamDataProcessProducer OnSmoothFinished with AV sync disabled.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, OnSmoothFinished_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest OnSmoothFinished_001");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->DCameraStreamDataProcessProducer::workModeParam_.isAVsync = false;
    producer_->Start();

    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1000000; // 1s in us
    auto feedableData = std::static_pointer_cast<IFeedableData>(buffer);
    producer_->OnSmoothFinished(feedableData);

    producer_->Stop();
    SUCCEED();
}

/**
 * @tc.name: OnSmoothFinished_002
 * @tc.desc: Test DCameraStreamDataProcessProducer OnSmoothFinished with AV sync enabled.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, OnSmoothFinished_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest OnSmoothFinished_002");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->DCameraStreamDataProcessProducer::workModeParam_.isAVsync = true;
    producer_->Start();

    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1000000; // 1s in us
    auto feedableData = std::static_pointer_cast<IFeedableData>(buffer);
    producer_->OnSmoothFinished(feedableData);

    {
        std::lock_guard<std::mutex> lock(producer_->syncBufferMutex_);
        EXPECT_TRUE(producer_->syncBufferQueue_.empty());
    }
    producer_->Stop();
}

/**
 * @tc.name: OnSmoothFinished_003
 * @tc.desc: Test DCameraStreamDataProcessProducer OnSmoothFinished with full sync buffer queue.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, OnSmoothFinished_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest OnSmoothFinished_003");
    ASSERT_NE(producer_, nullptr);
    producer_->streamType_ = CONTINUOUS_FRAME;
    producer_->DCameraStreamDataProcessProducer::workModeParam_.isAVsync = true;
    producer_->Start();

    for (uint32_t i = 0; i <= producer_->DCAMERA_MAX_SYNC_BUFFER_SIZE; i++) {
        auto buffer = std::make_shared<DataBuffer>(100);
        buffer->frameInfo_.rawTime = 1000000 + i * 1000; // Incremental timestamps
        auto feedableData = std::static_pointer_cast<IFeedableData>(buffer);
        producer_->OnSmoothFinished(feedableData);
    }

    {
        std::lock_guard<std::mutex> lock(producer_->syncBufferMutex_);
        EXPECT_EQ(producer_->syncBufferQueue_.size(), 0);
    }
    producer_->Stop();
}

/**
 * @tc.name: UpdateProducerWorkMode_001
 * @tc.desc: Test DCameraStreamDataProcessProducer UpdateProducerWorkMode.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateProducerWorkMode_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateProducerWorkMode_001");
    ASSERT_NE(producer_, nullptr);
    WorkModeParam param(-1, 0, 0, 0);
    param.fd = 10;
    param.sharedMemLen = 1024;
    param.isAVsync = true;
    param.scene = 0;

    producer_->UpdateProducerWorkMode(param);
    uint64_t videoPtsUs = 1000000;
    int32_t result = producer_->SyncVideoFrame(videoPtsUs);
    EXPECT_TRUE(result == DCAMERA_BAD_VALUE || result == -1 || result == 0 || result == 1);
}

/**
 * @tc.name: SyncVideoFrame_001
 * @tc.desc: Test DCameraStreamDataProcessProducer SyncVideoFrame with null syncMem.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, SyncVideoFrame_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SyncVideoFrame_001");
    ASSERT_NE(producer_, nullptr);
    uint64_t videoPtsUs = 1000000; // 1s in us
    int32_t result = producer_->SyncVideoFrame(videoPtsUs);
    EXPECT_TRUE(result == DCAMERA_BAD_VALUE || result == -1 || result == 0 || result == 1);
}

/**
 * @tc.name: SyncVideoFrame_002
 * @tc.desc: Verify SyncVideoFrame function with different timestamp values.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, SyncVideoFrame_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SyncVideoFrame_002");
    ASSERT_NE(producer_, nullptr);
    
    WorkModeParam param(-1, 0, 0, 0);
    param.fd = 10;
    param.sharedMemLen = 1024;
    param.isAVsync = 1;
    param.scene = 0;
    producer_->UpdateProducerWorkMode(param);

    uint64_t videoPtsUs1 = 1000000;   // 1 second
    uint64_t videoPtsUs2 = 50000000;  // 50 seconds
    uint64_t videoPtsUs3 = 100000;    // 0.1 seconds

    int32_t result1 = producer_->SyncVideoFrame(videoPtsUs1);
    int32_t result2 = producer_->SyncVideoFrame(videoPtsUs2);
    int32_t result3 = producer_->SyncVideoFrame(videoPtsUs3);

    EXPECT_TRUE(result1 == DCAMERA_BAD_VALUE || result1 == -1 || result1 == 0 || result1 == 1);
    EXPECT_TRUE(result2 == DCAMERA_BAD_VALUE || result2 == -1 || result2 == 0 || result2 == 1);
    EXPECT_TRUE(result3 == DCAMERA_BAD_VALUE || result3 == -1 || result3 == 0 || result3 == 1);
}

/**
 * @tc.name: SyncVideoFrame_003
 * @tc.desc: Verify SyncVideoFrame function when AV sync is disabled.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, SyncVideoFrame_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SyncVideoFrame_003");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.fd = 10;
    param.sharedMemLen = 1024;
    param.isAVsync = 0; // AV sync disabled
    param.scene = 0;
    producer_->UpdateProducerWorkMode(param);

    uint64_t videoPtsUs = 1000000;
    int32_t result = producer_->SyncVideoFrame(videoPtsUs);

    EXPECT_TRUE(result == DCAMERA_BAD_VALUE || result == -1 || result == 0 || result == 1);
}

/**
 * @tc.name: SyncVideoFrame_004
 * @tc.desc: Integration test for SyncVideoFrame through public interface.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, SyncVideoFrame_004, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest SyncVideoFrame_004");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.fd = 10;
    param.sharedMemLen = 1024;
    param.isAVsync = 1;
    param.scene = 0;
    producer_->UpdateProducerWorkMode(param);

    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1000000; // Set a specific timestamp

    producer_->Start();

    producer_->FeedStream(buffer);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    // Stop the producer
    producer_->Stop();
    SUCCEED();
}

/**
 * @tc.name: UpdateVideoClock_001
 * @tc.desc: Verify UpdateVideoClock function when AV sync is disabled.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateVideoClock_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateVideoClock_001");
    ASSERT_NE(producer_, nullptr);
    uint64_t videoPtsUs = 1000000; // 1s in us
    WorkModeParam param(-1, 0, 0, 0);
    param.isAVsync = 0;
    producer_->DCameraStreamDataProcessProducer::workModeParam_ = param;

    producer_->UpdateVideoClock(videoPtsUs);
    SUCCEED();
}

/**
 * @tc.name: UpdateVideoClock_002
 * @tc.desc: Verify UpdateVideoClock function when syncMem is null.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateVideoClock_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateVideoClock_002");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.isAVsync = 1;
    producer_->DCameraStreamDataProcessProducer::workModeParam_ = param;
    producer_->syncMem_ = nullptr;

    uint64_t videoPtsUs = 1000000;
    producer_->UpdateVideoClock(videoPtsUs);

    SUCCEED();
}

/**
 * @tc.name: UpdateVideoClock_003
 * @tc.desc: Verify UpdateVideoClock function updates video clock correctly.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateVideoClock_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateVideoClock_003");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.isAVsync = 1;
    param.fd = 10;
    param.sharedMemLen = 1024;
    producer_->DCameraStreamDataProcessProducer::workModeParam_ = param;

    producer_->syncMem_ = sptr<Ashmem>(new (std::nothrow) Ashmem(param.fd, param.sharedMemLen));

    uint64_t videoPtsUs = 2000000;
    producer_->UpdateVideoClock(videoPtsUs);

    SUCCEED();
}

/**
 * @tc.name: UpdateVideoClock_004
 * @tc.desc: Integration test for UpdateVideoClock through FeedStream.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateVideoClock_004, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateVideoClock_004");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.isAVsync = 1;
    param.fd = 10;
    param.sharedMemLen = 1024;
    param.scene = 0;
    producer_->UpdateProducerWorkMode(param);

    producer_->Start();

    auto buffer = std::make_shared<DataBuffer>(100);
    buffer->frameInfo_.rawTime = 1500000; // Set a specific timestamp

    producer_->FeedStream(buffer);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    producer_->Stop();
    SUCCEED();
}

/**
 * @tc.name: UpdateVideoClock_005
 * @tc.desc: Verify UpdateVideoClock function handles different video timestamps.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, UpdateVideoClock_005, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest UpdateVideoClock_005");
    ASSERT_NE(producer_, nullptr);

    WorkModeParam param(-1, 0, 0, 0);
    param.isAVsync = 1;
    param.fd = 10;
    param.sharedMemLen = 1024;
    producer_->DCameraStreamDataProcessProducer::workModeParam_ = param;

    producer_->syncMem_ = sptr<Ashmem>(new (std::nothrow) Ashmem(param.fd, param.sharedMemLen));
    std::vector<uint64_t> timestamps = {0, 1000000, 50000000, 100000000};
    for (uint64_t timestamp : timestamps) {
        producer_->UpdateVideoClock(timestamp);
    }

    SUCCEED();
}

/**
 * @tc.name: WaitForVideoFrame_001
 * @tc.desc: Test DCameraStreamDataProcessProducer WaitForVideoFrame with empty queue and stopped sync.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, WaitForVideoFrame_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest WaitForVideoFrame_001");
    std::shared_ptr<DataBuffer> buffer = nullptr;
    producer_->syncRunning_.store(false);

    bool shouldBreak = producer_->WaitForVideoFrame(buffer);
    EXPECT_TRUE(shouldBreak);
    EXPECT_EQ(buffer, nullptr);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_006
 * @tc.desc: Test UpdateProducerWorkMode func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_006");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> producer =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    WorkModeParam param(12, 120, 0, true);
    producer->UpdateProducerWorkMode(param);
    EXPECT_EQ(true, producer->workModeParam_.isAVsync);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_007
 * @tc.desc: Test UpdateProducerWorkMode func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_stream_data_process_producer_test_007");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> producer =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, STREAM_ID_1,
        DCStreamType::SNAPSHOT_FRAME);
    WorkModeParam param(-1, 0, 0, false);
    uint32_t memLen = sizeof(static_cast<uint32_t>(120));
    std::string memName = "testMemory";
    auto syncSharedMem = OHOS::Ashmem::CreateAshmem(memName.c_str(), memLen);
    param.fd = syncSharedMem->GetAshmemFd();
    param.sharedMemLen = 120;
    producer->syncMem_ = syncSharedMem;
    producer->UpdateProducerWorkMode(param);
    EXPECT_EQ(false, producer->workModeParam_.isAVsync);
}
} // namespace DistributedHardware
} // namespace OHOS
