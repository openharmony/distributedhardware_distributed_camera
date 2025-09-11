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

#include "dcamera_sink_callback.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkCallback::~DCameraSinkCallback()
{
    privacyResCallback_.clear();
}

int32_t DCameraSinkCallback::OnNotifyResourceInfo(const ResourceEventType &type, const std::string &subtype,
    const std::string &networkId, bool &isSensitive, bool &isSameAccout)
{
    DHLOGI("DCameraSinkCallback OnNotifyResourceInfo type: %{public}d, subtype: %{public}s, networkId: %{public}s, "
        "isSensitive: %{public}d, isSameAccout: %{public}d", (uint32_t)type, subtype.c_str(),
        GetAnonyString(networkId).c_str(), isSensitive, isSameAccout);
    int32_t ret = DCAMERA_OK;
    std::lock_guard<std::mutex> lock(privacyResMutex_);
    auto iter = privacyResCallback_.begin();
    if (iter != privacyResCallback_.end()) {
        if (*iter != nullptr) {
            ret = (*iter)->OnPrivaceResourceMessage(type, subtype, networkId, isSensitive, isSameAccout);
        }
    }
    return ret;
}

int32_t DCameraSinkCallback::OnHardwareStateChanged(const std::string &devId, const std::string &dhId,
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
    BusinessSinkState currentState = static_cast<BusinessSinkState>(status);
    stateListener_->OnStateChanged(devId, dhId, currentState);
    return DCAMERA_OK;
}

void DCameraSinkCallback::PushPrivacyResCallback(const std::shared_ptr<PrivacyResourcesListener> &listener)
{
    DHLOGI("push resource info callback.");
    std::lock_guard<std::mutex> lock(privacyResMutex_);
    privacyResCallback_.push_back(listener);
}

void DCameraSinkCallback::RegisterStateListener(const std::shared_ptr<DistributedHardwareSinkStateListener> listener)
{
    DHLOGD("Register sink state listener.");
    std::lock_guard<std::mutex> stateLck(stateListenerMtx_);
    stateListener_ = listener;
}

void DCameraSinkCallback::UnRegisterStateListener()
{
    DHLOGD("UnRegister sink state listener.");
    std::lock_guard<std::mutex> stateLck(stateListenerMtx_);
    stateListener_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
