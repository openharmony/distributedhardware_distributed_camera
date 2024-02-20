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

#include "dcamera_sink_hidumper.h"
#include "distributed_hardware_log.h"

using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DcameraSinkHidumperTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DcameraSinkHidumperTest::SetUpTestCase(void)
{
    DHLOGI("enter");
}

void DcameraSinkHidumperTest::TearDownTestCase(void)
{
    DHLOGI("enter");
}

void DcameraSinkHidumperTest::SetUp(void)
{
    DHLOGI("enter");
}

void DcameraSinkHidumperTest::TearDown(void)
{
    DHLOGI("enter");
}

/**
 * @tc.name: dcamera_sink_hidumper_test_001
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_001, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_001");
    std::vector<std::string> args;
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_002
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_002, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_002");
    std::vector<std::string> args;
    std::string str1 = "--version";
    std::string str2 = "--camNum";
    args.push_back(str1);
    args.push_back(str2);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_003
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_003, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_003");
    std::vector<std::string> args;
    std::string str1 = "--version";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_004
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_004, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_004");
    std::vector<std::string> args;
    std::string str1 = "--camNum";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_005
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_005, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_005");
    std::vector<std::string> args;
    std::string str1 = "--opened";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_006
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_006, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_006");
    std::vector<std::string> args;
    std::string str1 = "-h";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_007
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_007, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_007");
    std::vector<std::string> args;
    std::string str1 = "-help";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_008
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_008, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_008");
    std::vector<std::string> args;
    std::string str1 = "--startdump";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}

/**
 * @tc.name: dcamera_sink_hidumper_test_009
 * @tc.desc: Verify the Dump function.
 * @tc.type: FUNC
 * @tc.require: issue
 */
HWTEST_F(DcameraSinkHidumperTest, dcamera_sink_hidumper_test_009, TestSize.Level1)
{
    DHLOGI("dcamera_sink_hidumper_test_009");
    std::vector<std::string> args;
    std::string str1 = "--stopdump";
    args.push_back(str1);
    std::string result;
    bool ret = DcameraSinkHidumper::GetInstance().Dump(args, result);
    EXPECT_EQ(true, ret);
}
} // namespace DistributedHardware
} // namespace OHOS