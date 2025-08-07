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

#include "sourceservicedump_fuzzer.h"
#include "distributed_camera_source_service.h"
#include "distributed_camera_constants.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <algorithm>

namespace OHOS {
namespace DistributedHardware {

void SourceServiceDumpFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    auto sourceService = std::make_shared<DistributedCameraSourceService>(
        DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    
    std::vector<std::u16string> args;
    std::u16string arg(reinterpret_cast<const char16_t*>(data), size / sizeof(char16_t));
    if (arg.empty() || std::all_of(arg.begin(), arg.end(), [](char16_t c) { return c == 0; })) {
        return;
    }
    args.push_back(arg);
    int fd = STDOUT_FILENO;

    sourceService->Dump(fd, args);
}

} // namespace DistributedHardware
} // namespace OHOS

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SourceServiceDumpFuzzTest(data, size);
    return 0;
}

