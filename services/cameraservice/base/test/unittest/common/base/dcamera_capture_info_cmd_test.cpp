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
#include <memory>

#include "dcamera_capture_info_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraCaptureInfoCmdlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_TYPE = R"({
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_TYPE_EXCEPTION = R"({
    "Type": 0,
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_DHID = R"({
    "Type": "OPERATION",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_DHID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": 0,
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_COMMAND = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_COMMAND_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": 0,
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_ARRAY = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value":"[
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]"
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": "[
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]"
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_WIDTH = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_WIDTH_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": "1920", "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_HEIGHT = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_HEIGHT_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": "1080", "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_FORMAT = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_FORMAT_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": "1", "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_DATASPACE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1,
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_DATASPACE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": "1",
        "IsCapture": true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_ISCAPTURE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_ISCAPTURE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": "true", "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_ENCODETYPE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_STREAMTYPE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_ENCODETYPE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture": true, "EncodeType": "1", "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_CAPTURESETTINGS = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_CAPTURESETTINGS_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": "[{"SettingType": 1, "SettingValue": "TestSetting"}]"}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_CAPTURESETTINGS_BODY_TYPE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_BODY_TYPE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": "1", "SettingValue": "TestSetting"}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_LACK_CAPTURESETTINGS_BODY_VALUE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_BODY_VALUE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": 0}]}
    ]
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": 0}]}
    ],
    "mode": 1,
    "userId": 100,
    "tokenId": 0,
    "accountId": "accountId"
})";

static const std::string TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK_EIS_TRUE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": 0}]}
    ],
    "mode": 1,
    "userId": 100,
    "tokenId": 0,
    "accountId": "accountId",
    "eis": true
})";
 
static const std::string TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK_EIS_FALSE = R"({
    "Type": "OPERATION",
    "dhId": "camera_0",
    "Command": "CAPTURE",
    "Value": [
        {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
        "IsCapture":true, "EncodeType": 1, "StreamType": 1,
        "CaptureSettings": [{"SettingType": 1, "SettingValue": 0}]}
    ],
    "mode": 1,
    "userId": 100,
    "tokenId": 0,
    "accountId": "accountId",
    "eis": false
})";

void DCameraCaptureInfoCmdlTest::SetUpTestCase(void)
{
}

void DCameraCaptureInfoCmdlTest::TearDownTestCase(void)
{
}

void DCameraCaptureInfoCmdlTest::SetUp(void)
{
}

void DCameraCaptureInfoCmdlTest::TearDown(void)
{
}

/**
 * @tc.name: Unmarshal_001.
 * @tc.desc: Verify CaptureInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_001, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    std::string str = "0";
    int32_t ret = cmd.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_DHID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_DHID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_COMMAND);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_COMMAND_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Unmarshal_002.
 * @tc.desc: Verify CaptureInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_002, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_WIDTH);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_WIDTH_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_HEIGHT);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_HEIGHT_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_FORMAT);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_FORMAT_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_DATASPACE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_DATASPACE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

        ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_ISCAPTURE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_ISCAPTURE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_ENCODETYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_ENCODETYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_CAPTURESETTINGS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_BODY_CAPTURESETTINGS_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Unmarshal_003.
 * @tc.desc: Verify CaptureInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_003, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_CAPTURESETTINGS_BODY_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_BODY_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_CAPTURESETTINGS_BODY_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_BODY_VALUE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_LACK_VALUE_BODY_STREAMTYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_VALUE_ARRAY);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK_EIS_FALSE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
 
    ret = cmd.Unmarshal(TEST_CAPTURE_INFO_CMD_JSON_CAPTURESETTINGS_CHECK_EIS_TRUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_004, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.UmarshalValue(nullptr);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    std::shared_ptr<DCameraCaptureInfo> captureInfo;
    ret = cmd.UmarshalSettings(nullptr, captureInfo);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Unmarshal_005.
 * @tc.desc: Verify Unmarshal with triggerFirstTokenId and triggerFirstUserId fields present.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_005, TestSize.Level1)
{
    static const std::string jsonWithTriggerFields = R"({
        "Type": "OPERATION",
        "dhId": "camera_0",
        "Command": "CAPTURE",
        "Value": [
            {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
            "IsCapture": true, "EncodeType": 1, "StreamType": 1,
            "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
        ],
        "mode": 1,
        "userId": 100,
        "tokenId": 200,
        "accountId": "accountId",
        "eis": true,
        "triggerFirstTokenId": 12345,
        "triggerFirstUserId": 100
    })";
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(jsonWithTriggerFields);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(cmd.triggerFirstTokenId_, static_cast<uint32_t>(12345));
    EXPECT_EQ(cmd.triggerFirstUserId_, 100);
    EXPECT_EQ(cmd.sceneMode_, 1);
    EXPECT_EQ(cmd.userId_, 100);
    EXPECT_EQ(cmd.tokenId_, static_cast<uint64_t>(200));
    EXPECT_TRUE(cmd.eis_);
}

/**
 * @tc.name: Unmarshal_006.
 * @tc.desc: Verify Unmarshal without triggerFirstTokenId and triggerFirstUserId fields defaults to 0 and -1.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_006, TestSize.Level1)
{
    static const std::string jsonWithoutTriggerFields = R"({
        "Type": "OPERATION",
        "dhId": "camera_0",
        "Command": "CAPTURE",
        "Value": [
            {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
            "IsCapture": true, "EncodeType": 1, "StreamType": 1,
            "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
        ],
        "mode": 0,
        "userId": 0,
        "tokenId": 0,
        "accountId": ""
    })";
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(jsonWithoutTriggerFields);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(cmd.triggerFirstTokenId_, static_cast<uint32_t>(0));
    EXPECT_EQ(cmd.triggerFirstUserId_, -1);
    EXPECT_FALSE(cmd.eis_);
}

/**
 * @tc.name: Unmarshal_007.
 * @tc.desc: Verify Unmarshal with triggerFirstTokenId/triggerFirstUserId of wrong type defaults to 0 and -1.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Unmarshal_007, TestSize.Level1)
{
    static const std::string jsonTriggerWrongType = R"({
        "Type": "OPERATION",
        "dhId": "camera_0",
        "Command": "CAPTURE",
        "Value": [
            {"Width": 1920, "Height": 1080, "Format": 1, "DataSpace": 1,
            "IsCapture": true, "EncodeType": 1, "StreamType": 1,
            "CaptureSettings": [{"SettingType": 1, "SettingValue": "TestSetting"}]}
        ],
        "mode": 1,
        "userId": 100,
        "tokenId": 200,
        "accountId": "accountId",
        "triggerFirstTokenId": "not_a_number",
        "triggerFirstUserId": "not_a_number"
    })";
    DCameraCaptureInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(jsonTriggerWrongType);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_EQ(cmd.triggerFirstTokenId_, static_cast<uint32_t>(0));
    EXPECT_EQ(cmd.triggerFirstUserId_, -1);
}

/**
 * @tc.name: Marshal_002.
 * @tc.desc: Verify Marshal with empty value_ vector.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Marshal_002, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    cmd.dhId_ = "camera_0";
    cmd.type_ = "OPERATION";
    cmd.command_ = "CAPTURE";
    cmd.triggerFirstTokenId_ = 99999;
    cmd.triggerFirstUserId_ = 200;
    std::string jsonStr;
    int32_t ret = cmd.Marshal(jsonStr);
    EXPECT_EQ(DCAMERA_OK, ret);
    EXPECT_NE(jsonStr.find("triggerFirstTokenId"), std::string::npos);
    EXPECT_NE(jsonStr.find("99999"), std::string::npos);
    EXPECT_NE(jsonStr.find("triggerFirstUserId"), std::string::npos);
    EXPECT_NE(jsonStr.find("200"), std::string::npos);
}

/**
 * @tc.name: Marshal_003.
 * @tc.desc: Verify Marshal with null capture in value_ vector returns DCAMERA_BAD_VALUE.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraCaptureInfoCmdlTest, Marshal_003, TestSize.Level1)
{
    DCameraCaptureInfoCmd cmd;
    cmd.dhId_ = "camera_0";
    cmd.type_ = "OPERATION";
    cmd.command_ = "CAPTURE";
    std::shared_ptr<DCameraCaptureInfo> nullCapture = nullptr;
    cmd.value_.push_back(nullCapture);
    std::string jsonStr;
    int32_t ret = cmd.Marshal(jsonStr);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS
