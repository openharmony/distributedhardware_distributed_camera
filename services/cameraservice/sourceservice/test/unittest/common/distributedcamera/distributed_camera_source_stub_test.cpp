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

#include "dcamera_source_callback.h"
#include "distributed_camera_source_proxy.h"
#include "distributed_camera_source_stub.h"
#include "distributed_hardware_log.h"
#include "mock_distributed_camera_source_stub.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraSourceStubTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraSourceStubTest::SetUpTestCase(void)
{
    DHLOGI("DcameraSourceStubTest::SetUpTestCase");
}

void DcameraSourceStubTest::TearDownTestCase(void)
{
    DHLOGI("DcameraSourceStubTest::TearDownTestCase");
}

void DcameraSourceStubTest::SetUp(void)
{
    DHLOGI("DcameraSourceStubTest::SetUp");
}

void DcameraSourceStubTest::TearDown(void)
{
    DHLOGI("DcameraSourceStubTest::TearDown");
}

/**
 * @tc.name: dcamera_source_stub_test_001
 * @tc.desc: Verify the InitSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_001, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_001");
    sptr<IRemoteObject> sourceStubPtr = new MockDistributedCameraSourceStub();
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string params = "params000";
    sptr<IDCameraSourceCallback> callback = new DCameraSourceCallback();
    int32_t ret = sourceProxy.InitSource(params, callback);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_002
 * @tc.desc: Verify the ReleaseSource function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_002, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_002");
    sptr<IRemoteObject> sourceStubPtr = new MockDistributedCameraSourceStub();
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    int32_t ret = sourceProxy.ReleaseSource();
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_003
 * @tc.desc: Verify the RegisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_003, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_003");
    sptr<IRemoteObject> sourceStubPtr = new MockDistributedCameraSourceStub();
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    EnableParam param;
    param.version = "1";
    param.attrs = "attrs";
    int32_t ret = sourceProxy.RegisterDistributedHardware(devId, dhId, reqId, param);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_004
 * @tc.desc: Verify the UnregisterDistributedHardware function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_004, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_004");
    sptr<IRemoteObject> sourceStubPtr = new MockDistributedCameraSourceStub();
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string reqId = "reqId000";
    int32_t ret = sourceProxy.UnregisterDistributedHardware(devId, dhId, reqId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: dcamera_source_stub_test_005
 * @tc.desc: Verify the DCameraNotify function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSourceStubTest, dcamera_source_stub_test_005, TestSize.Level1)
{
    DHLOGI("DcameraSourceStubTest::dcamera_source_stub_test_005");
    sptr<IRemoteObject> sourceStubPtr = new MockDistributedCameraSourceStub();
    DistributedCameraSourceProxy sourceProxy(sourceStubPtr);
    std::string devId = "devId000";
    std::string dhId = "dhId000";
    std::string events = "events000";
    int32_t ret = sourceProxy.DCameraNotify(devId, dhId, events);
    EXPECT_EQ(DCAMERA_OK, ret);
}
} // namespace DistributedHardware
} // namespace OHOS