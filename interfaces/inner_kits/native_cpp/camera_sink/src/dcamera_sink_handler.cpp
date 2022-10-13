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

#include "dcamera_sink_handler.h"

#include <chrono>
#include <new>
#include <string>

#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_sink_handler_ipc.h"
#include "dcamera_sink_load_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "idistributed_camera_sink.h"
#include "isystem_ability_load_callback.h"
#include "refbase.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraSinkHandler);

DCameraSinkHandler::~DCameraSinkHandler()
{
    DHLOGI("~DCameraSinkHandler");
}

int32_t DCameraSinkHandler::InitSink(const std::string& params)
{
    DHLOGI("DCameraSinkHandler::InitSink");
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
    ReportSaEvent(INIT_SA_EVENT, DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, "init sink sa event.");
    sptr<DCameraSinkLoadCallback> loadCallback = new DCameraSinkLoadCallback(params);
    int32_t ret = sm->LoadSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, loadCallback);
    if (ret != DCAMERA_OK) {
        DHLOGE("systemAbilityId: %d load filed, result code: %d.", DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, ret);
        return DCAMERA_INIT_ERR;
    }
    {
        uint32_t interval = 1;
        std::unique_lock<std::mutex> lock(producerMutex_);
        producerCon_.wait_for(lock, std::chrono::minutes(interval), [this] {
            return (this->state_ == DCAMERA_SA_STATE_START);
        });
        if (state_ == DCAMERA_SA_STATE_STOP) {
            DHLOGE("SinkSA Start failed!");
            return DCAMERA_INIT_ERR;
        }
    }
    DHLOGI("DCameraSinkHandler InitSink end, result: %d", ret);
    return DCAMERA_OK;
}

void DCameraSinkHandler::FinishStartSA(const std::string& params)
{
    DCameraSinkHandlerIpc::GetInstance().Init();
    sptr<IDistributedCameraSink> dCameraSinkSrv = DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
    if (dCameraSinkSrv == nullptr) {
        DHLOGE("DCameraSinkHandler::InitSink get Service failed");
        return;
    }
    dCameraSinkSrv->InitSink(params);
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_START;
    producerCon_.notify_one();
}

void DCameraSinkHandler::FinishStartSAFailed(int32_t systemAbilityId)
{
    DHLOGI("SinkSA Start failed, systemAbilityId: %d.", systemAbilityId);
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    producerCon_.notify_one();
}

int32_t DCameraSinkHandler::ReleaseSink()
{
    DHLOGI("DCameraSinkHandler::ReleaseSink");
    sptr<IDistributedCameraSink> dCameraSinkSrv = DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
    if (dCameraSinkSrv == nullptr) {
        DHLOGE("DCameraSinkHandler::ReleaseSink get Service failed");
        return DCAMERA_BAD_VALUE;
    }

    ReportSaEvent(RELEASE_SA_EVENT, DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, "release sink sa event.");
    int32_t ret = dCameraSinkSrv->ReleaseSink();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSinkHandler::ReleaseSink sink service release failed, ret: %d", ret);
        return ret;
    }

    DCameraSinkHandlerIpc::GetInstance().UnInit();
    std::unique_lock<std::mutex> lock(producerMutex_);
    state_ = DCAMERA_SA_STATE_STOP;
    DHLOGI("DCameraSinkHandler::ReleaseSink success");
    return DCAMERA_OK;
}

int32_t DCameraSinkHandler::SubscribeLocalHardware(const std::string& dhId, const std::string& parameters)
{
    DHLOGI("DCameraSinkHandler::SubscribeLocalHardware dhId: %s", GetAnonyString(dhId).c_str());
    sptr<IDistributedCameraSink> dCameraSinkSrv = DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
    if (dCameraSinkSrv == nullptr) {
        DHLOGE("DCameraSinkHandler::SubscribeLocalHardware get Service failed");
        return DCAMERA_BAD_VALUE;
    }
    return dCameraSinkSrv->SubscribeLocalHardware(dhId, parameters);
}

int32_t DCameraSinkHandler::UnsubscribeLocalHardware(const std::string& dhId)
{
    DHLOGI("DCameraSinkHandler::UnsubscribeLocalHardware dhId: %s", GetAnonyString(dhId).c_str());
    sptr<IDistributedCameraSink> dCameraSinkSrv = DCameraSinkHandlerIpc::GetInstance().GetSinkLocalCamSrv();
    if (dCameraSinkSrv == nullptr) {
        DHLOGE("DCameraSinkHandler::UnsubscribeLocalHardware get Service failed");
        return DCAMERA_BAD_VALUE;
    }
    return dCameraSinkSrv->UnsubscribeLocalHardware(dhId);
}

IDistributedHardwareSink *GetSinkHardwareHandler()
{
    DHLOGI("DCameraSinkHandler::GetSinkHardwareHandler");
    return &DCameraSinkHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS