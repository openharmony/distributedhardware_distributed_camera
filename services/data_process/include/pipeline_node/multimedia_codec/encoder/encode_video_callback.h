/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DECODE_VIDEO_CALLBACK_H
#define OHOS_DECODE_VIDEO_CALLBACK_H

#include "avcodec_errors.h"
#include "avcodec_common.h"
#include "meta/format.h"

#include "encode_data_process.h"

namespace OHOS {
namespace DistributedHardware {
class EncodeDataProcess;

class EncodeVideoCallback : public MediaAVCodec::AVCodecCallback {
public:
    explicit EncodeVideoCallback(const std::weak_ptr<EncodeDataProcess>& encodeVideoNode)
        : encodeVideoNode_(encodeVideoNode) {}
    ~EncodeVideoCallback() override = default;

    void OnError(MediaAVCodec::AVCodecErrorType errorType, int32_t errorCode) override;
    void OnInputBufferAvailable(uint32_t index, std::shared_ptr<Media::AVSharedMemory> buffer) override;
    void OnOutputFormatChanged(const Media::Format &format) override;
    void OnOutputBufferAvailable(uint32_t index, MediaAVCodec::AVCodecBufferInfo info,
        MediaAVCodec::AVCodecBufferFlag flag, std::shared_ptr<Media::AVSharedMemory> buffer) override;
private:
    std::weak_ptr<EncodeDataProcess> encodeVideoNode_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DECODE_VIDEO_CALLBACK_H
