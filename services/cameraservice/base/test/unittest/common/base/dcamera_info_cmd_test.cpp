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

#include "dcamera_info_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraInfoCmdTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static const std::string TEST_INFO_CMD_JSON_LACK_TYPE = R"({
    "dhId": "camrea_0",
    "Command": "GET_INFO",
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_TYPE_EXCEPTION = R"({
    "Type": 0,
    "dhId": "camrea_0",
    "Command": "GET_INFO",
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_LACK_DHID = R"({
    "Type": "OPERATION",
    "Command": "GET_INFO",
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_DHID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": 0,
    "Command": "GET_INFO",
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_LACK_COMMAND = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_COMMAND_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": 0,
    "Value": {"State": 1}
})";

static const std::string TEST_INFO_CMD_JSON_LACK_VALUE = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "GET_INFO"
})";

static const std::string TEST_INFO_CMD_JSON_LACK_VALUE_BODY_STATE = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "GET_INFO",
    "Value": {}
})";

static const std::string TEST_INFO_CMD_JSON_VALUE_BODY_STATE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "GET_INFO",
    "Value": {"State": "1"}
})";


void DCameraInfoCmdTest::SetUpTestCase(void)
{
}

void DCameraInfoCmdTest::TearDownTestCase(void)
{
}

void DCameraInfoCmdTest::SetUp(void)
{
}

void DCameraInfoCmdTest::TearDown(void)
{
}

/**
 * @tc.name: Unmarshal_001.
 * @tc.desc: Verify InfoCmdTest Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraInfoCmdTest, Unmarshal_001, TestSize.Level1)
{
    DCameraInfoCmd cmd;
    std::string str = "0";
    int32_t ret = cmd.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_LACK_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_LACK_DHID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_DHID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_LACK_COMMAND);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_COMMAND_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_LACK_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_LACK_VALUE_BODY_STATE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_INFO_CMD_JSON_VALUE_BODY_STATE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS