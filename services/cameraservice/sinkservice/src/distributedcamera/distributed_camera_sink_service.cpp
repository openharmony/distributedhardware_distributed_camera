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

#include "distributed_camera_sink_service.h"

#include "icamera_channel_listener.h"
#include "if_system_ability_manager.h"
#include "ipc_skeleton.h"
#include "ipc_types.h"
#include "iservice_registry.h"
#include "string_ex.h"
#include "system_ability_definition.h"

#include "anonymous_string.h"
#include "dcamera_handler.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_sink_service_ipc.h"
#include "dcamera_softbus_adapter.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_allconnect_manager.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "token_setproc.h"

namespace OHOS {
namespace DistributedHardware {
REGISTER_SYSTEM_ABILITY_BY_ID(DistributedCameraSinkService, DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, true);

static CameraDumpInfo g_camDump;
static std::mutex g_camDumpMutex;
DistributedCameraSinkService* DistributedCameraSinkService::dcSinkService;
DistributedCameraSinkService::DistributedCameraSinkService(int32_t saId, bool runOnCreate)
    : SystemAbility(saId, runOnCreate)
{
    dcSinkService = this;
}

void DistributedCameraSinkService::OnStart()
{
    // LCOV_EXCL_START
    DHLOGI("DistributedCameraSinkService OnStart");
    CHECK_AND_RETURN_LOG(state_ == DCameraServiceState::DCAMERA_SRV_STATE_RUNNING,
        "sink service has already started.");

    if (!Init()) {
        DHLOGE("DistributedCameraSinkService init failed");
        return;
    }
    state_ = DCameraServiceState::DCAMERA_SRV_STATE_RUNNING;
    if (!DCameraAllConnectManager::IsInited()) {
        DCameraAllConnectManager::GetInstance().InitDCameraAllConnectManager();
        int32_t ret = DCameraAllConnectManager::GetInstance().RegisterLifecycleCallback();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect sink init and register lifecycle callback failed");
        } else {
            DHLOGI("DCamera allconnect sink init and register lifecycle callback success");
        }
    }
    DHLOGI("DCameraServiceState OnStart service success.");
    // LCOV_EXCL_STOP
}

bool DistributedCameraSinkService::Init()
{
    // LCOV_EXCL_START
    DHLOGI("DistributedCameraSinkService start init");
    DCameraSinkServiceIpc::GetInstance().Init();
    if (!registerToService_) {
        bool ret = Publish(this);
        CHECK_AND_RETURN_RET_LOG(!ret, false, "Publish service failed");
        registerToService_ = true;
    }
    DHLOGI("DistributedCameraSinkService init success");
    return true;
    // LCOV_EXCL_STOP
}

void DistributedCameraSinkService::OnStop()
{
    // LCOV_EXCL_START
    DHLOGI("DistributedCameraSinkService OnStop service");
    state_ = DCameraServiceState::DCAMERA_SRV_STATE_NOT_START;
    registerToService_ = false;
    if (DCameraAllConnectManager::IsInited()) {
        int32_t ret = DCameraAllConnectManager::GetInstance().UnRegisterLifecycleCallback();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect sink UnRegisterLifecycle failed");
        } else {
            DHLOGI("DCamera allconnect sink UnRegisterLifecycle success");
        }
        DCameraAllConnectManager::GetInstance().UnInitDCameraAllConnectManager();
    }

    DCameraSinkServiceIpc::GetInstance().UnInit();
    // LCOV_EXCL_STOP
}

