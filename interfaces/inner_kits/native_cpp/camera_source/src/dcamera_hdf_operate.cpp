/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "dcamera_hdf_operate.h"

#include <hdf_base.h>
#include <hdf_device_class.h>

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "iproxy_broker.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DCameraHdfOperate"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraHdfOperate);
int32_t DCameraHdfOperate::LoadDcameraHDFImpl(std::shared_ptr<HdfDeathCallback> callback)
{
    DHLOGI("Load camera hdf impl begin!");
    int32_t ret = LoadDevice();
    if (ret != DCAMERA_OK) {
        DHLOGE("LoadDevice failed, ret: %{public}d.", ret);
        return ret;
    }
    ret = RegisterHdfListener();
    if (ret != DCAMERA_OK) {
        DHLOGE("RegisterHdfListener failed, ret: %{public}d.", ret);
        UnLoadDevice();
        return ret;
    }
    hdfDeathCallback_ = callback;
    ret = AddHdfDeathBind();
    if (ret != DCAMERA_OK) {
        DHLOGE("AddHdfDeathBind failed, ret: %{public}d.", ret);
        UnRegisterHdfListener();
        UnLoadDevice();
        return ret;
    }
    DHLOGI("Load camera hdf impl end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::UnLoadDcameraHDFImpl()
{
    DHLOGI("UnLoad camera hdf impl begin!");
    int32_t ret = RemoveHdfDeathBind();
    if (ret != DCAMERA_OK) {
        DHLOGE("RemoveHdfDeathBind failed, ret: %{public}d.", ret);
    }
    ret = UnRegisterHdfListener();
    if (ret != DCAMERA_OK) {
        DHLOGE("UnRegisterHdfListener failed, ret: %{public}d.", ret);
    }
    ret = UnLoadDevice();
    if (ret != DCAMERA_OK) {
        DHLOGE("UnLoadDevice failed, ret: %{public}d.", ret);
    }
    DHLOGI("UnLoad camera hdf impl end!");
    return DCAMERA_OK;
}

void DCameraHdfOperate::OnHdfHostDied()
{
    DHLOGI("On hdf host died begin!");
    if (hdfDeathCallback_) {
        DHLOGI("Call hdf host died callback!");
        hdfDeathCallback_->OnHdfHostDied();
    }
    DHLOGI("On hdf host died end!");
}

int32_t DCameraHdfOperate::WaitLoadCameraService()
{
    DHLOGI("wait Load camera service.");
    std::unique_lock<std::mutex> lock(hdfOperateMutex_);
    hdfOperateCon_.wait_for(lock, std::chrono::milliseconds(CAMERA_WAIT_TIME), [this] {
        return (this->cameraServStatus_.load() == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    });
    if (cameraServStatus_.load() != OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGE("wait load cameraService failed, status %{public}d", cameraServStatus_.load());
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::WaitLoadProviderService()
{
    DHLOGI("wait Load provider service.");
    std::unique_lock<std::mutex> lock(hdfOperateMutex_);
    hdfOperateCon_.wait_for(lock, std::chrono::milliseconds(CAMERA_WAIT_TIME), [this] {
        return (this->providerServStatus_.load() == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    });
    if (providerServStatus_.load() != OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGE("wait load providerService failed, status %{public}d", providerServStatus_.load());
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

OHOS::sptr<IServStatListener> DCameraHdfOperate::MakeServStatListener()
{
    return OHOS::sptr<IServStatListener>(
        new DCameraHdfServStatListener(DCameraHdfServStatListener::StatusCallback([&](const ServiceStatus& status) {
            DHLOGI("LoadCameraService service status callback, serviceName: %{public}s, status: %{public}d",
                status.serviceName.c_str(), status.status);
            std::unique_lock<std::mutex> lock(hdfOperateMutex_);
            if (status.serviceName == CAMERA_SERVICE_NAME) {
                cameraServStatus_.store(status.status);
                hdfOperateCon_.notify_one();
            } else if (status.serviceName == PROVIDER_SERVICE_NAME) {
                providerServStatus_.store(status.status);
                hdfOperateCon_.notify_one();
            }
        }))
    );
}

int32_t DCameraHdfOperate::LoadDevice()
{
    DHLOGI("LoadDevice for camera begin!");
    servMgr_ = IServiceManager::Get();
    devmgr_ = IDeviceManager::Get();
    if (servMgr_ == nullptr || devmgr_ == nullptr) {
        DHLOGE("get hdi service manager or device manager failed!");
        return DCAMERA_BAD_VALUE;
    }
    OHOS::sptr<IServStatListener> listener = MakeServStatListener();
    if (servMgr_->RegisterServiceStatusListener(listener, DEVICE_CLASS_CAMERA) != 0) {
        DHLOGE("RegisterServiceStatusListener failed!");
        return DCAMERA_BAD_OPERATE;
    }
    DHLOGI("Load camera service.");
    int32_t ret = devmgr_->LoadDevice(CAMERA_SERVICE_NAME);
    if (ret != HDF_SUCCESS && ret != HDF_ERR_DEVICE_BUSY) {
        DHLOGE("Load camera service failed!");
        servMgr_->UnregisterServiceStatusListener(listener);
        return DCAMERA_BAD_OPERATE;
    }
    if (WaitLoadCameraService() != DCAMERA_OK) {
        DHLOGE("Wait load camera service failed!");
        servMgr_->UnregisterServiceStatusListener(listener);
        return DCAMERA_BAD_OPERATE;
    }
    ret = devmgr_->LoadDevice(PROVIDER_SERVICE_NAME);
    if (ret != HDF_SUCCESS && ret != HDF_ERR_DEVICE_BUSY) {
        DHLOGE("Load camera provider service failed!");
        devmgr_->UnloadDevice(CAMERA_SERVICE_NAME);
        servMgr_->UnregisterServiceStatusListener(listener);
        return DCAMERA_BAD_OPERATE;
    }
    if (WaitLoadProviderService() != DCAMERA_OK) {
        DHLOGE("Wait load camera provider service failed!");
        devmgr_->UnloadDevice(CAMERA_SERVICE_NAME);
        servMgr_->UnregisterServiceStatusListener(listener);
        return DCAMERA_BAD_OPERATE;
    }
    if (servMgr_->UnregisterServiceStatusListener(listener) != 0) {
        DHLOGE("UnregisterServiceStatusListener failed!");
    }
    DHLOGI("LoadDevice for camera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::UnLoadDevice()
{
    DHLOGI("UnLoadDevice for camera begin!");
    if (devmgr_ == nullptr) {
        DHLOGE("hdi device manager is nullptr!");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = devmgr_->UnloadDevice(CAMERA_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload camera service failed, ret: %{public}d", ret);
    }
    ret = devmgr_->UnloadDevice(PROVIDER_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload provider service failed, ret: %d", ret);
    }
    cameraServStatus_.store(CAMERA_INVALID_VALUE);
    providerServStatus_.store(CAMERA_INVALID_VALUE);
    DHLOGI("UnLoadDevice for camera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::RegisterHdfListener()
{
    DHLOGI("RegisterHdfListener for camera begin!");
    camHdiProvider_ = IDCameraProvider::Get(PROVIDER_SERVICE_NAME);
    if (camHdiProvider_ == nullptr) {
        DHLOGE("Get hdi camera provider failed.");
        return DCAMERA_BAD_VALUE;
    }
    if (fwkDCameraHdfCallback_ == nullptr) {
        if (MakeFwkDCameraHdfCallback() != DCAMERA_OK) {
            DHLOGE("Create FwkDCameraHdfCallback failed.");
            return DCAMERA_BAD_VALUE;
        }
    }
    int32_t ret = camHdiProvider_->RegisterCameraHdfListener(HDF_LISTENER_SERVICE_NAME, fwkDCameraHdfCallback_);
    if (ret != DCAMERA_OK) {
        DHLOGE("Call hdf proxy RegisterCameraHdfListener failed, ret: %{public}d.", ret);
        return ret;
    }
    DHLOGI("RegisterHdfListener for camera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::UnRegisterHdfListener()
{
    DHLOGI("UnRegisterHdfListener for camera begin!");
    if (camHdiProvider_ == nullptr) {
        DHLOGE("hdi camera provider is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = camHdiProvider_->UnRegisterCameraHdfListener(HDF_LISTENER_SERVICE_NAME);
    if (ret != DCAMERA_OK) {
        DHLOGE("Call hdf proxy UnRegisterCameraHdfListener failed, ret: %{public}d.", ret);
        return ret;
    }
    DHLOGI("UnRegisterHdfListener for camera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::AddHdfDeathBind()
{
    DHLOGI("AddHdfDeathBind for dcamera begin!");
    if (camHdiProvider_ == nullptr) {
        DHLOGE("hdi dcamera manager is nullptr!");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<IDCameraProvider>(camHdiProvider_);
    if (remote == nullptr) {
        DHLOGE("Get remote from hdi dcamera manager failed!");
        return DCAMERA_BAD_VALUE;
    }
    if (remote->AddDeathRecipient(hdfDeathRecipient_) == false) {
        DHLOGE("Call AddDeathRecipient failed!");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("AddHdfDeathBind for dcamera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::RemoveHdfDeathBind()
{
    DHLOGI("RemoveHdfDeathBind for dcamera begin!");
    if (camHdiProvider_ == nullptr) {
        DHLOGE("hdi dcamera manager is nullptr!");
        return DCAMERA_BAD_VALUE;
    }
    sptr<IRemoteObject> remote = OHOS::HDI::hdi_objcast<IDCameraProvider>(camHdiProvider_);
    if (remote == nullptr) {
        DHLOGE("Get remote from hdi dcamera manager failed!");
        return DCAMERA_BAD_VALUE;
    }
    if (remote->RemoveDeathRecipient(hdfDeathRecipient_) == false) {
        DHLOGE("Call RemoveDeathRecipient failed!");
        return DCAMERA_BAD_OPERATE;
    }
    DHLOGI("RemoveHdfDeathBind for dcamera end!");
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::MakeFwkDCameraHdfCallback()
{
    std::lock_guard<std::mutex> locker(fwkDCameraHdfCallbackMutex_);
    if (fwkDCameraHdfCallback_ == nullptr) {
        fwkDCameraHdfCallback_ = OHOS::sptr<FwkDCameraHdfCallback>(new FwkDCameraHdfCallback());
        if (fwkDCameraHdfCallback_ == nullptr) {
            return DCAMERA_BAD_VALUE;
        }
    }
    return DCAMERA_OK;
}

void DCameraHdfServStatListener::OnReceive(const ServiceStatus& status)
{
    DHLOGI("service status on receive");
    if (status.serviceName == CAMERA_SERVICE_NAME || status.serviceName == PROVIDER_SERVICE_NAME) {
        callback_(status);
    }
}

int32_t FwkDCameraHdfCallback::NotifyEvent(int32_t devId, const DCameraHDFEvent& event)
{
    (void)devId;
    (void)event;
    return DCAMERA_OK;
}

void HdfDeathRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGI("On remote died!");
    DCameraHdfOperate::GetInstance().OnHdfHostDied();
}
} // namespace DistributedHardware
} // namespace OHOS
