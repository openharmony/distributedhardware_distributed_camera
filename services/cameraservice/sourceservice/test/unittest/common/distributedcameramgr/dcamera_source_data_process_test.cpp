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

#include "dcamera_source_data_process.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    std::shared_ptr<ICameraSourceDataProcess> testSrcDataProcess_;
};

namespace {
const std::string TEST_DEVICE_ID = "bb536a637105409e904d4da83790a4a7";
const std::string TEST_CAMERA_DH_ID_0 = "camera_0";
const int32_t TEST_WIDTH = 1920;
const int32_t TEST_HEIGTH = 1080;
const int32_t TEST_FORMAT = 4;
const int32_t TEST_DATASPACE = 8;
const int32_t TEST_STREAMID = 2;
std::vector<std::shared_ptr<DCStreamInfo>> g_streamInfos;
std::vector<int32_t> g_streamIds;
}

void DCameraSourceDataProcessTest::SetUpTestCase(void)
{
    std::shared_ptr<DCStreamInfo> streamInfo1 = std::make_shared<DCStreamInfo>();
    streamInfo1->streamId_ = 1;
    streamInfo1->width_ = TEST_WIDTH;
    streamInfo1->height_ = TEST_HEIGTH;
    streamInfo1->stride_ = 1;
    streamInfo1->format_ = TEST_FORMAT;
    streamInfo1->dataspace_ = TEST_DATASPACE;
    streamInfo1->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo1->type_ = SNAPSHOT_FRAME;

    std::shared_ptr<DCStreamInfo> streamInfo2 = std::make_shared<DCStreamInfo>();
    streamInfo2->streamId_ = TEST_STREAMID;
    streamInfo2->width_ = TEST_WIDTH;
    streamInfo2->height_ = TEST_HEIGTH;
    streamInfo2->stride_ = 1;
    streamInfo2->format_ = TEST_FORMAT;
    streamInfo2->dataspace_ = TEST_DATASPACE;
    streamInfo2->encodeType_ = ENCODE_TYPE_JPEG;
    streamInfo2->type_ = SNAPSHOT_FRAME;
    g_streamInfos.push_back(streamInfo1);
    g_streamInfos.push_back(streamInfo2);
    g_streamIds.push_back(1);
    g_streamIds.push_back(TEST_STREAMID);
}

void DCameraSourceDataProcessTest::TearDownTestCase(void)
{
}

void DCameraSourceDataProcessTest::SetUp(void)
{
    testSrcDataProcess_ = std::make_shared<DCameraSourceDataProcess>(
        TEST_DEVICE_ID, TEST_CAMERA_DH_ID_0, SNAPSHOT_FRAME);
}

void DCameraSourceDataProcessTest::TearDown(void)
{
    testSrcDataProcess_ = nullptr;
}

/**
 * @tc.name: dcamera_source_data_process_test_001
 * @tc.desc: Verify source data process ConfigStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    int32_t rc = testSrcDataProcess_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_002
 * @tc.desc: Verify source data process ReleaseStreams.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    int32_t rc = testSrcDataProcess_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testSrcDataProcess_->ReleaseStreams(g_streamIds);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_003
 * @tc.desc: Verify source data process StartCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    captureInfo->streamIds_.push_back(1);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    int32_t rc = testSrcDataProcess_->StartCapture(captureInfo);
    EXPECT_EQ(rc, DCAMERA_OK);
    captureInfo->isCapture_ = true;
    rc = testSrcDataProcess_->StartCapture(captureInfo);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_004
 * @tc.desc: Verify source data process StopCapture.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    captureInfo->streamIds_.push_back(1);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    int32_t rc = testSrcDataProcess_->StartCapture(captureInfo);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<int32_t> streamIds;
    streamIds.push_back(1);
    rc = testSrcDataProcess_->StopCapture(streamIds);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_005
 * @tc.desc: Verify source data process FeedStream.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> buffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    buffers.push_back(db);
    int32_t rc = testSrcDataProcess_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testSrcDataProcess_->FeedStream(buffers);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testSrcDataProcess_->ReleaseStreams(g_streamIds);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_006
 * @tc.desc: Verify source data process GetProducerSize.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    int32_t rc = testSrcDataProcess_->ConfigStreams(g_streamInfos);
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testSrcDataProcess_->GetProducerSize();
    EXPECT_EQ(rc, DCAMERA_OK);

    rc = testSrcDataProcess_->ReleaseStreams(g_streamIds);
    EXPECT_EQ(rc, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_source_data_process_test_007
 * @tc.desc: Verify UpdateProducerWorkMode.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSourceDataProcessTest, dcamera_source_data_process_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testSrcDataProcess_ == nullptr);

    std::shared_ptr<DCCaptureInfo> captureInfo = std::make_shared<DCCaptureInfo>();
    captureInfo->streamIds_.push_back(1);
    captureInfo->width_ = TEST_WIDTH;
    captureInfo->height_ = TEST_HEIGTH;
    captureInfo->stride_ = 1;
    captureInfo->format_ = 1;
    captureInfo->dataspace_ = 1;
    captureInfo->encodeType_ = ENCODE_TYPE_H265;
    captureInfo->type_ = CONTINUOUS_FRAME;
    int32_t rc = testSrcDataProcess_->StartCapture(captureInfo);
    EXPECT_EQ(rc, DCAMERA_OK);

    std::vector<int32_t> streamIds;
    streamIds.push_back(1);
    WorkModeParam param(12, 120, 0, false);
    rc = testSrcDataProcess_->UpdateProducerWorkMode(streamIds, param);
    EXPECT_EQ(rc, DCAMERA_OK);
}
} // namespace DistributedHardware
} // namespace OHOS
