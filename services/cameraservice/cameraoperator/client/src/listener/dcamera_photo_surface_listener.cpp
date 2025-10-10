/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#include "dcamera_photo_surface_listener.h"

#include <securec.h>

#include "data_buffer.h"
#include "dcamera_hidumper.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraPhotoSurfaceListener::DCameraPhotoSurfaceListener(const sptr<IConsumerSurface>& surface,
    const std::shared_ptr<ResultCallback>& callback) : surface_(surface), callback_(callback)
{
    photoCount_ = COUNT_INIT_NUM;
}

void DCameraPhotoSurfaceListener::OnBufferAvailable()
{
    DHLOGI("enter");
    if (callback_ == nullptr || surface_ == nullptr) {
        DHLOGE("DCameraPhotoSurfaceListener ResultCallback or Surface is null");
        return;
    }

    int32_t flushFence = 0;
    int64_t timestamp = 0;
    OHOS::Rect damage;
    OHOS::sptr<OHOS::SurfaceBuffer> buffer = nullptr;
    surface_->AcquireBuffer(buffer, flushFence, timestamp, damage);
    if (buffer == nullptr) {
        DHLOGE("DCameraPhotoSurfaceListener AcquireBuffer failed");
        return;
    }

    do {
        char *address = static_cast<char *>(buffer->GetVirAddr());
        int32_t size = -1;
        buffer->GetExtraData()->ExtraGet("dataSize", size);
        if (size <= 0) {
            size = static_cast<int32_t>(buffer->GetSize());
        }
        if ((address == nullptr) || (size <= 0) || (size > SURFACE_BUFFER_MAX_SIZE)) {
            DHLOGE("DCameraPhotoSurfaceListener invalid params, size: %{public}d", size);
            break;
        }
        DHLOGI("DCameraPhotoSurfaceListener size: %{public}d", size);
        std::shared_ptr<DataBuffer> dataBuffer = std::make_shared<DataBuffer>(size);
        int32_t ret = memcpy_s(dataBuffer->Data(), dataBuffer->Capacity(), address, size);
        if (ret != EOK) {
            DHLOGE("DCameraPhotoSurfaceListener Memory Copy failed, ret: %{public}d", ret);
            break;
        }
#ifdef DUMP_DCAMERA_FILE
        std::string name = std::to_string(photoCount_++) + SINK_PHOTO;
        if (DcameraHidumper::GetInstance().GetDumpFlag() && (IsUnderDumpMaxSize(DUMP_PHOTO_PATH, name) == DCAMERA_OK)) {
            DumpBufferToFile(DUMP_PHOTO_PATH, name, dataBuffer->Data(), dataBuffer->Size());
        }
#endif
        callback_->OnPhotoResult(dataBuffer);
    } while (0);
    surface_->ReleaseBuffer(buffer, -1);
}
} // namespace DistributedHardware
} // namespace OHOS