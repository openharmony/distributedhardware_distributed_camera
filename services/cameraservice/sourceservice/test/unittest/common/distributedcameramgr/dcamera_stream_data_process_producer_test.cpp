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
    int32_t ret = 0;
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
    int32_t ret = 0;
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
    int32_t ret = 0;
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess1 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess1->buffers_.push(buffer);
    streamProcess1->buffers_.push(buffer);
    streamProcess1->FeedStream(buffer);

    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess2 =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::CONTINUOUS_FRAME);
    streamProcess2->FeedStream(buffer);
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
    int32_t ret = 0;
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess->state_ = DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP;
    streamProcess->LooperContinue();
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
    int32_t ret = 0;
    int32_t streamId = 1;
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcessProducer> streamProcess =
        std::make_shared<DCameraStreamDataProcessProducer>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, streamId,
        DCStreamType::SNAPSHOT_FRAME);
    streamProcess->state_ = DCameraStreamDataProcessProducer::DCAMERA_PRODUCER_STATE_STOP;
    streamProcess->LooperSnapShot();
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
    sharedMemory.bufferHandle_ = new BufferHandleSequenceable();
    ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    BufferHandle *bufferHandle = new BufferHandle();
    sharedMemory.bufferHandle_ = new BufferHandleSequenceable(bufferHandle);
    ret = streamProcess->CheckSharedMemory(sharedMemory, buffer);
    EXPECT_EQ(DCAMERA_OK, ret);
}
}
}