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

#include "decode_video_callback.h"

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void DecodeVideoCallback::OnError(MediaAVCodec::AVCodecErrorType errorType, int32_t errorCode)
{
    DHLOGE("DecodeVideoCallback : OnError. Error type: %{public}d. Error code: %{public}d ", errorType, errorCode);
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    CHECK_AND_RETURN_LOG(targetDecoderNode == nullptr, "%{public}s", "decodeVideoNode_ is nullptr.");
    targetDecoderNode->OnError();
}

void DecodeVideoCallback::OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("DecodeVideoCallback : OnInputBufferAvailable.");
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("decodeVideoNode_ is nullptr.");
        return;
    }
    targetDecoderNode->OnInputBufferAvailable(index, buffer);
}

void DecodeVideoCallback::OnOutputFormatChanged(const Media::Format &format)
{
    DHLOGD("DecodeVideoCallback : OnOutputFormatChanged.");
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("decodeVideoNode_ is nullptr.");
        return;
    }
    targetDecoderNode->OnOutputFormatChanged(format);
}

void DecodeVideoCallback::OnOutputBufferAvailable(uint32_t index, MediaAVCodec::AVCodecBufferInfo info,
    MediaAVCodec::AVCodecBufferFlag flag, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("DecodeVideoCallback : OnOutputBufferAvailable. Only relaese buffer when using surface output.");
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("decodeVideoNode_ is nullptr.");
        return;
    }
    targetDecoderNode->OnOutputBufferAvailable(index, info, flag, buffer);
}
} // namespace DistributedHardware
} // namespace OHOS
