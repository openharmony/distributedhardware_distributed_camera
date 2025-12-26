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
    std::shared_ptr<AppExecFwk::EventHandler> pipeEventHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    testDecodeDataProcess_ = std::make_shared<DecodeDataProcess>(pipeEventHandler, sourcePipeline_);
}

void DecodeDataProcessTest::TearDown(void)
{
    DHLOGI("DecodeDataProcessTest TearDown");
    usleep(SLEEP_TIME);
    sourcePipeline_ = nullptr;
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
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
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

    rc = testDecodeDataProcess_->InitDecoderMetadataFormat();
    EXPECT_EQ(rc, DCAMERA_OK);
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
    testDecodeDataProcess_->isDecoderProcess_.store(true);
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
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_017
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_017, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_017");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    VideoConfigParams srcParams2(VideoCodecType::CODEC_H264,
        Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT, 4000, 4000);
    VideoConfigParams destParams2(VideoCodecType::NO_CODEC,
        Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT, 4000, 4000);
    VideoConfigParams procConfig2;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams2, destParams2, procConfig2);
    testDecodeDataProcess_->OnError();
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: decode_data_process_test_018
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_018, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_018");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    VideoConfigParams srcParams3(VideoCodecType::CODEC_H265,
        Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams destParams3(VideoCodecType::NO_CODEC,
        Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams procConfig3;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams3, destParams3, procConfig3);
    testDecodeDataProcess_->OnError();
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    DHLOGI("DecodeDataProcessTest 14");
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_019
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_019, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_019");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    VideoConfigParams srcParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams destParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams procConfig4;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams4, destParams4, procConfig4);
    testDecodeDataProcess_->OnError();
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_020
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_020, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_020");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    VideoConfigParams srcParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams destParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    destParams4.SetSystemSwitchFlagAndRotation(true, 0);
    VideoConfigParams procConfig4;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams4, destParams4, procConfig4);
    testDecodeDataProcess_->OnError();
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_021
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_021, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_021");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    VideoConfigParams srcParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    VideoConfigParams destParams4(VideoCodecType::NO_CODEC,
        Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT, TEST_WIDTH, TEST_HEIGTH);
    destParams4.SetSystemSwitchFlagAndRotation(true, 90);
    VideoConfigParams procConfig4;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams4, destParams4, procConfig4);
    testDecodeDataProcess_->OnError();
    testDecodeDataProcess_->isDecoderProcess_.store(true);
    EXPECT_EQ(rc, DCAMERA_OK);
}

