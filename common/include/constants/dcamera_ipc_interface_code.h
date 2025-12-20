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

#ifndef OHOS_DCAMERA_IPC_INTERFACE_CODE_H
#define OHOS_DCAMERA_IPC_INTERFACE_CODE_H

#include <cstdint>

namespace OHOS {
namespace DistributedHardware {
/* SAID: 4803 */
enum class IDCameraSourceInterfaceCode : uint32_t {
    INIT_SOURCE = 0,
    RELEASE_SOURCE = 1,
    REGISTER_DISTRIBUTED_HARDWARE = 2,
    UNREGISTER_DISTRIBUTED_HARDWARE = 3,
    CAMERA_NOTIFY = 4,
    UPDATE_WORKMODE = 5
};

/* SAID: 4804 */
enum class IDCameraSinkInterfaceCode : uint32_t {
    INIT_SINK = 0,
    RELEASE_SINK = 1,
    SUBSCRIBE_LOCAL_HARDWARE = 2,
    UNSUBSCRIBE_LOCAL_HARDWARE = 3,
    STOP_CAPTURE = 4,
    CHANNEL_NEG = 5,
    GET_CAMERA_INFO = 6,
    OPEN_CHANNEL = 7,
    CLOSE_CHANNEL = 8,
    PAUSE_DISTRIBUTED_HARDWARE = 9,
    RESUME_DISTRIBUTED_HARDWARE = 10,
    STOP_DISTRIBUTED_HARDWARE = 11,
    SET_ACCESS_LISTENER = 12,
    REMOVE_ACCESS_LISTENER = 13,
    SET_AUTHORIZATION_RESULT = 14,
};
} // namespace DistributedHardware
} // namespace OHOS
#endif