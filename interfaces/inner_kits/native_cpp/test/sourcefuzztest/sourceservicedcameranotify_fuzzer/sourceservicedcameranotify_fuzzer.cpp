/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "sourceservicedcameranotify_fuzzer.h"
#include "distributed_camera_source_service.h"
#include "distributed_camera_constants.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {

void SourceServiceDCameraNotifyFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    std::string devId = fdp.ConsumeRandomLengthString();
    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string events = fdp.ConsumeRemainingBytesAsString();

    sourceService->DCameraNotify(devId, dhId, events);
}
} // namespace DistributedHardware
} // namespace OHOS

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceDCameraNotifyFuzzTest(data, size);
    return 0;
}