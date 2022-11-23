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

#include "dcamera_pipeline_sink.h"
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
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
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
}

void DCameraPipelineSinkTest::TearDown(void)
{
    testSinkPipeline_ = nullptr;
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
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);
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
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);
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
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(
        PipelineType::PHOTO_JPEG, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
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
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    int32_t rc = testSinkPipeline_->CreateDataProcessPipeline(PipelineType::VIDEO, srcParams, destParams, listener);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> buffers;
    rc = testSinkPipeline_->ProcessData(buffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
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
}
} // namespace DistributedHardware
} // namespace OHOS
