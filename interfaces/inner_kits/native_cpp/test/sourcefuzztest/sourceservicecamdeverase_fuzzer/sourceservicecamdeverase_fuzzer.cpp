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
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {

void SourceServiceCamDevEraseFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    DCameraIndex index;
    const int32_t doubleNum = 2;
    // Consume half of the remaining bytes for devId_ and the rest for dhId_.
    // This mimics the original split logic using fdp's safe consumption.
    size_t halfRemaingSize = fdp.remaining_bytes() / doubleNum;
    index.devId_ = fdp.ConsumeBytesAsString(halfRemaingSize);
    index.dhId_ = fdp.ConsumeRemainingBytesAsString();

    sourceService->CamDevErase(index);
}
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceCamDevEraseFuzzTest(data, size);
    return 0;
}