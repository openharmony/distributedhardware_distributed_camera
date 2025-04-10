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
#include <memory>

#include "dcamera_sink_frame_info.h"
#include "distributed_camera_errno.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkFrameInfoTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

static const std::string TEST_SINK_FRAME_INFO_JSON_TYPE = R"({
    "type": "test",
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_INDEX = R"({
    "type": 0,
    "index": "test",
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_PTS = R"({
    "type": 0,
    "index": 1,
    "pts": "test",
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_STARTENCODET = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": "test",
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_FINISHENCODET = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": "test",
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_SENDT = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": "test",
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_SENDT2 = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "v1"
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_SENDT3 = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": 1
})";

static const std::string TEST_SINK_FRAME_INFO_JSON_VER = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": "test",
    "ver": 1,
})";

static const std::string TEST_SINK_FRAME_INFO_JSON = R"({
    "type": 0,
    "index": 1,
    "pts": 1,
    "startEncodeT": 1,
    "finishEncodeT": 1,
    "sendT": 1,
    "ver": "test",
})";

void DCameraSinkFrameInfoTest::SetUpTestCase(void)
{
}

void DCameraSinkFrameInfoTest::TearDownTestCase(void)
{
}

void DCameraSinkFrameInfoTest::SetUp()
{
}

void DCameraSinkFrameInfoTest::TearDown()
{
}

/**
 * @tc.name: dcamera_frame_info_test_001.
 * @tc.desc: Verify InfoframeTest Json.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DCameraSinkFrameInfoTest, dcamera_sink_frame_info_test_001, TestSize.Level1)
{
    DCameraSinkFrameInfo frame;
    std::string str = "0";
    int32_t ret = frame.Unmarshal(str);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_TYPE);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_INDEX);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_PTS);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_STARTENCODET);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_FINISHENCODET);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_SENDT);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_SENDT2);
    EXPECT_EQ(DCAMERA_OK, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_SENDT3);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON_VER);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);

    ret = frame.Unmarshal(TEST_SINK_FRAME_INFO_JSON);
    EXPECT_EQ(DCAMERA_BAD_VALUE, ret);
}
} // namespace DistributedHardware
} // namespace OHOS