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

#include "dcamera_channel_info_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraChannelInfoCmdlTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_TYPE = R"({
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_TYPE_EXCEPTION = R"({
    "Type": 0,
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_DHID = R"({
    "Type": "OPERATION",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_DHID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": 0,
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_COMMAND = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_COMMAND_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": 0,
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG"
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE_BODY_DEVID = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_VALUE_BODY_DEVID_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": 0, "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE_BODY_DETAIL = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId"}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_VALUE_BODY_DETAIL_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": "[{"DataSessionFlag": "TestFlag", "StreamType": 1}]"}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_DETAIL_BODY_FLAG = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_DETAIL_BODY_FLAG_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": 0, "StreamType": 1}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_LACK_DETAIL_BODY_TYPE = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag"}]}
})";

static const std::string TEST_CHANNEL_INFO_CMD_JSON_DETAIL_BODY_TYPE_EXCEPTION = R"({
    "Type": "OPERATION",
    "dhId": "camrea_0",
    "Command": "CHANNEL_NEG",
    "Value": {"SourceDevId": "TestDevId", "Detail": [{"DataSessionFlag": "TestFlag", "StreamType": "1"}]}
})";


void DCameraChannelInfoCmdlTest::SetUpTestCase(void)
{
}

void DCameraChannelInfoCmdlTest::TearDownTestCase(void)
{
}

void DCameraChannelInfoCmdlTest::SetUp(void)
{
}

void DCameraChannelInfoCmdlTest::TearDown(void)
{
}

/**
 * @tc.name: Unmarshal_001.
 * @tc.desc: Verify ChannelInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraChannelInfoCmdlTest, Unmarshal_001, TestSize.Level1)
{
    DCameraChannelInfoCmd cmd;
    std::string str = "0";
    int32_t ret = cmd.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_DHID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_DHID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_COMMAND);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_COMMAND_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}

/**
 * @tc.name: Unmarshal_002.
 * @tc.desc: Verify ChannelInfoCmd Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraChannelInfoCmdlTest, Unmarshal_002, TestSize.Level1)
{
    DCameraChannelInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE_BODY_DEVID);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_VALUE_BODY_DEVID_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_VALUE_BODY_DETAIL);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_VALUE_BODY_DETAIL_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_DETAIL_BODY_FLAG);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_DETAIL_BODY_FLAG_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_LACK_DETAIL_BODY_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = cmd.Unmarshal(TEST_CHANNEL_INFO_CMD_JSON_DETAIL_BODY_TYPE_EXCEPTION);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS