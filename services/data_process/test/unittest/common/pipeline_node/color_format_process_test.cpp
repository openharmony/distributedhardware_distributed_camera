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
#include "color_format_process.h"
#undef private
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class ColorFormatProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ColorFormatProcess> testColorFmtProcess_;
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;
}

void ColorFormatProcessTest::SetUpTestCase(void)
{
}

void ColorFormatProcessTest::TearDownTestCase(void)
{
}

void ColorFormatProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource(sourcePipeline);
    testColorFmtProcess_ = std::make_shared<ColorFormatProcess>(callbackPipelineSource);
}

void ColorFormatProcessTest::TearDown(void)
{
    testColorFmtProcess_ = nullptr;
}

/**
 * @tc.name: color_format_process_test_001
 * @tc.desc: Verify color format process InitNode normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

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
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: color_format_process_test_002
 * @tc.desc: Verify color format process InitNode abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: color_format_process_test_003
 * @tc.desc: Verify color format process ProcessData inputBuffers is empty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

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
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    rc = testColorFmtProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_004
 * @tc.desc: Verify color format process ProcessData inputBuffers[0] is nullptr.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

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
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db;
    inputBuffers.push_back(db);
    rc = testColorFmtProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_005
 * @tc.desc: Verify color format process ProcessData find timeStamp failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

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
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testColorFmtProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_006
 * @tc.desc: Verify color format process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

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
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 3200000;
    int64_t timeStamp = 10;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt64("timeUs", timeStamp);
    db->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::NV12));
    db->SetInt32("alignedWidth", TEST_WIDTH);
    db->SetInt32("alignedHeight", TEST_HEIGTH);
    db->SetInt32("width", TEST_WIDTH);
    db->SetInt32("height", TEST_HEIGTH);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testColorFmtProcess_->callbackPipelineSource_ = sourcePipeline;
    rc = testColorFmtProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
