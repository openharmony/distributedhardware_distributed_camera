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

#ifndef OHOS_SMOTTHER_CONSTANTS_H
#define OHOS_SMOTTHER_CONSTANTS_H

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    SMOOTH_START = 0,
    SMOOTH_STOP = 1,
} SmoothState;

typedef enum {
    SMOOTH_SUCCESS = 0,
    SMOOTH_IS_STARTED = 1,
    SMOOTH_IS_STOPED = 2,
} SmoothErrorCode;

typedef enum {
    NOTIFY_SUCCESS = 0,
    NOTIFY_FAILED = 1,
} SmootherListnerState;
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_SMOTTHER_CONSTANTS_H