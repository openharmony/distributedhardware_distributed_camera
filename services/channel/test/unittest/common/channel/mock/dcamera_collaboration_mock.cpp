/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dcamera_collaboration_mock.h"
#include "distributed_camera_errno.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraCollaborationMock::CameraCollaborationPublishServiceState(const char *peerNetworkId,
    const char *serviceName, const char *extraInfo, DCameraCollaborationBussinessStatus state)
{
    if (peerNetworkId == nullptr || strlen(peerNetworkId) == 0) {
        return -1;
    }
    return 0;
}

int32_t DCameraCollaborationMock::CameraCollaborationApplyAdvancedResource(const char *peerNetworkId,
    const char *serviceName,
    DCameraCollaborationResourceRequestInfoSets *resourceRequest,
    DCameraCollaborationCallback *callback)
{
    if (resourceRequest == nullptr || callback == nullptr || callback->applyResult == nullptr) {
        return -1;
    }
    if (peerNetworkId == nullptr || strlen(peerNetworkId) == 0) {
        callback->applyResult(DCAMERA_ERR_APPLY_RESULT, REJECT, "failed");
    } else {
        callback->applyResult(DCAMERA_OK, PASS, "success");
        if (callback->onStop) {
            callback->onStop(peerNetworkId);
        }
    }
    return 0;
}

int32_t DCameraCollaborationMock::CameraCollaborationRegisterLifecycleCallback(const char *serviceName,
    DCameraCollaborationCallback *callback)
{
    return 0;
}

int32_t DCameraCollaborationMock::CameraCollaborationUnRegisterLifecycleCallback(const char *serviceName)
{
    return 0;
}

int32_t DCameraCollaborationMock::DCameraCollaborationExport(DCameraCollaborationApi *exportApi)
{
    if (!exportApi) {
        return -1;
    }
    exportApi->dCameraCollaborationPublishServiceState = CameraCollaborationPublishServiceState;
    exportApi->dCameraCollaborationApplyAdvancedResource = CameraCollaborationApplyAdvancedResource;
    exportApi->dCameraCollaborationRegisterLifecycleCallback = CameraCollaborationRegisterLifecycleCallback;
    exportApi->dCameraCollaborationUnRegisterLifecycleCallback = CameraCollaborationUnRegisterLifecycleCallback;
    return 0;
}
} // namespace DistributedHardware
} // namespace OHOS
