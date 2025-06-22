/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#include "dcamera_source_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
inline std::string ExtractString(const uint8_t* data, size_t offset, size_t length)
{
    return std::string(reinterpret_cast<const char*>(data + offset), length);
}
namespace DistributedHardware {
void SourceServiceDCameraNotifyFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string dhId(reinterpret_cast<const char*>(data), size);
    std::string devId(reinterpret_cast<const char*>(data), size);
    std::string events(reinterpret_cast<const char*>(data), size);

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    sourceService->DCameraNotify(devId, dhId, events);
}

void SourceServiceOnStartFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    sourceService->OnStart();
}

void SourceServiceInitFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    sourceService->Init();
}

void SourceServiceOnStopFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    sourceService->OnStop();
}

void SourceServiceDumpFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::vector<std::u16string> args;
    std::u16string arg(reinterpret_cast<const char16_t*>(data), size / sizeof(char16_t));
    if (arg.empty() || std::all_of(arg.begin(), arg.end(), [](char16_t c) { return c == 0; })) {
        return;
    }
    args.push_back(arg);

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    int fd = STDOUT_FILENO;

    sourceService->Dump(fd, args);
}

void SourceServiceGetDumpInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    CameraDumpInfo camDump;

    sourceService->GetDumpInfo(camDump);
}

void SourceServiceCamDevEraseFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size < sizeof(DCameraIndex))) {
        return;
    }

    int32_t doubleNum = 2;
    DCameraIndex index;
    index.devId_ = ExtractString(data, 0, size / doubleNum);
    index.dhId_ = ExtractString(data, size / doubleNum, size / doubleNum);

    std::shared_ptr<DistributedCameraSourceService> sourceService =
        std::make_shared<DistributedCameraSourceService>(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

    sourceService->CamDevErase(index);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SourceServiceDCameraNotifyFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceOnStartFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceInitFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceOnStopFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceDumpFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceGetDumpInfoFuzzTest(data, size);
    OHOS::DistributedHardware::SourceServiceCamDevEraseFuzzTest(data, size);
    return 0;
}

