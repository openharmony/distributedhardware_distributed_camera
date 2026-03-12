/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <memory>
#include <vector>
 
#include "eis_data_process.h"
 
#include "distributed_camera_errno.h"
#include "dcamera_frame_info.h"
#include "data_buffer.h"
 
using namespace testing::ext;
 
namespace OHOS {
namespace DistributedHardware {
 
class EISDataProcessTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
 
    std::shared_ptr<EISDataProcess> testEISDataProcess_;
};
 
void EISDataProcessTest::SetUpTestCase(void)
{
}
 
void EISDataProcessTest::TearDownTestCase(void)
{
}
 
void EISDataProcessTest::SetUp(void)
{
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    testEISDataProcess_ = std::make_shared<EISDataProcess>(sourcePipeline);
}
 
void EISDataProcessTest::TearDown(void)
{
    testEISDataProcess_ = nullptr;
}
 
/**
 * @tc.name: eis_data_process_test_001
 * @tc.desc: Verify EIS data process InitNode normal.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_001, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC, Videoformat::NV12, 30, 1920, 1080);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC, Videoformat::NV12, 30, 1920, 1080);
    VideoConfigParams procConfig;
    int32_t rc = testEISDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
    EXPECT_EQ(true, testEISDataProcess_->isEISProcess_.load());
}
 
/**
 * @tc.name: eis_data_process_test_002
 * @tc.desc: Verify EIS data process ProcessData with empty buffers.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_002, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    testEISDataProcess_->isEISProcess_ = true;
    int32_t rc = testEISDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}
 
/**
 * @tc.name: eis_data_process_test_003
 * @tc.desc: Verify EIS data process ProcessData when node is disabled.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_003, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    testEISDataProcess_->isEISProcess_.store(false);
      
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
      
    int32_t rc = testEISDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_DISABLE_PROCESS);
}
 
/**
 * @tc.name: eis_data_process_test_004
 * @tc.desc: Verify EIS data process ProcessData normal.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_004, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    VideoConfigParams srcParams(VideoCodecType::NO_CODEC, Videoformat::NV12, 30, 1920, 1080);
    VideoConfigParams destParams(VideoCodecType::NO_CODEC, Videoformat::NV12, 30, 1920, 1080);
    VideoConfigParams procConfig;
    int32_t rc = testEISDataProcess_->InitNode(srcParams, destParams, procConfig);
    EXPECT_EQ(rc, DCAMERA_OK);
 
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> inputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    inputBuffers.push_back(db);
    auto pipelineSource = std::make_shared<DCameraPipelineSource>();
    testEISDataProcess_->callbackPipelineSource_ = pipelineSource;
      
    rc = testEISDataProcess_->ProcessData(inputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}
 
/**
 * @tc.name: eis_data_process_test_005
 * @tc.desc: Verify EIS data process EISDone with empty buffers.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_005, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    int32_t rc = testEISDataProcess_->EISDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}
 
/**
 * @tc.name: eis_data_process_test_006
 * @tc.desc: Verify EIS data process EISDone with null buffer.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_006, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    outputBuffers.push_back(nullptr);
    int32_t rc = testEISDataProcess_->EISDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}
 
/**
 * @tc.name: eis_data_process_test_007
 * @tc.desc: Verify EIS data process EISDone with next node.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_007, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    // Create mock next node
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    auto nextNode = std::make_shared<EISDataProcess>(sourcePipeline);
    nextNode->isEISProcess_ = true;
    testEISDataProcess_->nextDataProcess_ = nextNode;
 
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    outputBuffers.push_back(db);
      
    int32_t rc = testEISDataProcess_->EISDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}
 
/**
 * @tc.name: eis_data_process_test_008
 * @tc.desc: Verify EIS data process EISDone without next node (pipeline callback).
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_008, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    // Create mock pipeline source
    auto pipelineSource = std::make_shared<DCameraPipelineSource>();
    testEISDataProcess_->callbackPipelineSource_ = pipelineSource;
 
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    outputBuffers.push_back(db);
      
    int32_t rc = testEISDataProcess_->EISDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_OK);
}
 
/**
 * @tc.name: eis_data_process_test_009
 * @tc.desc: Verify EIS data process EISDone with null pipeline callback.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_009, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    size_t capacity = 100;
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    std::shared_ptr<DataBuffer> db = std::make_shared<DataBuffer>(capacity);
    outputBuffers.push_back(db);
      
    int32_t rc = testEISDataProcess_->EISDone(outputBuffers);
    EXPECT_EQ(rc, DCAMERA_BAD_VALUE);
}
 
/**
 * @tc.name: eis_data_process_test_010
 * @tc.desc: Verify EIS data process GetProperty.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_010, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    std::string propertyName = "test_property";
    PropertyCarrier propertyCarrier;
    int32_t rc = testEISDataProcess_->GetProperty(propertyName, propertyCarrier);
    EXPECT_EQ(rc, DCAMERA_OK);
}
 
/**
 * @tc.name: eis_data_process_test_011
 * @tc.desc: Verify EIS data process UpdateSettings.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_011, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    std::shared_ptr<Camera::CameraMetadata> settings = nullptr;
    int32_t rc = testEISDataProcess_->UpdateSettings(settings);
    EXPECT_EQ(rc, DCAMERA_OK);
}
 
/**
 * @tc.name: eis_data_process_test_012
 * @tc.desc: Verify EIS data process ReleaseProcessNode.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_012, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    // Set up next node
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    auto nextNode = std::make_shared<EISDataProcess>(sourcePipeline);
    testEISDataProcess_->nextDataProcess_ = nextNode;
    testEISDataProcess_->isEISProcess_.store(true);
 
    testEISDataProcess_->ReleaseProcessNode();
    EXPECT_EQ(false, testEISDataProcess_->isEISProcess_.load());
    EXPECT_EQ(nullptr, testEISDataProcess_->nextDataProcess_);
}
 
/**
 * @tc.name: eis_data_process_test_013
 * @tc.desc: Verify EIS data process destructor behavior.
 * @tc.type: FUNC
 */
HWTEST_F(EISDataProcessTest, eis_data_process_test_013, TestSize.Level1)
{
    EXPECT_EQ(false, testEISDataProcess_ == nullptr);
 
    // Set up node for destruction test
    testEISDataProcess_->isEISProcess_.store(true);
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    auto nextNode = std::make_shared<EISDataProcess>(sourcePipeline);
    testEISDataProcess_->nextDataProcess_ = nextNode;
 
    // Reset to trigger destructor
    testEISDataProcess_.reset();
    EXPECT_EQ(true, testEISDataProcess_ == nullptr);
}
 
} // namespace DistributedHardware
} // namespace OHOS
