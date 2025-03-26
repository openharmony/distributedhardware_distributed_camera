/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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

#ifndef DISTRIBUTEDCAMERA_ALL_CONNECT_MANAGER_H
#define DISTRIBUTEDCAMERA_ALL_CONNECT_MANAGER_H

#include <memory>
#include <map>
#include <mutex>
#include <string>

#include "dcamera_block_obj.h"
#include "dcamera_collaboration_manager_capi.h"
#include "distributed_camera_errno.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraAllConnectManager {
public:
    static DCameraAllConnectManager &GetInstance();
    int32_t InitDCameraAllConnectManager();
    int32_t UnInitDCameraAllConnectManager();
    int32_t PublishServiceState(const std::string &peerNetworkId, const std::string &dhId,
                                DCameraCollaborationBussinessStatus state);
    int32_t ApplyAdvancedResource(const std::string &peerNetworkId,
                                  DCameraCollaborationResourceRequestInfoSets *resourceRequest);
    std::shared_ptr<DCameraCollaborationResourceRequestInfoSets> BuildResourceRequest();

    static void SetSourceNetworkId(const std::string &networkId, int32_t socket);
    static void SetSinkNetWorkId(const std::string &networkId, int32_t socket);
    static void RemoveSinkNetworkId(int32_t sessionId);
    static void RemoveSourceNetworkId(int32_t sessionId);
    static std::string GetSinkDevIdBySocket(int32_t socket);
    static int32_t GetSinkSocketByNetWorkId(const std::string &networkId);
    static int32_t GetSourceSocketByNetworkId(const std::string &networkId);
    int32_t RegisterLifecycleCallback();
    int32_t UnRegisterLifecycleCallback();
    static bool IsInited();
private:
    DCameraAllConnectManager();
    ~DCameraAllConnectManager() = default;
    int32_t GetAllConnectSoLoad();

    static int32_t OnStop(const char *peerNetworkId);
    static int32_t ApplyResult(int32_t errorcode, int32_t result, const char *reason);

    std::mutex allConnectLock_;
    std::map<std::string, DCameraCollaborationBussinessStatus> dhIdStateMap_;
    void *dllHandle_ = nullptr;

    DCameraCollaborationApi allConnect_ = {
        .dCameraCollaborationPublishServiceState = nullptr,
        .dCameraCollaborationApplyAdvancedResource = nullptr,
        .dCameraCollaborationRegisterLifecycleCallback = nullptr,
        .dCameraCollaborationUnRegisterLifecycleCallback = nullptr,
    };
    DCameraCollaborationCallback allConnectCallback_;
    std::shared_ptr<DCameraCollaboration_HardwareRequestInfo> remoteHardwareList_;
    std::shared_ptr<DCameraCollaboration_HardwareRequestInfo> localHardwareList_;
    std::shared_ptr<DCameraCollaborationCommunicationRequestInfo> communicationRequest_;

    static std::shared_ptr<DCameraBlockObject<bool>> applyResultBlock_;
    static constexpr uint32_t BLOCK_INTERVAL_ALLCONNECT = 60 * 1000;
    static inline const std::string SERVICE_NAME {"DistributedCamera"};

    static std::map<std::string, int32_t> netwkIdSourceSessionIdMap_;
    static std::mutex netwkIdSourceSessionIdMapLock_;

    static std::map<std::string, int32_t> netwkIdSinkSessionIdMap_;
    static std::mutex netwkIdSinkSessionIdMapLock_;

    static bool bInited_;
    static std::mutex bInitedLock_;
};

} // namespace DistributedHardware
} // namespace OHOS
#endif // DISTRIBUTEDCAMERA_ALL_CONNECT_MANAGER_H