/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include <chrono>
#include <fstream>
#include <memory>
#include <thread>

#define private public
#include "dcamera_sink_data_process.h"
#undef private

#include <gtest/gtest.h>
#include <securec.h>

#include "dcamera_handler.h"
#include "distributed_camera_errno.h"
#include "mock_camera_channel.h"
#include "mock_data_process_pipeline.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
#ifdef DCAMERA_SUPPORT_RESERVE
class DCameraSinkDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<DCameraSinkDataProcess> dataProcess_;
    std::shared_ptr<ICameraChannel> channel_;
};

const int32_t SLEEP_TIME_MS = 500;
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGHT = 1080;
const std::string TEST_STRING = "test_string";
const int32_t TEST_TWENTY_MS = 20000;

std::shared_ptr<DCameraCaptureInfo> g_testCaptureInfoContinuousNotEncode;
std::shared_ptr<DCameraCaptureInfo> g_testCaptureInfoContinuousNeedEncode;
std::shared_ptr<DCameraCaptureInfo> g_testCaptureInfoSnapshot;

std::shared_ptr<DataBuffer> g_testDataBuffer;

void DCameraSinkDataProcessTest::SetUpTestCase(void)
{
    std::shared_ptr<DCameraSettings> cameraSetting = std::make_shared<DCameraSettings>();
    cameraSetting->type_ = UPDATE_METADATA;
    cameraSetting->value_ = "";

    g_testCaptureInfoContinuousNotEncode = std::make_shared<DCameraCaptureInfo>();
    g_testCaptureInfoContinuousNotEncode->width_ = TEST_WIDTH;
    g_testCaptureInfoContinuousNotEncode->height_ = TEST_HEIGHT;
    g_testCaptureInfoContinuousNotEncode->format_  = ENCODE_TYPE_H264;
    g_testCaptureInfoContinuousNotEncode->dataspace_ = 0;
    g_testCaptureInfoContinuousNotEncode->encodeType_ = ENCODE_TYPE_H264;
    g_testCaptureInfoContinuousNotEncode->streamType_ = CONTINUOUS_FRAME;
    g_testCaptureInfoContinuousNotEncode->captureSettings_.push_back(cameraSetting);

    g_testCaptureInfoContinuousNeedEncode = std::make_shared<DCameraCaptureInfo>();
    g_testCaptureInfoContinuousNeedEncode->width_ = TEST_WIDTH;
    g_testCaptureInfoContinuousNeedEncode->height_ = TEST_HEIGHT;
    g_testCaptureInfoContinuousNeedEncode->format_  = ENCODE_TYPE_H264;
    g_testCaptureInfoContinuousNeedEncode->dataspace_ = 0;
    g_testCaptureInfoContinuousNeedEncode->encodeType_ = ENCODE_TYPE_H265;
    g_testCaptureInfoContinuousNeedEncode->streamType_ = CONTINUOUS_FRAME;
    g_testCaptureInfoContinuousNeedEncode->captureSettings_.push_back(cameraSetting);

    g_testCaptureInfoSnapshot = std::make_shared<DCameraCaptureInfo>();
    g_testCaptureInfoSnapshot->width_ = TEST_WIDTH;
    g_testCaptureInfoSnapshot->height_ = TEST_HEIGHT;
    g_testCaptureInfoSnapshot->format_  = ENCODE_TYPE_JPEG;
    g_testCaptureInfoSnapshot->dataspace_ = 0;
    g_testCaptureInfoSnapshot->encodeType_ = ENCODE_TYPE_JPEG;
    g_testCaptureInfoSnapshot->streamType_ = SNAPSHOT_FRAME;
    g_testCaptureInfoSnapshot->captureSettings_.push_back(cameraSetting);

    g_testDataBuffer = std::make_shared<DataBuffer>(TEST_STRING.length() + 1);
    memcpy_s(g_testDataBuffer->Data(), g_testDataBuffer->Capacity(),
        (uint8_t *)TEST_STRING.c_str(), TEST_STRING.length());
}

void DCameraSinkDataProcessTest::TearDownTestCase(void)
{
}

void DCameraSinkDataProcessTest::SetUp(void)
{
    channel_ = std::make_shared<MockCameraChannel>();
    DCameraHandler::GetInstance().Initialize();
    std::vector<std::string> cameras = DCameraHandler::GetInstance().GetCameras();
    dataProcess_ = std::make_shared<DCameraSinkDataProcess>(cameras[0], channel_);
    dataProcess_->Init();

    dataProcess_->pipeline_ = std::make_shared<MockDataProcessPipeline>();
    dataProcess_->captureInfo_ = g_testCaptureInfoContinuousNeedEncode;
}

void DCameraSinkDataProcessTest::TearDown(void)
{
    usleep(TEST_TWENTY_MS);
    channel_ = nullptr;
    dataProcess_ = nullptr;
}

/**
 * @tc.name: dcamera_sink_data_process_test_001
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_001, TestSize.Level1)
{
    dataProcess_->pipeline_ = nullptr;
    int32_t ret = dataProcess_->StartCapture(g_testCaptureInfoContinuousNotEncode);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_002
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_002, TestSize.Level1)
{
    dataProcess_->pipeline_ = nullptr;
    int32_t ret = dataProcess_->StartCapture(g_testCaptureInfoContinuousNeedEncode);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_003
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_003, TestSize.Level1)
{
    dataProcess_->pipeline_ = nullptr;
    int32_t ret = dataProcess_->StartCapture(g_testCaptureInfoSnapshot);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_004
 * @tc.desc: Verify the StopCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6N1
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_004, TestSize.Level1)
{
    int32_t ret = dataProcess_->StopCapture();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_005
 * @tc.desc: Verify the FeedStream function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_005, TestSize.Level1)
{
    dataProcess_->captureInfo_ = g_testCaptureInfoContinuousNotEncode;
    int32_t ret = dataProcess_->FeedStream(g_testDataBuffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_006
 * @tc.desc: Verify the FeedStream function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_006, TestSize.Level1)
{
    dataProcess_->captureInfo_ = g_testCaptureInfoContinuousNeedEncode;
    int32_t ret = dataProcess_->FeedStream(g_testDataBuffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_007
 * @tc.desc: Verify the FeedStream function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_007, TestSize.Level1)
{
    dataProcess_->captureInfo_ = g_testCaptureInfoSnapshot;
    int32_t ret = dataProcess_->FeedStream(g_testDataBuffer);
    std::this_thread::sleep_for(std::chrono::milliseconds(SLEEP_TIME_MS));
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_008
 * @tc.desc: Verify the GetPipelineCodecType GetPipelineFormat function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MV
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_008, TestSize.Level1)
{
    EXPECT_EQ(VideoCodecType::CODEC_H264, dataProcess_->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_H264));
    EXPECT_EQ(VideoCodecType::CODEC_H265, dataProcess_->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_H265));
    EXPECT_EQ(VideoCodecType::CODEC_MPEG4_ES, dataProcess_->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_MPEG4_ES));
    EXPECT_EQ(VideoCodecType::NO_CODEC, dataProcess_->GetPipelineCodecType(DCEncodeType::ENCODE_TYPE_NULL));
    EXPECT_EQ(Videoformat::RGBA_8888, dataProcess_->GetPipelineFormat(1));
    EXPECT_EQ(Videoformat::NV21, dataProcess_->GetPipelineFormat(0));
}

/**
 * @tc.name: dcamera_sink_data_process_test_009
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_009, TestSize.Level1)
{
    int32_t ret = dataProcess_->StartCapture(g_testCaptureInfoSnapshot);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_sink_data_process_test_010
 * @tc.desc: Verify the StartCapture function.
 * @tc.type: FUNC
 * @tc.require: AR000GK6MU
 */
HWTEST_F(DCameraSinkDataProcessTest, dcamera_sink_data_process_test_010, TestSize.Level1)
{
    std::string propertyName = "test010";
    PropertyCarrier propertyCarrier;
    int32_t ret = dataProcess_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(DCAMERA_OK, ret);
}
#endif
} // namespace DistributedHardware
} // namespace OHOS