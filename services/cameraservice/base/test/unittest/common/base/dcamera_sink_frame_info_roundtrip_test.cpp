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

#include <string>
#include <vector>

#include "dcamera_sink_frame_info.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {

// Constants for frame info testing
namespace TestConstants {
    // Frame type constants
    constexpr int32_t DEFAULT_FRAME_TYPE = -1;
    constexpr int32_t TEST_FRAME_TYPE_1 = 1;
    constexpr int32_t TEST_FRAME_TYPE_0 = 0;

    // Frame index constants
    constexpr int32_t DEFAULT_FRAME_INDEX = -1;
    constexpr int32_t TEST_FRAME_INDEX_7 = 7;
    constexpr int32_t TEST_FRAME_INDEX_2 = 2;
    constexpr int32_t TEST_FRAME_INDEX_10 = 10;
    constexpr int32_t TEST_FRAME_INDEX_11 = 11;
    constexpr int32_t TEST_FRAME_INDEX_1 = 1;

    // Timestamp constants
    constexpr int64_t TEST_PTS = 12345;
    constexpr int64_t TEST_START_ENCODE_TIME = 22345;
    constexpr int64_t TEST_FINISH_ENCODE_TIME = 32345;
    constexpr int64_t TEST_SEND_TIME = 42345;
    constexpr int64_t DEFAULT_TIMESTAMP = 0;

    // JSON timestamp test values
    constexpr int64_t TEST_JSON_PTS = 3;
    constexpr int64_t TEST_JSON_START_ENCODE_TIME = 4;
    constexpr int64_t TEST_JSON_FINISH_ENCODE_TIME = 5;
    constexpr int64_t TEST_JSON_SEND_TIME = 6;
    constexpr int64_t TEST_JSON_IMU_PTS_1 = 20;
    constexpr int64_t TEST_JSON_IMU_START_ENCODE_TIME = 30;
    constexpr int64_t TEST_JSON_IMU_FINISH_ENCODE_TIME = 40;
    constexpr int64_t TEST_JSON_IMU_SEND_TIME = 50;
    constexpr int64_t TEST_JSON_RAW_PTS = 21;
    constexpr int64_t TEST_JSON_RAW_START_ENCODE_TIME = 31;
    constexpr int64_t TEST_JSON_RAW_FINISH_ENCODE_TIME = 41;
    constexpr int64_t TEST_JSON_RAW_SEND_TIME = 51;

    // Version constants
    const std::string VERSION_1_0 = "1.0";
    const std::string VERSION_2_0 = "2.0";

    // RawTime JSON values
    constexpr int32_t RAW_TIME_CAPTURE = 100;
    constexpr int32_t RAW_TIME_ENCODE = 200;

    // IMU test values
    constexpr int32_t IMU_GYRO_X = 1;
    constexpr int32_t IMU_GYRO_Y = 2;
    constexpr int32_t IMU_GYRO_Z = 3;
    constexpr int32_t IMU_ACC_X = 4;
    constexpr int32_t IMU_ACC_Y = 5;
    constexpr int32_t IMU_ACC_Z = 6;

    // String field markers
    const std::string MARKER_TYPE = "type";
    const std::string MARKER_INDEX = "index";
    const std::string MARKER_PTS = "pts";
    const std::string MARKER_START_ENCODE_T = "startEncodeT";
    const std::string MARKER_FINISH_ENCODE_T = "finishEncodeT";
    const std::string MARKER_SEND_T = "sendT";
    const std::string MARKER_VER = "ver";
    const std::string MARKER_RAW_TIME = "rawTime";
    const std::string MARKER_IMU_INFO = "imuInfo";

    // Invalid test payload index
    constexpr int32_t INVALID_FIELD_VALUE_10 = 10;
} // namespace TestConstants
class DCameraSinkFrameInfoRoundTripTest : public testing::Test {
public:
    struct FramePayloadFields {
        std::string type;
        std::string index;
        std::string pts;
        std::string startEncodeT;
        std::string finishEncodeT;
        std::string sendT;
        std::string ver;
        std::string rawTime;
        std::string imuInfo;
    };

