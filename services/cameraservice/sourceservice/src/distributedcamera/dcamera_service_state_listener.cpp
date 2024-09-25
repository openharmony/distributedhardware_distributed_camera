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

#include "dcamera_service_state_listener.h"

#include <thread>

#include "anonymous_string.h"
#include "dcamera_index.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_source_service.h"
#include "distributed_hardware_log.h"
#include "ffrt_inner.h"
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
DCameraServiceStateListener::DCameraServiceStateListener()
{
    DHLOGI("DCameraServiceStateListener Create");
}

DCameraServiceStateListener::~DCameraServiceStateListener()
{
    DHLOGI("DCameraServiceStateListener Delete");
    callbackProxy_ = nullptr;
}

void DCameraServiceStateListener::SetCallback(sptr<IDCameraSourceCallback> callback)
{
    DHLOGI("enter");
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    callbackProxy_ = callback;
}

int32_t DCameraServiceStateListener::OnRegisterNotify(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("OnRegisterNotify devId: %{public}s, dhId: %{public}s, status: %{public}d",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), status);
    std::lock_guard<std::mutex> autoLock(proxyMutex_);

    if (status != DCAMERA_OK) {
        ffrt::submit([=]() mutable {
            DHLOGI("thread delete devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(),
                GetAnonyString(dhId).c_str());
            prctl(PR_SET_NAME, REGISTER_SERVICE_NOTIFY.c_str());
            DCameraIndex camIndex(devId, dhId);
            DistributedCameraSourceService::CamDevErase(camIndex);
            if (callbackProxy_ == nullptr) {
                DHLOGE("callbackProxy_ is nullptr");
                return;
            }
            int32_t ret = callbackProxy_->OnNotifyRegResult(devId, dhId, reqId, status, data);
            if (ret != DCAMERA_OK) {
                DHLOGE("OnNotifyRegResult failed: %{public}d", ret);
            }
        });
    } else {
        if (callbackProxy_ == nullptr) {
            DHLOGE("callbackProxy_ is nullptr");
            return DCAMERA_BAD_VALUE;
        }
        int32_t ret = callbackProxy_->OnNotifyRegResult(devId, dhId, reqId, status, data);
        if (ret != DCAMERA_OK) {
            DHLOGE("OnNotifyRegResult failed: %{public}d", ret);
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraServiceStateListener::OnUnregisterNotify(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("OnUnregisterNotify devId: %{public}s, dhId: %{public}s, status: %{public}d",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), status);
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    if (callbackProxy_ == nullptr) {
        DHLOGE("callbackProxy_ is nullptr");
        return DCAMERA_BAD_VALUE;
    }

    if (status == DCAMERA_OK) {
        ffrt::submit([=]() mutable {
            DHLOGI("thread delete devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(),
                GetAnonyString(dhId).c_str());
            prctl(PR_SET_NAME, UNREGISTER_SERVICE_NOTIFY.c_str());
            DCameraIndex camIndex(devId, dhId);
            DistributedCameraSourceService::CamDevErase(camIndex);

            int32_t ret = callbackProxy_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
            if (ret != DCAMERA_OK) {
                DHLOGE("OnNotifyUnregResult failed, ret: %{public}d", ret);
            }
        });
    } else {
        int32_t ret = callbackProxy_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
        if (ret != DCAMERA_OK) {
            DHLOGE("OnNotifyUnregResult failed, ret: %{public}d", ret);
        }
    }

    return DCAMERA_OK;
}

int32_t DCameraServiceStateListener::OnHardwareStateChanged(const std::string &devId,
    const std::string &dhId, int32_t status)
{
    DHLOGI("OnHardwareStateChanged devId: %{public}s, dhId: %{public}s, status: %{public}d",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), status);
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    if (callbackProxy_ == nullptr) {
        DHLOGE("callbackProxy_ is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    return callbackProxy_->OnHardwareStateChanged(devId, dhId, status);
}

int32_t DCameraServiceStateListener::OnDataSyncTrigger(const std::string &devId)
{
    DHLOGI("OnDataSyncTrigger devId: %{public}s.", GetAnonyString(devId).c_str());
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    if (callbackProxy_ == nullptr) {
        DHLOGE("callbackProxy_ is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    return callbackProxy_->OnDataSyncTrigger(devId);
}
} // namespace DistributedHardware
} // namespace OHOS
