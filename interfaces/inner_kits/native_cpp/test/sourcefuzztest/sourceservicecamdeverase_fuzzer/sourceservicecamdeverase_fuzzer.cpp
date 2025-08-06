/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "sourceservicecamdeverase_fuzzer.h"
#include "distributed_camera_source_service.h"
#include "distributed_camera_constants.h"

namespace OHOS {
namespace DistributedHardware {

inline std::string ExtractString(const uint8_t* data, size_t offset, size_t length)
{
    return std::string(reinterpret_cast<const char*>(data + offset), length);
}

void SourceServiceCamDevEraseFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(DCameraIndex))) {
        return;
    }

    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    
    int doubleNum = 2;
    DCameraIndex index;
    index.devId_ = ExtractString(data, 0, size / doubleNum);
    index.dhId_ = ExtractString(data, size / doubleNum, size / doubleNum);

    sourceService->CamDevErase(index);
}

} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceCamDevEraseFuzzTest(data, size);
    return 0;
}