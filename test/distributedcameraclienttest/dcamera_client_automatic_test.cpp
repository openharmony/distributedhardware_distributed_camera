/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "dcamera_client_demo.h"
#include "distributed_hardware_log.h"

#include "access_token.h"
#include "accesstoken_kit.h"
#include "hap_token_info.h"
#include "ipc_skeleton.h"
#include "nativetoken_kit.h"
#include "token_setproc.h"

using namespace testing::ext;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedHardware {
constexpr int32_t CAMERA_OK = 0;
constexpr int32_t SLEEP_OK_SECOND = 2;
constexpr int32_t SLEEP_FIVE_SECOND = 5;

class DCameraClientAutomaticTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
    DCameraClientAutomaticTest();
};

void DCameraClientAutomaticTest::SetUpTestCase(void)
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
    sleep(SLEEP_OK_SECOND);
}

void DCameraClientAutomaticTest::TearDownTestCase(void) {}
void DCameraClientAutomaticTest::SetUp(void) {}
void DCameraClientAutomaticTest::TearDown(void)
{
    ReleaseResource();
    sleep(SLEEP_OK_SECOND);
}
DCameraClientAutomaticTest::DCameraClientAutomaticTest(void) {}

/**
 * @tc.number    : DCameraTest_0100
 * @tc.name      ：InitCameraStandard()
 * @tc.desc      : rear camera acquisition
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0100, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
}

/**
 * @tc.number    : DCameraTest_0200
 * @tc.name      ：InitCameraStandard()
 * @tc.desc      : rear camera preview
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0200, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0300
 * @tc.name      ：InitCameraStandard()
 * @tc.desc      : front camera acquisition
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0300, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
}

/**
 * @tc.number    : DCameraTest_0400
 * @tc.name      ：InitCameraStandard()
 * @tc.desc      : front camera preview
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0400, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0500
 * @tc.name      ：InitCameraStandard()
 * @tc.desc      : front and rear camera switching preview
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0500, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    sleep(SLEEP_FIVE_SECOND);
    ReleaseResource();
    sleep(SLEEP_OK_SECOND);

    ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    sleep(SLEEP_FIVE_SECOND);
    ReleaseResource();
    sleep(SLEEP_OK_SECOND);

    ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0600
 * @tc.name      ：Capture()
 * @tc.desc      : rear camera taking photos
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0600, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    ret = Capture();
    EXPECT_EQ(CAMERA_OK, ret) << "Capture fail";
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0700
 * @tc.name      ：Capture()
 * @tc.desc      : front camera taking photos
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0700, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    ret = Capture();
    EXPECT_EQ(CAMERA_OK, ret) << "Capture fail";
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0800
 * @tc.name      ：Capture()
 * @tc.desc      : flash photography
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0800, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    InitCaptureInfo(0, 0);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    ConfigFocusFlashAndExposure(false);
    ret = Capture();
    EXPECT_EQ(CAMERA_OK, ret) << "Capture fail";
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_0900
 * @tc.name      ：Capture()
 * @tc.desc      : preview using resolution
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_0900, TestSize.Level1)
{
    int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
    EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
    std::vector<CameraStandard::Size> previewResolution = {};
    ret = GetPreviewProfiles(previewResolution);
    EXPECT_EQ(CAMERA_OK, ret) << "GetPreviewProfiles fail";
    CameraStandard::Size size = previewResolution.back();
    InitCaptureInfo(size.width, size.height);
    InitPhotoOutput();
    InitPreviewOutput();
    ConfigCaptureSession();
    ConfigFocusFlashAndExposure(false);
    ret = Capture();
    EXPECT_EQ(CAMERA_OK, ret) << "Capture fail";
    sleep(SLEEP_FIVE_SECOND);
}

/**
 * @tc.number    : DCameraTest_1000
 * @tc.name      ：Capture()
 * @tc.desc      : front and rear camera switching preview 50 times
 * @tc.desc      : swicth distributed cmera
 * @tc.type      : FUNC
 * @tc.size      : Medium test
 */
HWTEST_F(DCameraClientAutomaticTest, DCameraTest_1000, TestSize.Level1)
{
    int32_t i = 50;
    while (i > 0) {
        int32_t ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_BACK);
        EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
        InitCaptureInfo(0, 0);
        InitPhotoOutput();
        InitPreviewOutput();
        ConfigCaptureSession();
        sleep(SLEEP_FIVE_SECOND);
        ReleaseResource();
        sleep(SLEEP_OK_SECOND);

        ret = InitCameraStandard(OHOS::CameraStandard::CameraPosition::CAMERA_POSITION_FRONT);
        EXPECT_EQ(CAMERA_OK, ret) << "InitCameraStandard fail";
        InitCaptureInfo(0, 0);
        InitPhotoOutput();
        InitPreviewOutput();
        ConfigCaptureSession();
        sleep(SLEEP_FIVE_SECOND);
        ReleaseResource();
        sleep(SLEEP_OK_SECOND);
        i--;
    }
}
} // namespace DistributedHardware
} // namespace OHOS
