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

#include <cstring>
#include <fstream>
#include <iostream>

#include "accesstoken_kit.h"
#include "anonymous_string.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "iaccess_listener.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class TestAccessListener : public IAccessListener {
    sptr<IRemoteObject> AsObject()
    {
        return nullptr;
    }

    void OnRequestHardwareAccess(const std::string &requestId, AuthDeviceInfo info, const DHType dhType,
        const std::string &pkgName)
    {
        (void)requestId;
        (void)info;
        (void)dhType;
        (void)pkgName;
    }
};

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
    DHLOGI("enter");
}

void DcameraUtilsToolsTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraUtilsToolsTest::SetUp(void)
{
    DHLOGI("enter");
}

void DcameraUtilsToolsTest::TearDown(void)
{
    DHLOGI("enter");
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
    EXPECT_NE(ERR_DH_CAMERA_BASE, ret);
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
    int32_t value = 0;
    std::string toEncode = "testtest";
    std::string retsult = Base64Encode(reinterpret_cast<const unsigned char *>(toEncode.c_str()), toEncode.size());
    EXPECT_NE(0, retsult.size());
    Base64Decode(retsult);
    int32_t ret = GetAlignedHeight(value);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: Base64Encode_002
 * @tc.desc: Verify the Base64Encode function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, Base64Encode_002, TestSize.Level1)
{
    int32_t value = 0;
    std::string toEncode = "testtest";
    int32_t len = 0;
    Base64Encode(reinterpret_cast<const unsigned char *>(toEncode.c_str()), len);
    std::string toDncode = "";
    Base64Decode(toDncode);
    int32_t ret = GetAlignedHeight(value);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: GetAnonyInt32_001
 * @tc.desc: Verify the GetAnonyInt32 function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, GetAnonyInt32_001, TestSize.Level1)
{
    int32_t value = 0;
    GetAnonyInt32(value);
    int32_t ret = GetAlignedHeight(value);
    EXPECT_EQ(DCAMERA_OK, ret);
}

/**
 * @tc.name: IsOverDumpSize_001
 * @tc.desc: Verify the IsUnderDumpMaxSize function failed.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, IsOverDumpSize_001, TestSize.Level1)
{
    std::string dumpFileName = "/data/test.txt";
    std::ofstream ofs(dumpFileName, std::ios::out);
    if (!ofs) {
        DHLOGI("open file failed");
    } else {
        ofs.close();
    }

    std::string dumpPath = "";
    std::string fileName = "";
    uint8_t *buf = nullptr;
    size_t size = 0;
    DumpBufferToFile(dumpPath, fileName, buf, size);
    EXPECT_EQ(DCAMERA_INIT_ERR, IsUnderDumpMaxSize(dumpPath, fileName));

    dumpPath = "test";
    DumpBufferToFile(dumpPath, fileName, buf, size);
    EXPECT_EQ(DCAMERA_INIT_ERR, IsUnderDumpMaxSize(dumpPath, fileName));

    fileName = "test";
    DumpBufferToFile(dumpPath, fileName, buf, size);
    EXPECT_EQ(DCAMERA_INIT_ERR, IsUnderDumpMaxSize(dumpPath, fileName));

    uint8_t str[] = "test";
    size = strlen(reinterpret_cast<const char*>(str));
    DumpBufferToFile(dumpPath, fileName, str, size);
    EXPECT_EQ(DCAMERA_INIT_ERR, IsUnderDumpMaxSize(dumpPath, fileName));

    fileName = "test.txt";
    DumpBufferToFile("/data/", fileName, str, size);
    EXPECT_NE(DCAMERA_OK, IsUnderDumpMaxSize("/data/", fileName));
}

/**
 * @tc.name: IsUnderDumpMaxSize_001
 * @tc.desc: Verify the IsUnderDumpMaxSize.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, IsUnderDumpMaxSize_001, TestSize.Level1)
{
    std::string longFileName(PATH_MAX + 1, 'a');
    std::string fileName = "test";
    EXPECT_EQ(IsUnderDumpMaxSize(longFileName, fileName), DCAMERA_INIT_ERR);
}

/**
 * @tc.name: IsUnderDumpMaxSize_002
 * @tc.desc: Verify the IsUnderDumpMaxSize.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, IsUnderDumpMaxSize_002, TestSize.Level1)
{
    EXPECT_EQ(IsUnderDumpMaxSize("/nonexistent/file", "test"), DCAMERA_INIT_ERR);
}

/**
 * @tc.name: OpenDumpFile_001
 * @tc.desc: Verify the WriteDumpFile.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, OpenDumpFile_001, TestSize.Level1)
{
    FILE *dumpFile = nullptr;
    void *buffer = nullptr;
    size_t bufferSize = 0;
    DumpFileUtil::WriteDumpFile(dumpFile, buffer, bufferSize);
    const std::string dumpServerParaTest = "sys.dcamera.dump.write.enable w";
    const std::string dumpDcameraTestFilename = "opendumpfile.txt";
    DumpFileUtil::OpenDumpFile(dumpServerParaTest, dumpDcameraTestFilename, &dumpFile);
    DumpFileUtil::WriteDumpFile(dumpFile, buffer, bufferSize);
    const std::string dumpServerParaTest1 = "sys.dcamera.dump.write.enable a";
    const std::string dumpDcameraTestFilename1 = "opendumpfile1.txt";
    DumpFileUtil::OpenDumpFile(dumpServerParaTest1, dumpDcameraTestFilename1, &dumpFile);
    DumpFileUtil::WriteDumpFile(dumpFile, buffer, bufferSize);
    DumpFileUtil::CloseDumpFile(&dumpFile);
    EXPECT_EQ(true, dumpFile == nullptr);
}

/**
 * @tc.name: DCameraSystemSwitch_001
 * @tc.desc: Verify the DCameraSystemSwitchInfo.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, DCameraSystemSwitch_001, TestSize.Level1)
{
    int32_t res = DCameraSystemSwitchInfo::GetInstance().SetSystemSwitchFlagAndRotation("1", true, 90);
    EXPECT_EQ(res, DCAMERA_OK);
    bool ret = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchFlag("1");
    EXPECT_EQ(ret, true);
    ret = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchFlag("2");
    EXPECT_EQ(ret, false);
    int32_t rotate = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchRotation("1");
    EXPECT_EQ(rotate, 90);
    rotate = DCameraSystemSwitchInfo::GetInstance().GetSystemSwitchRotation("2");
    EXPECT_EQ(rotate, 0);
    res = DCameraSystemSwitchInfo::GetInstance().SetSystemSwitchFlagAndRotation("1", false, 0);
    EXPECT_EQ(res, DCAMERA_OK);
}

/**
 * @tc.name: SetAccessConfig_001
 * @tc.desc: Verify the SetAccessConfig.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, SetAccessConfig_001, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    sptr<IAccessListener> listenerNull = nullptr;
    int32_t timeOut = 0;
    std::string pkgName = "pkgName";
    int32_t ret = DCameraAccessConfigManager::GetInstance().SetAccessConfig(listenerNull, timeOut, pkgName);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    sptr<IAccessListener> listener(new TestAccessListener());
    std::string pkgNameNull = "";
    ret = DCameraAccessConfigManager::GetInstance().SetAccessConfig(listener, timeOut, pkgNameNull);
    EXPECT_EQ(ret, DCAMERA_BAD_VALUE);
    ret = DCameraAccessConfigManager::GetInstance().SetAccessConfig(listener, timeOut, pkgName);
    EXPECT_EQ(ret, DCAMERA_OK);
}

/**
 * @tc.name: IsAuthorizationGranted_001
 * @tc.desc: Verify the IsAuthorizationGranted.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, IsAuthorizationGranted_001, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    std::string networkIdDiff = "1";
    bool ret = DCameraAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, false);

    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    ret = DCameraAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId);
    EXPECT_EQ(ret, true);
}

/**
 * @tc.name: ClearAuthorizationResult_001
 * @tc.desc: Verify the ClearAuthorizationResult.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DcameraUtilsToolsTest, ClearAuthorizationResult_001, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    DCameraAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DCameraAccessConfigManager::GetInstance().ClearAuthorizationResult(networkId);
    bool ret = DCameraAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId);
    EXPECT_EQ(ret, false);
}

HWTEST_F(DcameraUtilsToolsTest, WaitForAuthorizationResult_001, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string networkId = "0";
    bool ret = DCameraAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId);
    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    ret = DCameraAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId);
    EXPECT_EQ(ret, true);
}

HWTEST_F(DcameraUtilsToolsTest, ClearAccessConfigByPkgName_001, TestSize.Level1)
{
    DCameraAccessConfigManager::GetInstance().ClearAccessConfig();
    std::string pkgNameNull = "";
    std::string networkId = "0";
    DCameraAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgNameNull);
    std::string pkgName = "pkgName";
    DCameraAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgName);
    DCameraAccessConfigManager::GetInstance().pkgName_ = pkgName;
    DCameraAccessConfigManager::GetInstance().SetAuthorizationGranted(networkId, true);
    DCameraAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgName);
    bool ret = DCameraAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId);
    EXPECT_EQ(ret, false);
}
} // namespace DistributedHardware
} // namespace OHOS