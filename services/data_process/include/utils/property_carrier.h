/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_PROPERTY_CARRIER_H
#define OHOS_PROPERTY_CARRIER_H

#include "distributed_hardware_log.h"
#include "distributed_camera_errno.h"
#include "surface.h"

namespace OHOS {
namespace DistributedHardware {
static const std::string SURFACE = "surface";
class PropertyCarrier {
public:
    ~PropertyCarrier();

    int32_t CarrySurfaceProperty(sptr<Surface>& surface);
public:
    sptr<Surface> surface_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_PROPERTY_CARRIER_H