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

#include "distributed_camera_source_service.h"

#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"
#include "xcollie/watchdog.h"

#include "anonymous_string.h"
#include "avcodec_info.h"
#include "avcodec_list.h"
#include "dcamera_hdf_operate.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_hitrace_adapter.h"
#include "dcamera_service_state_listener.h"
#include "dcamera_source_service_ipc.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_handler.h"

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
    DHLOGI("start");
    if (state_ == DCameraServiceState::DCAMERA_SRV_STATE_RUNNING) {
        DHLOGI("DistributedCameraSourceService has already started.");
        return;
    }

    if (!Init()) {
        DHLOGE("DistributedCameraSourceService init failed");
        return;
    }
#ifdef DCAMERA_MMAP_RESERVE
    ConverterHandle::GetInstance().InitConverter();
#endif
    state_ = DCameraServiceState::DCAMERA_SRV_STATE_RUNNING;
    DHLOGI("start service success.");
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
    if (!isHicollieRunning_.load()) {
        isHicollieRunning_.store(true);
        hicollieThread_ = std::thread(&DistributedCameraSourceService::StartHicollieThread, this);
    }
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
    isHicollieRunning_.store(false);
    if (hicollieThread_.joinable()) {
        hicollieThread_.join();
    }
#ifdef DCAMERA_MMAP_RESERVE
    ConverterHandle::GetInstance().DeInitConverter();
#endif
}

int32_t DistributedCameraSourceService::InitSource(const std::string& params,
    const sptr<IDCameraSourceCallback>& callback)
{
    DHLOGI("DistributedCameraSourceService InitSource param: %{public}s", params.c_str());
    int32_t ret = LoadDCameraHDF();
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService InitSource LoadHDF failed, ret: %{public}d", ret);
        return ret;
    }
    sourceVer_ = params;
    if (listener_ != nullptr) {
        listener_->SetCallback(callback);
    }
    return DCAMERA_OK;
}

int32_t DistributedCameraSourceService::ReleaseSource()
{
    DHLOGI("enter");
    int32_t ret = UnLoadCameraHDF();
    if (ret != DCAMERA_OK) {
        DHLOGE("DistributedCameraSourceService ReleaseSource UnLoadHDF failed, ret: %{public}d", ret);
        return ret;
    }

    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        DHLOGE("source systemAbilityMgr is null");
        return DCAMERA_BAD_VALUE;
    }
    ret = systemAbilityMgr->UnloadSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    if (ret != DCAMERA_OK) {
        DHLOGE("source systemAbilityMgr UnLoadSystemAbility failed, ret: %{public}d", ret);
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("source systemAbilityMgr UnLoadSystemAbility success");
    return DCAMERA_OK;
}

std::string DistributedCameraSourceService::GetCodecInfo()
{
    DHLOGI("Create avCodecList start");
    std::string sourceAttrs = "";
    cJSON *root = cJSON_CreateObject();
    if (root == nullptr) {
        return sourceAttrs;
    }
    std::shared_ptr<MediaAVCodec::AVCodecList> avCodecList = MediaAVCodec::AVCodecListFactory::CreateAVCodecList();
    if (avCodecList == nullptr) {
        DHLOGI("Create avCodecList failed");
        cJSON_Delete(root);
        return sourceAttrs;
    }
    const std::vector<std::string> encoderName = {std::string(MediaAVCodec::CodecMimeType::VIDEO_AVC),
                                                  std::string(MediaAVCodec::CodecMimeType::VIDEO_HEVC)};
    cJSON *array = cJSON_CreateArray();
    if (array == nullptr) {
        DHLOGI("Create arrray failed");
        cJSON_Delete(root);
        return sourceAttrs;
    }
    for (auto &coder : encoderName) {
        MediaAVCodec::CapabilityData *capData = avCodecList->GetCapability(coder, true,
            MediaAVCodec::AVCodecCategory::AVCODEC_HARDWARE);
        if (capData == nullptr) {
            continue;
        }
        std::string mimeType = capData->mimeType;
        cJSON_AddItemToArray(array, cJSON_CreateString(mimeType.c_str()));
        DHLOGI("codec name: %{public}s, mimeType: %{public}s", coder.c_str(), mimeType.c_str());
    }
    cJSON_AddItemToObject(root, CAMERA_CODEC_TYPE_KEY.c_str(), array);
    char *jsonstr = cJSON_Print(root);
    if (jsonstr == nullptr) {
        cJSON_Delete(root);
        return sourceAttrs;
    }
    sourceAttrs = jsonstr;
    cJSON_free(jsonstr);
    cJSON_Delete(root);
    return sourceAttrs;
}

