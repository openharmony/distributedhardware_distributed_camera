/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#include "dcamera_hisysevent_adapter.h"

#include "hisysevent.h"
#include "securec.h"

#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraHisyseventAdapterTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraHisyseventAdapterTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DcameraHisyseventAdapterTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraHisyseventAdapterTest::SetUp(void)
{
    DHLOGI("enter");
}

void DcameraHisyseventAdapterTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_hisysevent_adapter_test_001
 * @tc.desc: Verify the ReportDcamerInitFail function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHisyseventAdapterTest, dcamera_hisysevent_adapter_test_001, TestSize.Level1)
{
    std::string eventName = "test001";
    int32_t errCode = 2;
    std::string errMsg = "dcamera_hisysevent_adapter_test_001";
    ReportDcamerInitFail(eventName, errCode, errMsg);
    std::string ret = CreateMsg(eventName.c_str());
    EXPECT_NE(DCAMERA_OK, ret.size());
}

/**
 * @tc.name: dcamera_hisysevent_adapter_test_002
 * @tc.desc: Verify the ReportRegisterCameraFail function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHisyseventAdapterTest, dcamera_hisysevent_adapter_test_002, TestSize.Level1)
{
    std::string eventName = "test002";
    std::string devId = "bb536a637105409e904d4da83790a4a7";
    std::string dhId = "camera01";
    std::string version = "1.0";
    std::string errMsg = "error";
    ReportRegisterCameraFail(eventName, devId, dhId, version, errMsg);
    std::string ret = CreateMsg(eventName.c_str());
    EXPECT_NE(DCAMERA_OK, ret.size());
}

/**
 * @tc.name: dcamera_hisysevent_adapter_test_003
 * @tc.desc: Verify the ReportRegisterCameraEvent function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHisyseventAdapterTest, dcamera_hisysevent_adapter_test_003, TestSize.Level1)
{
    std::string eventName = "test003";
    std::string devId = "bb536a637105409e904d4da83790a4a7";
    std::string dhId = "camera01";
    std::string version = "1.0";
    std::string errMsg = "error";
    ReportRegisterCameraEvent(eventName, devId, dhId, version, errMsg);
    std::string ret = CreateMsg(eventName.c_str());
    EXPECT_NE(DCAMERA_OK, ret.size());
}

/**
 * @tc.name: dcamera_hisysevent_adapter_test_004
 * @tc.desc: Verify the ReportCameraOperaterEvent function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHisyseventAdapterTest, dcamera_hisysevent_adapter_test_004, TestSize.Level1)
{
    std::string eventName = "test004";
    std::string devId = "bb536a637105409e904d4da83790a4a7";
    std::string dhId = "camera01";
    std::string errMsg = "error";
    ReportCameraOperaterEvent(eventName, devId, dhId, errMsg);
    std::string ret = CreateMsg(eventName.c_str());
    EXPECT_NE(DCAMERA_OK, ret.size());
}

/**
 * @tc.name: dcamera_hisysevent_adapter_test_005
 * @tc.desc: Verify the ReportStartCaptureEvent function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraHisyseventAdapterTest, dcamera_hisysevent_adapter_test_005, TestSize.Level1)
{
    std::string eventName = "test005";
    EventCaptureInfo capture;
    capture.width_ = 640;
    capture.height_ = 480;
    capture.isCapture_ = true;
    capture.encodeType_ = 1;
    capture.type_ = 0;
    std::string errMsg = "error";
    ReportStartCaptureEvent(eventName, capture, errMsg);
    std::string ret = CreateMsg(eventName.c_str());
    EXPECT_NE(DCAMERA_OK, ret.size());
}
} // namespace DistributedHardware
} // namespace OHOS