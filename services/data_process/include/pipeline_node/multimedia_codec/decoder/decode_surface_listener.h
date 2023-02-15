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

#ifndef OHOS_DECODE_SURFACE_LISTENER_H
#define OHOS_DECODE_SURFACE_LISTENER_H

#include "surface.h"
#include "ibuffer_consumer_listener.h"

#include "decode_data_process.h"

namespace OHOS {
namespace DistributedHardware {
class DecodeDataProcess;

class DecodeSurfaceListener : public IBufferConsumerListener {
public:
    DecodeSurfaceListener(sptr<IConsumerSurface> surface, std::weak_ptr<DecodeDataProcess> decodeVideoNode)
        : surface_(surface), decodeVideoNode_(decodeVideoNode) {}
    ~DecodeSurfaceListener() override;

    void OnBufferAvailable() override;
    void SetSurface(const sptr<IConsumerSurface>& surface);
    void SetDecodeVideoNode(const std::weak_ptr<DecodeDataProcess>& decodeVideoNode);
    sptr<IConsumerSurface> GetSurface() const;
    std::shared_ptr<DecodeDataProcess> GetDecodeVideoNode() const;

private:
    sptr<IConsumerSurface> surface_;
    std::weak_ptr<DecodeDataProcess> decodeVideoNode_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DECODE_SURFACE_LISTENER_H
