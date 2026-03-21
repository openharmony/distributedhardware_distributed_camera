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

#include "dcamera_channel_info_cmd.h"
#include "dcamera_event_cmd.h"
#include "dcamera_info_cmd.h"
#include "dcamera_open_info_cmd.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_constants.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraControlCommandsRoundTripTest : public testing::Test {
public:
    static DCameraInfoCmd BuildInfoCmd()
    {
        DCameraInfoCmd cmd;
        cmd.type_ = "MESSAGE";
        cmd.dhId_ = "camera_info";
        cmd.command_ = "GET_INFO";
        cmd.value_ = std::make_shared<DCameraInfo>();
        cmd.value_->state_ = DCAMERA_LOCAL_OPEN;
        return cmd;
    }

    static DCameraEventCmd BuildEventCmd()
    {
        DCameraEventCmd cmd;
        cmd.type_ = "MESSAGE";
        cmd.dhId_ = "camera_event";
        cmd.command_ = "STATE_NOTIFY";
        cmd.value_ = std::make_shared<DCameraEvent>();
        cmd.value_->eventType_ = DCAMERA_MESSAGE;
        cmd.value_->eventResult_ = DCAMERA_EVENT_CAMERA_SUCCESS;
        cmd.value_->eventContent_ = "capture_started";
        return cmd;
    }

    static DCameraOpenInfoCmd BuildOpenCmd()
    {
        DCameraOpenInfoCmd cmd;
        cmd.type_ = "OPERATION";
        cmd.dhId_ = "camera_open";
        cmd.command_ = "OPEN_CHANNEL";
        cmd.value_ = std::make_shared<DCameraOpenInfo>("source_network_id");
        return cmd;
    }

    static DCameraChannelInfoCmd BuildChannelCmd()
    {
        DCameraChannelInfoCmd cmd;
        cmd.type_ = "MESSAGE";
        cmd.dhId_ = "camera_channel";
        cmd.command_ = "CHANNEL_NEG";
        cmd.value_ = std::make_shared<DCameraChannelInfo>();
        cmd.value_->sourceDevId_ = "source_network_id";
        cmd.value_->detail_.push_back(DCameraChannelDetail("dataContinue", CONTINUOUS_FRAME));
        cmd.value_->detail_.push_back(DCameraChannelDetail("dataSnapshot", SNAPSHOT_FRAME));
        return cmd;
    }

    static void AssertInfoCmdEqual(const DCameraInfoCmd &lhs, const DCameraInfoCmd &rhs)
    {
        EXPECT_EQ(lhs.type_, rhs.type_);
        EXPECT_EQ(lhs.dhId_, rhs.dhId_);
        EXPECT_EQ(lhs.command_, rhs.command_);
        ASSERT_NE(lhs.value_, nullptr);
        ASSERT_NE(rhs.value_, nullptr);
        EXPECT_EQ(lhs.value_->state_, rhs.value_->state_);
    }

    static void AssertEventCmdEqual(const DCameraEventCmd &lhs, const DCameraEventCmd &rhs)
    {
        EXPECT_EQ(lhs.type_, rhs.type_);
        EXPECT_EQ(lhs.dhId_, rhs.dhId_);
        EXPECT_EQ(lhs.command_, rhs.command_);
        ASSERT_NE(lhs.value_, nullptr);
        ASSERT_NE(rhs.value_, nullptr);
        EXPECT_EQ(lhs.value_->eventType_, rhs.value_->eventType_);
        EXPECT_EQ(lhs.value_->eventResult_, rhs.value_->eventResult_);
        EXPECT_EQ(lhs.value_->eventContent_, rhs.value_->eventContent_);
    }

    static void AssertOpenCmdEqual(const DCameraOpenInfoCmd &lhs, const DCameraOpenInfoCmd &rhs)
    {
        EXPECT_EQ(lhs.type_, rhs.type_);
        EXPECT_EQ(lhs.dhId_, rhs.dhId_);
        EXPECT_EQ(lhs.command_, rhs.command_);
        ASSERT_NE(lhs.value_, nullptr);
        ASSERT_NE(rhs.value_, nullptr);
        EXPECT_EQ(lhs.value_->sourceDevId_, rhs.value_->sourceDevId_);
    }

    static void AssertChannelCmdEqual(const DCameraChannelInfoCmd &lhs, const DCameraChannelInfoCmd &rhs)
    {
        EXPECT_EQ(lhs.type_, rhs.type_);
        EXPECT_EQ(lhs.dhId_, rhs.dhId_);
        EXPECT_EQ(lhs.command_, rhs.command_);
        ASSERT_NE(lhs.value_, nullptr);
        ASSERT_NE(rhs.value_, nullptr);
        EXPECT_EQ(lhs.value_->sourceDevId_, rhs.value_->sourceDevId_);
        ASSERT_EQ(lhs.value_->detail_.size(), rhs.value_->detail_.size());
        for (size_t index = 0; index < lhs.value_->detail_.size(); index++) {
            EXPECT_EQ(lhs.value_->detail_[index].dataSessionFlag_, rhs.value_->detail_[index].dataSessionFlag_);
            EXPECT_EQ(lhs.value_->detail_[index].streamType_, rhs.value_->detail_[index].streamType_);
        }
    }

    static std::vector<std::string> GetInfoBasicFormatErrors()
    {
        return {
            "not_json",
            R"({})"
        };
    }

    static std::vector<std::string> GetInfoTopLevelFieldErrors()
    {
        return {
            R"({
            "dhId": "camera_0",
            "Command": "GET_INFO",
            "Value": {"State": 1}
        })",
            R"({
            "Type": 1,
            "dhId": "camera_0",
            "Command": "GET_INFO",
            "Value": {"State": 1}
        })",
            R"({
            "Type": "MESSAGE",
            "Command": "GET_INFO",
            "Value": {"State": 1}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": 1,
            "Command": "GET_INFO",
            "Value": {"State": 1}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Value": {"State": 1}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": 1,
            "Value": {"State": 1}
        })"
        };
    }

    static std::vector<std::string> GetInfoValueFieldErrors()
    {
        return {
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "GET_INFO"
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "GET_INFO",
            "Value": []
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "GET_INFO",
            "Value": {}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "GET_INFO",
            "Value": {"State": "1"}
        })"
        };
    }

    static std::vector<std::string> GetInfoInvalidPayloads()
    {
        std::vector<std::string> result;
        auto basic = GetInfoBasicFormatErrors();
        auto topLevel = GetInfoTopLevelFieldErrors();
        auto value = GetInfoValueFieldErrors();
        result.insert(result.end(), basic.begin(), basic.end());
        result.insert(result.end(), topLevel.begin(), topLevel.end());
        result.insert(result.end(), value.begin(), value.end());
        return result;
    }

    static void VerifyInfoPayloadsRejected(const std::vector<std::string> &payloads)
    {
        for (const auto &payload : payloads) {
            DCameraInfoCmd cmd;
            EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
        }
    }

    static std::vector<std::string> GetEventBasicFormatErrors()
    {
        return {
            "not_json",
            R"({})"
        };
    }

    static std::vector<std::string> GetEventTopLevelFieldErrors()
    {
        return {
            R"({
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": 1,
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": 1,
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": 1,
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": "ok"}
        })"
        };
    }

    static std::vector<std::string> GetEventValueFieldErrors()
    {
        return {
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY"
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": []
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": "1", "EventResult": 1, "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventContent": "ok"}
        })"
        };
    }

    static std::vector<std::string> GetEventValueNestedFieldErrors()
    {
        return {
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": "1", "EventContent": "ok"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "STATE_NOTIFY",
            "Value": {"EventType": 1, "EventResult": 1, "EventContent": 0}
        })"
        };
    }

    static std::vector<std::string> GetEventInvalidPayloads()
    {
        std::vector<std::string> result;
        auto basic = GetEventBasicFormatErrors();
        auto topLevel = GetEventTopLevelFieldErrors();
        auto value = GetEventValueFieldErrors();
        auto nested = GetEventValueNestedFieldErrors();
        result.insert(result.end(), basic.begin(), basic.end());
        result.insert(result.end(), topLevel.begin(), topLevel.end());
        result.insert(result.end(), value.begin(), value.end());
        result.insert(result.end(), nested.begin(), nested.end());
        return result;
    }

    static void VerifyEventPayloadsRejected(const std::vector<std::string> &payloads)
    {
        for (const auto &payload : payloads) {
            DCameraEventCmd cmd;
            EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
        }
    }

    static std::vector<std::string> GetOpenInvalidPayloads()
    {
        return {
            "not_json",
            R"({})",
            R"({ "dhId": "camera_0", "Command": "OPEN_CHANNEL", "Value": {"SourceDevId": "source"} })",
            R"({ "Type": 0, "dhId": "camera_0", "Command": "OPEN_CHANNEL", "Value": {"SourceDevId": "source"} })",
            R"({ "Type": "OPERATION", "Command": "OPEN_CHANNEL", "Value": {"SourceDevId": "source"} })",
            R"({ "Type": "OPERATION", "dhId": 1, "Command": "OPEN_CHANNEL", "Value": {"SourceDevId": "source"} })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Value": {"SourceDevId": "source"} })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Command": 1, "Value": {"SourceDevId": "source"} })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Command": "OPEN_CHANNEL" })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Command": "OPEN_CHANNEL", "Value": [] })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Command": "OPEN_CHANNEL", "Value": {} })",
            R"({ "Type": "OPERATION", "dhId": "camera_0", "Command": "OPEN_CHANNEL", "Value": {"SourceDevId": 1} })"
        };
    }

    static void VerifyOpenPayloadsRejected(const std::vector<std::string> &payloads)
    {
        for (const auto &payload : payloads) {
            DCameraOpenInfoCmd cmd;
            EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
        }
    }

    static std::vector<std::string> GetChannelBasicFormatErrors()
    {
        return {
            "not_json",
            R"({})"
        };
    }

    static std::vector<std::string> GetChannelTopLevelFieldErrors()
    {
        return {
            R"({
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": []}
        })",
            R"({
            "Type": 1,
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": 1,
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Value": {"SourceDevId": "source", "Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": 1,
            "Value": {"SourceDevId": "source", "Detail": []}
        })"
        };
    }

    static std::vector<std::string> GetChannelValueFieldErrors()
    {
        return {
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": []
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": 1, "Detail": []}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source"}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": "not_array"}
        })"
        };
    }

    static std::vector<std::string> GetChannelDetailItemErrors()
    {
        return {
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": [{
                "StreamType": 1
            }]}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": [{
                "DataSessionFlag": 1,
                "StreamType": 1
            }]}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": [{
                "DataSessionFlag": "flag"
            }]}
        })",
            R"({
            "Type": "MESSAGE",
            "dhId": "camera_0",
            "Command": "CHANNEL_NEG",
            "Value": {"SourceDevId": "source", "Detail": [{
                "DataSessionFlag": "flag",
                "StreamType": "1"
            }]}
        })"
        };
    }

    static std::vector<std::string> GetChannelInvalidPayloads()
    {
        std::vector<std::string> result;
        auto basic = GetChannelBasicFormatErrors();
        auto topLevel = GetChannelTopLevelFieldErrors();
        auto value = GetChannelValueFieldErrors();
        auto detail = GetChannelDetailItemErrors();
        result.insert(result.end(), basic.begin(), basic.end());
        result.insert(result.end(), topLevel.begin(), topLevel.end());
        result.insert(result.end(), value.begin(), value.end());
        result.insert(result.end(), detail.begin(), detail.end());
        return result;
    }

    static void VerifyChannelPayloadsRejected(const std::vector<std::string> &payloads)
    {
        for (const auto &payload : payloads) {
            DCameraChannelInfoCmd cmd;
            EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
        }
    }
};

/**
 * @tc.name: InfoMarshalAndUnmarshal_001
 * @tc.desc: Verify info command round-trip preserves state.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, InfoMarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraInfoCmd source = BuildInfoCmd();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraInfoCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertInfoCmdEqual(source, target);
}

/**
 * @tc.name: InfoMarshalAndUnmarshal_002
 * @tc.desc: Verify marshal fails when info value is null.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, InfoMarshalAndUnmarshal_002, TestSize.Level1)
{
    DCameraInfoCmd cmd;
    cmd.type_ = "MESSAGE";
    cmd.dhId_ = "camera_info";
    cmd.command_ = "GET_INFO";
    std::string payload;
    EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Marshal(payload));
}

/**
 * @tc.name: InfoUnmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed info payloads are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, InfoUnmarshal_InvalidPayload_001, TestSize.Level1)
{
    std::vector<std::string> payloads = GetInfoInvalidPayloads();
    for (const auto &payload : payloads) {
        DCameraInfoCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: EventMarshalAndUnmarshal_001
 * @tc.desc: Verify event command round-trip preserves content.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, EventMarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraEventCmd source = BuildEventCmd();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraEventCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertEventCmdEqual(source, target);
}

/**
 * @tc.name: EventMarshalAndUnmarshal_002
 * @tc.desc: Verify marshal fails when event value is null.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, EventMarshalAndUnmarshal_002, TestSize.Level1)
{
    DCameraEventCmd cmd;
    cmd.type_ = "MESSAGE";
    cmd.dhId_ = "camera_event";
    cmd.command_ = "STATE_NOTIFY";
    std::string payload;
    EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Marshal(payload));
}

/**
 * @tc.name: EventMarshalAndUnmarshal_003
 * @tc.desc: Verify event round-trip preserves special characters in content.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, EventMarshalAndUnmarshal_003, TestSize.Level1)
{
    DCameraEventCmd source;
    source.type_ = "MESSAGE";
    source.dhId_ = "camera_event_special";
    source.command_ = "STATE_NOTIFY";
    source.value_ = std::make_shared<DCameraEvent>();
    source.value_->eventType_ = DCAMERA_OPERATION;
    source.value_->eventResult_ = DCAMERA_EVENT_UPDATE_SETTINGS_ERROR;
    source.value_->eventContent_ = "detail={\"reason\":\"bad_meta\",\"path\":\"/data/test\"}";

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraEventCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertEventCmdEqual(source, target);
}

/**
 * @tc.name: EventUnmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed event payloads are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, EventUnmarshal_InvalidPayload_001, TestSize.Level1)
{
    std::vector<std::string> payloads = GetEventInvalidPayloads();
    for (const auto &payload : payloads) {
        DCameraEventCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: OpenMarshalAndUnmarshal_001
 * @tc.desc: Verify open command round-trip preserves source device id.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, OpenMarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraOpenInfoCmd source = BuildOpenCmd();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraOpenInfoCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertOpenCmdEqual(source, target);
}

/**
 * @tc.name: OpenMarshalAndUnmarshal_002
 * @tc.desc: Verify marshal fails when open info value is null.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, OpenMarshalAndUnmarshal_002, TestSize.Level1)
{
    DCameraOpenInfoCmd cmd;
    cmd.type_ = "OPERATION";
    cmd.dhId_ = "camera_open";
    cmd.command_ = "OPEN_CHANNEL";
    std::string payload;
    EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Marshal(payload));
}

/**
 * @tc.name: OpenMarshalAndUnmarshal_003
 * @tc.desc: Verify open commands preserve structured source ids.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, OpenMarshalAndUnmarshal_003, TestSize.Level1)
{
    DCameraOpenInfoCmd source;
    source.type_ = "OPERATION";
    source.dhId_ = "camera_open_structured";
    source.command_ = "OPEN_CHANNEL";
    source.value_ = std::make_shared<DCameraOpenInfo>("source:network:id:01");

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraOpenInfoCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertOpenCmdEqual(source, target);
}

/**
 * @tc.name: OpenUnmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed open payloads are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, OpenUnmarshal_InvalidPayload_001, TestSize.Level1)
{
    std::vector<std::string> payloads = GetOpenInvalidPayloads();
    for (const auto &payload : payloads) {
        DCameraOpenInfoCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: ChannelMarshalAndUnmarshal_001
 * @tc.desc: Verify channel command round-trip preserves detail ordering.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, ChannelMarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraChannelInfoCmd source = BuildChannelCmd();
    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraChannelInfoCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertChannelCmdEqual(source, target);
}

/**
 * @tc.name: ChannelMarshalAndUnmarshal_002
 * @tc.desc: Verify marshal fails when channel info value is null.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, ChannelMarshalAndUnmarshal_002, TestSize.Level1)
{
    DCameraChannelInfoCmd cmd;
    cmd.type_ = "MESSAGE";
    cmd.dhId_ = "camera_channel";
    cmd.command_ = "CHANNEL_NEG";
    std::string payload;
    EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Marshal(payload));
}

/**
 * @tc.name: ChannelMarshalAndUnmarshal_003
 * @tc.desc: Verify channel commands preserve longer detail lists.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, ChannelMarshalAndUnmarshal_003, TestSize.Level1)
{
    DCameraChannelInfoCmd source;
    source.type_ = "MESSAGE";
    source.dhId_ = "camera_channel_long";
    source.command_ = "CHANNEL_NEG";
    source.value_ = std::make_shared<DCameraChannelInfo>();
    source.value_->sourceDevId_ = "source_network_id_long";
    source.value_->detail_.push_back(DCameraChannelDetail("dataContinue", CONTINUOUS_FRAME));
    source.value_->detail_.push_back(DCameraChannelDetail("dataSnapshot", SNAPSHOT_FRAME));
    source.value_->detail_.push_back(DCameraChannelDetail("dataContinue_aux", CONTINUOUS_FRAME));

    std::string payload;
    ASSERT_EQ(DCAMERA_OK, source.Marshal(payload));

    DCameraChannelInfoCmd target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertChannelCmdEqual(source, target);
}

/**
 * @tc.name: ChannelUnmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed channel payloads are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, ChannelUnmarshal_InvalidPayload_001, TestSize.Level1)
{
    std::vector<std::string> payloads = GetChannelInvalidPayloads();

    for (const auto &payload : payloads) {
        DCameraChannelInfoCmd cmd;
        EXPECT_EQ(DCAMERA_BAD_VALUE, cmd.Unmarshal(payload));
    }
}

/**
 * @tc.name: Marshal_001
 * @tc.desc: Verify control command marshal output contains expected markers.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraControlCommandsRoundTripTest, Marshal_001, TestSize.Level1)
{
    std::string infoPayload;
    std::string eventPayload;
    std::string openPayload;
    std::string channelPayload;

    ASSERT_EQ(DCAMERA_OK, BuildInfoCmd().Marshal(infoPayload));
    ASSERT_EQ(DCAMERA_OK, BuildEventCmd().Marshal(eventPayload));
    ASSERT_EQ(DCAMERA_OK, BuildOpenCmd().Marshal(openPayload));
    ASSERT_EQ(DCAMERA_OK, BuildChannelCmd().Marshal(channelPayload));

    EXPECT_NE(std::string::npos, infoPayload.find("\"State\""));
    EXPECT_NE(std::string::npos, eventPayload.find("\"EventContent\""));
    EXPECT_NE(std::string::npos, openPayload.find("\"SourceDevId\""));
    EXPECT_NE(std::string::npos, channelPayload.find("\"Detail\""));
    EXPECT_NE(std::string::npos, channelPayload.find("dataSnapshot"));
}
} // namespace DistributedHardware
} // namespace OHOS
