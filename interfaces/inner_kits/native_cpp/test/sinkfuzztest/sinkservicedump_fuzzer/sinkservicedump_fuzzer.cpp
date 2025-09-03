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

#include "sinkservicedump_fuzzer.h"
#include "distributed_camera_sink_service.h"
#include "distributed_camera_constants.h"
#include <vector>
#include <string>
#include <unistd.h>
#include <algorithm>
#include "fuzzer/FuzzedDataProvider.h"
namespace OHOS {
namespace DistributedHardware {
void SinkServiceDumpFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    int fd = fdp.ConsumeIntegral<int>();

    std::vector<std::u16string> args;
    int numArgs = fdp.ConsumeIntegralInRange<int>(0, 5);
    for (int i = 0; i < numArgs; ++i) {
        std::string s = fdp.ConsumeRandomLengthString();
        std::u16string u16s;
        u16s.reserve(s.length());
        for (char c : s) {
            u16s.push_back(static_cast<char16_t>(c));
        }
        args.push_back(u16s);
    }
    
    auto sinkService = std::make_shared<DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    if (sinkService == nullptr) {
        return;
    }

    sinkService->Dump(fd, args);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkServiceDumpFuzzTest(data, size);
    return 0;
}