    static DCameraSinkFrameInfo BuildFrameInfo()
    {
        DCameraSinkFrameInfo frameInfo;
        frameInfo.type_ = TestConstants::TEST_FRAME_TYPE_1;
        frameInfo.index_ = TestConstants::TEST_FRAME_INDEX_7;
        frameInfo.pts_ = TestConstants::TEST_PTS;
        frameInfo.startEncodeT_ = TestConstants::TEST_START_ENCODE_TIME;
        frameInfo.finishEncodeT_ = TestConstants::TEST_FINISH_ENCODE_TIME;
        frameInfo.sendT_ = TestConstants::TEST_SEND_TIME;
        frameInfo.ver_ = TestConstants::VERSION_2_0;
        frameInfo.rawTime_ = "{\"capture\":" + std::to_string(TestConstants::RAW_TIME_CAPTURE) +
                              ",\"encode\":" + std::to_string(TestConstants::RAW_TIME_ENCODE) + "}";
        frameInfo.imuInfo_ = "{\"gyro\":[" + std::to_string(TestConstants::IMU_GYRO_X) + "," +
                             std::to_string(TestConstants::IMU_GYRO_Y) + "," +
                             std::to_string(TestConstants::IMU_GYRO_Z) + "],\"acc\":[" +
                             std::to_string(TestConstants::IMU_ACC_X) + "," +
                             std::to_string(TestConstants::IMU_ACC_Y) + "," +
                             std::to_string(TestConstants::IMU_ACC_Z) + "]}";
        return frameInfo;
    }

    static void AssertFrameInfoEqual(const DCameraSinkFrameInfo &lhs, const DCameraSinkFrameInfo &rhs)
    {
        EXPECT_EQ(lhs.type_, rhs.type_);
        EXPECT_EQ(lhs.index_, rhs.index_);
        EXPECT_EQ(lhs.pts_, rhs.pts_);
        EXPECT_EQ(lhs.startEncodeT_, rhs.startEncodeT_);
        EXPECT_EQ(lhs.finishEncodeT_, rhs.finishEncodeT_);
        EXPECT_EQ(lhs.sendT_, rhs.sendT_);
        EXPECT_EQ(lhs.ver_, rhs.ver_);
        EXPECT_EQ(lhs.rawTime_, rhs.rawTime_);
        EXPECT_EQ(lhs.imuInfo_, rhs.imuInfo_);
    }

    static std::string Quote(const std::string &value)
    {
        return "\"" + value + "\"";
    }

    static FramePayloadFields BuildDefaultRequiredPayloadFields()
    {
        return {
            std::to_string(TestConstants::TEST_FRAME_TYPE_0),
            std::to_string(TestConstants::TEST_FRAME_INDEX_1),
            std::to_string(TestConstants::DEFAULT_TIMESTAMP),
            std::to_string(TestConstants::DEFAULT_TIMESTAMP),
            std::to_string(TestConstants::DEFAULT_TIMESTAMP),
            std::to_string(TestConstants::DEFAULT_TIMESTAMP),
            Quote("v1"),
            "",
            ""
        };
    }

    static std::string BuildTestPayload(const FramePayloadFields &fields)
    {
        std::string payload = R"({"type":)" + fields.type + R"(,"index":)" + fields.index +
            R"(,"pts":)" + fields.pts + R"(,"startEncodeT":)" + fields.startEncodeT +
            R"(,"finishEncodeT":)" + fields.finishEncodeT + R"(,"sendT":)" + fields.sendT +
            R"(,"ver":)" + fields.ver;

        if (!fields.rawTime.empty()) {
            payload += R"(,"rawTime":)" + fields.rawTime;
        }
        if (!fields.imuInfo.empty()) {
            payload += R"(,"imuInfo":)" + fields.imuInfo;
        }

        payload += "}";
        return payload;
    }

