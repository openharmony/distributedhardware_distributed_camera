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

#include "decode_data_process.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

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
};

namespace {
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_WIDTH2 = 640;
const int32_t TEST_HEIGTH2 = 480;
}

void DecodeDataProcessTest::SetUpTestCase(void)
{
}

void DecodeDataProcessTest::TearDownTestCase(void)
{
}

void DecodeDataProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::shared_ptr<EventBus> eventBusPipeline = std::make_shared<EventBus>("TestDecodeHandler");
    testDecodeDataProcess_ = std::make_shared<DecodeDataProcess>(eventBusPipeline, sourcePipeline);
}

void DecodeDataProcessTest::TearDown(void)
{
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
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}

/**
 * @tc.name: decode_data_process_test_007
 * @tc.desc: Verify decode data process ProcessData normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeDataProcessTest, decode_data_process_test_007, TestSize.Level1)
{
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
} // namespace DistributedHardware
} // namespace OHOS
