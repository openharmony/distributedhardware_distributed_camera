/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
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
#include <string>
#include <vector>

#include "dcamera_metadata_setting_cmd.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

// Constants for metadata setting command testing
namespace TestConstants {
    // Command type constants
    constexpr const char* COMMAND_TYPE_MESSAGE = "MESSAGE";

    // Command string constants
    constexpr const char* COMMAND_UPDATE_METADATA = "UPDATE_METADATA";

    // Camera ID constants
    constexpr const char* CAMERA_ID_META = "camera_meta";
    constexpr const char* CAMERA_ID_EMPTY_META = "camera_empty_meta";
    constexpr const char* CAMERA_ID_SPECIAL_META = "camera_special_meta";
    constexpr const char* CAMERA_ID_REPEAT = "camera_repeat";

    // Setting values
    constexpr const char* SETTING_VALUE_FOCUS_AUTO = "focus=auto";
    constexpr const char* SETTING_VALUE_DISABLE_NIGHT = "disable:night_mode";

    // Setting values for array tests
    constexpr int32_t SETTING_VALUE_A_INDEX = 0;
    constexpr int32_t SETTING_VALUE_B_INDEX = 1;
    constexpr int32_t SETTING_VALUE_C_INDEX = 2;

    // Array size constants
    constexpr size_t ARRAY_SIZE_THREE = 3;

    // Sequential values
    constexpr const char* SEQUENTIAL_VALUE_FIRST = "first";
    constexpr const char* SEQUENTIAL_VALUE_SECOND = "second";
    constexpr const char* SEQUENTIAL_VALUE_THIRD = "third";

    // String field markers
    constexpr const char* MARKER_TYPE = "Type";
    constexpr const char* MARKER_DH_ID = "dhId";
    constexpr const char* MARKER_COMMAND = "Command";
    constexpr const char* MARKER_VALUE = "Value";
} // namespace TestConstants
class DCameraMetadataSettingCmdRoundTripTest : public testing::Test {
public:
    static std::shared_ptr<DCameraSettings> MakeSetting(DCSettingsType type, const std::string &value)
    {
        auto setting = std::make_shared<DCameraSettings>();
        setting->type_ = type;
        setting->value_ = value;
        return setting;
    }

    static DCameraMetadataSettingCmd BuildRichCommand()
    {
        DCameraMetadataSettingCmd cmd;
        cmd.type_ = TestConstants::COMMAND_TYPE_MESSAGE;
        cmd.dhId_ = TestConstants::CAMERA_ID_META;
        cmd.command_ = TestConstants::COMMAND_UPDATE_METADATA;
        cmd.value_.push_back(MakeSetting(UPDATE_METADATA, TestConstants::SETTING_VALUE_FOCUS_AUTO));
        cmd.value_.push_back(MakeSetting(ENABLE_METADATA, "{\"beauty\":1,\"hdr\":0}"));
        cmd.value_.push_back(MakeSetting(DISABLE_METADATA, TestConstants::SETTING_VALUE_DISABLE_NIGHT));
        return cmd;
    }

    static void AssertSettingsEqual(const std::vector<std::shared_ptr<DCameraSettings>> &lhs,
        const std::vector<std::shared_ptr<DCameraSettings>> &rhs)
    {
        ASSERT_EQ(lhs.size(), rhs.size());
        for (size_t index = 0; index < lhs.size(); index++) {
            ASSERT_NE(lhs[index], nullptr);
            ASSERT_NE(rhs[index], nullptr);
            EXPECT_EQ(lhs[index]->type_, rhs[index]->type_);
            EXPECT_EQ(lhs[index]->value_, rhs[index]->value_);
        }
    }

    static std::vector<std::string> BuildInvalidTopLevelPayloads()
    {
        return {
            "not_json",
            R"({})",
            R"({"dhId":"camera_0","Command":"UPDATE_METADATA","Value":[]})",
            R"({"Type":1,"dhId":"camera_0","Command":"UPDATE_METADATA","Value":[]})",
            R"({"Type":"MESSAGE","Command":"UPDATE_METADATA","Value":[]})",
            R"({"Type":"MESSAGE","dhId":1,"Command":"UPDATE_METADATA","Value":[]})",
            R"({"Type":"MESSAGE","dhId":"camera_0","Value":[]})",
            R"({"Type":"MESSAGE","dhId":"camera_0","Command":1,"Value":[]})",
            R"({"Type":"MESSAGE","dhId":"camera_0","Command":"UPDATE_METADATA"})",
            R"({"Type":"MESSAGE","dhId":"camera_0","Command":"UPDATE_METADATA","Value":{}})",
        };
    }

    static std::vector<std::string> BuildInvalidSettingPayloads()
    {
        return {
            R"({
                "Type": "MESSAGE",
                "dhId": "camera_0",
                "Command": "UPDATE_METADATA",
                "Value": [{
                    "SettingValue": "missing_type"
                }]
            })",
            R"({
                "Type": "MESSAGE",
                "dhId": "camera_0",
                "Command": "UPDATE_METADATA",
                "Value": [{
                    "SettingType": "1",
                    "SettingValue": "wrong_type"
                }]
            })",
            R"({
                "Type": "MESSAGE",
                "dhId": "camera_0",
                "Command": "UPDATE_METADATA",
                "Value": [{
                    "SettingType": 1
                }]
            })",
            R"({
                "Type": "MESSAGE",
                "dhId": "camera_0",
                "Command": "UPDATE_METADATA",
                "Value":[{
                    "SettingType": 1,
                    "SettingValue": 1
                }]
            })",
            R"({
                "Type": "MESSAGE",
                "dhId": "camera_0",
                "Command": "UPDATE_METADATA",
                "Value": [{
                    "SettingType": 1,
                    "SettingValue": "valid"
                }, {
                    "SettingValue": "invalid_second"
                }]
            })",
        };
    }
};

/**
 * @tc.name: MarshalAndUnmarshal_001
 * @tc.desc: Verify metadata command round-trip preserves all settings and order.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, MarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraMetadataSettingCmd source = BuildRichCommand();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraMetadataSettingCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    EXPECT_EQ(source.type_, target.type_);
    EXPECT_EQ(source.dhId_, target.dhId_);
    EXPECT_EQ(source.command_, target.command_);
    AssertSettingsEqual(source.value_, target.value_);
}

/**
 * @tc.name: MarshalAndUnmarshal_002
 * @tc.desc: Verify empty metadata settings arrays remain empty after round-trip.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, MarshalAndUnmarshal_002, TestSize.Level1)
{
    DCameraMetadataSettingCmd source;
    source.type_ = TestConstants::COMMAND_TYPE_MESSAGE;
    source.dhId_ = TestConstants::CAMERA_ID_EMPTY_META;
    source.command_ = TestConstants::COMMAND_UPDATE_METADATA;

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraMetadataSettingCmd target;
    ASSERT_NE(DCAMERA_OK, target.Unmarshal(payload));
    EXPECT_TRUE(target.value_.empty());
}

/**
 * @tc.name: MarshalAndUnmarshal_003
 * @tc.desc: Verify metadata payloads preserve special characters and escaped content.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, MarshalAndUnmarshal_003, TestSize.Level1)
{
    DCameraMetadataSettingCmd source;
    source.type_ = TestConstants::COMMAND_TYPE_MESSAGE;
    source.dhId_ = TestConstants::CAMERA_ID_SPECIAL_META;
    source.command_ = TestConstants::COMMAND_UPDATE_METADATA;
    source.value_.push_back(MakeSetting(UPDATE_METADATA, "{\"filter\":\"cinema\",\"strength\":0.75}"));
    source.value_.push_back(MakeSetting(ENABLE_METADATA, "path=/data/local/tmp/meta.json"));
    source.value_.push_back(MakeSetting(DISABLE_METADATA, "quote:\"retained\""));

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraMetadataSettingCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertSettingsEqual(source.value_, target.value_);
}

/**
 * @tc.name: Unmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed top-level metadata payloads are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, Unmarshal_InvalidPayload_001, TestSize.Level1)
{
    const std::vector<std::string> payloads = BuildInvalidTopLevelPayloads();
    for (const auto &payload : payloads) {
        DCameraMetadataSettingCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: Unmarshal_InvalidPayload_002
 * @tc.desc: Verify malformed setting entries are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, Unmarshal_InvalidPayload_002, TestSize.Level1)
{
    const std::vector<std::string> payloads = BuildInvalidSettingPayloads();
    for (const auto &payload : payloads) {
        DCameraMetadataSettingCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: Unmarshal_ValidPayload_001
 * @tc.desc: Verify valid multi-entry metadata payloads decode exactly.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, Unmarshal_ValidPayload_001, TestSize.Level1)
{
    std::string payload = R"({
        "Type": "MESSAGE",
        "dhId": "camera_valid_meta",
        "Command": "UPDATE_METADATA",
        "Value": [{
            "SettingType": 1,
            "SettingValue": "value_a"
        }, {
            "SettingType": 2,
            "SettingValue": "value_b"
        }, {
            "SettingType": 3,
            "SettingValue": "value_c"
        }]
    })";

    DCameraMetadataSettingCmd cmd;
    ASSERT_EQ(DCAMERA_OK, cmd.Unmarshal(payload));
    ASSERT_EQ(TestConstants::ARRAY_SIZE_THREE, cmd.value_.size());
    EXPECT_EQ("value_a", cmd.value_[TestConstants::SETTING_VALUE_A_INDEX]->value_);
    EXPECT_EQ("value_b", cmd.value_[TestConstants::SETTING_VALUE_B_INDEX]->value_);
    EXPECT_EQ("value_c", cmd.value_[TestConstants::SETTING_VALUE_C_INDEX]->value_);
}

/**
 * @tc.name: Marshal_001
 * @tc.desc: Verify metadata marshal output contains required protocol markers.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, Marshal_001, TestSize.Level1)
{
    DCameraMetadataSettingCmd cmd = BuildRichCommand();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, cmd.Marshal(payload));

    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_TYPE));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_DH_ID));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_COMMAND));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_VALUE));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::SETTING_VALUE_FOCUS_AUTO));
}

/**
 * @tc.name: MarshalAndUnmarshal_004
 * @tc.desc: Verify repeated setting types are preserved in metadata commands.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, MarshalAndUnmarshal_004, TestSize.Level1)
{
    DCameraMetadataSettingCmd source;
    source.type_ = TestConstants::COMMAND_TYPE_MESSAGE;
    source.dhId_ = TestConstants::CAMERA_ID_REPEAT;
    source.command_ = TestConstants::COMMAND_UPDATE_METADATA;
    source.value_.push_back(MakeSetting(UPDATE_METADATA, TestConstants::SEQUENTIAL_VALUE_FIRST));
    source.value_.push_back(MakeSetting(UPDATE_METADATA, TestConstants::SEQUENTIAL_VALUE_SECOND));
    source.value_.push_back(MakeSetting(UPDATE_METADATA, TestConstants::SEQUENTIAL_VALUE_THIRD));

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraMetadataSettingCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    ASSERT_EQ(TestConstants::ARRAY_SIZE_THREE, target.value_.size());
    EXPECT_EQ(TestConstants::SEQUENTIAL_VALUE_FIRST, target.value_[TestConstants::SETTING_VALUE_A_INDEX]->value_);
    EXPECT_EQ(TestConstants::SEQUENTIAL_VALUE_SECOND, target.value_[TestConstants::SETTING_VALUE_B_INDEX]->value_);
    EXPECT_EQ(TestConstants::SEQUENTIAL_VALUE_THIRD, target.value_[TestConstants::SETTING_VALUE_C_INDEX]->value_);
}

/**
 * @tc.name: Unmarshal_InvalidPayload_003
 * @tc.desc: Verify mixed valid and invalid setting entries are rejected as a whole.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraMetadataSettingCmdRoundTripTest, Unmarshal_InvalidPayload_003, TestSize.Level1)
{
    std::string payload = R"({
        "Type": "MESSAGE",
        "dhId": "camera_partial_invalid",
        "Command": "UPDATE_METADATA",
        "Value": [{
            "SettingType": 1,
            "SettingValue": "ok"
        }, {
            "SettingType": 2,
            "SettingValue": "also_ok"
        }, {
            "SettingType": "3",
            "SettingValue": "bad"
        }]
    })";

    DCameraMetadataSettingCmd cmd;
    EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
}
} // namespace DistributedHardware
} // namespace OHOS