    static std::vector<std::string> BuildInvalidRequiredPayloads()
    {
        const FramePayloadFields base = BuildDefaultRequiredPayloadFields();
        return {
            "not_json",
            R"({})",
            BuildTestPayload({ R"("bad")", base.index, base.pts, base.startEncodeT, base.finishEncodeT, base.sendT,
                base.ver, "", "" }),
            BuildTestPayload({ base.type, R"("bad")", base.pts, base.startEncodeT, base.finishEncodeT, base.sendT,
                base.ver, "", "" }),
            BuildTestPayload({ base.type, base.index, R"("bad")", base.startEncodeT, base.finishEncodeT, base.sendT,
                base.ver, "", "" }),
            BuildTestPayload({ base.type, base.index, base.pts, R"("bad")", base.finishEncodeT, base.sendT,
                base.ver, "", "" }),
            BuildTestPayload({ base.type, base.index, base.pts, base.startEncodeT, R"("bad")", base.sendT,
                base.ver, "", "" }),
            BuildTestPayload({ base.type, base.index, base.pts, base.startEncodeT, base.finishEncodeT, R"("bad")",
                base.ver, "", "" }),
            BuildTestPayload({ base.type, base.index, base.pts, base.startEncodeT, base.finishEncodeT, base.sendT,
                "1", "", "" }),
        };
    }
};

