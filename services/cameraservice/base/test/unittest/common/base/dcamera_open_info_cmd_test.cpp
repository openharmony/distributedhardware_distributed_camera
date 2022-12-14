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

#include "dcamera_open_info_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraOpenInfoCmdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};


void DCameraOpenInfoCmdTest::SetUpTestCase(void)
{
}

void DCameraOpenInfoCmdTest::TearDownTestCase(void)
{
}

void DCameraOpenInfoCmdTest::SetUp(void)
{
}

void DCameraOpenInfoCmdTest::TearDown(void)
{
}

static const std::string TEST_OPEN_INFO_CMD_JSON = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_LACK_TYPE = R"({
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_TYPE_EXCEPTION = R"({
    "Type": 0,
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_LACK_DHID = R"({
    "Type": "OPERATION",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_DHID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": 0,
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_LACK_COMMAND = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_COMMAND_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": 0,
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_LACK_VALUE = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL"
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_LACK_VALUE_BODY_DEVID = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {}
})";

static const std::string TEST_OPEN_INFO_CMD_JSON_VALUE_BODY_DEVID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "OPEN_CHANNEL",
    "Value": {"SourceDevId": 0}
})";

/**
 * @tc.name: dcamera_open_info_cmd_001.
 * @tc.desc: Verify OpenInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraOpenInfoCmdTest, dcamera_open_info_cmd_001, TestSize.Level1)
{
    DCameraOpenInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::string jsonStr;
    ret = cmd.Marshal(jsonStr);
    EXPECT_EQ(DCAMERA_OK, ret);

    std::string str = "0";
    ret = cmd.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_LACK_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_LACK_DHID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_DHID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_LACK_COMMAND);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_COMMAND_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_LACK_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_LACK_VALUE_BODY_DEVID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_OPEN_INFO_CMD_JSON_VALUE_BODY_DEVID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS