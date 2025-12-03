/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "encodeonoutputformatchanged_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "encode_video_callback.h"

namespace OHOS {
namespace DistributedHardware {
void EncodeOnOutputFormatChangedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int64_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t tempStrLen = 32;
    Media::Format format;
    format.PutIntValue("width", fdp.ConsumeIntegral<int32_t>());
    format.PutIntValue("height", fdp.ConsumeIntegral<int32_t>());
    format.PutStringValue("mime", fdp.ConsumeRandomLengthString(tempStrLen));
    std::shared_ptr<DCameraPipelineSink> sinkPipeline = std::make_shared<DCameraPipelineSink>();
    std::shared_ptr<EncodeDataProcess> encodeDataProcess = std::make_shared<EncodeDataProcess>(sinkPipeline);
    std::shared_ptr<EncodeVideoCallback> encodeVideoCallback = std::make_shared<EncodeVideoCallback>(encodeDataProcess);
    encodeVideoCallback->OnOutputFormatChanged(format);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::EncodeOnOutputFormatChangedFuzzTest(data, size);
    return 0;
}

