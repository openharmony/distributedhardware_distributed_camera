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

#ifndef OHOS_DCAMERA_COLLABORATION_MOCK_H
#define OHOS_DCAMERA_COLLABORATION_MOCK_H

#include "dcamera_collaboration_manager_capi.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraCollaborationMock {
public:
    DCameraCollaborationMock() = default;
    virtual ~DCameraCollaborationMock() = default;

    static int32_t DCameraCollaborationExport(DCameraCollaborationApi *exportApi);
private:
    static int32_t CameraCollaborationPublishServiceState(const char *peerNetworkId, const char *serviceName,
        const char *extraInfo, DCameraCollaborationBussinessStatus state);
    static int32_t CameraCollaborationApplyAdvancedResource(const char *peerNetworkId, const char *serviceName,
        DCameraCollaborationResourceRequestInfoSets *resourceRequest,
        DCameraCollaborationCallback *callback);
    static int32_t CameraCollaborationRegisterLifecycleCallback(const char *serviceName,
        DCameraCollaborationCallback *callback);
    static int32_t CameraCollaborationUnRegisterLifecycleCallback(const char *serviceName);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_COLLABORATION_MOCK_H
