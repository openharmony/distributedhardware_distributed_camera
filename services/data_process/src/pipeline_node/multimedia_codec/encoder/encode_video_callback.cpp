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
#include "encode_video_callback.h"

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void EncodeVideoCallback::OnError(MediaAVCodec::AVCodecErrorType errorType, int32_t errorCode)
{
    DHLOGD("EncodeVideoCallback : OnError. Error type: %{public}d. Error code: %{public}d ", errorType, errorCode);
    std::shared_ptr<EncodeDataProcess> targetEncoderNode = encodeVideoNode_.lock();
    CHECK_AND_RETURN_LOG(targetEncoderNode == nullptr, "%{public}s", "encodeVideoNode_ is nullptr.");
    targetEncoderNode->OnError();
}

void EncodeVideoCallback::OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("EncodeVideoCallback : OnInputBufferAvailable. No operation when using surface input.");
    std::shared_ptr<EncodeDataProcess> targetEncoderNode = encodeVideoNode_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("encodeVideoNode_ is nullptr.");
        return;
    }
    targetEncoderNode->OnInputBufferAvailable(index, buffer);
}

void EncodeVideoCallback::OnOutputFormatChanged(const Media::Format &format)
{
    DHLOGD("EncodeVideoCallback : OnOutputFormatChanged.");
    std::shared_ptr<EncodeDataProcess> targetEncoderNode = encodeVideoNode_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("encodeVideoNode_ is nullptr.");
        return;
    }
    targetEncoderNode->OnOutputFormatChanged(format);
}

void EncodeVideoCallback::OnOutputBufferAvailable(uint32_t index, MediaAVCodec::AVCodecBufferInfo info,
    MediaAVCodec::AVCodecBufferFlag flag, std::shared_ptr<Media::AVSharedMemory> buffer)
{
    DHLOGD("EncodeVideoCallback : OnOutputBufferAvailable.");
    std::shared_ptr<EncodeDataProcess> targetEncoderNode = encodeVideoNode_.lock();
    if (targetEncoderNode == nullptr) {
        DHLOGE("encodeVideoNode_ is nullptr.");
        return;
    }
    targetEncoderNode->OnOutputBufferAvailable(index, info, flag, buffer);
}
} // namespace DistributedHardware
} // namespace OHOS
