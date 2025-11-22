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

#ifndef OHOS_IMAGE_COMMON_TYPE_H
#define OHOS_IMAGE_COMMON_TYPE_H

#include <cstdlib>
#include <cstdint>
#include "data_buffer.h"

namespace OHOS {
namespace DistributedHardware {
enum class PipelineType : int32_t {
    VIDEO = 0,
    PHOTO_JPEG = 1,
};

enum class VideoCodecType : int32_t {
    NO_CODEC = 0,
    CODEC_H264 = 1,
    CODEC_H265 = 2,
    CODEC_MPEG4_ES = 3,
};

enum class Videoformat : int32_t {
    YUVI420 = 0,
    NV12 = 1,
    NV21 = 2,
    RGBA_8888 = 3,
    P010 = 4,
};

class VideoConfigParams {
public:
    VideoConfigParams() : videoCodec_(VideoCodecType::NO_CODEC), pixelFormat_(Videoformat::YUVI420),
        frameRate_(0), width_ (0), height_(0)
        {}
    VideoConfigParams(VideoCodecType videoCodec, Videoformat pixelFormat, int32_t frameRate, int32_t width,
        int32_t height)
        : videoCodec_(videoCodec), pixelFormat_(pixelFormat), frameRate_(frameRate), width_ (width), height_(height)
        {}
    ~VideoConfigParams() = default;

    void SetVideoCodecType(VideoCodecType videoCodec);
    void SetVideoformat(Videoformat pixelFormat);
    void SetFrameRate(int32_t frameRate);
    void SetWidthAndHeight(int32_t width, int32_t height);
    void SetSystemSwitchFlagAndRotation(bool flag, int32_t rotation);
    VideoCodecType GetVideoCodecType() const;
    Videoformat GetVideoformat() const;
    int32_t GetFrameRate() const;
    int32_t GetWidth() const;
    int32_t GetHeight() const;
    bool GetIsSystemSwitch() const;
    int32_t GetRotation() const;

private:
    VideoCodecType videoCodec_;
    Videoformat pixelFormat_;
    int32_t frameRate_;
    int32_t width_;
    int32_t height_;
    bool isSystemSwitch_ = false;
    int32_t rotation_ = 0;
};

struct ImageUnitInfo {
    Videoformat colorFormat;
    int32_t width;
    int32_t height;
    int32_t alignedWidth;
    int32_t alignedHeight;
    size_t chromaOffset;
    size_t imgSize;
    std::shared_ptr<DataBuffer> imgData;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IMAGE_COMMON_TYPE_H
