/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_ERRNO_H
#define OHOS_DISTRIBUTED_CAMERA_ERRNO_H

namespace OHOS {
namespace DistributedHardware {
    enum DistributedCameraErrno {
        DCAMERA_OK = 0,
        ERR_DH_CAMERA_BASE = 0x05C20000,
        DCAMERA_MEMORY_OPT_ERROR = ERR_DH_CAMERA_BASE + 1,
        DCAMERA_BAD_VALUE = ERR_DH_CAMERA_BASE + 2,
        DCAMERA_BAD_TYPE = ERR_DH_CAMERA_BASE + 3,
        DCAMERA_ALREADY_EXISTS = ERR_DH_CAMERA_BASE + 4,
        DCAMERA_INIT_ERR = ERR_DH_CAMERA_BASE + 5,
        DCAMERA_NOT_FOUND = ERR_DH_CAMERA_BASE + 6,
        DCAMERA_WRONG_STATE = ERR_DH_CAMERA_BASE + 7,
        DCAMERA_BAD_OPERATE = ERR_DH_CAMERA_BASE + 8,
        DCAMERA_OPEN_CONFLICT = ERR_DH_CAMERA_BASE + 9,
        DCAMERA_DISABLE_PROCESS = ERR_DH_CAMERA_BASE + 10,
        DCAMERA_INDEX_OVERFLOW = ERR_DH_CAMERA_BASE + 11,
        DCAMERA_REGIST_HAL_FAILED = ERR_DH_CAMERA_BASE + 12,
        DCAMERA_UNREGIST_HAL_FAILED = ERR_DH_CAMERA_BASE + 13,
        DCAMERA_ALLOC_ERROR = ERR_DH_CAMERA_BASE + 14,
        DCAMERA_DEVICE_BUSY = ERR_DH_CAMERA_BASE + 15,
        DCAMERA_ERR_APPLY_RESULT = ERR_DH_CAMERA_BASE + 16,
        DCAMERA_ERR_DLOPEN = ERR_DH_CAMERA_BASE + 17,
        DCAMERA_ERR_PUBLISH_STATE = ERR_DH_CAMERA_BASE + 18,
        DCAMERA_ERR_ALLCONNECT = ERR_DH_CAMERA_BASE + 19,
    };
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_ERRNO_H