/**
 * @tc.name: MarshalAndUnmarshal_001
 * @tc.desc: Verify sink frame info round-trip preserves all populated fields.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, MarshalAndUnmarshal_001, TestSize.Level1)
{
    DCameraSinkFrameInfo source = BuildFrameInfo();
    std::string payload;
    source.Marshal(payload);
    ASSERT_FALSE(payload.empty());

    DCameraSinkFrameInfo target;
    ASSERT_EQ(DCAMERA_OK, target.Unmarshal(payload));
    AssertFrameInfoEqual(source, target);
}

/**
 * @tc.name: MarshalAndUnmarshal_002
 * @tc.desc: Verify missing optional fields fall back to expected defaults.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, MarshalAndUnmarshal_002, TestSize.Level1)
{
    std::string payload = BuildTestPayload({
        std::to_string(TestConstants::TEST_FRAME_TYPE_1),
        std::to_string(TestConstants::TEST_FRAME_INDEX_2),
        std::to_string(TestConstants::TEST_JSON_PTS),
        std::to_string(TestConstants::TEST_JSON_START_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_FINISH_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_SEND_TIME),
        Quote(TestConstants::VERSION_1_0),
        "",
        ""
    });

    DCameraSinkFrameInfo frame;
    ASSERT_EQ(DCAMERA_OK, frame.Unmarshal(payload));
    EXPECT_EQ(TestConstants::TEST_FRAME_TYPE_1, frame.type_);
    EXPECT_EQ(TestConstants::TEST_FRAME_INDEX_2, frame.index_);
    EXPECT_EQ(TestConstants::TEST_JSON_PTS, frame.pts_);
    EXPECT_EQ(TestConstants::TEST_JSON_START_ENCODE_TIME, frame.startEncodeT_);
    EXPECT_EQ(TestConstants::TEST_JSON_FINISH_ENCODE_TIME, frame.finishEncodeT_);
    EXPECT_EQ(TestConstants::TEST_JSON_SEND_TIME, frame.sendT_);
    EXPECT_EQ(TestConstants::VERSION_1_0, frame.ver_);
    EXPECT_EQ("", frame.rawTime_);
    EXPECT_EQ("", frame.imuInfo_);
}

/**
 * @tc.name: MarshalAndUnmarshal_003
 * @tc.desc: Verify IMU-only and rawTime-only optional payloads both decode correctly.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, MarshalAndUnmarshal_003, TestSize.Level1)
{
    std::string imupayload = BuildTestPayload({
        std::to_string(TestConstants::TEST_FRAME_TYPE_0),
        std::to_string(TestConstants::TEST_FRAME_INDEX_10),
        std::to_string(TestConstants::TEST_JSON_IMU_PTS_1),
        std::to_string(TestConstants::TEST_JSON_IMU_START_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_IMU_FINISH_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_IMU_SEND_TIME),
        Quote("imu_only"),
        "",
        "{\"gyro\":[0.1,0.2,0.3]}"
    });

    std::string rawPayload = BuildTestPayload({
        std::to_string(TestConstants::TEST_FRAME_TYPE_0),
        std::to_string(TestConstants::TEST_FRAME_INDEX_11),
        std::to_string(TestConstants::TEST_JSON_RAW_PTS),
        std::to_string(TestConstants::TEST_JSON_RAW_START_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_RAW_FINISH_ENCODE_TIME),
        std::to_string(TestConstants::TEST_JSON_RAW_SEND_TIME),
        Quote("raw_only"),
        "{\"capture\":1,\"send\":2}",
        ""
    });

    std::vector<std::string> payloads = { imupayload, rawPayload };

    for (const auto &payload : payloads) {
        DCameraSinkFrameInfo frame;
        frame.Unmarshal(payload);
        EXPECT_FALSE(frame.ver_.empty());
    }
}

/**
 * @tc.name: Unmarshal_InvalidPayload_001
 * @tc.desc: Verify malformed required sink frame fields are rejected.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, Unmarshal_InvalidPayload_001, TestSize.Level1)
{
    const std::vector<std::string> payloads = BuildInvalidRequiredPayloads();
    for (const auto &payload : payloads) {
        DCameraSinkFrameInfo frame;
        EXPECT_EQ(DCAMERA_BAD_VALUE, frame.Unmarshal(payload));
    }
}

/**
 * @tc.name: Unmarshal_InvalidPayload_002
 * @tc.desc: Verify malformed optional rawTime fields are rejected while IMU stays optional.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, Unmarshal_InvalidPayload_002, TestSize.Level1)
{
    FramePayloadFields fields = BuildDefaultRequiredPayloadFields();
    fields.rawTime = std::to_string(TestConstants::INVALID_FIELD_VALUE_10);
    std::string payload = BuildTestPayload(fields);

    DCameraSinkFrameInfo frame;
    EXPECT_EQ(DCAMERA_BAD_VALUE, frame.Unmarshal(payload));
}

/**
 * @tc.name: Marshal_001
 * @tc.desc: Verify sink frame info marshal emits all expected field markers.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, Marshal_001, TestSize.Level1)
{
    DCameraSinkFrameInfo frame = BuildFrameInfo();
    std::string payload;
    frame.Marshal(payload);

    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_TYPE));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_INDEX));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_PTS));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_START_ENCODE_T));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_FINISH_ENCODE_T));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_SEND_T));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_VER));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_RAW_TIME));
    EXPECT_NE(std::string::npos, payload.find(TestConstants::MARKER_IMU_INFO));
}

/**
 * @tc.name: ConstructorDefaults_001
 * @tc.desc: Verify default constructor establishes stable default values.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DCameraSinkFrameInfoRoundTripTest, ConstructorDefaults_001, TestSize.Level1)
{
    DCameraSinkFrameInfo frame;
    EXPECT_EQ(TestConstants::DEFAULT_FRAME_TYPE, frame.type_);
    EXPECT_EQ(TestConstants::DEFAULT_FRAME_INDEX, frame.index_);
    EXPECT_EQ(TestConstants::DEFAULT_TIMESTAMP, frame.pts_);
    EXPECT_EQ(TestConstants::DEFAULT_TIMESTAMP, frame.startEncodeT_);
    EXPECT_EQ(TestConstants::DEFAULT_TIMESTAMP, frame.finishEncodeT_);
    EXPECT_EQ(TestConstants::DEFAULT_TIMESTAMP, frame.sendT_);
    EXPECT_EQ(TestConstants::VERSION_1_0, frame.ver_);
    EXPECT_EQ("", frame.rawTime_);
    EXPECT_EQ("", frame.imuInfo_);
}
} // namespace DistributedHardware
} // namespace OHOS
