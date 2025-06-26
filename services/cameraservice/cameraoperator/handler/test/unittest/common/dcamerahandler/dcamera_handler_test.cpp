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

#include <gtest/gtest.h>

#define private public
#include "dcamera_handler.h"
#undef private

#include "accesstoken_kit.h"
#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraHandlerTestPluginListener : public PluginListener {
public:
    DCameraHandlerTestPluginListener() = default;
    virtual ~DCameraHandlerTestPluginListener() = default;

    void PluginHardware(const std::string &dhId, const std::string &attrs, const std::string &subtype) override
    {
        DHLOGI("dhId: %{public}s, attrs: %{public}s, subtype: %{public}s.", GetAnonyString(dhId).c_str(),
            attrs.c_str(), subtype.c_str());
    }

    void UnPluginHardware(const std::string &dhId) override
    {
        DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    }
};

class DCameraHandlerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    void SetTokenID();
};

void DCameraHandlerTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DCameraHandlerTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DCameraHandlerTest::SetUp(void)
{
    DHLOGI("enter");
}

void DCameraHandlerTest::TearDown(void)
{
    DHLOGI("enter");
}

void DCameraHandlerTest::SetTokenID()
{
    uint64_t tokenId;
    const char *perms[2];
    perms[0] = "ohos.permission.DISTRIBUTED_DATASYNC";
    perms[1] = "ohos.permission.CAMERA";
    NativeTokenInfoParams infoInstance = {
        .dcapsNum = 0,
        .permsNum = 2,
        .aclsNum = 0,
        .dcaps = NULL,
        .perms = perms,
        .acls = NULL,
        .processName = "dcamera_client_demo",
        .aplStr = "system_basic",
    };
    tokenId = GetAccessTokenId(&infoInstance);
    SetSelfTokenID(tokenId);
    OHOS::Security::AccessToken::AccessTokenKit::ReloadNativeTokenInfo();
}

/**
 * @tc.name: dcamera_handler_test_001
 * @tc.desc: Verify Initialize
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_001, TestSize.Level1)
{
    int32_t ret = DCameraHandler::GetInstance().Initialize();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_handler_test_002
 * @tc.desc: Verify GetCameras
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_002, TestSize.Level1)
{
    int32_t ret = DCameraHandler::GetInstance().GetCameras().size();
    EXPECT_GT(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_handler_test_003
 * @tc.desc: Verify Query
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_003, TestSize.Level1)
{
    SetTokenID();
    int32_t ret = DCameraHandler::GetInstance().Query().size();
    EXPECT_GE(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_handler_test_004
 * @tc.desc: Verify QueryExtraInfo
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_004, TestSize.Level1)
{
    int32_t ret = DCameraHandler::GetInstance().QueryExtraInfo().size();
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: dcamera_handler_test_005
 * @tc.desc: Verify IsSupportPlugin, RegisterPluginListener and UnRegisterPluginListener
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_005, TestSize.Level1)
{
    auto listener = std::make_shared<DCameraHandlerTestPluginListener>();
    DCameraHandler::GetInstance().RegisterPluginListener(listener);

    listener = nullptr;
    DCameraHandler::GetInstance().RegisterPluginListener(listener);

    DCameraHandler::GetInstance().UnRegisterPluginListener();

    int32_t ret = DCameraHandler::GetInstance().IsSupportPlugin();
    EXPECT_EQ(ret, false);
}

/**
 * @tc.name: dcamera_handler_test_006
 * @tc.desc: Verify GetCameraPosition
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_006, TestSize.Level1)
{
    std::string ret = DCameraHandler::GetInstance().GetCameraPosition(
        CameraStandard::CameraPosition::CAMERA_POSITION_UNSPECIFIED);
    EXPECT_EQ(ret, CAMERA_POSITION_UNSPECIFIED);

    ret = DCameraHandler::GetInstance().GetCameraPosition(
        CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(ret, CAMERA_POSITION_BACK);

    ret = DCameraHandler::GetInstance().GetCameraPosition(
        CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(ret, CAMERA_POSITION_FRONT);

    int32_t invalidParam = 3;
    auto position = static_cast<CameraStandard::CameraPosition>(invalidParam);
    ret = DCameraHandler::GetInstance().GetCameraPosition(position);
    EXPECT_EQ(ret, "");
}

/**
 * @tc.name: dcamera_handler_test_007
 * @tc.desc: Verify CovertToDcameraFormat
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_007, TestSize.Level1)
{
    int32_t ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_INVALID);
    EXPECT_EQ(ret, -1);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_RGBA_8888);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_RGBA_8888);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_420_888);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_YCBCR_420_888);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_YUV_420_SP);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_YCRCB_420_SP);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_JPEG);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_JPEG);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_P010);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_YCBCR_P010);

    ret = DCameraHandler::GetInstance().CovertToDcameraFormat(
        CameraStandard::CameraFormat::CAMERA_FORMAT_YCRCB_P010);
    EXPECT_EQ(ret, OHOS_CAMERA_FORMAT_YCRCB_P010);
}

/**
 * @tc.name: dcamera_handler_test_008
 * @tc.desc: Verify IsValid
 * @tc.type: FUNC
 * @tc.require: AR000GK6MF
 */
HWTEST_F(DCameraHandlerTest, dcamera_handler_test_008, TestSize.Level1)
{
    CameraStandard::Size size{ 640, 480 };
    bool ret = DCameraHandler::GetInstance().IsValid(CONTINUOUS_FRAME, size);
    EXPECT_EQ(ret, true);

    ret = DCameraHandler::GetInstance().IsValid(SNAPSHOT_FRAME, size);
    EXPECT_EQ(ret, true);

    CameraStandard::Size size1{ 65540, 480 };
    ret = DCameraHandler::GetInstance().IsValid(SNAPSHOT_FRAME, size1);
    EXPECT_EQ(ret, false);

    ret = DCameraHandler::GetInstance().IsValid(CONTINUOUS_FRAME, size1);
    EXPECT_EQ(ret, false);

    int32_t invalidParam = 2;
    auto type = static_cast<DCStreamType>(invalidParam);
    ret = DCameraHandler::GetInstance().IsValid(type, size);
    EXPECT_EQ(ret, false);
}
} // namespace DistributedHardware
} // namespace OHOS