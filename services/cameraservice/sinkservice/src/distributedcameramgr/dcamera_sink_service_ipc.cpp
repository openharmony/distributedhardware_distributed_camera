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

#include "dcamera_sink_service_ipc.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkServiceIpc::DCameraSinkServiceIpc() : isInit_(false)
{
    DHLOGI("DCameraSinkServiceIpc Create");
}

DCameraSinkServiceIpc::~DCameraSinkServiceIpc()
{
    DHLOGI("DCameraSinkServiceIpc Delete");
    UnInit();
}

IMPLEMENT_SINGLE_INSTANCE(DCameraSinkServiceIpc);

void DCameraSinkServiceIpc::Init()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("DCameraSinkServiceIpc Init Start");
    if (isInit_) {
        DHLOGI("DCameraSinkServiceIpc has already init");
        return;
    }
    sourceRemoteRecipient_ = sptr<SourceRemoteRecipient>(new SourceRemoteRecipient());
    isInit_ = true;
    DHLOGI("DCameraSinkServiceIpc Init End");
}

void DCameraSinkServiceIpc::UnInit()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("DCameraSinkServiceIpc UnInit Start");
    if (!isInit_) {
        DHLOGI("DCameraSinkServiceIpc has already UnInit");
        return;
    }
    ClearSourceRemoteCamSrv();
    DHLOGI("DCameraSinkServiceIpc Start free recipient");
    sourceRemoteRecipient_ = nullptr;
    isInit_ = false;
    DHLOGI("DCameraSinkServiceIpc UnInit End");
}

sptr<IDistributedCameraSource> DCameraSinkServiceIpc::GetSourceRemoteCamSrv(const std::string& deviceId)
{
    if (deviceId.empty()) {
        DHLOGE("GetSourceRemoteCamSrv deviceId is empty");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sourceRemoteCamSrvLock_);
        auto iter = remoteSources_.find(deviceId);
        if (iter != remoteSources_.end()) {
            auto object = iter->second;
            if (object != nullptr) {
                DHLOGI("DCameraSinkServiceIpc GetSourceRemoteCamSrv from cache devId: %{public}s",
                    GetAnonyString(deviceId).c_str());
                return object;
            }
        }
    }
    DHLOGI("GetSourceRemoteCamSrv remote deviceid is %{public}s", GetAnonyString(deviceId).c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DHLOGE("GetSourceRemoteCamSrv failed to connect to systemAbilityMgr!");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID, deviceId);
    if (object == nullptr) {
        DHLOGE("GetSourceRemoteCamSrv failed get remote CamSrv %{public}s", GetAnonyString(deviceId).c_str());
        return nullptr;
    }
    int32_t ret = object->AddDeathRecipient(sourceRemoteRecipient_);
    sptr<IDistributedCameraSource> remoteCamSrvObj = iface_cast<IDistributedCameraSource>(object);
    if (remoteCamSrvObj == nullptr) {
        DHLOGI("GetSourceRemoteCamSrv failed, remoteCamSrvObj is null ret: %{public}d", ret);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sourceRemoteCamSrvLock_);
        auto iter = remoteSources_.find(deviceId);
        if (iter != remoteSources_.end() && iter->second != nullptr) {
            iter->second->AsObject()->RemoveDeathRecipient(sourceRemoteRecipient_);
        }
        remoteSources_[deviceId] = remoteCamSrvObj;
    }
    DHLOGI("GetSourceRemoteCamSrv success, AddDeathRecipient ret: %{public}d", ret);
    return remoteCamSrvObj;
}

void DCameraSinkServiceIpc::DeleteSourceRemoteCamSrv(const std::string& deviceId)
{
    DHLOGI("DeleteSourceRemoteCamSrv devId: %{public}s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> autoLock(sourceRemoteCamSrvLock_);
    auto item = remoteSources_.find(deviceId);
    if (item == remoteSources_.end()) {
        DHLOGI("DeleteSourceRemoteCamSrv not found device: %{public}s", GetAnonyString(deviceId).c_str());
        return;
    }

    if (item->second != nullptr) {
        item->second->AsObject()->RemoveDeathRecipient(sourceRemoteRecipient_);
    }
    remoteSources_.erase(item);
}

void DCameraSinkServiceIpc::ClearSourceRemoteCamSrv()
{
    DHLOGI("ClearSourceRemoteCamSrv Start");
    std::lock_guard<std::mutex> autoLock(sourceRemoteCamSrvLock_);
    for (auto iter = remoteSources_.begin(); iter != remoteSources_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->AsObject()->RemoveDeathRecipient(sourceRemoteRecipient_);
        }
    }
    remoteSources_.clear();
    DHLOGI("ClearSourceRemoteCamSrv end");
}

void DCameraSinkServiceIpc::SourceRemoteRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("SourceRemoteRecipient OnRemoteDied received died notify!");
    DCameraSinkServiceIpc::GetInstance().OnSourceRemoteCamSrvDied(remote);
}

void DCameraSinkServiceIpc::OnSourceRemoteCamSrvDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("OnSourceRemoteCamSrvDied delete diedRemoted");
    std::lock_guard<std::mutex> autoLock(sourceRemoteCamSrvLock_);
    sptr<IRemoteObject> diedRemoted = remote.promote();
    if (diedRemoted == nullptr) {
        DHLOGE("OnSourceRemoteCamSrvDied promote failed!");
        return;
    }
    auto iter = std::find_if(remoteSources_.begin(), remoteSources_.end(), [&](
        const std::pair<std::string, sptr<IDistributedCameraSource>> &item)->bool {
            CHECK_AND_RETURN_RET_LOG(item.second == nullptr, false,
                "OnSourceRemoteCamSrvDied item.second is null");
            return item.second->AsObject() == diedRemoted;
        });
    if (iter == remoteSources_.end()) {
        DHLOGI("OnSourceRemoteCamSrvDied not found remote object");
        return;
    }
    DHLOGI("OnSourceRemoteCamSrvDied remote.devId: %{public}s", GetAnonyString(iter->first).c_str());
    if (iter->second != nullptr) {
        iter->second->AsObject()->RemoveDeathRecipient(sourceRemoteRecipient_);
    }
    remoteSources_.erase(iter);
}
} // namespace DistributedHardware
} // namespace OHOS
