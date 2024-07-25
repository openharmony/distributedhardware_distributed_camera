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

#include "dcamera_source_handler_ipc.h"

#include <cstdint>
#include <new>

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "dcamera_source_handler.h"
#include "distributed_camera_constants.h"
#include "distributed_hardware_log.h"
#include "iremote_broker.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceHandlerIpc::DCameraSourceHandlerIpc() : isInit_(false)
{
    DHLOGI("DCameraSourceHandlerIpc Create");
}

DCameraSourceHandlerIpc::~DCameraSourceHandlerIpc()
{
    DHLOGI("DCameraSourceHandlerIpc Delete");
    UnInit();
}

IMPLEMENT_SINGLE_INSTANCE(DCameraSourceHandlerIpc);

void DCameraSourceHandlerIpc::Init()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("Start");
    if (isInit_) {
        DHLOGI("DCameraSourceHandlerIpc has already init");
        return;
    }
    sourceLocalRecipient_ = sptr<SourceLocalRecipient>(new SourceLocalRecipient());
    isInit_ = true;
    DHLOGI("End");
}

void DCameraSourceHandlerIpc::UnInit()
{
    std::lock_guard<std::mutex> autoLock(initCamSrvLock_);
    DHLOGI("Start");
    if (!isInit_) {
        DHLOGI("DCameraSourceHandlerIpc has already UnInit");
        return;
    }
    DeleteSourceLocalCamSrv();
    DHLOGI("DCameraSourceHandlerIpc Start free recipient");
    sourceLocalRecipient_ = nullptr;
    isInit_ = false;
    DHLOGI("End");
}

sptr<IDistributedCameraSource> DCameraSourceHandlerIpc::GetSourceLocalCamSrv()
{
    {
        std::lock_guard<std::mutex> autoLock(sourceLocalCamSrvLock_);
        if (localSource_ != nullptr) {
            DHLOGI("DCameraSourceHandlerIpc GetSourceLocalCamSrv from cache");
            return localSource_;
        }
    }
    DHLOGI("Start");
    sptr<ISystemAbilityManager> sm = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sm == nullptr) {
        DHLOGE("GetSystemAbilityManager failed");
        return nullptr;
    }

    sptr<IRemoteObject> object = sm->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID);
    if (object == nullptr) {
        DHLOGE("GetSystemAbility failed");
        return nullptr;
    }
    int32_t ret = object->AddDeathRecipient(sourceLocalRecipient_);
    sptr<IDistributedCameraSource> localSource = iface_cast<IDistributedCameraSource>(object);
    if (localSource == nullptr) {
        DHLOGI("GetSourceLocalCamSrv failed, localSource is null ret: %{public}d", ret);
        return nullptr;
    }
    {
        std::lock_guard<std::mutex> autoLock(sourceLocalCamSrvLock_);
        if (localSource_ != nullptr && localSource_->AsObject() != nullptr) {
            localSource_->AsObject()->RemoveDeathRecipient(sourceLocalRecipient_);
        }
        localSource_ = localSource;
    }
    DHLOGI("success, AddDeathRecipient ret: %{public}d", ret);
    return localSource;
}

void DCameraSourceHandlerIpc::DeleteSourceLocalCamSrv()
{
    DHLOGI("start");
    std::lock_guard<std::mutex> autoLock(sourceLocalCamSrvLock_);
    if (localSource_ != nullptr && localSource_->AsObject() != nullptr) {
        localSource_->AsObject()->RemoveDeathRecipient(sourceLocalRecipient_);
    }
    localSource_ = nullptr;
    DHLOGI("end");
}

void DCameraSourceHandlerIpc::SourceLocalRecipient::OnRemoteDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("SourceLocalRecipient OnRemoteDied received died notify!");
    DCameraSourceHandlerIpc::GetInstance().OnSourceLocalCamSrvDied(remote);
}

void DCameraSourceHandlerIpc::OnSourceLocalCamSrvDied(const wptr<IRemoteObject>& remote)
{
    DHLOGI("OnSourceLocalCamSrvDied delete diedRemoted");
    std::lock_guard<std::mutex> autoLock(sourceLocalCamSrvLock_);
    if (localSource_ == nullptr) {
        DHLOGE("localSource is null.");
        return;
    }
    sptr<IRemoteObject> diedRemoted = remote.promote();
    if (diedRemoted == nullptr) {
        DHLOGE("OnSourceLocalCamSrvDied promote failed!");
        return;
    }
    if (localSource_->AsObject() != diedRemoted) {
        DHLOGI("OnSourceLocalCamSrvDied not found remote object.");
        return;
    }
    DHLOGI("OnSourceLocalCamSrvDied Clear");
    localSource_->AsObject()->RemoveDeathRecipient(sourceLocalRecipient_);
    localSource_ = nullptr;
    DCameraSourceHandler::GetInstance().SetSAState();
}
} // namespace DistributedHardware
} // namespace OHOS
