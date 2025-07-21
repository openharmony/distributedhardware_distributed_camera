/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dcamera_source_callback.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceCallback::~DCameraSourceCallback()
{
    regCallbacks_.clear();
    unregCallbacks_.clear();
}

int32_t DCameraSourceCallback::OnNotifyRegResult(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("DCameraSourceCallback OnNotifyRegResult devId: %{public}s dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto iter = regCallbacks_.find(reqId);
    if (iter == regCallbacks_.end()) {
        DHLOGE("DCameraSourceCallback OnNotifyRegResult not found devId: %{public}s dhId: %{public}s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return DCAMERA_NOT_FOUND;
    }

    if (iter->second == nullptr) {
        DHLOGE("DCameraSourceCallback OnNotifyRegResult callback is null");
        regCallbacks_.erase(iter);
        return DCAMERA_BAD_VALUE;
    }
    
    int32_t ret = iter->second->OnRegisterResult(devId, dhId, status, data);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceCallback OnNotifyRegResult failed, devId: %{public}s dhId: %{public}s ret: %{public}d",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), ret);
    }
    regCallbacks_.erase(iter);
    return ret;
}

int32_t DCameraSourceCallback::OnNotifyUnregResult(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("DCameraSourceCallback OnNotifyUnregResult devId: %{public}s dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    std::lock_guard<std::mutex> lock(mapMutex_);
    auto iter = unregCallbacks_.find(reqId);
    if (iter == unregCallbacks_.end()) {
        DHLOGE("DCameraSourceCallback OnNotifyUnregResult not found devId: %{public}s dhId: %{public}s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return DCAMERA_NOT_FOUND;
    }
    if (iter->second == nullptr) {
        DHLOGE("DCameraSourceCallback OnNotifyUnregResult callback is null");
        unregCallbacks_.erase(iter);
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = iter->second->OnUnregisterResult(devId, dhId, status, data);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceCallback OnNotifyUnregResult failed, devId: %{public}s dhId: %{public}s ret: %{public}d",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), ret);
    }
    unregCallbacks_.erase(iter);
    return ret;
}

int32_t DCameraSourceCallback::OnHardwareStateChanged(const std::string &devId, const std::string &dhId,
    int32_t status)
{
    DHLOGI("On hardware state changed, devId: %{public}s, dhId: %{public}s, status: %{public}d",
        GetAnonyString(devId).c_str(), dhId.c_str(), status);
    std::lock_guard<std::mutex> stateLck(stateListenerMtx_);
    if (stateListener_ == nullptr) {
        DHLOGE("State listener is null.");
        return DCAMERA_BAD_VALUE;
    }
    if (devId.empty() || devId.size() > DID_MAX_SIZE || dhId.empty() || dhId.size() > DID_MAX_SIZE) {
        DHLOGE("devId or dhId is invalid");
        return DCAMERA_BAD_VALUE;
    }
    if (status < 0) {
        DHLOGE("status in invalid.");
        return DCAMERA_BAD_VALUE;
    }
    BusinessState currentState = static_cast<BusinessState>(status);
    stateListener_->OnStateChanged(devId, dhId, currentState);
    return DCAMERA_OK;
}

int32_t DCameraSourceCallback::OnDataSyncTrigger(const std::string &devId)
{
    DHLOGI("On data sync trigger, devId: %{public}s", GetAnonyString(devId).c_str());
    std::lock_guard<std::mutex> triggerLck(triggerListenerMtx_);
    if (devId.empty() || devId.size() > DID_MAX_SIZE) {
        DHLOGE("devId is invalid");
        return DCAMERA_BAD_VALUE;
    }
    if (triggerListener_ == nullptr) {
        DHLOGE("Trigger listener is null.");
        return DCAMERA_BAD_VALUE;
    }
    triggerListener_->OnDataSyncTrigger(devId);
    return DCAMERA_OK;
}

void DCameraSourceCallback::PushRegCallback(std::string& reqId, std::shared_ptr<RegisterCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    regCallbacks_.emplace(reqId, callback);
}

void DCameraSourceCallback::PopRegCallback(std::string& reqId)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    regCallbacks_.erase(reqId);
}

void DCameraSourceCallback::PushUnregCallback(std::string& reqId, std::shared_ptr<UnregisterCallback>& callback)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    unregCallbacks_.emplace(reqId, callback);
}

void DCameraSourceCallback::PopUnregCallback(std::string& reqId)
{
    std::lock_guard<std::mutex> lock(mapMutex_);
    unregCallbacks_.erase(reqId);
}

void DCameraSourceCallback::RegisterStateListener(const std::shared_ptr<DistributedHardwareStateListener> listener)
{
    DHLOGD("Register state listener.");
    std::lock_guard<std::mutex> stateLck(stateListenerMtx_);
    stateListener_ = listener;
}

void DCameraSourceCallback::UnRegisterStateListener()
{
    DHLOGD("UnRegister state listener.");
    std::lock_guard<std::mutex> stateLck(stateListenerMtx_);
    stateListener_ = nullptr;
}

void DCameraSourceCallback::RegisterTriggerListener(const std::shared_ptr<DataSyncTriggerListener> listener)
{
    DHLOGD("Register trigger listener.");
    std::lock_guard<std::mutex> triggerLck(triggerListenerMtx_);
    triggerListener_ = listener;
}

void DCameraSourceCallback::UnRegisterTriggerListener()
{
    DHLOGD("UnRegister trigger listener.");
    std::lock_guard<std::mutex> triggerLck(triggerListenerMtx_);
    triggerListener_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
