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

#include "dcamera_source_service_ipc.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"

#include "anonymous_string.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceServiceIpc::DCameraSourceServiceIpc() : isInit_(false)
{
    DHLOGI("DCameraSourceServiceIpc Create");
}

DCameraSourceServiceIpc::~DCameraSourceServiceIpc()
{
    DHLOGI("DCameraSourceServiceIpc Delete");
    UnInit();
}

IMPLEMENT_SINGLE_INSTANCE(DCameraSourceServiceIpc);

void DCameraSourceServiceIpc::Init()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("Start");
    if (isInit_) {
        DHLOGI("DCameraSourceServiceIpc has already init");
        return;
    }
    sinkRemoteRecipient_ = sptr<SinkRemoteRecipient>(new SinkRemoteRecipient());
    isInit_ = true;
    DHLOGI("End");
}

void DCameraSourceServiceIpc::UnInit()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("Start");
    if (!isInit_) {
        DHLOGI("DCameraSourceServiceIpc has already UnInit");
        return;
    }
    ClearSinkRemoteCamSrv();
    DHLOGI("Start free recipient");
    sinkRemoteRecipient_ = nullptr;
    isInit_ = false;
    DHLOGI("End");
}

sptr<IDistributedCameraSink> DCameraSourceServiceIpc::GetSinkRemoteCamSrv(const std::string& deviceId)
{
    if (deviceId.empty()) {
        DHLOGE("GetSinkRemoteCamSrv deviceId is empty");
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sinkRemoteCamSrvLock_);
        auto iter = remoteSinks_.find(deviceId);
        if (iter != remoteSinks_.end()) {
            auto object = iter->second;
            if (object != nullptr) {
                DHLOGI("DCameraSourceServiceIpc GetSinkRemoteCamSrv from cache devId: %{public}s",
                    GetAnonyString(deviceId).c_str());
                return object;
            }
        }
    }
    DHLOGI("GetSinkRemoteCamSrv remote deviceid is %{public}s", GetAnonyString(deviceId).c_str());
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        DHLOGE("GetSinkRemoteCamSrv failed to connect to systemAbilityMgr!");
        return nullptr;
    }

    auto object = samgr->CheckSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID, deviceId);
    if (object == nullptr) {
        DHLOGE("GetSinkRemoteCamSrv failed get remote CamSrv %{public}s", GetAnonyString(deviceId).c_str());
        return nullptr;
    }
    int32_t ret = object->AddDeathRecipient(sinkRemoteRecipient_);
    sptr<IDistributedCameraSink> remoteCamSrvObj = iface_cast<IDistributedCameraSink>(object);
    if (remoteCamSrvObj == nullptr) {
        DHLOGI("GetSinkRemoteCamSrv failed, remoteCamSrvObj is null ret: %{public}d", ret);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sinkRemoteCamSrvLock_);
        auto iter = remoteSinks_.find(deviceId);
        if (iter != remoteSinks_.end() && iter->second != nullptr) {
            iter->second->AsObject()->RemoveDeathRecipient(sinkRemoteRecipient_);
        }
        remoteSinks_[deviceId] = remoteCamSrvObj;
    }
    DHLOGI("GetSinkRemoteCamSrv success, AddDeathRecipient ret: %{public}d", ret);
    return remoteCamSrvObj;
}

void DCameraSourceServiceIpc::DeleteSinkRemoteCamSrv(const std::string& deviceId)
{
    DHLOGI("DeleteSinkRemoteCamSrv devId: %{public}s", GetAnonyString(deviceId).c_str());
    std::lock_guard<std::mutex> autoLock(sinkRemoteCamSrvLock_);
    auto item = remoteSinks_.find(deviceId);
    if (item == remoteSinks_.end()) {
        DHLOGI("DeleteSinkRemoteCamSrv not found device: %{public}s", GetAnonyString(deviceId).c_str());
        return;
    }

    if (item->second != nullptr) {
        item->second->AsObject()->RemoveDeathRecipient(sinkRemoteRecipient_);
    }
    remoteSinks_.erase(item);
}

void DCameraSourceServiceIpc::ClearSinkRemoteCamSrv()
{
    DHLOGI("ClearSinkRemoteCamSrv Start");
    std::lock_guard<std::mutex> autoLock(sinkRemoteCamSrvLock_);
    for (auto iter = remoteSinks_.begin(); iter != remoteSinks_.end(); iter++) {
        if (iter->second != nullptr) {
            iter->second->AsObject()->RemoveDeathRecipient(sinkRemoteRecipient_);
        }
    }
    remoteSinks_.clear();
    DHLOGI("ClearSinkRemoteCamSrv end");
}

void DCameraSourceServiceIpc::SinkRemoteRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("SinkRemoteRecipient OnRemoteDied received died notify!");
    DCameraSourceServiceIpc::GetInstance().OnSinkRemoteCamSrvDied(remote);
}

void DCameraSourceServiceIpc::OnSinkRemoteCamSrvDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("OnSinkRemoteCamSrvDied delete diedRemoted");
    std::lock_guard<std::mutex> autoLock(sinkRemoteCamSrvLock_);
    sptr<IRemoteObject> diedRemoted = remote.promote();
    if (diedRemoted == nullptr) {
        DHLOGE("OnSinkRemoteCamSrvDied promote failed!");
        return;
    }
    auto iter = std::find_if(remoteSinks_.begin(), remoteSinks_.end(), [&](
        const std::pair<std::string, sptr<IDistributedCameraSink>> &item)->bool {
            CHECK_AND_RETURN_RET_LOG(item.second == nullptr, false,
                "OnSinkRemoteCamSrvDied item.second is null");
            return item.second->AsObject() == diedRemoted;
        });
    if (iter == remoteSinks_.end()) {
        DHLOGI("OnSinkRemoteCamSrvDied not found remote object");
        return;
    }

    DHLOGI("OnSinkRemoteCamSrvDied remote.devId: %{public}s", GetAnonyString(iter->first).c_str());
    if (iter->second != nullptr) {
        iter->second->AsObject()->RemoveDeathRecipient(sinkRemoteRecipient_);
    }
    remoteSinks_.erase(iter);
}
} // namespace DistributedHardware
} // namespace OHOS
