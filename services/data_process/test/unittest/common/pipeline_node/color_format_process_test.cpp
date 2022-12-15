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

/**
 * @tc.name: color_format_process_test_007
 * @tc.desc: Verify color format process InitNode abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV12,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: color_format_process_test_008
 * @tc.desc: Verify color format process InitNode abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV12,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testColorFmtProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: color_format_process_test_009
 * @tc.desc: Verify color format process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_009, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV12,
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
 * @tc.name: color_format_process_test_010
 * @tc.desc: Verify color format process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_010, TestSize.Level1)
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
    int64_t timeStamp = 10;
    db->SetInt64("timeUs", timeStamp);
    inputBuffers.push_back(db);
    rc = testColorFmtProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_011
 * @tc.desc: Verify color format process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_011, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    std::shared_ptr<DataBuffer> imgBuf = nullptr;
    int32_t rc = testColorFmtProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    size_t capacity = 100;
    imgBuf = std::make_shared<DataBuffer>(capacity);
    rc = testColorFmtProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::RGBA_8888));
    rc = testColorFmtProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: color_format_process_test_012
 * @tc.desc: Verify color format process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_012, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    size_t capacity = 100;
    std::shared_ptr<DataBuffer> imgBuf = std::make_shared<DataBuffer>(capacity);
    int64_t timeStamp = 10;
    imgBuf->SetInt64("timeUs", timeStamp);
    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::NV12));
    imgBuf->SetInt32("alignedWidth", TEST_WIDTH);
    imgBuf->SetInt32("width", TEST_WIDTH);
    imgBuf->SetInt32("height", TEST_HEIGTH);

    int32_t rc = testColorFmtProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: color_format_process_test_013
 * @tc.desc: Verify color format process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_013, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    size_t capacity = 100;
    std::shared_ptr<DataBuffer> imgBuf = std::make_shared<DataBuffer>(capacity);
    int64_t timeStamp = 10;
    imgBuf->SetInt64("timeUs", timeStamp);
    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::NV12));
    imgBuf->SetInt32("alignedWidth", TEST_WIDTH);
    imgBuf->SetInt32("alignedHeight", TEST_HEIGTH);
    imgBuf->SetInt32("width", TEST_WIDTH);
    imgBuf->SetInt32("height", TEST_HEIGTH);

    int32_t rc = testColorFmtProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: color_format_process_test_014
 * @tc.desc: Verify color format process CheckColorProcessInputInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_014, TestSize.Level1)
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

    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    bool ret = testColorFmtProcess_->CheckColorProcessInputInfo(srcImgInfo);
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: color_format_process_test_015
 * @tc.desc: Verify color format process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_015, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    ImageUnitInfo dstImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    bool ret = testColorFmtProcess_->CheckColorConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(ret, false);

    int32_t rc = testColorFmtProcess_->CopyYPlane(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testColorFmtProcess_->SeparateNV12UVPlane(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testColorFmtProcess_->ColorConvertNV12ToI420(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_016
 * @tc.desc: Verify color format process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_016, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    uint8_t *imgData = new uint8_t(10);
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, imgData};
    ImageUnitInfo dstImgInfo {Videoformat::YUVI420, TEST_WIDTH2, TEST_HEIGTH2, 0, 0, 0, 0, imgData};
    int32_t rc = testColorFmtProcess_->CopyYPlane(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testColorFmtProcess_->SeparateNV12UVPlane(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testColorFmtProcess_->ColorConvertNV12ToI420(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: color_format_process_test_017
 * @tc.desc: Verify color format process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ColorFormatProcessTest, color_format_process_test_017, TestSize.Level1)
{
    EXPECT_EQ(false, testColorFmtProcess_ == nullptr);

    uint8_t *imgData = new uint8_t(10);
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, imgData};
    ImageUnitInfo dstImgInfo {Videoformat::RGBA_8888, TEST_WIDTH2, TEST_HEIGTH2, 0, 0, 0, 0, imgData};
    int32_t rc = testColorFmtProcess_->ColorConvertByColorFormat(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_OPERATE);
}
} // namespace DistributedHardware
} // namespace OHOS