int32_t DistributedCameraSinkService::InitSink(const std::string& params,
    const sptr<IDCameraSinkCallback> &sinkCallback)
{
    DHLOGI("start");
    sinkVer_ = params;
    int32_t ret = DCameraHandler::GetInstance().Initialize();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "handler initialize failed, ret: %{public}d", ret);

    std::vector<std::string> cameras = DCameraHandler::GetInstance().GetCameras();
    CHECK_AND_RETURN_RET_LOG(cameras.empty(), DCAMERA_BAD_VALUE, "no camera device");
    {
        std::lock_guard<std::mutex> lock(g_camDumpMutex);
        g_camDump.version = sinkVer_;
        g_camDump.camNumber = static_cast<int32_t>(cameras.size());
    }
    for (auto& dhId : cameras) {
        std::shared_ptr<DCameraSinkDev> sinkDevice = std::make_shared<DCameraSinkDev>(dhId, sinkCallback);
        sinkDevice->SetTokenId(GetFirstCallerTokenID());
        ret = sinkDevice->Init();
        CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "sink device init failed, ret: %{public}d", ret);
        {
            std::lock_guard<std::mutex> lock(mapMutex_);
            camerasMap_.emplace(dhId, sinkDevice);
        }
    }
    DHLOGI("success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::ReleaseSink()
{
    DHLOGI("enter");
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto iter = camerasMap_.begin(); iter != camerasMap_.end(); iter++) {
            std::shared_ptr<DCameraSinkDev> sinkDevice = iter->second;
            int32_t ret = sinkDevice->UnInit();
            CHECK_AND_LOG(ret != DCAMERA_OK, "release sink device failed, ret: %{public}d", ret);
        }
        camerasMap_.clear();
    }

    auto systemAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(systemAbilityMgr == nullptr, DCAMERA_BAD_VALUE, "sink systemAbilityMgr is null");
    int32_t ret = systemAbilityMgr->UnloadSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, DCAMERA_BAD_VALUE,
        "sink systemAbilityMgr UnLoadSystemAbility failed, ret: %{public}d", ret);
    DHLOGI("sink systemAbilityMgr UnLoadSystemAbility success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::SubscribeLocalHardware(const std::string& dhId, const std::string& parameters)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }
    CHECK_AND_RETURN_RET_LOG(sinkDevice == nullptr, DCAMERA_BAD_VALUE,
        "sink device is null, dhId: %{public}s", GetAnonyString(dhId).c_str());
    int32_t ret = sinkDevice->SubscribeLocalHardware(parameters);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "SubscribeLocalHardware failed, ret: %{public}d", ret);
    DHLOGI("SubscribeLocalHardware success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::UnsubscribeLocalHardware(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }
    int32_t ret = sinkDevice->UnsubscribeLocalHardware();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "UnsubscribeLocalHardware failed, ret: %{public}d", ret);
    DHLOGI("UnsubscribeLocalHardware success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::StopCapture(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }
    int32_t ret = sinkDevice->StopCapture();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "StopCapture failed, ret: %{public}d", ret);
    DHLOGI("StopCapture success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::ChannelNeg(const std::string& dhId, std::string& channelInfo)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }

    int32_t ret = sinkDevice->ChannelNeg(channelInfo);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "ChannelNeg failed, ret: %{public}d", ret);
    DHLOGI("ChannelNeg success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::GetCameraInfo(const std::string& dhId, std::string& cameraInfo)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }
    int32_t ret = sinkDevice->GetCameraInfo(cameraInfo);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "GetCameraInfo failed, ret: %{public}d", ret);
    DHLOGI("GetCameraInfo success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::OpenChannel(const std::string& dhId, std::string& openInfo)
{
    DHLOGI("DistributedCameraSinkService OpenChannel Begin, dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }
    int32_t ret = sinkDevice->OpenChannel(openInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("OpenChannel failed, ret: %{public}d", ret);
        ReportDcamerOptFail(DCAMERA_OPT_FAIL, DCAMERA_SINK_OPEN_CAM_ERROR,
            CreateMsg("sink service open channel failed, dhId: %s", GetAnonyString(dhId).c_str()));
        return ret;
    }
    DHLOGI("DistributedCameraSinkService OpenChannel success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::CloseChannel(const std::string& dhId)
{
    DHLOGI("dhId: %{public}s", GetAnonyString(dhId).c_str());
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        auto iter = camerasMap_.find(dhId);
        if (iter == camerasMap_.end()) {
            DHLOGE("device not found");
            return DCAMERA_NOT_FOUND;
        }
        sinkDevice = iter->second;
    }

    int32_t ret = sinkDevice->CloseChannel();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "CloseChannel failed, ret: %{public}d", ret);
    DHLOGI("CloseChannel success");
    return DCAMERA_OK;
}

int DistributedCameraSinkService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    DHLOGI("DistributedCameraSinkService Dump.");
    if (args.size() > DUMP_MAX_SIZE) {
        DHLOGE("DistributedCameraSinkService Dump input is invalid");
        return DCAMERA_BAD_VALUE;
    }
    std::string result;
    std::vector<std::string> argsStr;
    for (auto item : args) {
        argsStr.emplace_back(Str16ToStr8(item));
    }

    if (!DcameraSinkHidumper::GetInstance().Dump(argsStr, result)) {
        DHLOGE("Hidump error");
        return DCAMERA_BAD_VALUE;
    }

    int ret = dprintf(fd, "%s\n", result.c_str());
    CHECK_AND_RETURN_RET_LOG(ret < 0, DCAMERA_BAD_VALUE, "dprintf error");

    return DCAMERA_OK;
}

void DistributedCameraSinkService::GetCamIds()
{
    std::vector<std::string> camIds;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto it = camerasMap_.begin(); it != camerasMap_.end(); it++) {
            camIds.push_back(it->second->GetDhid());
        }
    }
    std::lock_guard<std::mutex> lock(g_camDumpMutex);
    g_camDump.camIds = camIds;
}

void DistributedCameraSinkService::GetCamDumpInfo(CameraDumpInfo& camDump)
{
    dcSinkService->GetCamIds();
    std::lock_guard<std::mutex> lock(g_camDumpMutex);
    camDump = g_camDump;
}

bool DistributedCameraSinkService::IsCurSinkDev(std::shared_ptr<DCameraSinkDev> sinkDevice)
{
    std::string camInfoJson;
    CHECK_AND_RETURN_RET_LOG(sinkDevice == nullptr, false, "sinkDevice is null.");
    int32_t ret = sinkDevice->GetCameraInfo(camInfoJson);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false, "GetCameraInfo failed, ret: %{public}d", ret);
    DCameraInfoCmd cmd;
    ret = cmd.Unmarshal(camInfoJson);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false, "DCameraInfoCmd Unmarshal failed: %{public}d", ret);
    std::shared_ptr<DCameraInfo> camInfo = cmd.value_;
    if (camInfo->state_ == DCAMERA_CHANNEL_STATE_CONNECTED) {
        return true;
    }
    return false;
}

int32_t DistributedCameraSinkService::PauseDistributedHardware(const std::string &networkId)
{
    DHLOGI("start.");
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto iter = camerasMap_.begin(); iter != camerasMap_.end(); iter++) {
            if (IsCurSinkDev(iter->second)) {
                sinkDevice = iter->second;
                break;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(sinkDevice == nullptr, DCAMERA_BAD_VALUE,
        "PauseDistributedHardware sinkDevice is nullptr.");

    int32_t ret = sinkDevice->PauseDistributedHardware(networkId);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "PauseDistributedHardware failed, ret: %{public}d", ret);
    DHLOGI("PauseDistributedHardware success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::ResumeDistributedHardware(const std::string &networkId)
{
    DHLOGI("start.");
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto iter = camerasMap_.begin(); iter != camerasMap_.end(); iter++) {
            if (IsCurSinkDev(iter->second)) {
                sinkDevice = iter->second;
                break;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(sinkDevice == nullptr, DCAMERA_BAD_VALUE,
        "ResumeDistributedHardware sinkDevice is nullptr.");

    int32_t ret = sinkDevice->ResumeDistributedHardware(networkId);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "ResumeDistributedHardware failed, ret: %{public}d", ret);
    DHLOGI("ResumeDistributedHardware success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::StopDistributedHardware(const std::string &networkId)
{
    DHLOGI("start.");
    std::shared_ptr<DCameraSinkDev> sinkDevice = nullptr;
    {
        std::lock_guard<std::mutex> lock(mapMutex_);
        for (auto iter = camerasMap_.begin(); iter != camerasMap_.end(); iter++) {
            if (IsCurSinkDev(iter->second)) {
                sinkDevice = iter->second;
                break;
            }
        }
    }
    CHECK_AND_RETURN_RET_LOG(sinkDevice == nullptr, DCAMERA_BAD_VALUE,
        "StopDistributedHardware sinkDevice is nullptr.");

    int32_t ret = sinkDevice->StopDistributedHardware(networkId);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "StopDistributedHardware failed, ret: %{public}d", ret);
    DHLOGI("StopDistributedHardware success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::SetAccessListener(const sptr<IAccessListener> &listener,
    int32_t timeOut, const std::string &pkgName)
{
    DHLOGI("SetAccessListener, pkgName: %{public}s, timeOut: %{public}d", pkgName.c_str(), timeOut);
    if (listener == nullptr) {
        DHLOGE("listener is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ret = DCameraAccessConfigManager::GetInstance().SetAccessConfig(listener, timeOut, pkgName);
    if (ret != DCAMERA_OK) {
        DHLOGE("SetAccessConfig failed, ret: %{public}d", ret);
        return ret;
    }

    DHLOGI("SetAccessListener success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::RemoveAccessListener(const std::string &pkgName)
{
    DHLOGI("RemoveAccessListener, pkgName: %{public}s", pkgName.c_str());
    if (pkgName.empty()) {
        DHLOGE("pkgName is empty");
        return DCAMERA_BAD_VALUE;
    }
    DCameraAccessConfigManager::GetInstance().ClearAccessConfigByPkgName(pkgName);

    DHLOGI("RemoveAccessListener success");
    return DCAMERA_OK;
}

int32_t DistributedCameraSinkService::SetAuthorizationResult(const std::string &requestId, bool granted)
{
    DHLOGI("SetAuthorizationResult, requestId: %{public}s, granted: %{public}d",
        GetAnonyString(requestId).c_str(), granted);
    if (requestId.empty()) {
        DHLOGE("requestId is empty");
        return DCAMERA_BAD_VALUE;
    }
    DCameraSoftbusAdapter::GetInstance().ProcessAuthorizationResult(requestId, granted);

    DHLOGI("SetAuthorizationResult success");
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS