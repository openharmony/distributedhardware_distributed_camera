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

#include "dcamera_buffer_handle.h"

#include <cerrno>
#include <cstddef>
#include <string>
#include <sys/mman.h>

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void* DCameraMemoryMap(const BufferHandle *buffer)
{
    if (buffer == nullptr) {
        DHLOGE("mmap the buffer handle is null");
        return nullptr;
    }

    void* virAddr = mmap(NULL, buffer->size, PROT_READ | PROT_WRITE, MAP_SHARED, buffer->fd, 0);
    if (virAddr == MAP_FAILED) {
        DHLOGE("mmap failed errno %{public}s, fd : %{public}d", strerror(errno), buffer->fd);
        return nullptr;
    }
    return virAddr;
}

void DCameraMemoryUnmap(BufferHandle *buffer)
{
    if (buffer == nullptr) {
        DHLOGE("unmmap the buffer handle is NULL");
        return;
    }
    if (buffer->virAddr != nullptr) {
        int ret = munmap(buffer->virAddr, buffer->size);
        if (ret != 0) {
            DHLOGE("munmap failed err: %{public}s", strerror(errno));
        }
    }
    buffer->virAddr = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS