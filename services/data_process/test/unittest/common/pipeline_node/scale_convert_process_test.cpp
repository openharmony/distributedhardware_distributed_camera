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
#include "scale_convert_process.h"
#undef private
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class ScaleConvertProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ScaleConvertProcess> testScaleConvertProcess_;
};

void ScaleConvertProcessTest::SetUpTestCase(void)
{
}

void ScaleConvertProcessTest::TearDownTestCase(void)
{
}

void ScaleConvertProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testScaleConvertProcess_ = std::make_shared<ScaleConvertProcess>(sourcePipeline);
}

void ScaleConvertProcessTest::TearDown(void)
{
    testScaleConvertProcess_ = nullptr;
}

#ifdef DCAMERA_SUPPORT_FFMPEG
namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;

VideoConfigParams SRC_PARAMS1(VideoCodecType::CODEC_H264,
                            Videoformat::NV12,
                            DCAMERA_PRODUCER_FPS_DEFAULT,
                            TEST_WIDTH,
                            TEST_HEIGTH);
VideoConfigParams DEST_PARAMS1(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
VideoConfigParams DEST_PARAMS2(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH2,
                                TEST_HEIGTH2);

VideoConfigParams PROC_CONFIG;
}
/**
 * @tc.name: scale_convert_process_test_001
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_001, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_002
 * @tc.desc: Verify scale convert process InitNode IsConvertible false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_002, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_003
 * @tc.desc: Verify scale convert process ProcessData isScaleConvert_ false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_003, TestSize.Level1)
{
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    int32_t rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_DISABLE_PROCESS);
}

/**
 * @tc.name: scale_convert_process_test_004
 * @tc.desc: Verify scale convert process ProcessData inputBuffers is empty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_004, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_005
 * @tc.desc: Verify scale convert process ProcessData IsConvertible false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_005, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_006
 * @tc.desc: Verify scale convert process ProcessData IsConvertible true find timeStamp fail.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_006, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_007
 * @tc.desc: Verify scale convert process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_007, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
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
    testScaleConvertProcess_->callbackPipelineSource_ = sourcePipeline;
    testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_008
 * @tc.desc: Verify scale convert process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_008, TestSize.Level1)
{
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 3200000;
    int64_t timeStamp = 10;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt64("timeUs", timeStamp);
    inputBuffers.push_back(db);
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_009
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_009, TestSize.Level1)
{
    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    std::shared_ptr<DataBuffer> imgBuf = nullptr;
    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    size_t capacity = 100;
    imgBuf = std::make_shared<DataBuffer>(capacity);
    rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::RGBA_8888));
    rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: scale_convert_process_test_010
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_010, TestSize.Level1)
{
    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    size_t capacity = 100;
    std::shared_ptr<DataBuffer> imgBuf = std::make_shared<DataBuffer>(capacity);
    int64_t timeStamp = 10;
    imgBuf->SetInt64("timeUs", timeStamp);
    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::NV12));
    imgBuf->SetInt32("alignedWidth", TEST_WIDTH);
    imgBuf->SetInt32("width", TEST_WIDTH);
    imgBuf->SetInt32("height", TEST_HEIGTH);

    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: scale_convert_process_test_011
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_011, TestSize.Level1)
{
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

    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_012
 * @tc.desc: Verify scale convert process CheckScaleConvertInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_012, TestSize.Level1)
{
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    ImageUnitInfo dstImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};

    bool rc = testScaleConvertProcess_->CheckScaleConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, false);

    std::shared_ptr<DataBuffer> dtBuf = std::make_shared<DataBuffer>(10);
    srcImgInfo.imgData = dtBuf;
    dstImgInfo.imgData = dtBuf;
    rc = testScaleConvertProcess_->CheckScaleConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, false);
}

/**
 * @tc.name: scale_convert_process_test_013
 * @tc.desc: Verify scale convert process CheckScaleConvertInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_013, TestSize.Level1)
{
    std::shared_ptr<DataBuffer> imgData = std::make_shared<DataBuffer>(10);
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, imgData};
    ImageUnitInfo dstImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, imgData};

    bool rc = testScaleConvertProcess_->CheckScaleConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, false);
}

/**
 * @tc.name: scale_convert_process_test_014
 * @tc.desc: Verify scale convert process ScaleConvert.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_014, TestSize.Level1)
{
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    ImageUnitInfo dstImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};

    int32_t rc = testScaleConvertProcess_->ScaleConvert(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_015
 * @tc.desc: Verify scale convert process CopyYUV420SrcData CopyNV21SrcData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_015, TestSize.Level1)
{
    ImageUnitInfo srcImgInfo {};
    std::shared_ptr<DataBuffer> dtBuf = std::make_shared<DataBuffer>(10);
    srcImgInfo.alignedWidth = TEST_WIDTH2;
    srcImgInfo.alignedHeight = TEST_HEIGTH2;
    srcImgInfo.imgData = dtBuf;

    int32_t rc = testScaleConvertProcess_->CopyYUV420SrcData(srcImgInfo);
    EXPECT_EQ(rc, DCAMERA_MEMORY_OPT_ERROR);

    rc = testScaleConvertProcess_->CopyNV21SrcData(srcImgInfo);
    EXPECT_EQ(rc, DCAMERA_MEMORY_OPT_ERROR);
}

/**
 * @tc.name: scale_convert_process_test_016
 * @tc.desc: Verify scale convert process ConvertDone.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_016, TestSize.Level1)
{
    Videoformat colorFormat = Videoformat::NV21;
    AVPixelFormat f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    colorFormat = Videoformat::RGBA_8888;
    f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    int32_t rc = testScaleConvertProcess_->ConvertDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_017
 * @tc.desc: Verify scale convert process ConvertDone.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_017, TestSize.Level1)
{
    Videoformat colorFormat = Videoformat::NV12;
    AVPixelFormat f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    EXPECT_EQ(f, AVPixelFormat::AV_PIX_FMT_NV12);

    colorFormat = Videoformat::NV21;
    f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    EXPECT_EQ(f, AVPixelFormat::AV_PIX_FMT_NV21);

    colorFormat = Videoformat::RGBA_8888;
    f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    EXPECT_EQ(f, AVPixelFormat::AV_PIX_FMT_RGBA);

    colorFormat = Videoformat::YUVI420;
    f = testScaleConvertProcess_->GetAVPixelFormat(colorFormat);
    EXPECT_EQ(f, AVPixelFormat::AV_PIX_FMT_YUV420P);
}
#else
namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;
const int32_t TEST_ALIGNEDWIDTH = 1920;
const int32_t TEST_ALIGNEDHEIGTH = 1088;

VideoConfigParams SRC_PARAMS1(VideoCodecType::CODEC_H264,
                            Videoformat::NV12,
                            DCAMERA_PRODUCER_FPS_DEFAULT,
                            TEST_WIDTH,
                            TEST_HEIGTH);
VideoConfigParams DEST_PARAMS1(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
VideoConfigParams DEST_PARAMS2(VideoCodecType::CODEC_H264,
                                Videoformat::NV21,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH2,
                                TEST_HEIGTH2);

VideoConfigParams PROC_CONFIG;
}
/**
 * @tc.name: scale_convert_process_test_018
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_018, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_018.");
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_019
 * @tc.desc: Verify scale convert process ProcessData isScaleConvert_ false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_019, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_019.");
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    int32_t rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_DISABLE_PROCESS);
}

/**
 * @tc.name: scale_convert_process_test_020
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_020, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_020.");
    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    std::shared_ptr<DataBuffer> imgBuf = nullptr;
    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    size_t capacity = 100;
    imgBuf = std::make_shared<DataBuffer>(capacity);
    rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::RGBA_8888));
    rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: scale_convert_process_test_021
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_021, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_021.");
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

    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_022
 * @tc.desc: Verify scale convert process CheckScaleConvertInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_022, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_022.");
    ImageUnitInfo srcImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};
    ImageUnitInfo dstImgInfo {Videoformat::NV12, TEST_WIDTH, TEST_HEIGTH, 0, 0, 0, 0, nullptr};

    bool rc = testScaleConvertProcess_->CheckScaleConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, false);

    std::shared_ptr<DataBuffer> dtBuf = std::make_shared<DataBuffer>(10);
    srcImgInfo.imgData = dtBuf;
    dstImgInfo.imgData = dtBuf;
    rc = testScaleConvertProcess_->CheckScaleConvertInfo(srcImgInfo, dstImgInfo);
    EXPECT_EQ(rc, false);
}

/**
 * @tc.name: scale_convert_process_test_023
 * @tc.desc: Verify scale convert process GetImageUnitInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_023, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_023.");
    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    size_t capacity = 100;
    std::shared_ptr<DataBuffer> imgBuf = std::make_shared<DataBuffer>(capacity);
    int64_t timeStamp = 10;
    imgBuf->SetInt64("timeUs", timeStamp);
    imgBuf->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::NV12));
    imgBuf->SetInt32("alignedWidth", TEST_WIDTH);
    imgBuf->SetInt32("width", TEST_WIDTH);
    imgBuf->SetInt32("height", TEST_HEIGTH);

    int32_t rc = testScaleConvertProcess_->GetImageUnitInfo(srcImgInfo, imgBuf);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: scale_convert_process_test_024
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_024, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_024.");
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_025
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_025, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_025.");
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS1, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: scale_convert_process_test_026
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_026, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_026.");
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 3200000;
    int64_t timeStamp = 10;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt64("timeUs", timeStamp);
    db->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::YUVI420));
    db->SetInt32("alignedWidth", TEST_ALIGNEDWIDTH);
    db->SetInt32("alignedHeight", TEST_ALIGNEDHEIGTH);
    db->SetInt32("width", TEST_WIDTH);
    db->SetInt32("height", TEST_HEIGTH);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testScaleConvertProcess_->callbackPipelineSource_ = sourcePipeline;
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: scale_convert_process_test_027
 * @tc.desc: Verify scale convert process InitNode IsConvertible true.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(ScaleConvertProcessTest, scale_convert_process_test_027, TestSize.Level1)
{
    DHLOGI("ScaleConvertProcessTest scale_convert_process_test_027.");
    int32_t rc = testScaleConvertProcess_->InitNode(SRC_PARAMS1, DEST_PARAMS2, PROC_CONFIG);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 3200000;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    db->SetInt32("Videoformat", static_cast<int32_t>(Videoformat::YUVI420));
    db->SetInt32("width", TEST_WIDTH);
    db->SetInt32("height", TEST_HEIGTH);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testScaleConvertProcess_->callbackPipelineSource_ = sourcePipeline;
    rc = testScaleConvertProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}
#endif
} // namespace DistributedHardware
} // namespace OHOS
