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

#include "sinkservicegetcamerainfo_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "dcamera_sink_controller.h"
#include "dcamera_sink_access_control.h"
#include "dcamera_sink_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_service.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
void SinkServiceGetCameraInfoFuzzTest(const uint8_t* data, size_t size)
{
    if ((data == nullptr) || (size == 0)) {
        return;
    }

    std::string dhId = "1";
    std::string cameraInfo(reinterpret_cast<const char*>(data), size);

    std::shared_ptr<DistributedCameraSinkService> sinkService =
        std::make_shared<DistributedCameraSinkService>(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);
    sptr<IDCameraSinkCallback> sinkCallback(new DCameraSinkCallback());
    std::shared_ptr<DCameraSinkDev> sinkDevice = std::make_shared<DCameraSinkDev>(dhId, sinkCallback);
    sinkDevice->accessControl_ = std::make_shared<DCameraSinkAccessControl>();
    sinkDevice->controller_ = std::make_shared<DCameraSinkController>(sinkDevice->accessControl_, sinkCallback);
    sinkService->camerasMap_.emplace(dhId, sinkDevice);
    sinkService->GetCameraInfo(dhId, cameraInfo);
    sinkService->GetCamIds();
    sinkService->IsCurSinkDev(sinkDevice);
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SinkServiceGetCameraInfoFuzzTest(data, size);
    return 0;
}

