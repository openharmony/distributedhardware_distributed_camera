/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "encodeoninputbufferavailable_fuzzer.h"
#include "encode_video_callback.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <memory>

namespace OHOS {
namespace DistributedHardware {
void EncodeOnInputBufferAvailableFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    uint32_t index = fdp.ConsumeIntegral<uint32_t>();

    std::shared_ptr<DCameraPipelineSink> sinkPipeline = std::make_shared<DCameraPipelineSink>();
    std::shared_ptr<EncodeDataProcess> encodeDataProcess = std::make_shared<EncodeDataProcess>(sinkPipeline);
    std::shared_ptr<EncodeVideoCallback> encodeVideoCallback = std::make_shared<EncodeVideoCallback>(encodeDataProcess);

    std::shared_ptr<Media::AVSharedMemory> buffer;
    encodeVideoCallback->OnInputBufferAvailable(index, buffer);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::EncodeOnInputBufferAvailableFuzzTest(data, size);
    return 0;
}