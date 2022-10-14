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

#include "dcamera_source_handler.h"

#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_source_callback.h"
#include "dcamera_source_handler_ipc.h"
#include "dcamera_source_load_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraSourceHandler);

DCameraSourceHandler::DCameraSourceHandler()
{
    DHLOGI("DCameraSourceHandler construct.");
    callback_ = new DCameraSourceCallback();
}

DCameraSourceHandler::~DCameraSourceHandler()
{
    DHLOGI("~DCameraSourceHandler");
}

int32_t DCameraSourceHandler::InitSource(const std::string& params)
{
    DHLOGI("DCameraSourceHandler InitSource Start");
    {
        std::lock_guard<std::mutex> autoLock(producerMutex_);
        if (state_ == DCAMERA_SA_STATE_START) {
            return DCAMERA_OK;
        }
    }
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        DHLOGE("GetSourceLocalCamSrv GetSystemAbilityManager failed");
        return DCAMERA_INIT_ERR;
    }
    ReportSaEvent(INIT_SA_EVENT, DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, "init source sa event.");
    sptr<DCameraSourceLoadCallback> loadCallback = new DCameraSourceLoadCallback(params);
    int32_t ret = sm->LoadSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, loadCallback);
    if (ret != ERR_OK) {
        DHLOGE("systemAbilityId: %d load failed, result code: %d.", DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, ret);
        return DCAMERA_INIT_ERR;
    }
    {
        uint32_t interval = 1;
        std::unique_lock<std::mutex> lock(producerMutex_);
        producerCon_.wait_for(lock, std::chrono::minutes(interval), [this] {
            return (this->state_ == DCAMERA_SA_STATE_START);
        });
        if (state_ == DCAMERA_SA_STATE_STOP) {
            DHLOGE("SourceSA Start failed!");
            return DCAMERA_INIT_ERR;
        }
    }
    DHLOGI("DCameraSourceHandler InitSource end, result: %d", ret);
    return DCAMERA_OK;
}

void DCameraSourceHandler::FinishStartSA(const std::string &params)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("DCameraSourceHandler InitSource get Service failed");
        return;
    }

    dCameraSourceSrv->InitSource(params, callback_);
    std::lock_guard<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_START;
    producerCon_.notify_one();
}

void DCameraSourceHandler::FinishStartSAFailed(const int32_t systemAbilityId)
{
    DHLOGE("SourceSA Start failed, systemAbilityId: %d.", systemAbilityId);
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    producerCon_.notify_one();
}

int32_t DCameraSourceHandler::ReleaseSource()
{
    DHLOGI("DCameraSourceHandler ReleaseSource Start");
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("DCameraSourceHandler ReleaseSource get Service failed");
        return DCAMERA_INIT_ERR;
    }
    ReportSaEvent(RELEASE_SA_EVENT, DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, "release source sa event.");
    dCameraSourceSrv->ReleaseSource();
    DCameraSourceHandlerIpc::GetInstance().UnInit();
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    DHLOGI("DCameraSourceHandler ReleaseSource end");
    return DCAMERA_OK;
}

int32_t DCameraSourceHandler::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const EnableParam& param, std::shared_ptr<RegisterCallback> callback)
{
    DHLOGI("DCameraSourceHandler RegisterDistributedHardware devId: %s dhId: %s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    if (callback == nullptr) {
        DHLOGI("DCameraSourceHandler RegisterDistributedHardware RegisterCallback is null.");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("DCameraSourceHandler RegisterDistributedHardware get Service failed");
        return DCAMERA_BAD_VALUE;
    }

    std::string reqId = GetRandomID();
    std::lock_guard<std::mutex> autoLock(optLock_);
    
    callback_->PushRegCallback(reqId, callback);
    int32_t ret = dCameraSourceSrv->RegisterDistributedHardware(devId, dhId, reqId, param);
    if (ret != DCAMERA_OK) {
        callback_->PopRegCallback(reqId);
    }
    DHLOGI("DCameraSourceHandler RegisterDistributedHardware end, ret: %d devId: %s dhId: %s version: %s",
        ret, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.version.c_str());
    return ret;
}

int32_t DCameraSourceHandler::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    std::shared_ptr<UnregisterCallback> callback)
{
    DHLOGI("DCameraSourceHandler UnregisterDistributedHardware devId: %s dhId: %s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    if (callback == nullptr) {
        DHLOGI("DCameraSourceHandler RegisterDistributedHardware UnregisterCallback is null.");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("DCameraSourceHandler UnregisterDistributedHardware get Service failed");
        return DCAMERA_BAD_VALUE;
    }

    std::string reqId = GetRandomID();
    std::lock_guard<std::mutex> autoLock(optLock_);

    callback_->PushUnregCallback(reqId, callback);
    int32_t ret = dCameraSourceSrv->UnregisterDistributedHardware(devId, dhId, reqId);
    if (ret != DCAMERA_OK) {
        callback_->PopUnregCallback(reqId);
    }
    DHLOGI("DCameraSourceHandler UnregisterDistributedHardware end, ret: %d devId: %s dhId: %s", ret,
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    return ret;
}

int32_t DCameraSourceHandler::ConfigDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& key, const std::string& value)
{
    return DCAMERA_OK;
}

IDistributedHardwareSource *GetSourceHardwareHandler()
{
    DHLOGI("DCameraSourceHandler GetSourceHardwareHandler Start");
    return &DCameraSourceHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
