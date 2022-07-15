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

#include <cstdint>
#include <string>

#include "hitrace_meter.h"

#define DCAMERA_SYNC_TRACE(value) HITRACE_METER_NAME(DCAMERA_HITRACE_LABEL, value)

namespace OHOS {
namespace DistributedHardware {
constexpr uint64_t DCAMERA_HITRACE_LABEL = HITRACE_TAG_DISTRIBUTED_CAMERA;
const std::string DCAMERA_LOAD_HDF = "DCAMERA_LOAD_HDF";
const std::string DCAMERA_REGISTER_CAMERA = "DCAMERA_REGISTER_CAMERA";
const std::string DCAMERA_UNREGISTER_CAMERA = "DCAMERA_UNREGISTER_CAMERA";
const std::string DCAMERA_SOURCE_CREATE_PIPELINE = "DCAMERA_SOURCE_CREATE_PIPELINE";
const std::string DCAMERA_SOURCE_DESTORY_PIPELINE = "DCAMERA_SOURCE_DESTORY_PIPELINE";
const std::string DCAMERA_SINK_CREATE_PIPELINE = "DCAMERA_SINK_CREATE_PIPELINE";
const std::string DCAMERA_SINK_DESTORY_PIPELINE = "DCAMERA_SINK_DESTORY_PIPELINE";
const std::string DCAMERA_OPEN_CHANNEL_CONTROL = "DCAMERA_OPEN_CHANNEL_CONTROL";
const std::string DCAMERA_OPEN_DATA_CONTINUE = "DCAMERA_OPEN_DATA_CONTINUE";
const std::string DCAMERA_OPEN_DATA_SNAPSHOT = "DCAMERA_OPEN_DATA_SNAPSHOT";
const std::string DCAMERA_CONTINUE_FIRST_FRAME = "DCAMERA_CONTINUE_FIRST_FRAME";
const std::string DCAMERA_SNAPSHOT_FIRST_FRAME = "DCAMERA_SNAPSHOT_FIRST_FRAME";
enum DcameraTaskId {
    DCAMERA_OPEN_CHANNEL_TASKID = 0,
    DCAMERA_OPEN_DATA_CONTINUE_TASKID = 1,
    DCAMERA_OPEN_DATA_SNAPSHOT_TASKID = 2,
    DCAMERA_CONTINUE_FIRST_FRAME_TASKID = 3,
    DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID = 4,
};

void DcameraStartAsyncTrace(const std::string& str, int32_t taskId);
void DcameraFinishAsyncTrace(const std::string& str, int32_t taskId);
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HITRACE_ADAPTER_H