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

#ifndef OHOS_DCAMERA_HITRACE_ADAPTER_H
#define OHOS_DCAMERA_HITRACE_ADAPTER_H

#include <string>

#include "hitrace_meter.h"

#define DCAMERA_SYNC_TRACE HITRACE_METER(DCAMERA_HITRACE_LABEL)

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DCAMERA_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_CAMERA;
const std::string DCAMERA_OPEN_CHANNEL_START = "DCAMERA_OPEN_CHANNEL_START";
const std::string DCAMERA_INPUT_OPEN_START = "DCAMERA_INPUT_OPEN_START";
const std::string DCAMERA_CAPTURE_FIRST_FRAME = "DCAMERA_CAPTURE_GET_FIRST_FRAME";
enum DcameraTaskId {
    DCAMERA_OPEN_CHANNEL_TASKID = 0,
    DCAMERA_INPUT_OPEN_TASKID = 1,
    DCAMERA_CAPTURE_FIRST_FRAM_TASKID = 2,
};

void DcameraStartAsyncTrace(const std::string& str, int32_t taskId);
void DcameraFinishAsyncTrace(const std::string& str, int32_t taskId);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HITRACE_ADAPTER_H