/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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
#include "dcamera_hdf_operate.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_radar.h"
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
    callback_ = sptr<DCameraSourceCallback>(new DCameraSourceCallback());
}

DCameraSourceHandler::~DCameraSourceHandler()
{
    DHLOGI("~DCameraSourceHandler");
}

int32_t DCameraSourceHandler::InitSource(const std::string& params)
{
    DHLOGI("Start");
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
    sptr<DCameraSourceLoadCallback> loadCallback(new DCameraSourceLoadCallback(params));
    int32_t ret = sm->LoadSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, loadCallback);
    DcameraRadar::GetInstance().ReportDcameraInit("LoadSystemAbility", CameraInit::SERVICE_INIT,
        BizState::BIZ_STATE_START, ret);
    if (ret != ERR_OK) {
        DHLOGE("systemAbilityId: %{public}d load failed, result code: %{public}d.",
            DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, ret);
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
    DHLOGI("end, result: %{public}d", ret);
    return DCAMERA_OK;
}

void DCameraSourceHandler::FinishStartSA(const std::string &params)
{
    DCameraSourceHandlerIpc::GetInstance().Init();
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("get Service failed");
        return;
    }

    DcameraRadar::GetInstance().ReportDcameraInitProgress("InitSource", CameraInit::SOURCE_CAMERA_INIT, DCAMERA_OK);
    dCameraSourceSrv->InitSource(params, callback_);
    std::lock_guard<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_START;
    producerCon_.notify_one();
}

void DCameraSourceHandler::FinishStartSAFailed(const int32_t systemAbilityId)
{
    DHLOGE("SourceSA Start failed, systemAbilityId: %{public}d.", systemAbilityId);
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    producerCon_.notify_one();
}

int32_t DCameraSourceHandler::ReleaseSource()
{
    DHLOGI("Start");
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("get Service failed");
        return DCAMERA_INIT_ERR;
    }
    ReportSaEvent(RELEASE_SA_EVENT, DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, "release source sa event.");
    dCameraSourceSrv->ReleaseSource();
    DCameraSourceHandlerIpc::GetInstance().UnInit();
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    DHLOGI("end");
    return DCAMERA_OK;
}

int32_t DCameraSourceHandler::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const EnableParam& param, std::shared_ptr<RegisterCallback> callback)
{
    DHLOGI("devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    if (callback == nullptr) {
        DHLOGI("RegisterCallback is null.");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("get Service failed");
        return DCAMERA_BAD_VALUE;
    }

    std::string reqId = GetRandomID();
    std::lock_guard<std::mutex> autoLock(optLock_);

    CHECK_AND_RETURN_RET_LOG(callback_ == nullptr, DCAMERA_BAD_VALUE, "ipc callback is null.");
    callback_->PushRegCallback(reqId, callback);
    int32_t ret = dCameraSourceSrv->RegisterDistributedHardware(devId, dhId, reqId, param);
    if (ret != DCAMERA_OK) {
        callback_->PopRegCallback(reqId);
    }
    DHLOGI("end, ret: %{public}d devId: %{public}s dhId: %{public}s version: %{public}s",
        ret, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.sinkVersion.c_str());
    return ret;
}

int32_t DCameraSourceHandler::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    std::shared_ptr<UnregisterCallback> callback)
{
    DHLOGI("devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    if (callback == nullptr) {
        DHLOGI("UnregisterCallback is null.");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    if (dCameraSourceSrv == nullptr) {
        DHLOGE("get Service failed");
        return DCAMERA_BAD_VALUE;
    }

    std::string reqId = GetRandomID();
    std::lock_guard<std::mutex> autoLock(optLock_);

    CHECK_AND_RETURN_RET_LOG(callback_ == nullptr, DCAMERA_BAD_VALUE, "ipc callback is null.");
    callback_->PushUnregCallback(reqId, callback);
    int32_t ret = dCameraSourceSrv->UnregisterDistributedHardware(devId, dhId, reqId);
    if (ret != DCAMERA_OK) {
        callback_->PopUnregCallback(reqId);
    }
    DHLOGI("end, ret: %{public}d devId: %{public}s dhId: %{public}s", ret,
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    return ret;
}

int32_t DCameraSourceHandler::ConfigDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& key, const std::string& value)
{
    return DCAMERA_OK;
}

void DCameraSourceHandler::RegisterDistributedHardwareStateListener(
    std::shared_ptr<DistributedHardwareStateListener> listener)
{
    CHECK_AND_RETURN_LOG(callback_ == nullptr, "%{public}s", "ipc callback is null.");
    callback_->RegisterStateListener(listener);
}

void DCameraSourceHandler::UnregisterDistributedHardwareStateListener()
{
    CHECK_AND_RETURN_LOG(callback_ == nullptr, "%{public}s", "ipc callback is null.");
    callback_->UnRegisterStateListener();
}

void DCameraSourceHandler::RegisterDataSyncTriggerListener(std::shared_ptr<DataSyncTriggerListener> listener)
{
    CHECK_AND_RETURN_LOG(callback_ == nullptr, "%{public}s", "ipc callback is null.");
    callback_->RegisterTriggerListener(listener);
}

void DCameraSourceHandler::UnregisterDataSyncTriggerListener()
{
    CHECK_AND_RETURN_LOG(callback_ == nullptr, "%{public}s", "ipc callback is null.");
    callback_->UnRegisterTriggerListener();
}

int32_t DCameraSourceHandler::LoadDistributedHDF(std::shared_ptr<HdfDeathCallback> callback)
{
    return DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl(callback);
}

int32_t DCameraSourceHandler::UnLoadDistributedHDF()
{
    return DCameraHdfOperate::GetInstance().UnLoadDcameraHDFImpl();
}

void DCameraSourceHandler::SetSAState()
{
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
}

int32_t DCameraSourceHandler::UpdateDistributedHardwareWorkMode(const std::string& devId, const std::string& dhId,
    const WorkModeParam& param)
{
    DHLOGI("update camera workmode, devId: %{public}s dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    sptr<IDistributedCameraSource> dCameraSourceSrv = DCameraSourceHandlerIpc::GetInstance().GetSourceLocalCamSrv();
    CHECK_AND_RETURN_RET_LOG(dCameraSourceSrv == nullptr, DCAMERA_BAD_VALUE, "get service failed");
    int32_t ret = dCameraSourceSrv->UpdateDistributedHardwareWorkMode(devId, dhId, param);
    if (ret != DCAMERA_OK) {
        DHLOGE("update camera workmode failed, ret:%{public}d", ret);
    }
    return ret;
}

IDistributedHardwareSource *GetSourceHardwareHandler()
{
    DHLOGI("DCameraSourceHandler GetSourceHardwareHandler Start");
    return &DCameraSourceHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS
