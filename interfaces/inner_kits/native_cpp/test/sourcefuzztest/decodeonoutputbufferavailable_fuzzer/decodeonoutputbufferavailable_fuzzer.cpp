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

#include "decodeonoutputbufferavailable_fuzzer.h"

#include <fuzzer/FuzzedDataProvider.h>
#include "decode_video_callback.h"

namespace OHOS {
namespace DistributedHardware {
void DecodeOnOutputBufferAvailableFuzzTest(const uint8_t* data, size_t size)
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
    std::shared_ptr<DCameraPipelineSource> sourcePipeline = std::make_shared<DCameraPipelineSource>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<AppExecFwk::EventHandler> pipeEventHandler = std::make_shared<AppExecFwk::EventHandler>(runner);
    std::shared_ptr<DecodeDataProcess> decodeDataProcess =
        std::make_shared<DecodeDataProcess>(pipeEventHandler, sourcePipeline);
    std::shared_ptr<DecodeVideoCallback> decodeVideoCallback = std::make_shared<DecodeVideoCallback>(decodeDataProcess);
    std::shared_ptr<Media::AVSharedMemory> buffer = nullptr;

    decodeVideoCallback->OnOutputBufferAvailable(index, info, flag, buffer);
}

void DecodeVideoCallbackOnErrorFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    MediaAVCodec::AVCodecErrorType errorType =
        static_cast<MediaAVCodec::AVCodecErrorType>(fdp.ConsumeIntegral<int32_t>());
    int32_t errorCode = fdp.ConsumeIntegral<int32_t>();

    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    std::shared_ptr<DCameraPipelineSource> source = std::make_shared<DCameraPipelineSource>();

    std::shared_ptr<OHOS::DistributedHardware::DecodeDataProcess> decodeDataProcess =
        std::make_shared<OHOS::DistributedHardware::DecodeDataProcess>(handler, source);
    std::shared_ptr<DecodeVideoCallback> decodeVideoCallback =
        std::make_shared<DecodeVideoCallback>(decodeDataProcess);

    decodeVideoCallback->OnError(errorType, errorCode);
}

void DecodeVideoCallbackOnOutputFormatChangedFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t tempStrLen = 32;
    Media::Format format;
    format.PutIntValue("width", fdp.ConsumeIntegral<int32_t>());
    format.PutIntValue("height", fdp.ConsumeIntegral<int32_t>());
    format.PutStringValue("mime", fdp.ConsumeRandomLengthString(tempStrLen));

    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<AppExecFwk::EventHandler> handler = std::make_shared<AppExecFwk::EventHandler>(runner);
    std::shared_ptr<DCameraPipelineSource> source = std::make_shared<DCameraPipelineSource>();

    std::shared_ptr<OHOS::DistributedHardware::DecodeDataProcess> decodeDataProcess =
        std::make_shared<OHOS::DistributedHardware::DecodeDataProcess>(handler, source);
    std::shared_ptr<DecodeVideoCallback> decodeVideoCallback =
        std::make_shared<DecodeVideoCallback>(decodeDataProcess);

    decodeVideoCallback->OnOutputFormatChanged(format);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::DecodeOnOutputBufferAvailableFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeVideoCallbackOnErrorFuzzTest(data, size);
    OHOS::DistributedHardware::DecodeVideoCallbackOnOutputFormatChangedFuzzTest(data, size);
    return 0;
}

