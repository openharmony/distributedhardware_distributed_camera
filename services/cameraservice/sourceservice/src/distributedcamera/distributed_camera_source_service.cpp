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

#include "distributed_camera_source_service.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "dcamera_hdf_operate.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_hitrace_adapter.h"
#include "dcamera_service_state_listener.h"
#include "dcamera_source_service_ipc.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_sa_process_state.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedCameraSourceService, DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, true);

std::map<DCameraIndex, std::shared_ptr<DCameraSourceDev>> DistributedCameraSourceService::camerasMap_;
std::mutex DistributedCameraSourceService::camDevMutex_;

DistributedCameraSourceService::DistributedCameraSourceService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
}

void DistributedCameraSourceService::OnStart()
{
    DHLOGI("DistributedCameraSourceService::OnStart");
    if (state_ == DCameraServiceState::DCAMERA_SRV_STATE_RUNNING) {
        DHLOGI("DistributedCameraSourceService has already started.");
        return;
    }

    if (!Init()) {
        DHLOGE("DistributedCameraSourceService init failed");
        return;
    }
    state_ = DCameraServiceState::DCAMERA_SRV_STATE_RUNNING;
    DHLOGI("DCameraServiceState OnStart service success.");
}

bool DistributedCameraSourceService::Init()
{
    DHLOGI("DistributedCameraSourceService start init");
    DCameraSourceServiceIpc::GetInstance().Init();
    if (!registerToService_) {
        bool ret = Publish(this);
        if (!ret) {
            DHLOGE("DistributedCameraSourceService Publish service failed");
            return false;
        }
        registerToService_ = true;
    }
    listener_ = std::make_shared<DCameraServiceStateListener>();
    DHLOGI("DistributedCameraSourceService init success");
    return true;
}

void DistributedCameraSourceService::OnStop()
{
    DHLOGI("DistributedCameraSourceService OnStop service");
    state_ = DCameraServiceState::DCAMERA_SRV_STATE_NOT_START;
    registerToService_ = false;
    listener_ = nullptr;
    DCameraSourceServiceIpc::GetInstance().UnInit();
}

int32_t DistributedCameraSourceService::InitSource(const std::string& params,
    const sptr<IDCameraSourceCallback>& callback)
{
    DHLOGI("DistributedCameraSourceService InitSource param: %s", params.c_str());
    int32_t ret = LoadDCameraHDF();
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService InitSource LoadHDF failed, ret: %d", ret);
        return ret;
    }
    sourceVer_ = params;
    listener_->SetCallback(callback);
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceService::ReleaseSource()
{
    DHLOGI("DistributedCameraSourceService ReleaseSource");
    int32_t ret = UnLoadCameraHDF();
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService ReleaseSource UnLoadHDF failed, ret: %d", ret);
        return ret;
    }
    DHLOGI("check source sa state.");
    SetSourceProcessExit();
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceService::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    DHLOGI("DistributedCameraSourceService RegisterDistributedHardware devId: %s, dhId: %s, version: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.version.c_str());
    if (GetCamDevNum() > MAX_CAMERAS_NUMBER) {
        DHLOGE("DistributedCameraSourceService RegisterDistributedHardware cameras exceed the upper limit");
        return DCAMERA_BAD_VALUE;
    }
    DCameraIndex camIndex(devId, dhId);
    int32_t ret = DCAMERA_OK;
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGI("DistributedCameraSourceService RegisterDistributedHardware new dev devId: %s, dhId: %s, version: %s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.version.c_str());
        camDev = std::make_shared<DCameraSourceDev>(devId, dhId, listener_);
        ret = camDev->InitDCameraSourceDev();
        if (ret != DCAMERA_OK) {
            DHLOGE("DistributedCameraSourceService RegisterDistributedHardware failed %d InitDev devId: %s, dhId: %s",
                ret, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
        CamDevInsert(camIndex, camDev);
    } else {
        DHLOGE("DistributedCameraSourceService RegisterDistributedHardware exist devId: %s, dhId: %s, version: %s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.version.c_str());
        return DCAMERA_ALREADY_EXISTS;
    }

    ret = camDev->RegisterDistributedHardware(devId, dhId, reqId, param.version, param.attrs);
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService RegisterDistributedHardware failed, ret: %d", ret);
        ReportRegisterCameraFail(DCAMERA_REGISTER_FAIL, GetAnonyString(devId), dhId, param.version,
            "dcamera source RegisterDistributedHardware fail.");
        CamDevErase(camIndex);
    }
    DHLOGI("DistributedCameraSourceService RegisterDistributedHardware end devId: %s, dhId: %s, version: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.version.c_str());
    return ret;
}

int32_t DistributedCameraSourceService::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId)
{
    DHLOGI("DistributedCameraSourceService UnregisterDistributedHardware devId: %s, dhId: %s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    DCameraIndex camIndex(devId, dhId);
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGE("DistributedCameraSourceService UnregisterDistributedHardware not found device");
        return DCAMERA_NOT_FOUND;
    }

    int32_t ret = camDev->UnRegisterDistributedHardware(devId, dhId, reqId);
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService UnregisterDistributedHardware failed, ret: %d", ret);
    }
    return ret;
}

int32_t DistributedCameraSourceService::DCameraNotify(const std::string& devId, const std::string& dhId,
    std::string& events)
{
    DHLOGI("DistributedCameraSourceService DCameraNotify devId: %s, dhId: %s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    DCameraIndex camIndex(devId, dhId);
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGE("DistributedCameraSourceService DCameraNotify not found device");
        return DCAMERA_NOT_FOUND;
    }

    int32_t ret = camDev->DCameraNotify(events);
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService DCameraNotify failed, ret: %d", ret);
    }
    return ret;
}

