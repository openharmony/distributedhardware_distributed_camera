/*
* Copyright (c) 2024-2025 Huawei Device Co., Ltd.
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

#include "distributed_camera_allconnect_manager.h"
#include "anonymous_string.h"
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <thread>

#include "dcamera_protocol.h"
#include "dcamera_softbus_adapter.h"
#include "distributed_hardware_log.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"

namespace OHOS {
namespace DistributedHardware {
#ifdef __LP64__
constexpr const char *ALL_CONNECT_SO_PATH = "/system/lib64/";
#else
constexpr const char *ALL_CONNECT_SO_PATH = "/system/lib/";
#endif
constexpr const char *ALL_CONNECT_SO_NAME = "libcfwk_allconnect_client.z.so";
std::shared_ptr<DCameraBlockObject<bool>> DCameraAllConnectManager::applyResultBlock_;
DCameraAllConnectManager::DCameraAllConnectManager()
{
    allConnectCallback_.onStop = &DCameraAllConnectManager::OnStop;
    allConnectCallback_.applyResult = &DCameraAllConnectManager::ApplyResult;
}

DCameraAllConnectManager &DCameraAllConnectManager::GetInstance()
{
    static DCameraAllConnectManager instance;
    return instance;
}

std::mutex DCameraAllConnectManager::netwkIdSourceSessionIdMapLock_;
std::map<std::string, int> DCameraAllConnectManager::netwkIdSourceSessionIdMap_;
std::mutex DCameraAllConnectManager::netwkIdSinkSessionIdMapLock_;
std::map<std::string, int> DCameraAllConnectManager::netwkIdSinkSessionIdMap_;
bool DCameraAllConnectManager::bInited_ = false;
std::mutex DCameraAllConnectManager::bInitedLock_;

int32_t DCameraAllConnectManager::InitDCameraAllConnectManager()
{
    DHLOGI("DCamera allconnect InitDCameraAllConnectManager begin");
    std::lock_guard<std::mutex> lock(bInitedLock_);
    int32_t ret = GetAllConnectSoLoad();
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        DHLOGE("DCamera allconnect InitDCameraAllConnectManager failed, so not exist or load so error, ret %{public}d",
            ret);
        return ret;
    }

    DHLOGI("DCamera allconnect InitDCameraAllConnectManager success");
    bInited_ = true;
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::UnInitDCameraAllConnectManager()
{
    DHLOGI("DCamera allconnect UnInitDCameraAllConnectManager begin");
    std::lock_guard<std::mutex> lock(bInitedLock_);
    if (dllHandle_ != nullptr) {
        dlclose(dllHandle_);
    }
    dllHandle_ = nullptr;
    bInited_ = false;
    return DistributedCameraErrno::DCAMERA_OK;
}

bool DCameraAllConnectManager::IsInited()
{
    std::lock_guard<std::mutex> lock(bInitedLock_);
    return bInited_;
}

int32_t DCameraAllConnectManager::PublishServiceState(const std::string &peerNetworkId,
    const std::string &dhId, DCameraCollaborationBussinessStatus state)
{
    DHLOGI("DCamera allconnect PublishServiceState begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    dhIdStateMap_[dhId] = state;
    DHLOGI("DCamera allconnect PublishServiceState dhId:%{public}s, state:%{public}d",
        dhId.c_str(), state);
    DCameraCollaborationBussinessStatus maxState = SCM_IDLE;
    std::string dhIdOfmaxState;
    for (const auto& statePair : dhIdStateMap_) {
        if (statePair.second > maxState) {
            maxState = statePair.second;
            dhIdOfmaxState = statePair.first;
        }
    }
    if (dllHandle_ == nullptr) {
        DHLOGE("DCamera allconnect dllHandle_ is nullptr, all connect not support.");
        return DistributedCameraErrno::DCAMERA_OK;
    }
    if (allConnect_.dCameraCollaborationPublishServiceState == nullptr) {
        DHLOGE("DCamera allconnect PublishServiceState is nullptr, all connect function not load.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    DHLOGI("DCamera allconnect PublishDhId:%{public}s, pulishState:%{public}d",
        dhIdOfmaxState.c_str(), maxState);
    int32_t ret = allConnect_.dCameraCollaborationPublishServiceState(peerNetworkId.c_str(),
        SERVICE_NAME.c_str(),
        "", maxState);
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        DHLOGE("DCamera allconnect PublishServiceState %{public}d fail, ret %{public}d", state, ret);
        return DistributedCameraErrno::DCAMERA_ERR_PUBLISH_STATE;
    }
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::ApplyAdvancedResource(const std::string &peerNetworkId,
    DCameraCollaborationResourceRequestInfoSets *resourceRequest)
{
    DHLOGI("DCamera allconnect ApplyAdvancedResource begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        DHLOGE("DCamera allconnect dllHandle_ is nullptr, all connect not support.");
        return DistributedCameraErrno::DCAMERA_OK;
    }
    if (allConnect_.dCameraCollaborationApplyAdvancedResource == nullptr) {
        DHLOGE("DCamera allconnect PublishServiceState is nullptr, all connect function not load.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    if (applyResultBlock_ == nullptr) {
        applyResultBlock_ = std::make_shared<DCameraBlockObject<bool>>(BLOCK_INTERVAL_ALLCONNECT, false);
    }

    int32_t ret = allConnect_.dCameraCollaborationApplyAdvancedResource(peerNetworkId.c_str(),
        SERVICE_NAME.c_str(),
        resourceRequest,
        &allConnectCallback_);
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        DHLOGE("DCamera allconnect ApplyAdvancedResource fail, ret %{public}d", ret);
        return DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT;
    }
    auto success = applyResultBlock_->GetValue();
    if (!success) {
        DHLOGE("DCamera allconnect applyResult is reject");
        return DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT;
    }
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::GetAllConnectSoLoad()
{
    DHLOGI("DCamera allconnect GetAllConnectSoLoad begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    char path[PATH_MAX + 1] = {0x00};
    std::string soPathName = std::string(ALL_CONNECT_SO_PATH) + std::string(ALL_CONNECT_SO_NAME);
    if (soPathName.empty() || (soPathName.length() > PATH_MAX) || (realpath(soPathName.c_str(), path) == nullptr)) {
        DHLOGE("DCamera allconnect all connect so load failed, soPath=%{public}s not exist.", soPathName.c_str());
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    int32_t (*allConnectProxy)(DCameraCollaborationApi *exportapi) = nullptr;

    dllHandle_ = dlopen(path, RTLD_LAZY);
    if (dllHandle_ == nullptr) {
        DHLOGE("DCamera allconnect dlopen fail");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    allConnectProxy = reinterpret_cast<int32_t (*)(DCameraCollaborationApi *exportapi)>(
        dlsym(dllHandle_, "ServiceCollaborationManager_Export"));
    if (allConnectProxy == nullptr) {
        dlclose(dllHandle_);
        dllHandle_ = nullptr;
        DHLOGE("DCamera allconnect dlsym allConnectProxy fail");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    int32_t ret = allConnectProxy(&allConnect_);
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        dlclose(dllHandle_);
        dllHandle_ = nullptr;
        DHLOGE("DCamera allconnect get function struct fail, ret %{public}d", ret);
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }
    DHLOGI("DCamera allconnect so load success");
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::RegisterLifecycleCallback()
{
    DHLOGI("DCamera allconnect RegisterLifecycleCallback begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        DHLOGE("DCamera allconnect dllHandle_ is nullptr, all connect so has not been loaded.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }
    if (allConnect_.dCameraCollaborationRegisterLifecycleCallback == nullptr) {
        DHLOGE("DCamera allconnect RegisterLifecycleCallback is nullptr, all connect function not load.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    int32_t ret = allConnect_.dCameraCollaborationRegisterLifecycleCallback(SERVICE_NAME.c_str(),
        &allConnectCallback_);
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        DHLOGE("DCamera allconnect RegisterLifecycleCallback fail, ret %{public}d", ret);
        return DistributedCameraErrno::DCAMERA_ERR_ALLCONNECT;
    }
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::UnRegisterLifecycleCallback()
{
    DHLOGI("DCamera allconnect UnRegisterLifecycleCallback begin");
    std::lock_guard<std::mutex> lock(allConnectLock_);
    if (dllHandle_ == nullptr) {
        DHLOGE("DCamera allconnect dllHandle_ is nullptr, all connect so has not been loaded.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }
    if (allConnect_.dCameraCollaborationUnRegisterLifecycleCallback == nullptr) {
        DHLOGE("DCamera allconnect UnRegisterLifecycleCallback is nullptr, all connect function not load.");
        return DistributedCameraErrno::DCAMERA_ERR_DLOPEN;
    }

    int32_t ret = allConnect_.dCameraCollaborationUnRegisterLifecycleCallback(SERVICE_NAME.c_str());
    if (ret != DistributedCameraErrno::DCAMERA_OK) {
        DHLOGE("DCamera allconnect UnRegisterLifecycleCallback fail, ret %{public}d", ret);
        return DistributedCameraErrno::DCAMERA_ERR_ALLCONNECT;
    }
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::ApplyResult(int32_t errorcode, int32_t result, const char *reason)
{
    CHECK_AND_RETURN_RET_LOG(applyResultBlock_ == nullptr, DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT,
        "DCamera allconnect ApplyResult applyResultBlock_ is nullptr");
    DHLOGI("DCamera allconnect ApplyResult begin");
    if (result != PASS) {
        DHLOGE("DCamera allconnect Apply Result is Reject, errorcode is %{public}d, reason is %{public}s",
            errorcode, reason);
        applyResultBlock_->SetValue(false);
        return DistributedCameraErrno::DCAMERA_ERR_APPLY_RESULT;
    }
    applyResultBlock_->SetValue(true);
    return DistributedCameraErrno::DCAMERA_OK;
}

int32_t DCameraAllConnectManager::OnStop(const char *peerNetworkId)
{
    DHLOGI("DCamera allconnect OnStop begin peerNetworkId:%{public}s", GetAnonyString(peerNetworkId).c_str());
    DCameraSoftbusAdapter::GetInstance().CloseSessionWithNetWorkId(peerNetworkId);

    return DistributedCameraErrno::DCAMERA_OK;
}

std::shared_ptr<DCameraCollaborationResourceRequestInfoSets> DCameraAllConnectManager::BuildResourceRequest()
{
    auto resourceRequest = std::make_shared<DCameraCollaborationResourceRequestInfoSets>();

    if (remoteHardwareList_ == nullptr) {
        remoteHardwareList_ = std::make_shared<DCameraCollaboration_HardwareRequestInfo>();
        remoteHardwareList_->hardWareType = DCameraCollaborationHardwareType::SCM_CAMERA;
        remoteHardwareList_->canShare = true;
    }
    resourceRequest->remoteHardwareListSize = 1;
    resourceRequest->remoteHardwareList = remoteHardwareList_.get();

    if (localHardwareList_ == nullptr) {
        localHardwareList_ = std::make_shared<DCameraCollaboration_HardwareRequestInfo>();
        localHardwareList_->hardWareType = DCameraCollaborationHardwareType::SCM_CAMERA;
        localHardwareList_->canShare = true;
    }
    resourceRequest->localHardwareListSize = 1;
    resourceRequest->localHardwareList = localHardwareList_.get();

    if (communicationRequest_ == nullptr) {
        communicationRequest_ = std::make_shared<DCameraCollaborationCommunicationRequestInfo>();
        communicationRequest_->minBandwidth = DCAMERA_QOS_TYPE_MIN_BW;
        communicationRequest_->maxLatency = DCAMERA_QOS_TYPE_MAX_LATENCY;
        communicationRequest_->minLatency = DCAMERA_QOS_TYPE_MIN_LATENCY;
        communicationRequest_->maxWaitTime = 0;
        communicationRequest_->dataType = "DATA_TYPE_VIDEO_STREAM";
    }
    resourceRequest->communicationRequest = communicationRequest_.get();

    return resourceRequest;
}

void  DCameraAllConnectManager::RemoveSinkNetworkId(int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(netwkIdSinkSessionIdMapLock_);
    for (auto it: netwkIdSinkSessionIdMap_) {
        if (it.second == sessionId) {
            netwkIdSinkSessionIdMap_.erase(it.first);
            break;
        }
    }
}

void DCameraAllConnectManager::RemoveSourceNetworkId(int32_t sessionId)
{
    std::lock_guard<std::mutex> lock(netwkIdSourceSessionIdMapLock_);
    for (auto it: netwkIdSourceSessionIdMap_) {
        if (it.second == sessionId) {
            netwkIdSourceSessionIdMap_.erase(it.first);
            break;
        }
    }
}

void DCameraAllConnectManager::SetSourceNetworkId(const std::string &networkId, int32_t socket)
{
    if (networkId.empty()) {
        return ;
    }
    if (socket < 0) {
        return ;
    }
    std::lock_guard<std::mutex> lock(netwkIdSourceSessionIdMapLock_);
    netwkIdSourceSessionIdMap_[networkId] = socket;
}

void DCameraAllConnectManager::SetSinkNetWorkId(const std::string &networkId, int32_t socket)
{
    if (networkId.empty()) {
        return ;
    }
    if (socket < 0) {
        return ;
    }
    std::lock_guard<std::mutex> lock(netwkIdSinkSessionIdMapLock_);
    netwkIdSinkSessionIdMap_[networkId] = socket;
}

std::string DCameraAllConnectManager::GetSinkDevIdBySocket(int32_t socket)
{
    std::lock_guard<std::mutex> lock(netwkIdSinkSessionIdMapLock_);
    for (auto it: netwkIdSinkSessionIdMap_) {
        if (it.second == socket) {
            return it.first;
        }
    }
    return "";
}

int32_t DCameraAllConnectManager::GetSinkSocketByNetWorkId(const std::string &networkId)
{
    int32_t sessionId = -1;
    std::lock_guard<std::mutex> lock(netwkIdSinkSessionIdMapLock_);
    auto it = netwkIdSinkSessionIdMap_.find(networkId);
    if (it != netwkIdSinkSessionIdMap_.end()) {
        sessionId = it->second;
    }
    return sessionId;
}

int32_t DCameraAllConnectManager::GetSourceSocketByNetworkId(const std::string &networkId)
{
    int32_t sessionId = -1;
    std::lock_guard<std::mutex> lock(netwkIdSourceSessionIdMapLock_);
    auto it = netwkIdSourceSessionIdMap_.find(networkId);
    if (it != netwkIdSourceSessionIdMap_.end()) {
        sessionId = it->second;
    }
    return sessionId;
}
} // namespace DistributedHardware
} // namespace OHOS