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

#include "dcamera_pipeline_sink.h"
#include "fps_controller_process.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "mock_dcamera_data_process_listener.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraPipelineSinkTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<IDataProcessPipeline> testSinkPipeline_;
    std::shared_ptr<DCameraPipelineSink> testPipelineSink_;
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGHT = 1080;
const int32_t SLEEP_TIME = 200000;
}

void DCameraPipelineSinkTest::SetUpTestCase(void)
{
}

void DCameraPipelineSinkTest::TearDownTestCase(void)
{
}

void DCameraPipelineSinkTest::SetUp(void)
{
    testSinkPipeline_ = std::make_shared<DCameraPipelineSink>();
    testPipelineSink_ = std::make_shared<DCameraPipelineSink>();
}

void DCameraPipelineSinkTest::TearDown(void)
{
    testSinkPipeline_ = nullptr;
    testPipelineSink_ = nullptr;
}

/**
 * @tc.name: dcamera_pipeline_sink_test_001
 * @tc.desc: Verify pipeline sink CreateDataProcessPipeline normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testSinkPipeline_ == nullptr);

    std::shared_ptr<DataProcessListener> listener = std::make_shared<MockDCameraDataProcessListener>();
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);
    usleep(SLEEP_TIME);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_002
 * @tc.desc: Verify pipeline sink ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testSinkPipeline_ == nullptr);

    std::shared_ptr<DataProcessListener> listener = std::make_shared<MockDCameraDataProcessListener>();
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(db);
    rc = testSinkPipeline_->ProcessData(buffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    usleep(SLEEP_TIME);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_003
 * @tc.desc: Verify pipeline sink CreateDataProcessPipeline abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testSinkPipeline_ == nullptr);

    std::shared_ptr<DataProcessListener> listener = std::make_shared<MockDCameraDataProcessListener>();
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(
        PipelineType::PHOTO_JPEG, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
    usleep(SLEEP_TIME);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_004
 * @tc.desc: Verify pipeline sink ProcessData abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testSinkPipeline_ == nullptr);

    std::shared_ptr<DataProcessListener> listener = std::make_shared<MockDCameraDataProcessListener>();
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> buffers;
    rc = testSinkPipeline_->ProcessData(buffers);
    EXPECT_NE(rc, DCAMERA_OK);
    usleep(SLEEP_TIME);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_005
 * @tc.desc: Verify pipeline sink ProcessData abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testSinkPipeline_ == nullptr);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(db);
    int32_t rc = testSinkPipeline_->ProcessData(buffers);
    EXPECT_EQ(rc, DCAMERA_INIT_ERR);
    usleep(SLEEP_TIME);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_006
 * @tc.desc: Verify pipeline sink IsInRange abnormal.
 * @tc.type: FUNC
 * @tc.require: I9NPV9
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testPipelineSink_ == nullptr);

    VideoConfigParams vcParams(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        TEST_WIDTH, TEST_HEIGHT);
    bool vc = testPipelineSink_->IsInRange(vcParams);
    EXPECT_EQ(true, vc);

    VideoConfigParams vcParams1(VideoCodecType::NO_CODEC, Videoformat::NV21, -1,
        TEST_WIDTH, TEST_HEIGHT);
    bool vc1 = testPipelineSink_->IsInRange(vcParams1);
    EXPECT_EQ(false, vc1);

    VideoConfigParams vcParams2(VideoCodecType::NO_CODEC, Videoformat::NV21, 31,
        TEST_WIDTH, TEST_HEIGHT);
    bool vc2 = testPipelineSink_->IsInRange(vcParams2);
    EXPECT_EQ(false, vc2);

    VideoConfigParams vcParams3(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        300, TEST_HEIGHT);
    bool vc3 = testPipelineSink_->IsInRange(vcParams3);
    EXPECT_EQ(false, vc3);

    VideoConfigParams vcParams4(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        2000, TEST_HEIGHT);
    bool vc4 = testPipelineSink_->IsInRange(vcParams4);
    EXPECT_EQ(false, vc4);

    VideoConfigParams vcParams5(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        TEST_WIDTH, 200);
    bool vc5 = testPipelineSink_->IsInRange(vcParams5);
    EXPECT_EQ(false, vc5);

    VideoConfigParams vcParams6(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        TEST_WIDTH, 1100);
    bool vc6 = testPipelineSink_->IsInRange(vcParams6);
    EXPECT_EQ(false, vc6);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_007
 * @tc.desc: Verify pipeline sink GetProperty abnormal.
 * @tc.type: FUNC
 * @tc.require: I9NPV9
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testPipelineSink_ == nullptr);

    std::string propertyName = "propertyName";
    PropertyCarrier propertyCarrier;
    int32_t rc1 = testPipelineSink_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(DCAMERA_BAD_VALUE, rc1);

    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource(sourcePipeline);
    testPipelineSink_->pipelineHead_ = std::make_shared<FpsControllerProcess>(callbackPipelineSource);
    int32_t rc2 = testPipelineSink_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(DCAMERA_OK, rc2);
    
    testPipelineSink_->pipelineHead_ = std::make_shared<FpsControllerProcess>(callbackPipelineSource);
    int32_t rc3 = testPipelineSink_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(DCAMERA_OK, rc3);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_008
 * @tc.desc: Verify pipeline sink OnProcessedVideoBuffer abnormal.
 * @tc.type: FUNC
 * @tc.require: I9NPV9
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testPipelineSink_ == nullptr);

    size_t i = 1;
    std::shared_ptr<DataBuffer> videoResult = std::make_shared<DataBuffer>(i);
    testPipelineSink_->OnProcessedVideoBuffer(videoResult);
    EXPECT_TRUE(true);

    testPipelineSink_->processListener_ = std::make_shared<MockDCameraDataProcessListener>();
    testPipelineSink_->OnProcessedVideoBuffer(videoResult);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_009
 * @tc.desc: Verify pipeline sink OnError abnormal.
 * @tc.type: FUNC
 * @tc.require: I9NPV9
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_009, TestSize.Level1)
{
    EXPECT_EQ(false, testPipelineSink_ == nullptr);

    DataProcessErrorType errorType = DataProcessErrorType::ERROR_PIPELINE_ENCODER;
    testPipelineSink_->OnError(errorType);
    EXPECT_TRUE(true);

    testPipelineSink_->processListener_ = std::make_shared<MockDCameraDataProcessListener>();
    testPipelineSink_->OnError(errorType);
    EXPECT_TRUE(true);
}

/**
 * @tc.name: dcamera_pipeline_sink_test_010
 * @tc.desc: Verify pipeline sink DestroyDataProcessPipeline abnormal.
 * @tc.type: FUNC
 * @tc.require: I9NPV9
 */
HWTEST_F(DCameraPipelineSinkTest, dcamera_pipeline_sink_test_010, TestSize.Level1)
{
    EXPECT_EQ(false, testPipelineSink_ == nullptr);

    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource(sourcePipeline);
    testPipelineSink_->pipelineHead_ = std::make_shared<FpsControllerProcess>(callbackPipelineSource);
    testPipelineSink_->piplineType_ = PipelineType::PHOTO_JPEG;
    testPipelineSink_->DestroyDataProcessPipeline();
    EXPECT_EQ(PipelineType::VIDEO, testPipelineSink_->piplineType_);

    testPipelineSink_->pipelineHead_ = nullptr;
    testPipelineSink_->piplineType_ = PipelineType::PHOTO_JPEG;
    testPipelineSink_->DestroyDataProcessPipeline();
    EXPECT_EQ(PipelineType::VIDEO, testPipelineSink_->piplineType_);
}
} // namespace DistributedHardware
} // namespace OHOS
