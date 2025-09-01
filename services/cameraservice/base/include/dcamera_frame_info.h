/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_FRAME_INFO_H
#define OHOS_DCAMERA_FRAME_INFO_H
#include <string>
#include <dcamera_sink_frame_info.h>

namespace OHOS {
namespace DistributedHardware {
struct DCameraFrameProcessTimePoint {
    int64_t startEncode = 0;
    int64_t finishEncode = 0;
    int64_t send = 0;
    int64_t recv = 0;
    int64_t startDecode = 0;
    int64_t finishDecode = 0;
    int64_t startScale = 0;
    int64_t finishScale = 0;
    int64_t startSmooth = 0;
    int64_t finishSmooth = 0;
};
struct DCameraFrameInfo {
    int8_t type = 0;
    std::string ver = "";
    int32_t index = 0;
    int32_t offset = 0;
    int64_t pts = 0;
    int64_t rawTime = 0;
    DCameraFrameProcessTimePoint timePonit {0};
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_FRAME_INFO_H