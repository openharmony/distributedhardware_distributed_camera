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

#include "sourceserviceupdateworkmode_fuzzer.h"
#include <fuzzer/FuzzedDataProvider.h>

#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_service.h"
#include "idistributed_hardware_source.h"

namespace OHOS {
namespace DistributedHardware {
void SourceServiceUpdateWorkModeFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(int32_t))) {
        return;
    }

    FuzzedDataProvider fdp(data, size);
    int32_t tempLen = 32;
    std::string devId(fdp.ConsumeRandomLengthString(tempLen));
    std::string dhId(fdp.ConsumeRandomLengthString(tempLen));
    WorkModeParam param(-1, 0, 0, 0);
    param.fd = fdp.ConsumeIntegral<int32_t>();
    param.sharedMemLen = fdp.ConsumeIntegral<int32_t>();
    param.scene = fdp.ConsumeIntegral<uint32_t>();
    param.isAVsync = fdp.ConsumeBool();

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);
    sptr<IDCameraSourceCallback> callback(new DCameraSourceCallback());

    sourceService->UpdateDistributedHardwareWorkMode(devId, dhId, param);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SourceServiceUpdateWorkModeFuzzTest(data, size);
    return 0;
}

