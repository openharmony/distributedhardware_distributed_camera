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

#include "dcamera_sink_handler_ipc.h"

#include <cstdint>
#include <new>

#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "iremote_broker.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkHandlerIpc::DCameraSinkHandlerIpc() : isInit_(false)
{
    DHLOGI("DCameraSinkHandlerIpc Create");
}

DCameraSinkHandlerIpc::~DCameraSinkHandlerIpc()
{
    DHLOGI("DCameraSinkHandlerIpc Delete");
    UnInit();
}

IMPLEMENT_SINGLE_INSTANCE(DCameraSinkHandlerIpc);

void DCameraSinkHandlerIpc::Init()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("DCameraSinkHandlerIpc Init Start");
    if (isInit_) {
        DHLOGI("DCameraSinkHandlerIpc has already init");
        return;
    }
    sinkLocalRecipient_ = new SinkLocalRecipient();
    isInit_ = true;
    DHLOGI("DCameraSinkHandlerIpc Init End");
}

void DCameraSinkHandlerIpc::UnInit()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("DCameraSinkHandlerIpc UnInit Start");
    if (!isInit_) {
        DHLOGI("DCameraSinkHandlerIpc has already UnInit");
        return;
    }
    DeleteSinkLocalCamSrv();
    DHLOGI("DCameraSinkHandlerIpc Start free recipient");
    sinkLocalRecipient_ = nullptr;
    isInit_ = false;
    DHLOGI("DCameraSinkHandlerIpc UnInit End");
}

sptr<IDistributedCameraSink> DCameraSinkHandlerIpc::GetSinkLocalCamSrv()
{
    {
        std::lock_guard<std::mutex> autoLock(sinkLocalCamSrvLock_);
        if (localSink_ != nullptr) {
            DHLOGI("DCameraSinkHandlerIpc GetSinkLocalCamSrv from cache");
            return localSink_;
        }
    }
    DHLOGI("GetSinkLocalCamSrv Start");
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        DHLOGE("GetSinkLocalCamSrv GetSystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> object = sm->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    if (object == nullptr) {
        DHLOGE("GetSinkLocalCamSrv GetSystemAbility failed");
        return nullptr;
    }
    int32_t ret = object->AddDeathRecipient(sinkLocalRecipient_);
    sptr<IDistributedCameraSink> localSink = iface_cast<IDistributedCameraSink>(object);
    if (localSink == nullptr) {
        DHLOGI("GetSinkLocalCamSrv failed, localSink is null ret: %d", ret);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sinkLocalCamSrvLock_);
        if (localSink_ != nullptr) {
            localSink_->AsObject()->RemoveDeathRecipient(sinkLocalRecipient_);
        }
        localSink_ = localSink;
    }
    DHLOGI("GetSinkLocalCamSrv success, AddDeathRecipient ret: %d", ret);
    return localSink;
}

void DCameraSinkHandlerIpc::DeleteSinkLocalCamSrv()
{
    DHLOGI("DeleteSinkLocalCamSrv start");
    std::lock_guard<std::mutex> autoLock(sinkLocalCamSrvLock_);
    if (localSink_ != nullptr) {
        localSink_->AsObject()->RemoveDeathRecipient(sinkLocalRecipient_);
    }
    localSink_ = nullptr;
    DHLOGI("DeleteSinkLocalCamSrv end");
}

void DCameraSinkHandlerIpc::SinkLocalRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("SinkLocalRecipient OnRemoteDied received died notify!");
    DCameraSinkHandlerIpc::GetInstance().OnSinkLocalCamSrvDied(remote);
}

void DCameraSinkHandlerIpc::OnSinkLocalCamSrvDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("OnSinkLocalCamSrvDied delete diedRemoted");
    std::lock_guard<std::mutex> autoLock(sinkLocalCamSrvLock_);
    if (localSink_ == nullptr) {
        DHLOGE("DCameraSinkHandlerIpc::OnSinkLocalCamSrvDied, localSink is null.");
        return;
    }
    sptr<IRemoteObject> diedRemoted = remote.promote();
    if (diedRemoted == nullptr) {
        DHLOGE("OnSinkLocalCamSrvDied promote failed!");
        return;
    }
    if (localSink_->AsObject() != diedRemoted) {
        DHLOGI("OnSinkLocalCamSrvDied not found remote object.");
        return;
    }

    DHLOGI("OnSinkLocalCamSrvDied Clear");
    localSink_->AsObject()->RemoveDeathRecipient(sinkLocalRecipient_);
    localSink_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
