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
#include "encode_data_process.h"
#undef private
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class EncodeDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<EncodeDataProcess> testEncodeDataProcess_;
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGHT = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGHT2 = 480;
}

void EncodeDataProcessTest::SetUpTestCase(void)
{
}

void EncodeDataProcessTest::TearDownTestCase(void)
{
}

void EncodeDataProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSink> sinkPipeline = std::make_shared<DCameraPipelineSink>();
    testEncodeDataProcess_ = std::make_shared<EncodeDataProcess>(sinkPipeline);
}

void EncodeDataProcessTest::TearDown(void)
{
    testEncodeDataProcess_ = nullptr;
}

/**
 * @tc.name: encode_data_process_test_001
 * @tc.desc: Verify encode data process InitNode IsInEncoderRange false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    int32_t frameRate = 50;
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::YUVI420,
                                frameRate,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 frameRate,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_002
 * @tc.desc: Verify encode data process InitNode IsConvertible false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H265,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGHT2);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: encode_data_process_test_003
 * @tc.desc: Verify encode data process InitNode normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGHT2);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_004
 * @tc.desc: Verify encode data process InitEncoder abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_MPEG4_ES,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGHT2);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_INIT_ERR);

    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_005
 * @tc.desc: Verify encode data process ProcessData inputBuffers is nullptr.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    int32_t rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testEncodeDataProcess_->StartVideoEncoder();
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testEncodeDataProcess_->StopVideoEncoder();
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    size_t capacity = 100;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    testEncodeDataProcess_->sourceConfig_.videoCodec_ = VideoCodecType::NO_CODEC;
    testEncodeDataProcess_->processedConfig_.videoCodec_ = VideoCodecType::CODEC_H264;
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_INIT_ERR);
}

/**
 * @tc.name: encode_data_process_test_006
 * @tc.desc: Verify encode data process ProcessData codetype equality.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_007
 * @tc.desc: Verify encode data process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H265,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_008
 * @tc.desc: Verify encode data process InitEncoder abnormal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::RGBA_8888,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGHT2);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_009
 * @tc.desc: Verify encode data process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_009, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    constexpr int64_t NORM_YUV420_BUFFER_SIZE = 1920 * 1080 * 3 / 2;
    size_t capacity = NORM_YUV420_BUFFER_SIZE + 5;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    size_t offset = 1;
    size_t size = NORM_YUV420_BUFFER_SIZE + 1;
    db->SetRange(offset, size);
    inputBuffers.push_back(db);
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_MEMORY_OPT_ERROR);

    inputBuffers.clear();
    std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(capacity);
    size_t len = NORM_YUV420_BUFFER_SIZE - 1;
    db->SetRange(offset, len);
    inputBuffers.push_back(db);
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_010
 * @tc.desc: Verify encode data process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_010, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::YUVI420,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_011
 * @tc.desc: Verify encode data process ProcessData.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_011, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::YUVI420,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);

    uint32_t index = 0;
    MediaAVCodec::AVCodecBufferInfo info;
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    MediaAVCodec::AVCodecBufferFlag flag = MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    rc = testEncodeDataProcess_->GetEncoderOutputBuffer(index, info, flag, buffer);
    EXPECT_EQ(rc, DCAMERA_BAD_OPERATE);
}

/**
 * @tc.name: encode_data_process_test_012
 * @tc.desc: Verify encode data process GetEncoderOutputBuffer.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_012, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    testEncodeDataProcess_->ReduceWaitEncodeCnt();
    testEncodeDataProcess_->inputTimeStampUs_ = 1;
    int64_t time = testEncodeDataProcess_->GetEncoderTimeStamp();
    testEncodeDataProcess_->IncreaseWaitEncodeCnt();
    testEncodeDataProcess_->ReduceWaitEncodeCnt();
    uint32_t index = time;
    MediaAVCodec::AVCodecBufferInfo info;
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    MediaAVCodec::AVCodecBufferFlag flag = MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    int32_t rc = testEncodeDataProcess_->GetEncoderOutputBuffer(index, info, flag, buffer);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_013
 * @tc.desc: Verify encode data process EncodeDone.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_013, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    int32_t rc = testEncodeDataProcess_->EncodeDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_014
 * @tc.desc: Verify encode data process ProcessData codetype equality.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_014, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    std::shared_ptr<DCameraPipelineSink> callbackPipelineSink = std::make_shared<DCameraPipelineSink>();
    testEncodeDataProcess_->callbackPipelineSink_ = callbackPipelineSink;
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    uint32_t index = 0;
    MediaAVCodec::AVCodecBufferInfo info;
    MediaAVCodec::AVCodecBufferFlag flag = MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    testEncodeDataProcess_->OnOutputBufferAvailable(index, info, flag, buffer);
    testEncodeDataProcess_->OnError();
    testEncodeDataProcess_->OnInputBufferAvailable(index, buffer);
    Media::Format format;
    testEncodeDataProcess_->OnOutputFormatChanged(format);
    testEncodeDataProcess_->OnOutputBufferAvailable(index, info, flag, buffer);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_015
 * @tc.desc: Verify encode data process GetProperty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_015, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    std::string propertyName = "propertyName";
    PropertyCarrier propertyCarrier;
    int32_t rc = testEncodeDataProcess_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_016
 * @tc.desc: Verify encode data process GetProperty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_016, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::NO_CODEC,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGHT);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGHT);
    VideoConfigParams procConfig;
    int32_t rc = testEncodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::string propertyName = "surface";
    PropertyCarrier propertyCarrier;
    rc = testEncodeDataProcess_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: encode_data_process_test_017
 * @tc.desc: Verify encode data process GetProperty.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_017, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    std::string propertyName = "surface";
    PropertyCarrier propertyCarrier;
    int32_t rc = testEncodeDataProcess_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_018
 * @tc.desc: Verify encode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_018, TestSize.Level1)
{
    EXPECT_EQ(false, testEncodeDataProcess_ == nullptr);

    testEncodeDataProcess_->targetConfig_.SetVideoCodecType(VideoCodecType::NO_CODEC);
    int32_t rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    testEncodeDataProcess_->targetConfig_.SetVideoCodecType(VideoCodecType::CODEC_MPEG4_ES);
    testEncodeDataProcess_->sourceConfig_.SetVideoformat(Videoformat::NV21);
    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);

    testEncodeDataProcess_->sourceConfig_.SetVideoformat(static_cast<Videoformat>(-1));
    rc = testEncodeDataProcess_->InitEncoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = nullptr;
    inputBuffers.push_back(db);
    rc = testEncodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: encode_data_process_test_019
 * @tc.desc: Verify encode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(EncodeDataProcessTest, encode_data_process_test_019, TestSize.Level1)
{
    ASSERT_NE(testEncodeDataProcess_, nullptr);
    testEncodeDataProcess_->currentBitrate_ = 3400000;
    testEncodeDataProcess_->maxBitrate_ = 3400000;
    testEncodeDataProcess_->AdjustBitrateBasedOnNetworkConditions(true);
    EXPECT_EQ(testEncodeDataProcess_->currentBitrate_, testEncodeDataProcess_->maxBitrate_);
    testEncodeDataProcess_->currentBitrate_ = 1500000;
    testEncodeDataProcess_->minBitrate_ = 1500000;
    testEncodeDataProcess_->AdjustBitrateBasedOnNetworkConditions(false);
    EXPECT_EQ(testEncodeDataProcess_->currentBitrate_, testEncodeDataProcess_->minBitrate_);
    testEncodeDataProcess_->currentBitrate_ = 1800000;
    testEncodeDataProcess_->AdjustBitrateBasedOnNetworkConditions(true);
    EXPECT_EQ(testEncodeDataProcess_->currentBitrate_, 1800000);
}
} // namespace DistributedHardware
} // namespace OHOS
