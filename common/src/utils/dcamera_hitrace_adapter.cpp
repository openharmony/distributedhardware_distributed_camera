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

#include "dcamera_hitrace_adapter.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
void DcameraStartAsyncTrace(const std::string& str, int32_t taskId)
{
    DHLOGI("cmh++ DcameraStartAsyncTrace str: %s, taskId %d", str.c_str(), taskId);
    StartAsyncTrace(DCAMERA_HITRACE_LABEL, str, taskId, -1);
}

void DcameraFinishAsyncTrace(const std::string& str, int32_t taskId)
{
    DHLOGI("cmh++ DcameraFinishAsyncTrace str: %s, taskId %d", str.c_str(), taskId);
    FinishAsyncTrace(DCAMERA_HITRACE_LABEL, str, taskId);
}
} // namespace DistributedHardware
} // namespace OHOS