#ifndef DCAMERA_SUPPORT_FFMPEG
/**
 * @tc.name: decode_data_process_test_022
 * @tc.desc: Verify CopyDecodedImage func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_022, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_022");
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
 * @tc.name: decode_data_process_test_023
 * @tc.desc: Verify CopyDecodedImage func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_023, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_023");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    int dstSizeY = TEST_WIDTH * TEST_HEIGTH;
    int dstSizeUV = TEST_WIDTH * TEST_HEIGTH / 4;
    uint8_t* srcY = (uint8_t *)malloc(dstSizeY);
    int srcStrideY = TEST_WIDTH;
    uint8_t* srcUV = (uint8_t *)malloc(dstSizeUV);
    int srcStrideUV = TEST_WIDTH / 2;
    int srcWidth = TEST_WIDTH;
    int srcHeight = TEST_HEIGTH;
    uint8_t* dstY = static_cast<uint8_t*>(aligned_alloc(16, dstSizeY));
    int dstStrideY = TEST_WIDTH;
    uint8_t* dstU = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));
    int dstStrideU = TEST_WIDTH;
    uint8_t* dstV = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));
    int dstStrideV = TEST_WIDTH;
    int angleDegrees = 0;
    ImageDataInfo srcInfo = { .width = srcWidth, .height = srcHeight,
        .dataY = srcY, .strideY = srcStrideY, .dataU = srcUV, .strideU = srcStrideUV };
    ImageDataInfo dstInfo = { .dataY = dstY, .strideY = dstStrideY, .dataU = dstU, .strideU = dstStrideU,
        .dataV = dstV, .strideV = dstStrideV };
    srcInfo.dataY = nullptr;
    bool ret = testDecodeDataProcess_->UniversalRotateCropAndPadNv12ToI420(srcInfo, dstInfo, angleDegrees);
    EXPECT_EQ(ret, false);
    srcInfo.dataY = srcY;
    srcInfo.width = 0;
    ret = testDecodeDataProcess_->UniversalRotateCropAndPadNv12ToI420(srcInfo, dstInfo, angleDegrees);
    EXPECT_EQ(ret, false);
    srcInfo.width = TEST_WIDTH;
    ret = testDecodeDataProcess_->UniversalRotateCropAndPadNv12ToI420(srcInfo, dstInfo, angleDegrees);
    EXPECT_EQ(ret, true);
    angleDegrees = 90;
    ret = testDecodeDataProcess_->UniversalRotateCropAndPadNv12ToI420(srcInfo, dstInfo, angleDegrees);
    EXPECT_EQ(ret, true);
    free(srcY);
    free(srcUV);
    free(dstY);
    free(dstU);
    free(dstV);
}

/**
 * @tc.name: decode_data_process_test_024
 * @tc.desc: Verify I420CopyBySystemSwitch func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_024, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_024");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    int dstSizeY = TEST_WIDTH * TEST_HEIGTH;
    int dstSizeUV = TEST_WIDTH * TEST_HEIGTH / 4;
    uint8_t* rotatedY = (uint8_t *)malloc(dstSizeY);
    int rotatedStrideY = TEST_WIDTH;
    uint8_t* rotatedU = (uint8_t *)malloc(dstSizeUV);
    int rotatedStrideU = TEST_WIDTH / 2;
    uint8_t* rotatedV = (uint8_t *)malloc(dstSizeUV);
    int rotatedStrideV = TEST_WIDTH / 2;

    uint8_t* dstY = static_cast<uint8_t*>(aligned_alloc(16, dstSizeY));
    int dstStrideY = TEST_WIDTH;
    uint8_t* dstU = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));
    int dstStrideU = TEST_WIDTH / 2;
    uint8_t* dstV = static_cast<uint8_t*>(aligned_alloc(16, dstSizeUV));
    int dstStrideV = TEST_WIDTH / 2;

    int srcWidth = TEST_WIDTH;
    int srcHeight = TEST_HEIGTH;
    int rotatedWidth = TEST_WIDTH;
    int rotatedHeight = TEST_HEIGTH;
    int normalizedAngle = 90;
    ImageDataInfo rotateInfo = { .width = rotatedWidth, .height = rotatedHeight, .dataY = rotatedY,
        .strideY = rotatedStrideY, .dataU = rotatedU, .strideU = rotatedStrideU, .dataV = rotatedV,
        .strideV = rotatedStrideV };
    ImageDataInfo dstInfo = { .width = TEST_WIDTH, .height = TEST_HEIGTH, .dataY = dstY,
        .strideY = dstStrideY, .dataU = dstU, .strideU = dstStrideU, .dataV = dstV,
        .strideV = dstStrideV };
    bool ret = testDecodeDataProcess_->I420CopyBySystemSwitch(rotateInfo, dstInfo, 0, srcHeight, normalizedAngle);
    EXPECT_EQ(ret, false);
    ret = testDecodeDataProcess_->I420CopyBySystemSwitch(rotateInfo, dstInfo, srcWidth, srcHeight, normalizedAngle);
    EXPECT_EQ(ret, true);
    normalizedAngle = 0;
    ret = testDecodeDataProcess_->I420CopyBySystemSwitch(rotateInfo, dstInfo, srcWidth, srcHeight, normalizedAngle);
    EXPECT_EQ(ret, true);
    free(rotatedY);
    free(rotatedU);
    free(rotatedV);
    free(dstY);
    free(dstU);
    free(dstV);
}

/**
 * @tc.name: decode_data_process_test_025
 * @tc.desc: Verify InitNode func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_025, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_025");
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
    destParams.SetSystemSwitchFlagAndRotation(true, 90);
    VideoConfigParams procConfig;
    int32_t rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
    testDecodeDataProcess_->ReleaseProcessNode();

    destParams.SetSystemSwitchFlagAndRotation(true, 0);
    rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
    testDecodeDataProcess_->ReleaseProcessNode();

    destParams.SetSystemSwitchFlagAndRotation(false, 0);
    rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
    testDecodeDataProcess_->ReleaseProcessNode();
}

/**
 * @tc.name: decode_data_process_test_026
 * @tc.desc: Verify ParseAngle func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_026, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_026");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);

    OpenSourceLibyuv::RotationMode mode = testDecodeDataProcess_->ParseAngle(0);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate0);

    mode = testDecodeDataProcess_->ParseAngle(90);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate90);
    
    mode = testDecodeDataProcess_->ParseAngle(180);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate180);

    mode = testDecodeDataProcess_->ParseAngle(270);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate270);

    mode = testDecodeDataProcess_->ParseAngle(360);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate0);

    mode = testDecodeDataProcess_->ParseAngle(1);
    EXPECT_EQ(mode, OpenSourceLibyuv::RotationMode::kRotate0);
}

/**
 * @tc.name: decode_data_process_test_027
 * @tc.desc: Verify CheckParameters func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_027, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_027");
    EXPECT_EQ(false, testDecodeDataProcess_ == nullptr);
    uint8_t* bytes = (uint8_t *)malloc(1);
    ImageDataInfo srcInfo = {0};
    ImageDataInfo dstInfo = {0};

    srcInfo.dataY = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    srcInfo.dataY = bytes;
    srcInfo.dataU = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    srcInfo.dataU = bytes;
    srcInfo.dataV = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    srcInfo.dataV = bytes;
    dstInfo.dataY = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    dstInfo.dataY = bytes;
    dstInfo.dataU = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    dstInfo.dataU = bytes;
    dstInfo.dataV = nullptr;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    dstInfo.dataV = bytes;
    srcInfo.width = -1;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    srcInfo.width = 1920;
    srcInfo.height = -1;
    EXPECT_FALSE(testDecodeDataProcess_->CheckParameters(srcInfo, dstInfo));

    free(bytes);
}

/**
 * @tc.name: decode_data_process_test_028
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_028, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_028");
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

    destParams.SetSystemSwitchFlagAndRotation(true, 90);
    testDecodeDataProcess_->CopyDecodedImage(surBuf, alignedWidth, alignedHeight);
    EXPECT_EQ(rc, DCAMERA_OK);

    alignedWidth = -1;
    testDecodeDataProcess_->CopyDecodedImage(surBuf, alignedWidth, alignedHeight);
    EXPECT_EQ(rc, DCAMERA_OK);

    destParams.SetSystemSwitchFlagAndRotation(false, 90);
    testDecodeDataProcess_->CopyDecodedImage(surBuf, alignedWidth, alignedHeight);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: decode_data_process_test_029
 * @tc.desc: Verify decode data process func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_029, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_029");
    uint8_t* dataY = nullptr;
    uint8_t* dataU = (uint8_t *)malloc(1);
    uint8_t* dataV = (uint8_t *)malloc(1);
    bool rc = testDecodeDataProcess_->FreeYUVBuffer(dataY, dataU, dataU);
    EXPECT_EQ(rc, true);

    dataY = (uint8_t *)malloc(1);
    dataU = nullptr;
    dataV = (uint8_t *)malloc(1);
    rc = testDecodeDataProcess_->FreeYUVBuffer(dataY, dataU, dataU);
    EXPECT_EQ(rc, true);
    dataY = (uint8_t *)malloc(1);
    dataU = (uint8_t *)malloc(1);;
    dataV = nullptr;
    rc = testDecodeDataProcess_->FreeYUVBuffer(dataY, dataU, dataV);
    EXPECT_EQ(rc, true);
}

/**
 * @tc.name: decode_data_process_test_030
 * @tc.desc: Verify decode UpdateSettings func.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_030, TestSize.Level1)
{
    DHLOGI("DecodeDataProcessTest decode_data_process_test_030");
    int32_t rc = testDecodeDataProcess_->UpdateSettings(nullptr);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);

    auto metaData = std::make_shared<OHOS::Camera::CameraMetadata>(100, 200);

    rc = testDecodeDataProcess_->UpdateSettings(metaData);
    EXPECT_EQ(rc, DCAMERA_OK);

    VideoConfigParams srcParams(VideoCodecType::CODEC_H264, Videoformat::NV12, DCAMERA_PRODUCER_FPS_DEFAULT,
        TEST_WIDTH2, TEST_HEIGTH2);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC, Videoformat::NV21, DCAMERA_PRODUCER_FPS_DEFAULT,
        TEST_WIDTH2, TEST_HEIGTH2);
    destParams.SetSystemSwitchFlagAndRotation(true, 90);
    VideoConfigParams procConfig;
    rc = testDecodeDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testDecodeDataProcess_->UpdateSettings(metaData);
    EXPECT_EQ(rc, DCAMERA_OK);

    int32_t rotate[2] = {90, 0};
    metaData->addEntry(OHOS_CONTROL_CAMERA_SWITCH_INFOS, rotate, 2);
    rc = testDecodeDataProcess_->UpdateSettings(metaData);
    EXPECT_EQ(rc, DCAMERA_OK);
}
#endif
} // namespace DistributedHardware
} // namespace OHOS
