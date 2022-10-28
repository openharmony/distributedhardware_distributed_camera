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

#include "accesstoken_kit.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraUtilsToolsTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();

    void SetTokenID();
};

void DcameraUtilsToolsTest::SetUpTestCase(void)
{
    DHLOGI("DcameraUtilsToolsTest::SetUpTestCase");
}

void DcameraUtilsToolsTest::TearDownTestCase(void)
{
    DHLOGI("DcameraUtilsToolsTest::TearDownTestCase");
}

void DcameraUtilsToolsTest::SetUp(void)
{
    DHLOGI("DcameraUtilsToolsTest::SetUp");
}

void DcameraUtilsToolsTest::TearDown(void)
{
    DHLOGI("DcameraUtilsToolsTest::TearDown");
}

void DcameraUtilsToolsTest::SetTokenID()
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
 * @tc.name: GetLocalDeviceNetworkId_001
 * @tc.desc: Verify the GetLocalDeviceNetworkId function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, GetLocalDeviceNetworkId_001, TestSize.Level1)
{
    std::string networkId;
    SetTokenID();
    int32_t ret = GetLocalDeviceNetworkId(networkId);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: GetNowTimeStampMs_001
 * @tc.desc: Verify the GetNowTimeStampMs function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, GetNowTimeStampMs_001, TestSize.Level1)
{
    int32_t ret = GetNowTimeStampMs();
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: GetNowTimeStampUs_001
 * @tc.desc: Verify the GetNowTimeStampUs function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, GetNowTimeStampUs_001, TestSize.Level1)
{
    int32_t ret = GetNowTimeStampUs();
    EXPECT_NE(DCAMERA_OK, ret);
}

/**
 * @tc.name: GetAlignedHeight_001
 * @tc.desc: Verify the GetAlignedHeight function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, GetAlignedHeight_001, TestSize.Level1)
{
    int32_t width = 0;
    int32_t ret = GetAlignedHeight(width);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: Base64Encode_001
 * @tc.desc: Verify the Base64Encode function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, Base64Encode_001, TestSize.Level1)
{
    std::string toEncode = "testtest";
    std::string retsult = Base64Encode(reinterpret_cast<const unsigned char *>(toEncode.c_str()), toEncode.size());
    EXPECT_NE(0, retsult.size());
    std::string ret = Base64Decode(retsult);
    EXPECT_EQ(toEncode.size(), ret.size());
}
} // namespace DistributedHardware
} // namespace OHOS