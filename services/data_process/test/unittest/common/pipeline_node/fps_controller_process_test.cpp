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

#define private public
#include "fps_controller_process.h"
#undef private
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class FpsControllerProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<FpsControllerProcess> testFpsControllerProcess_;
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;
}

void FpsControllerProcessTest::SetUpTestCase(void)
{
}

void FpsControllerProcessTest::TearDownTestCase(void)
{
}

void FpsControllerProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource(sourcePipeline);
    testFpsControllerProcess_ = std::make_shared<FpsControllerProcess>(callbackPipelineSource);
}

void FpsControllerProcessTest::TearDown(void)
{
    testFpsControllerProcess_ = nullptr;
}

/**
 * @tc.name: fps_controller_process_test_001
 * @tc.desc: Verify fps controller process InitNode normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: fps_controller_process_test_002
 * @tc.desc: Verify fps controller process InitNode abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    int32_t frameRate = 50;
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 frameRate,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: fps_controller_process_test_003
 * @tc.desc: Verify fps controller process ProcessData inputBuffers is empty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: fps_controller_process_test_004
 * @tc.desc: Verify fps controller process ProcessData isFpsControllerProcess_ false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    int32_t rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);

    std::shared_ptr<DataBuffer> db;
    inputBuffers.push_back(db);
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);

    inputBuffers.clear();
    size_t capacity = 100;
    int64_t timeStamp = 10;
    std::shared_ptr<DataBuffer> dataBuf = std::make_shared<DataBuffer>(capacity);
    dataBuf->SetInt64("timeUs", timeStamp);
    inputBuffers.push_back(dataBuf);
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_DISABLE_PROCESS);
}

/**
 * @tc.name: fps_controller_process_test_005
 * @tc.desc: Verify fps controller process ProcessData find timeStamp failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: fps_controller_process_test_006
 * @tc.desc: Verify fps controller process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    int64_t timeStamp = 10;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt64("timeUs", timeStamp);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testFpsControllerProcess_->callbackPipelineSource_ = sourcePipeline;
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: fps_controller_process_test_007
 * @tc.desc: Verify fps controller process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 0,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    int64_t timeStamp = 10;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt64("timeUs", timeStamp);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testFpsControllerProcess_->callbackPipelineSource_ = sourcePipeline;
    rc = testFpsControllerProcess_->ProcessData(inputBuffers);
    int64_t nowMs = 100;
    testFpsControllerProcess_->UpdateFrameRateCorrectionFactor(nowMs);
    testFpsControllerProcess_->UpdateIncomingFrameTimes(nowMs);
    float ret = testFpsControllerProcess_->CalculateFrameRate(nowMs);
    bool brc = testFpsControllerProcess_->IsDropFrame(ret);
    EXPECT_EQ(brc, true);

    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    rc = testFpsControllerProcess_->FpsControllerDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: fps_controller_process_test_008
 * @tc.desc: Verify fps controller process UpdateFrameRateCorrectionFactor.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(FpsControllerProcessTest, fps_controller_process_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testFpsControllerProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testFpsControllerProcess_->InitNode(srcParams, destParams, procConfig);
    int64_t nowMs = 100;
    testFpsControllerProcess_->UpdateFrameRateCorrectionFactor(nowMs);
    testFpsControllerProcess_->UpdateIncomingFrameTimes(nowMs);
    float ret = testFpsControllerProcess_->CalculateFrameRate(nowMs);
    bool brc = testFpsControllerProcess_->IsDropFrame(ret);
    EXPECT_EQ(brc, false);
    EXPECT_EQ(rc, DCAMERA_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
