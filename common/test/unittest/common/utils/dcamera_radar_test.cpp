/*
 * Copyright (c) 2024 Huawei Device Co., Ltd.
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

#include "dcamera_radar.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "gtest/gtest.h"

using namespace testing;
using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

class DcameraRadarTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraRadarTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DcameraRadarTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraRadarTest::SetUp(void)
{
    DHLOGI("enter");
}

void DcameraRadarTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: ReportDcameraInit_001
 * @tc.desc: check ReportDcameraInit
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraInit_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraInit_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraInit(FUNC, CameraInit::SERVICE_INIT,
        BizState::BIZ_STATE_START, DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraInit_001 end");
}

/**
 * @tc.name: ReportDcameraInit_002
 * @tc.desc: check ReportDcameraInit
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraInit_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraInit_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraInit(FUNC, CameraInit::SERVICE_INIT,
        BizState::BIZ_STATE_START, DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraInit_002 end");
}

/**
 * @tc.name: ReportDcameraInitProgress_001
 * @tc.desc: check ReportDcameraInitProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraInitProgress_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraInitProgress_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraInitProgress(FUNC, CameraInit::SERVICE_INIT,
        DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraInitProgress_001 end");
}

/**
 * @tc.name: ReportDcameraInitProgress_002
 * @tc.desc: check ReportDcameraInitProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraInitProgress_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraInitProgress_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraInitProgress(FUNC, CameraInit::SERVICE_INIT,
        DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraInitProgress_002 end");
}

/**
 * @tc.name: ReportDcameraOpen_001
 * @tc.desc: check ReportDcameraOpen
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraOpen_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraOpen_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraOpen(FUNC, CameraOpen::OPEN_CAMERA,
        BizState::BIZ_STATE_START, DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraOpen_001 end");
}
/**
 * @tc.name: ReportDcameraOpen_002
 * @tc.desc: check ReportDcameraOpen
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraOpen_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraOpen_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraOpen(FUNC, CameraOpen::INIT_DECODE,
        BizState::BIZ_STATE_START, DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraOpen_002 end");
}

/**
 * @tc.name: ReportDcameraOpenProgress_001
 * @tc.desc: check ReportDcameraOpenProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraOpenProgress_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraOpenProgress_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraOpenProgress(FUNC, CameraOpen::OPEN_CHANNEL,
        DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraOpenProgress_001 end");
}

/**
 * @tc.name: ReportDcameraOpenProgress_002
 * @tc.desc: check ReportDcameraOpenProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraOpenProgress_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraOpenProgress_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraOpenProgress(FUNC, CameraOpen::OPEN_CHANNEL,
        DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraOpenProgress_002 end");
}

/**
 * @tc.name: ReportDcameraClose_001
 * @tc.desc: check ReportDcameraClose
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraClose_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraClose_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraClose(FUNC, CameraClose::STOP_CAPTURE,
        BizState::BIZ_STATE_START, DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraClose_001 end");
}
/**
 * @tc.name: ReportDcameraClose_002
 * @tc.desc: check ReportDcameraClose
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraClose_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraClose_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraClose(FUNC, CameraClose::RELEASE_STREAM,
        BizState::BIZ_STATE_START, DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraClose_002 end");
}

/**
 * @tc.name: ReportDcameraCloseProgress_001
 * @tc.desc: check ReportDcameraCloseProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraCloseProgress_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraCloseProgress_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraCloseProgress(FUNC, CameraClose::RELEASE_STREAM,
        DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraCloseProgress_001 end");
}

/**
 * @tc.name: ReportDcameraCloseProgress_002
 * @tc.desc: check ReportDcameraCloseProgress
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraCloseProgress_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraCloseProgress_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraCloseProgress(FUNC, CameraClose::RELEASE_STREAM,
        DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraCloseProgress_002 end");
}

/**
 * @tc.name: ReportDcameraUnInit_001
 * @tc.desc: check ReportDcameraUnInit
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraUnInit_001, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraUnInit_001 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraUnInit(FUNC, CameraUnInit::UNLOAD_HDF_DRIVER,
        BizState::BIZ_STATE_START, DCAMERA_OK);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraUnInit_001 end");
}

/**
 * @tc.name: ReportDcameraUnInit_002
 * @tc.desc: check ReportDcameraUnInit
 * @tc.type: FUNC
 */
HWTEST_F(DcameraRadarTest, ReportDcameraUnInit_002, TestSize.Level1)
{
    DHLOGI("DcameraRadarTest ReportDcameraUnInit_002 begin");
    bool ret = DcameraRadar::GetInstance().ReportDcameraUnInit(FUNC, CameraUnInit::SERVICE_RELEASE,
        BizState::BIZ_STATE_START, DCAMERA_MEMORY_OPT_ERROR);
    EXPECT_EQ(ret, true);
    DHLOGI("DcameraRadarTest ReportDcameraUnInit_002 end");
}
}
}