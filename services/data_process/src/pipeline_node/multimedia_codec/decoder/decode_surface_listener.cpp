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

#include "decode_surface_listener.h"

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DecodeSurfaceListener::~DecodeSurfaceListener()
{
    DHLOGD("DecodeSurfaceListener : ~DecodeSurfaceListener.");
    surface_ = nullptr;
}

void DecodeSurfaceListener::OnBufferAvailable()
{
    DHLOGD("DecodeSurfaceListener : OnBufferAvailable.");
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("decodeVideoNode_ is nullptr.");
        return;
    }
    targetDecoderNode->OnSurfaceOutputBufferAvailable(surface_);
}

void DecodeSurfaceListener::SetSurface(const sptr<IConsumerSurface>& surface)
{
    surface_ = surface;
}

void DecodeSurfaceListener::SetDecodeVideoNode(const std::weak_ptr<DecodeDataProcess>& decodeVideoNode)
{
    decodeVideoNode_ = decodeVideoNode;
}

sptr<IConsumerSurface> DecodeSurfaceListener::GetSurface() const
{
    return surface_;
}

std::shared_ptr<DecodeDataProcess> DecodeSurfaceListener::GetDecodeVideoNode() const
{
    std::shared_ptr<DecodeDataProcess> targetDecoderNode = decodeVideoNode_.lock();
    if (targetDecoderNode == nullptr) {
        DHLOGE("decodeVideoNode_ is nullptr.");
    }
    return targetDecoderNode;
}
} // namespace DistributedHardware
} // namespace OHOS
