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

#include "image_common_type.h"

namespace OHOS {
namespace DistributedHardware {
void VideoConfigParams::SetVideoCodecType(VideoCodecType videoCodec)
{
    videoCodec_ = videoCodec;
}

void VideoConfigParams::SetVideoformat(Videoformat pixelFormat)
{
    pixelFormat_ = pixelFormat;
}

void VideoConfigParams::SetFrameRate(int32_t frameRate)
{
    frameRate_ = frameRate;
}

void VideoConfigParams::SetWidthAndHeight(int32_t width, int32_t height)
{
    width_ = width;
    height_ = height;
}

void VideoConfigParams::SetSystemSwitchFlagAndRotation(bool flag, int32_t rotation)
{
    isSystemSwitch_ = flag;
    rotation_ = rotation;
}

VideoCodecType VideoConfigParams::GetVideoCodecType() const
{
    return videoCodec_;
}

Videoformat VideoConfigParams::GetVideoformat() const
{
    return pixelFormat_;
}

int32_t VideoConfigParams::GetFrameRate() const
{
    return frameRate_;
}

int32_t VideoConfigParams::GetWidth() const
{
    return width_;
}

int32_t VideoConfigParams::GetHeight() const
{
    return height_;
}

bool VideoConfigParams::GetIsSystemSwitch() const
{
    return isSystemSwitch_;
}
int32_t VideoConfigParams::GetRotation() const
{
    return rotation_;
}
} // namespace DistributedHardware
} // namespace OHOS
