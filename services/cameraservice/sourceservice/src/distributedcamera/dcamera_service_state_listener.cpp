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

#include "dcamera_service_state_listener.h"

#include <thread>

#include "anonymous_string.h"
#include "dcamera_index.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_source_service.h"
#include "distributed_hardware_log.h"

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
    DHLOGI("DCameraServiceStateListener SetCallback");
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    callbackProxy_ = callback;
}

int32_t DCameraServiceStateListener::OnRegisterNotify(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("DCameraServiceStateListener OnRegisterNotify devId: %s, dhId: %s, status: %d",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), status);
    std::lock_guard<std::mutex> autoLock(proxyMutex_);

    if (status != DCAMERA_OK) {
        std::thread([=]() mutable {
            DHLOGI("DCameraServiceStateListener OnRegisterNotify thread delete devId: %s dhId: %s",
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            DCameraIndex camIndex(devId, dhId);
            DistributedCameraSourceService::CamDevErase(camIndex);
            if (callbackProxy_ == nullptr) {
                DHLOGE("DCameraServiceStateListener OnRegisterNotify callbackProxy_ is nullptr");
                return;
            }
            int32_t ret = callbackProxy_->OnNotifyRegResult(devId, dhId, reqId, status, data);
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraServiceStateListener OnRegisterNotify OnNotifyRegResult failed: %d", ret);
            }
        }).detach();
    } else {
        if (callbackProxy_ == nullptr) {
            DHLOGE("DCameraServiceStateListener OnRegisterNotify callbackProxy_ is nullptr");
            return DCAMERA_BAD_VALUE;
        }
        int32_t ret = callbackProxy_->OnNotifyRegResult(devId, dhId, reqId, status, data);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraServiceStateListener OnRegisterNotify OnNotifyRegResult failed: %d", ret);
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraServiceStateListener::OnUnregisterNotify(const std::string& devId, const std::string& dhId,
    const std::string& reqId, int32_t status, std::string& data)
{
    DHLOGI("DCameraServiceStateListener OnUnregisterNotify devId: %s, dhId: %s, status: %d",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), status);
    std::lock_guard<std::mutex> autoLock(proxyMutex_);
    if (callbackProxy_ == nullptr) {
        DHLOGE("DCameraServiceStateListener OnUnregisterNotify callbackProxy_ is nullptr");
        return DCAMERA_BAD_VALUE;
    }

    if (status == DCAMERA_OK) {
        std::thread([=]() mutable {
            DHLOGI("DCameraServiceStateListener OnUnregisterNotify thread delete devId: %s dhId: %s",
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            DCameraIndex camIndex(devId, dhId);
            DistributedCameraSourceService::CamDevErase(camIndex);

            int32_t ret = callbackProxy_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraServiceStateListener OnUnregisterNotify failed, ret: %d", ret);
            }
        }).detach();
    } else {
        int32_t ret = callbackProxy_->OnNotifyUnregResult(devId, dhId, reqId, status, data);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraServiceStateListener OnUnregisterNotify failed, ret: %d", ret);
        }
    }

    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
