/*
 * Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "sinkservicestopdistributedhardware_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<DistributedCameraSinkService> sinkService_ =
    std::make_shared<OHOS::DistributedHardware::DistributedCameraSinkService>(
        DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);

void SinkServiceStopDistributedHardwareFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string networkId(reinterpret_cast<const char*>(data), size);
    sinkService_->StopDistributedHardware(networkId);
}

void SinkServiceOnStartFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sinkService_->OnStart();
}

void DistributedCameraSinkServiceInitFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sinkService_->Init();
}

void DistributedCameraSinkServiceDumpFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::vector<std::u16string> args;
    std::u16string arg(reinterpret_cast<const char16_t*>(data), size / sizeof(char16_t));
    args.push_back(arg);
    int fd = STDOUT_FILENO;

    sinkService_->Dump(fd, args);
}

void DistributedCameraSinkServiceGetCamDumpInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    CameraDumpInfo camDump;

    sinkService_->GetCamDumpInfo(camDump);
}

void DistributedCameraSinkServiceOnStopFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    sinkService_->OnStop();
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SinkServiceStopDistributedHardwareFuzzTest(data, size);
    OHOS::DistributedHardware::SinkServiceOnStartFuzzTest(data, size);
    OHOS::DistributedHardware::DistributedCameraSinkServiceInitFuzzTest(data, size);
    OHOS::DistributedHardware::DistributedCameraSinkServiceOnStopFuzzTest(data, size);
    OHOS::DistributedHardware::DistributedCameraSinkServiceDumpFuzzTest(data, size);
    OHOS::DistributedHardware::DistributedCameraSinkServiceGetCamDumpInfoFuzzTest(data, size);
    return 0;
}

