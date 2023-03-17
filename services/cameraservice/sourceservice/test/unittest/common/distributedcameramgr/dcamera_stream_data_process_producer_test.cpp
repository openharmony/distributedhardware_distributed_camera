/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#define private public
#include "dcamera_stream_data_process_producer.h"
#undef private

#include "anonymous_string.h"
#include "dcamera_buffer_handle.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "v1_0/dcamera_types.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraStreamDataProcessProducerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int64_t TEST_TIMESTAMP = 1;
const int64_t TEST_DURATION = 1;
const int64_t TEST_CLOCK = 1;
#define DCAMERA_PRODUCER_MAX_BUFFER_SIZE 1
}
void DCameraStreamDataProcessProducerTest::SetUpTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::SetUpTestCase");
}

void DCameraStreamDataProcessProducerTest::TearDownTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::TearDownTestCase");
}

void DCameraStreamDataProcessProducerTest::SetUp(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::SetUp");
}

void DCameraStreamDataProcessProducerTest::TearDown(void)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::TearDown");
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_001
 * @tc.desc: Verify FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_001");
    int32_t streamId = 1;
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess1->Start();
    sleep(1);
    streamProcess1->Stop();
    streamId = 2;
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess2->Start();
    sleep(1);
    streamProcess2->Stop();
    int32_t ret = streamProcess2->SyncClock(TEST_TIMESTAMP, TEST_DURATION, TEST_CLOCK);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_002
 * @tc.desc: Verify FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_002");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess1->FeedStream(buffer);

    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess2->FeedStream(buffer);
    int32_t ret = streamProcess2->SyncClock(TEST_TIMESTAMP, TEST_DURATION, TEST_CLOCK);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_003
 * @tc.desc: Verify FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_003");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess1->buffers_.push_back(buffer);
    streamProcess1->buffers_.push_back(buffer);
    streamProcess1->FeedStream(buffer);

    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess2->FeedStream(buffer);
    int32_t ret = streamProcess2->SyncClock(TEST_TIMESTAMP, TEST_DURATION, TEST_CLOCK);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_004
 * @tc.desc: Verify LooperContinue func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_004, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_004");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess->state_ = DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP;
    streamProcess->LooperContinue();
    int32_t ret = streamProcess->SyncClock(TEST_TIMESTAMP, TEST_DURATION, TEST_CLOCK);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_005
 * @tc.desc: Verify LooperContinue func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_005, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_005");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess->state_ = DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP;
    streamProcess->LooperSnapShot();
    int32_t ret = streamProcess->SyncClock(TEST_TIMESTAMP, TEST_DURATION, TEST_CLOCK);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_006
 * @tc.desc: Verify FeedStreamToDriver func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_006, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_006");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    DHBase dhBase;
    dhBase.deviceId_ = TEST_DEVICE_ID;
    dhBase.dhId_ = TEST_CAMERA_DH_ID_0;
    int32_t ret = streamProcess->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
    streamProcess->Start();
    streamProcess->FeedStreamToDriver(dhBase, buffer);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_007
 * @tc.desc: Verify CheckSharedMemory func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_007, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_007");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    sharedMemory.bufferHandle_ = nullptr;
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    sharedMemory.bufferHandle_ = new NativeBuffer();
    ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_008
 * @tc.desc: Verify ControlFrameRate func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_008, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_008");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);

    streamProcess->ControlFrameRate(timeStamp);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_009
 * @tc.desc: Verify InitTime func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_009, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_009");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    streamProcess->lastTimeStamp_ = timeStamp;
    streamProcess->InitTime(timeStamp);

    streamProcess->lastTimeStamp_ = 0;
    streamProcess->InitTime(timeStamp);

    streamProcess->lastTimeStamp_ = timeStamp + 1;
    streamProcess->InitTime(timeStamp);

    streamProcess->sysTimeBaseline_ = 0;
    streamProcess->InitTime(timeStamp);

    streamProcess->sysTimeBaseline_ = timeStamp;
    streamProcess->InitTime(timeStamp);

    streamProcess->leaveTime_ = 0;
    streamProcess->InitTime(timeStamp);

    streamProcess->leaveTime_ = timeStamp;
    streamProcess->InitTime(timeStamp);

    streamProcess->lastEnterTime_ = 0;
    streamProcess->InitTime(timeStamp);

    streamProcess->lastEnterTime_ = timeStamp;
    streamProcess->InitTime(timeStamp);

    streamProcess->timeStampBaseline_ = 0;
    streamProcess->InitTime(timeStamp);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_010
 * @tc.desc: Verify ControlDisplay func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_010, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_010");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    int64_t duration = 100;
    int64_t clock = 1;
    streamProcess->ControlDisplay(timeStamp, duration, clock);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_011
 * @tc.desc: Verify AdjustSleep func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_011, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_011");
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    int64_t duration = 100;

    streamProcess->AdjustSleep(duration);

    streamProcess->delta_ = duration;
    streamProcess->sleep_ = 0;
    streamProcess->AdjustSleep(duration);

    streamProcess->delta_ = duration - 1;
    streamProcess->sleep_ = 1;
    streamProcess->AdjustSleep(duration);

    streamProcess->delta_ = -100;
    streamProcess->sleep_ = 1;
    streamProcess->AdjustSleep(duration);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_012
 * @tc.desc: Verify SyncClock func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_012, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_012");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    int64_t duration = 100;
    int64_t clock = 1;

    streamProcess->sleep_ = duration;
    streamProcess->SyncClock(timeStamp, duration, clock);

    streamProcess->sleep_ = -1;
    duration = -100;
    timeStamp = 1;
    streamProcess->SyncClock(timeStamp, duration, clock);

    timeStamp = -10;
    clock = 1;
    streamProcess->SyncClock(timeStamp, duration, clock);

    duration = -10;
    streamProcess->SyncClock(timeStamp, duration, clock);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_013
 * @tc.desc: Verify LocateBaseline func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_013, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_013");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    int64_t duration = 100;
    int64_t offset = 1;

    streamProcess->buffers_.push_back(buffer);
    streamProcess->displayBufferSize_ = 2;
    streamProcess->sleep_ = duration;
    streamProcess->LocateBaseline(timeStamp, duration, offset);

    streamProcess->sleep_ = duration;
    streamProcess->LocateBaseline(timeStamp, duration, offset);

    streamProcess->buffers_.push_back(buffer);
    streamProcess->displayBufferSize_ = 0;
    streamProcess->sleep_ = 1;
    offset = -duration;
    streamProcess->LocateBaseline(timeStamp, duration, offset);

    offset = duration;
    streamProcess->sleep_ = 1;
    streamProcess->LocateBaseline(timeStamp, duration, offset);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_014
 * @tc.desc: Verify FinetuneBaseline func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_014, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_014");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);

    streamProcess->needFinetune_.store(true);
    streamProcess->FinetuneBaseline(timeStamp);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_producer_test_015
 * @tc.desc: Verify CalculateAverFeedInterval func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessProducerTest, dcamera_stream_data_process_producer_test_015, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessProducerTest::dcamera_stream_data_process_producer_test_015");
    int32_t streamId = 1;
    size_t capacity = 1;
    int64_t timeStamp = 421476047;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    DCameraBuffer sharedMemory;
    sharedMemory.index_ = 1;
    sharedMemory.size_ = 1;
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new NativeBuffer(bufferHandle);
    int32_t ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);

    streamProcess->feedTime_ = 0;
    streamProcess->CalculateAverFeedInterval(timeStamp);

    streamProcess->feedTime_ = 1;
    streamProcess->CalculateAverFeedInterval(timeStamp);
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}