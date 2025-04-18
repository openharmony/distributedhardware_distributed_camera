/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_CODEC_EVENT_H
#define OHOS_DCAMERA_CODEC_EVENT_H

#include <vector>

#include "data_buffer.h"
#include "image_common_type.h"
#include "surface.h"

namespace OHOS {
namespace DistributedHardware {
enum class VideoCodecAction : int32_t {
    NO_ACTION = 0,
    ACTION_ONCE_AGAIN = 1,
    ACTION_GET_DECODER_OUTPUT_BUFFER = 2,
};

class CodecPacket {
public:
    CodecPacket() : videoCodec_(VideoCodecType::NO_CODEC) {}
    CodecPacket(VideoCodecType videoCodec, const std::vector<std::shared_ptr<DataBuffer>>& multiDataBuffers)
        : videoCodec_(videoCodec), multiDataBuffers_(multiDataBuffers) {}
    CodecPacket(const sptr<IConsumerSurface>& surface)
        : videoCodec_(VideoCodecType::NO_CODEC), surface_(surface) {}
    ~CodecPacket() = default;

    void SetVideoCodecType(VideoCodecType videoCodec)
    {
        videoCodec_ = videoCodec;
    }

    VideoCodecType GetVideoCodecType() const
    {
        return videoCodec_;
    }

    void SetDataBuffers(std::vector<std::shared_ptr<DataBuffer>>& multiDataBuffers)
    {
        multiDataBuffers_ = multiDataBuffers;
    }

    std::vector<std::shared_ptr<DataBuffer>> GetDataBuffers() const
    {
        return multiDataBuffers_;
    }

    void SetSurface(sptr<IConsumerSurface> surface)
    {
        surface_ = surface;
    }

    sptr<IConsumerSurface> GetSurface() const
    {
        return surface_;
    }

private:
    VideoCodecType videoCodec_;
    std::vector<std::shared_ptr<DataBuffer>> multiDataBuffers_;
    sptr<IConsumerSurface> surface_;
};

class DCameraCodecEvent {
public:
    DCameraCodecEvent(const std::shared_ptr<CodecPacket>& codecPacket)
        : codecPacket_(codecPacket), action_(VideoCodecAction::NO_ACTION) {}
    DCameraCodecEvent(const std::shared_ptr<CodecPacket>& codecPacket,
        VideoCodecAction otherAction)
        : codecPacket_(codecPacket), action_(otherAction) {}
    ~DCameraCodecEvent() {}

    std::shared_ptr<CodecPacket> GetCodecPacket() const
    {
        return codecPacket_;
    }

    VideoCodecAction GetAction() const
    {
        return action_;
    }

private:
    std::shared_ptr<CodecPacket> codecPacket_;
    VideoCodecAction action_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_CODEC_EVENT_H
