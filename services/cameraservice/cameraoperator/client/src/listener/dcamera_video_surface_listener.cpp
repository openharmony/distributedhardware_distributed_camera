/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "dcamera_video_surface_listener.h"

#include <securec.h>

#include "data_buffer.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraVideoSurfaceListener::DCameraVideoSurfaceListener(const sptr<Surface>& surface,
    const std::shared_ptr<ResultCallback>& callback) : surface_(surface), callback_(callback)
{
}

void DCameraVideoSurfaceListener::OnBufferAvailable()
{
    if (callback_ == nullptr || surface_ == nullptr) {
        DHLOGE("DCameraVideoSurfaceListener ResultCallback or Surface is null");
        return;
    }

    int32_t flushFence = 0;
    int64_t timestamp = 0;
    OHOS::Rect damage;
    OHOS::sptr<OHOS::SurfaceBuffer> buffer = nullptr;
    surface_->AcquireBuffer(buffer, flushFence, timestamp, damage);
    if (buffer == nullptr) {
        DHLOGE("DCameraVideoSurfaceListener AcquireBuffer failed");
        return;
    }

    do {
        int32_t width = buffer->GetWidth();
        int32_t height = buffer->GetHeight();
        int32_t size = buffer->GetSize();
        char *address = static_cast<char *>(buffer->GetVirAddr());
        if ((address == nullptr) || (size <= 0) || (width <= 0) || (height <= 0) || (size > SURFACE_BUFFER_MAX_SIZE)) {
            DHLOGE("DCameraVideoSurfaceListener invalid params, width: %d, height: %d, size: %d", width, height, size);
            break;
        }

        DHLOGI("DCameraVideoSurfaceListener width: %d, height: %d, size: %d", width, height, size);
        int32_t y2UvRatio = 2;
        int32_t bytesPerPixel = 3;
        size_t validImgSize = static_cast<size_t>(width * height * bytesPerPixel / y2UvRatio);
        std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(validImgSize);
        int32_t ret = memcpy_s(dataBuffer->Data(), dataBuffer->Capacity(), address, validImgSize);
        if (ret != EOK) {
            DHLOGE("DCameraVideoSurfaceListener Memory Copy failed, ret: %d", ret);
            break;
        }

        callback_->OnVideoResult(dataBuffer);
    } while (0);
    surface_->ReleaseBuffer(buffer, -1);
}
} // namespace DistributedHardware
} // namespace OHOS