int32_t DistributedCameraSourceService::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    DHLOGI("RegisterDistributedHardware devId: %{public}s, dhId: %{public}s, sinkVersion: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), param.sinkVersion.c_str());
    if (GetCamDevNum() > MAX_CAMERAS_NUMBER) {
        DHLOGE("cameras exceed the upper limit");
        return DCAMERA_BAD_VALUE;
    }
    EnableParam params = const_cast<EnableParam&>(param);
    params.sourceAttrs = GetCodecInfo();
    DHLOGI("RegisterDistributedHardware sourceAttrs: %{public}s.", params.sourceAttrs.c_str());
    DCameraIndex camIndex(devId, dhId);
    int32_t ret = DCAMERA_OK;
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGI("new dev devId: %{public}s, dhId: %{public}s, sinkVersion: %{public}s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), params.sinkVersion.c_str());
        camDev = std::make_shared<DCameraSourceDev>(devId, dhId, listener_);
        ret = camDev->InitDCameraSourceDev();
        if (ret != DCAMERA_OK) {
            DHLOGE("RegisterDistributedHardware failed %{public}d InitDev devId: %{public}s, dhId: %{public}s",
                ret, GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
        CamDevInsert(camIndex, camDev);
    } else {
        DHLOGE("RegisterDistributedHardware exist devId: %{public}s, dhId: %{public}s, sinkVersion: %{public}s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), params.sinkVersion.c_str());
        return DCAMERA_ALREADY_EXISTS;
    }

    ret = camDev->RegisterDistributedHardware(devId, dhId, reqId, params);
    if (ret != DCAMERA_OK) {
        DHLOGE("RegisterDistributedHardware failed, ret: %{public}d", ret);
        ReportRegisterCameraFail(DCAMERA_REGISTER_FAIL, GetAnonyString(devId), dhId, params.sinkVersion,
            "dcamera source RegisterDistributedHardware fail.");
        CamDevErase(camIndex);
    }
    DHLOGI("RegisterDistributedHardware end devId: %{public}s, dhId: %{public}s, sinkVersion: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), params.sinkVersion.c_str());
    return ret;
}

int32_t DistributedCameraSourceService::UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId)
{
    DHLOGI("UnregisterDistributedHardware devId: %{public}s, dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    DCameraIndex camIndex(devId, dhId);
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGE("DistributedCameraSourceService UnregisterDistributedHardware not found device");
        return DCAMERA_NOT_FOUND;
    }

    int32_t ret = camDev->UnRegisterDistributedHardware(devId, dhId, reqId);
    if (ret != DCAMERA_OK) {
        DHLOGE("UnregisterDistributedHardware failed, ret: %{public}d", ret);
    }
    return ret;
}

int32_t DistributedCameraSourceService::DCameraNotify(const std::string& devId, const std::string& dhId,
    std::string& events)
{
    DHLOGI("DCameraNotify devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    DCameraIndex camIndex(devId, dhId);
    std::shared_ptr<DCameraSourceDev> camDev = GetCamDevByIndex(camIndex);
    if (camDev == nullptr) {
        DHLOGE("DistributedCameraSourceService DCameraNotify not found device");
        return DCAMERA_NOT_FOUND;
    }

    int32_t ret = camDev->DCameraNotify(events);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraNotify failed, ret: %{public}d", ret);
    }
    return ret;
}

int32_t DistributedCameraSourceService::LoadDCameraHDF()
{
    DCAMERA_SYNC_TRACE(DCAMERA_LOAD_HDF);
    DHLOGI("load hdf driver start");
    int32_t ret = DCameraHdfOperate::GetInstance().LoadDcameraHDFImpl();
    if (ret != DCAMERA_OK) {
        DHLOGE("load hdf driver failed, ret %{public}d", ret);
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
        DHLOGE("unload hdf driver failed, ret %{public}d", ret);
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

void DistributedCameraSourceService::StartHicollieThread()
{
    auto taskFunc = [this]() {
        std::lock_guard<std::mutex> lock(camDevMutex_);
        for (auto &iter : camerasMap_) {
            if (iter.second->GetHicollieFlag()) {
                iter.second->SetHicollieFlag(false);
            } else {
                DHLOGE("Hicollie : Flag = false, exit the current process");
                _Exit(0);
            }
        }
    };
    OHOS::HiviewDFX::Watchdog::GetInstance().RunPeriodicalTask(CAMERA_HICOLLIE, taskFunc,
        HICOLLIE_INTERVAL_TIME_MS, HICOLLIE_DELAY_TIME_MS);

    while (isHicollieRunning_.load()) {
        {
            std::lock_guard<std::mutex> lock(camDevMutex_);
            if (!camerasMap_.empty()) {
                for (auto &iter : camerasMap_) {
                    iter.second->PostHicollieEvent();
                }
            }
        }
        usleep(HICOLLIE_SLEEP_TIME_US);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
