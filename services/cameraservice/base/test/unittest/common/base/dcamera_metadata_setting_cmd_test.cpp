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
#include <memory>

#include "dcamera_metadata_setting_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraMetadataSettingCmdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_TYPE = R"({
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_TYPE_EXCEPTION = R"({
    "Type": 0,
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_DHID = R"({
    "Type": "MESSAGE",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_DHID_EXCEPTION = R"({
    "Type": "MESSAGE",
    "dhId": 0,
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_COMMAND = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_COMMAND_EXCEPTION = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": 0,
    "Value": [{"SettingType": 1, "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA"
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_VALUE_EXCEPTION = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": "[{"SettingType": 1, "SettingValue": "TestSetting"}]"
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE_BODY_TYPE = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_VALUE_BODY_TYPE_EXCEPTION = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": "1", "SettingValue": "TestSetting"}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE_BODY_VALUE = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1}]
})";

static const std::string TEST_METADATA_SETTING_CMD_JSON_VALUE_BODY_VALUE_EXCEPTION = R"({
    "Type": "MESSAGE",
    "dhId": "camrea_0",
    "Command": "UPDATE_METADATA",
    "Value": [{"SettingType": 1, "SettingValue": 0}]
})";


void DCameraMetadataSettingCmdTest::SetUpTestCase(void)
{
}

void DCameraMetadataSettingCmdTest::TearDownTestCase(void)
{
}

void DCameraMetadataSettingCmdTest::SetUp(void)
{
}

void DCameraMetadataSettingCmdTest::TearDown(void)
{
}

/**
 * @tc.name: Unmarshal_001.
 * @tc.desc: Verify MetadataSettingCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraMetadataSettingCmdTest, Unmarshal_001, TestSize.Level1)
{
    DCameraMetadataSettingCmd cmd;
    std::string str = "0";
    int32_t ret = cmd.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_DHID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_DHID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_COMMAND);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_COMMAND_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_VALUE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Unmarshal_002.
 * @tc.desc: Verify MetadataSettingCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraMetadataSettingCmdTest, Unmarshal_002, TestSize.Level1)
{
    DCameraMetadataSettingCmd cmd;
    int32_t ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE_BODY_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_VALUE_BODY_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_LACK_VALUE_BODY_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_METADATA_SETTING_CMD_JSON_VALUE_BODY_VALUE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS