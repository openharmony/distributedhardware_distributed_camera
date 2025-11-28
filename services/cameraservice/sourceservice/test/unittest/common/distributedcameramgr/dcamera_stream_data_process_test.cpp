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

#define private public
#include "dcamera_stream_data_process.h"
#undef private

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

#include "dcamera_pipeline_source.h"
#include "dcamera_stream_data_process_pipeline_listener.h"
#include "dcamera_utils_tools.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraStreamDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_FORMAT = 1;
const int32_t TEST_DATASPACE = 1;
const int32_t SLEEP_TIME = 200000;
}
void DCameraStreamDataProcessTest::SetUpTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessTest::SetUpTestCase");
}

void DCameraStreamDataProcessTest::TearDownTestCase(void)
{
    DHLOGI("DCameraStreamDataProcessTest::TearDownTestCase");
}

void DCameraStreamDataProcessTest::SetUp(void)
{
    DHLOGI("DCameraStreamDataProcessTest::SetUp");
}

void DCameraStreamDataProcessTest::TearDown(void)
{
    DHLOGI("DCameraStreamDataProcessTest::TearDown");
}

/**
 * @tc.name: dcamera_stream_data_process_test_001
 * @tc.desc: Verify FeedStream func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_001, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_001");
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamDataProcess> streamProcess1 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::SNAPSHOT_FRAME);

    int32_t ret = streamProcess1->GetProducerSize();
    streamProcess1->FeedStream(buffer);
    EXPECT_EQ(DCAMERA_OK, ret);
    std::shared_ptr<DCameraStreamDataProcess> streamProcess2 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);

    streamProcess2->FeedStream(buffer);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::shared_ptr<DCameraStreamDataProcess> streamProcess3 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, static_cast<DCStreamType>(-1));

    streamProcess3->FeedStream(buffer);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_002
 * @tc.desc: Verify StartCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_002, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_002");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    std::shared_ptr<DCameraStreamConfig> srcConfig =
        std::make_shared<DCameraStreamConfig>(TEST_WIDTH, TEST_HEIGTH, TEST_FORMAT, TEST_DATASPACE,
        DCEncodeType::ENCODE_TYPE_H264, DCStreamType::SNAPSHOT_FRAME);

    std::set<int32_t> streamIds;
    streamIds.insert(1);
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->StartCapture(srcConfig, streamIds);
    streamProcess->StopCapture(streamIds);

    std::shared_ptr<DCameraStreamDataProcess> streamProcess1 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::SNAPSHOT_FRAME);
    streamIds.clear();
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->StartCapture(srcConfig, streamIds);
    streamProcess->StopCapture(streamIds);
    int32_t ret = streamProcess->GetProducerSize();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_003
 * @tc.desc: Verify FeedStreamToSnapShot func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_003, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_003");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);

    streamProcess->FeedStreamToSnapShot(buffer);
    int32_t ret = streamProcess->GetProducerSize();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_004
 * @tc.desc: Verify FeedStreamToContinue func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_004, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_004");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    std::shared_ptr<DCameraStreamConfig> srcConfig =
        std::make_shared<DCameraStreamConfig>(TEST_WIDTH, TEST_HEIGTH, TEST_FORMAT, TEST_DATASPACE,
        DCEncodeType::ENCODE_TYPE_H264, DCStreamType::SNAPSHOT_FRAME);

    std::set<int32_t> streamIds;
    streamIds.insert(1);
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->StartCapture(srcConfig, streamIds);
    streamProcess->FeedStreamToContinue(buffer);
    streamProcess->CreatePipeline();
    usleep(SLEEP_TIME);
    streamProcess->FeedStreamToContinue(buffer);
    int32_t ret = streamProcess->GetProducerSize();
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_005
 * @tc.desc: Verify DestroyPipeline func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_005, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_005");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    streamProcess->pipeline_ = std::make_shared<DCameraPipelineSource>();
    std::shared_ptr<DCameraStreamConfig> srcConfig =
        std::make_shared<DCameraStreamConfig>(TEST_WIDTH, TEST_HEIGTH, TEST_FORMAT, TEST_DATASPACE,
        DCEncodeType::ENCODE_TYPE_H264, DCStreamType::SNAPSHOT_FRAME);

    std::set<int32_t> streamIds;
    streamIds.insert(1);
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->CreatePipeline();
    streamProcess->DestroyPipeline();
    streamProcess->pipeline_ = nullptr;
    streamProcess->DestroyPipeline();
    int32_t ret = streamProcess->GetProducerSize();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_006
 * @tc.desc: Verify DestroyPipeline func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_006, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_006");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);

    streamProcess->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_H264);
    streamProcess->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_H265);
    streamProcess->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_NULL);
    streamProcess->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_MPEG4_ES);
    int32_t ret = streamProcess->GetProducerSize();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_007
 * @tc.desc: Verify DestroyPipeline func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_007, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_007");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess1 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    std::shared_ptr<DCameraStreamDataProcessPipelineListener> listener1 =
        std::make_shared<DCameraStreamDataProcessPipelineListener>(streamProcess1);


    size_t capacity = 1;
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(capacity);
    listener1->OnProcessedVideoBuffer(buffer);
    listener1->OnError(DataProcessErrorType::ERROR_PIPELINE_ENCODER);

    std::shared_ptr<DCameraStreamDataProcess> streamProcess2 = nullptr;
    std::shared_ptr<DCameraStreamDataProcessPipelineListener> listener2 =
        std::make_shared<DCameraStreamDataProcessPipelineListener>(streamProcess2);
    listener2->OnProcessedVideoBuffer(buffer);
    listener2->OnError(DataProcessErrorType::ERROR_PIPELINE_ENCODER);
    int32_t ret = streamProcess1->GetProducerSize();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_008
 * @tc.desc: Verify GetPipelineFormat func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_008, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_008");
    int32_t format = OHOS_CAMERA_FORMAT_RGBA_8888;
    std::shared_ptr<DCameraStreamDataProcess> streamProcess1 =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    Videoformat ret = streamProcess1->GetPipelineFormat(format);
    EXPECT_EQ(Videoformat::RGBA_8888, ret);
    format = -1;
    ret = streamProcess1->GetPipelineFormat(format);
    EXPECT_EQ(Videoformat::NV21, ret);
}

/**
 * @tc.name: dcamera_stream_data_process_test_002
 * @tc.desc: Verify StartCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_009, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_009");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    std::shared_ptr<DCameraStreamConfig> srcConfig =
        std::make_shared<DCameraStreamConfig>(TEST_WIDTH, TEST_HEIGTH, TEST_FORMAT, TEST_DATASPACE,
        DCEncodeType::ENCODE_TYPE_H264, DCStreamType::SNAPSHOT_FRAME);

    std::set<int32_t> streamIds;
    streamIds.insert(1);
    WorkModeParam param(12, 120, 0, false);
    std::vector<int32_t> streamId;
    streamId.push_back(1);
    int32_t ret = streamProcess->UpdateProducerWorkMode(streamId, param);
    EXPECT_EQ(DCAMERA_OK, ret);
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->StartCapture(srcConfig, streamIds);
    ret = streamProcess->UpdateProducerWorkMode(streamId, param);
    EXPECT_EQ(DCAMERA_OK, ret);
    streamProcess->StopCapture(streamIds);
}

/**
 * @tc.name: dcamera_stream_data_process_test_010
 * @tc.desc: Verify StartCapture func.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraStreamDataProcessTest, dcamera_stream_data_process_test_010, TestSize.Level1)
{
    DHLOGI("DCameraStreamDataProcessTest::dcamera_stream_data_process_test_010");
    std::shared_ptr<DCameraStreamDataProcess> streamProcess =
        std::make_shared<DCameraStreamDataProcess>(TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, DCStreamType::CONTINUOUS_FRAME);
    std::shared_ptr<DCameraStreamConfig> srcConfig =
        std::make_shared<DCameraStreamConfig>(TEST_WIDTH, TEST_HEIGTH, TEST_FORMAT, TEST_DATASPACE,
        DCEncodeType::ENCODE_TYPE_H264, DCStreamType::SNAPSHOT_FRAME);

    std::set<int32_t> streamIds;
    streamIds.insert(1);
    streamProcess->ConfigStreams(srcConfig, streamIds);
    streamProcess->StartCapture(srcConfig, streamIds);
    DCameraSystemSwitchInfo::GetInstance().SetSystemSwitchFlagAndRotation(TEST_DEVICE_ID, true, 90);
    streamProcess->CreatePipeline();
    int32_t ret = streamProcess->GetProducerSize();
    streamProcess->DestroyPipeline();
    EXPECT_EQ(1, ret);
}
}
}