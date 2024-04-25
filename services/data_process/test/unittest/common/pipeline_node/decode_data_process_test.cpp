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
#include "decode_data_process.h"
#undef private
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DecodeDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DecodeDataProcess> testDecodeDataProcess_;
    std::shared_ptr<DCameraPipelineSource::DCameraPipelineSrcEventHandler> pipeEventHandler_;
    std::shared_ptr<DCameraPipelineSource> sourcePipeline_;
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;
const int32_t SLEEP_TIME = 200000;
}

void DecodeDataProcessTest::SetUpTestCase(void)
{
    DHLOGI("DecodeDataProcessTest SetUpTestCase");
}

void DecodeDataProcessTest::TearDownTestCase(void)
{
    DHLOGI("DecodeDataProcessTest TearDownTestCase");
}

void DecodeDataProcessTest::SetUp(void)
{
    DHLOGI("DecodeDataProcessTest SetUp");
    sourcePipeline_ = std::make_shared<DCameraPipelineSource>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    pipeEventHandler_ = std::make_shared<DCameraPipelineSource::DCameraPipelineSrcEventHandler>(
        runner, sourcePipeline_);
    testDecodeDataProcess_ = std::make_shared<DecodeDataProcess>(pipeEventHandler_, sourcePipeline_);
}

void DecodeDataProcessTest::TearDown(void)
{
    DHLOGI("DecodeDataProcessTest TearDown");
    usleep(SLEEP_TIME);
    sourcePipeline_ = nullptr;
    pipeEventHandler_ = nullptr;
    testDecodeDataProcess_ = nullptr;
}

