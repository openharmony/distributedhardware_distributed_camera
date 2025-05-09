/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "decode_data_process.h"
#include "decode_video_callback.h"
#include "decode_surface_listener.h"
#include "dcamera_pipeline_sink.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "fps_controller_process.h"
#include "mock_dcamera_data_process_listener.h"


using namespace testing::ext;

namespace OHOS {
namespace DistributedHardware {
class DecodeSurfaceListenerTest : public testing::Test {
public:
    static void SetUpTestCase(void);
    static void TearDownTestCase(void);
    void SetUp();
    void TearDown();
};

void DecodeSurfaceListenerTest::SetUpTestCase(void)
{
}

void DecodeSurfaceListenerTest::TearDownTestCase(void)
{
}

void DecodeSurfaceListenerTest::SetUp(void)
{
}

void DecodeSurfaceListenerTest::TearDown(void)
{
}

/**
 * @tc.name: decode_surface_listener_test_001
 * @tc.desc: Verify decode surface listener normal.
 * @tc.type: FUNC
 * @tc.require: Issue Number
 */
HWTEST_F(DecodeSurfaceListenerTest, decode_surface_listener_test_001, TestSize.Level1)
{
    std::shared_ptr<AppExecFwk::EventHandler> pipeEventHandler = std::make_shared<AppExecFwk::EventHandler>();
    std::weak_ptr<DCameraPipelineSource> callbackPipSource = std::make_shared<DCameraPipelineSource>();
    sptr<IConsumerSurface> surface = IConsumerSurface::Create();
    std::weak_ptr<DecodeDataProcess> decodeVideoNode = std::make_shared<DecodeDataProcess>(pipeEventHandler,
        callbackPipSource);
    std::shared_ptr<DecodeSurfaceListener> listener = std::make_shared<DecodeSurfaceListener>(surface,
        decodeVideoNode);
    listener->SetDecodeVideoNode(decodeVideoNode);
    listener->GetDecodeVideoNode();
    listener->OnBufferAvailable();
    std::shared_ptr<DecodeVideoCallback> dCallBack = std::make_shared<DecodeVideoCallback>(decodeVideoNode);
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    dCallBack->OnInputBufferAvailable(0, buffer);
    Media::Format format;
    dCallBack->OnOutputFormatChanged(format);
    std::shared_ptr<Media::AVSharedMemory> buffer2 = nullptr;
    MediaAVCodec::AVCodecBufferInfo info;
    MediaAVCodec::AVCodecBufferFlag flag = MediaAVCodec::AVCODEC_BUFFER_FLAG_CODEC_DATA;
    dCallBack->OnOutputBufferAvailable(0, info, flag, buffer2);
    EXPECT_TRUE(buffer2 == nullptr);
}
} // namespace DistributedHardware
} // namespace OHOS
