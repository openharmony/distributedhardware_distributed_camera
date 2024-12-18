/*
 * Copyright (c) 2023-2024 Huawei Device Co., Ltd.
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

#include "encodeonoutputbufferavailable_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "encode_video_callback.h"

namespace OHOS {
namespace DistributedHardware {
void EncodeOnOutputBufferAvailableFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    uint32_t index = fdp.ConsumeIntegral<uint32_t>();
    int64_t presentTimeUs = fdp.ConsumeIntegral<int64_t>();
    int32_t infoSize = fdp.ConsumeIntegral<int32_t>();
    int32_t offset = fdp.ConsumeIntegral<int32_t>();
    int32_t bufferFlag = fdp.ConsumeIntegral<int32_t>();
    MediaAVCodec::AVCodecBufferInfo info = { presentTimeUs, infoSize, offset};
    MediaAVCodec::AVCodecBufferFlag flag = static_cast<MediaAVCodec::AVCodecBufferFlag>(bufferFlag);
    std::shared_ptr<DCameraPipelineSink> sinkPipeline = std::make_shared<DCameraPipelineSink>();
    std::shared_ptr<EncodeDataProcess> encodeDataProcess = std::make_shared<EncodeDataProcess>(sinkPipeline);
    std::shared_ptr<EncodeVideoCallback> encodeVideoCallback = std::make_shared<EncodeVideoCallback>(encodeDataProcess);
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;
    encodeVideoCallback->OnOutputBufferAvailable(index, info, flag, buffer);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::EncodeOnOutputBufferAvailableFuzzTest(data, size);
    return 0;
}