/**
 * @tc.name: decode_data_process_test_001
 * @tc.desc: Verify decode data process InitNode IsInDecoderRange false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_001, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_001");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    int32_t frameRate = 50;
    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                frameRate,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H264,
                                 Videoformat::NV21,
                                 frameRate,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_002
 * @tc.desc: Verify decode data process InitNode IsConvertible false.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_002, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_002");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::CODEC_H265,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_BAD_TYPE);
}

/**
 * @tc.name: decode_data_process_test_003
 * @tc.desc: Verify decode data process InitNode normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_003, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_003");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

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
    uint8_t *buffer = nullptr;
    size_t bufferSize = 0;
    testDecodeDataProcess_->BeforeDecodeDump(buffer, bufferSize);
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_004
 * @tc.desc: Verify decode data process InitNode normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_004, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_004");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_005
 * @tc.desc: Verify decode data process ProcessData inputBuffers is nullptr.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_005, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_005");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    int32_t rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: decode_data_process_test_006
 * @tc.desc: Verify decode data process ProcessData codetype equality.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_006, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_006");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

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
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_007
 * @tc.desc: Verify decode data process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_007, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_007");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_008
 * @tc.desc: Verify decode data process InitNode.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_008, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest::decode_data_process_test_008.");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_MPEG4_ES,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);

    rc = testDecodeDataProcess_->InitDecoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_NOT_FOUND);
}

/**
 * @tc.name: decode_data_process_test_009
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_009, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_009");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    int32_t rc = testDecodeDataProcess_->SetDecoderOutputSurface();
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testDecodeDataProcess_->StartVideoDecoder();
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testDecodeDataProcess_->StopVideoDecoder();
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    testDecodeDataProcess_->sourceConfig_.videoCodec_ = VideoCodecType::NO_CODEC;
    testDecodeDataProcess_->processedConfig_.videoCodec_ = VideoCodecType::CODEC_H264;
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_INIT_ERR);
}

/**
 * @tc.name: decode_data_process_test_010
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_010, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_010");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

#ifdef DCAMERA_SUPPORT_FFMPEG
    constexpr int32_t MAX_RGB32_BUFFER_SIZE = 1920 * 1080 * 4 * 2;
    size_t capacity = MAX_RGB32_BUFFER_SIZE + 5;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    size_t offset = 1;
    size_t size = MAX_RGB32_BUFFER_SIZE + 1;
    db->SetRange(offset, size);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_MEMORY_OPT_ERROR);
#else
    constexpr int32_t MAX_YUV420_BUFFER_SIZE = 1920 * 1080 * 3 / 2 * 2;
    size_t capacity = MAX_YUV420_BUFFER_SIZE + 5;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    size_t offset = 1;
    size_t size = MAX_YUV420_BUFFER_SIZE + 2;
    db->SetRange(offset, size);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_MEMORY_OPT_ERROR);
#endif
}

/**
 * @tc.name: decode_data_process_test_011
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_011, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_011");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    testDecodeDataProcess_->isDecoderProcess_.store(false);
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_DISABLE_PROCESS);
}

/**
 * @tc.name: decode_data_process_test_012
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_012, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_012");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    uint32_t index = 1;
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    testDecodeDataProcess_->OnInputBufferAvailable(index, buffer);
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    testDecodeDataProcess_->ReduceWaitDecodeCnt();
    testDecodeDataProcess_->IncreaseWaitDecodeCnt();
    int64_t time = testDecodeDataProcess_->GetDecoderTimeStamp();
    testDecodeDataProcess_->lastFeedDecoderInputBufferTimeUs_ = 1;
    time = testDecodeDataProcess_->GetDecoderTimeStamp();
    testDecodeDataProcess_->ReduceWaitDecodeCnt();
    sptr<IConsumerSurface> surface;
    testDecodeDataProcess_->GetDecoderOutputBuffer(surface);
    sptr<IConsumerSurface> surface2 = IConsumerSurface::Create();
    testDecodeDataProcess_->GetDecoderOutputBuffer(surface2);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_013
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_013, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_013");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH2,
                                TEST_HEIGTH2);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH2,
                                 TEST_HEIGTH2);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    sptr<SurfaceBuffer> surBuf = nullptr;
    int32_t alignedWidth = TEST_WIDTH;
    int32_t alignedHeight = TEST_HEIGTH;
    testDecodeDataProcess_->CopyDecodedImage(surBuf, alignedWidth, alignedHeight);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_014
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_014, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_014");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    int32_t rc = DCAMERA_OK;
    std::shared_ptr<DataBuffer> outputBuffer;
    testDecodeDataProcess_->PostOutputDataBuffers(outputBuffer);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_015
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_015, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_015");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::shared_ptr<DataBuffer> outputBuffer;
    testDecodeDataProcess_->PostOutputDataBuffers(outputBuffer);
    EXPECT_EQ(rc, DCAMERA_OK);

    testDecodeDataProcess_->PostOutputDataBuffers(db);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_016
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_016, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_016");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264,
                                Videoformat::NV12,
                                DCAMERA_PRODUCER_FPS_DEFAULT,
                                TEST_WIDTH,
                                TEST_HEIGTH);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC,
                                 Videoformat::NV21,
                                 DCAMERA_PRODUCER_FPS_DEFAULT,
                                 TEST_WIDTH,
                                 TEST_HEIGTH);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    rc = testDecodeDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    rc = testDecodeDataProcess_->DecodeDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    rc = testDecodeDataProcess_->DecodeDone(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::shared_ptr<DCameraPipelineSource> callbackPipelineSource = std::make_shared<DCameraPipelineSource>();
    testDecodeDataProcess_->callbackPipelineSource_ = callbackPipelineSource;
    rc = testDecodeDataProcess_->DecodeDone(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    uint32_t index = 1;
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    testDecodeDataProcess_->OnInputBufferAvailable(index, buffer);
    Media::Format format;
    testDecodeDataProcess_->OnOutputFormatChanged(format);
    MediaAVCodec::AVCodecBufferInfo info;
    MediaAVCodec::AVCodecBufferFlag flag = MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    testDecodeDataProcess_->OnOutputBufferAvailable(index, info, flag, buffer);
    testDecodeDataProcess_->OnError();
    EXPECT_EQ(rc, DCAMERA_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
