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

#define private public
#include "dcamera_sink_service_ipc.h"
#undef private
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkServiceIpcTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DCameraSinkServiceIpcTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkServiceIpcTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraSinkServiceIpcTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraSinkServiceIpcTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_service_ipc_test_001
 * @tc.desc: Verify the dcamera sink service ipc function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkServiceIpcTest, dcamera_sink_service_ipc_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_ipc_test_001");
    DCameraSinkServiceIpc::GetInstance().UnInit();
    DCameraSinkServiceIpc::GetInstance().Init();
    DCameraSinkServiceIpc::GetInstance().UnInit();
    DCameraSinkServiceIpc::GetInstance().isInit_ = true;
    DCameraSinkServiceIpc::GetInstance().Init();
    DCameraSinkServiceIpc::GetInstance().UnInit();
    EXPECT_EQ(false, DCameraSinkServiceIpc::GetInstance().isInit_);
}

/**
 * @tc.name: dcamera_sink_service_ipc_test_002
 * @tc.desc: Verify the dcamera sink service ipc function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkServiceIpcTest, dcamera_sink_service_ipc_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_service_ipc_test_002");
    std::string devId = "";
    sptr<IDistributedCameraSource> sourceSA = DCameraSinkServiceIpc::GetInstance().GetSourceRemoteCamSrv(devId);
    EXPECT_EQ(nullptr, sourceSA);

    devId = "devId000";
    sourceSA = DCameraSinkServiceIpc::GetInstance().GetSourceRemoteCamSrv(devId);
    EXPECT_EQ(nullptr, sourceSA);

    DCameraSinkServiceIpc::GetInstance().DeleteSourceRemoteCamSrv(devId);
    DCameraSinkServiceIpc::GetInstance().ClearSourceRemoteCamSrv();
    wptr<IRemoteObject> remote;
    DCameraSinkServiceIpc::GetInstance().OnSourceRemoteCamSrvDied(remote);
    EXPECT_EQ(nullptr, sourceSA);
}
} // namespace DistributedHardware
} // namespace OHOS