int32_t DistributedCameraSourceService::LoadDCameraHDF()
{
    DCAMERA_SYNC_TRACE(DCAMERA_LOAD_HDF);
    DHLOGI("load hdf driver start");
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl();
    if (ret != DCAMERA_OK) {
        DHLOGE("load hdf driver failed, ret %d", ret);
        ReportDcamerInitFail(DCAMERA_INIT_FAIL, DCAMERA_HDF_ERROR, CreateMsg("dcamera load hdf driver fail."));
        return ret;
    }
    DHLOGI("load hdf driver end");
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceService::UnLoadCameraHDF()
{
    DHLOGI("unload hdf driver start");
    int32_t ret = DCameraHdfOperate::GetInstance().UnLoadDcameraHDFImpl();
    if (ret != DCAMERA_OK) {
        DHLOGE("unload hdf driver failed, ret %d", ret);
        return ret;
    }
    DHLOGI("unload hdf driver end");
    return DCAMERA_OK;
}

int DistributedCameraSourceService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedCameraSourceService Dump.");
    if (args.size() > DUMP_MAX_SIZE) {
        DHLOGE("DistributedCameraSourceService Dump input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    std::string result;
    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    if (!DcameraSourceHidumper::GetInstance().Dump(argsStr, result)) {
        DHLOGE("Hidump error");
        return DCAMERA_BAD_VALUE;
    }

    int ret = dprintf(fd, "%s\n", result.c_str());
    if (ret < 0) {
        DHLOGE("dprintf error");
        return DCAMERA_BAD_VALUE;
    }

    return DCAMERA_OK;
}

void DistributedCameraSourceService::GetDumpInfo(CameraDumpInfo& camDump)
{
    std::lock_guard<std::mutex> camLock(camDevMutex_);
    camDump.regNumber = static_cast<int32_t>(camerasMap_.size());
    std::map<std::string, int32_t> curState;
    for (auto it = camerasMap_.begin(); it != camerasMap_.end(); it++) {
        DCameraIndex cam = it->first;
        std::shared_ptr<DCameraSourceDev> camSourceDev = it->second;
        camDump.version = camSourceDev->GetVersion();
        std::string deviceId = GetAnonyString(cam.devId_);
        deviceId.append(cam.dhId_);
        int32_t devState = camSourceDev->GetStateInfo();
        curState[deviceId] = devState;
    }
    camDump.curState = curState;
}

void DistributedCameraSourceService::CamDevInsert(DCameraIndex& index, std::shared_ptr<DCameraSourceDev>& camDev)
{
    std::lock_guard<std::mutex> camLock(camDevMutex_);
    camerasMap_.emplace(index, camDev);
}

std::shared_ptr<DCameraSourceDev> DistributedCameraSourceService::GetCamDevByIndex(DCameraIndex& index)
{
    std::lock_guard<std::mutex> camLock(camDevMutex_);
    auto iter = camerasMap_.find(index);
    if (iter == camerasMap_.end()) {
        return nullptr;
    }
    return iter->second;
}

void DistributedCameraSourceService::CamDevErase(DCameraIndex& index)
{
    std::lock_guard<std::mutex> camLock(camDevMutex_);
    camerasMap_.erase(index);
}

uint32_t DistributedCameraSourceService::GetCamDevNum()
{
    std::lock_guard<std::mutex> camLock(camDevMutex_);
    return camerasMap_.size();
}
} // namespace DistributedHardware
} // namespace OHOS
