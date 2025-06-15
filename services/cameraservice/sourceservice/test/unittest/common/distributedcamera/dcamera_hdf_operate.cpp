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
#include "distributed_hardware_errno.h"
#include "distributed_hardware_log.h"

#undef DH_LOG_TAG
#define DH_LOG_TAG "DCameraHdfOperate"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraHdfOperate);
int32_t DCameraHdfOperate::LoadDcameraHDFImpl()
{
    DHLOGI("Load camera hdf impl begin!");
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    if (hdfLoadRef_ > 0) {
        hdfLoadRef_++;
        DHLOGI("The camera hdf impl has been loaded, just inc ref!");
        return DH_FWK_SUCCESS;
    }
    int32_t ret = LoadDevice();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("LoadDevice failed, ret: %{public}d.", ret);
        return ret;
    }
    ret = RegisterHdfListener();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("RegisterHdfListener failed, ret: %{public}d.", ret);
        UnLoadDevice();
        return ret;
    }
    hdfLoadRef_++;
    DHLOGI("Load camera hdf impl end!");
    return DH_FWK_SUCCESS;
}

int32_t DCameraHdfOperate::UnLoadDcameraHDFImpl()
{
    DHLOGI("UnLoad camera hdf impl begin!");
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    if (hdfLoadRef_ == 0) {
        DHLOGI("The camera hdf impl has been unloaded!");
        return DH_FWK_SUCCESS;
    }
    if (hdfLoadRef_ > 1) {
        hdfLoadRef_--;
        DHLOGI("The camera hdf impl has been loaded, just dec ref!");
        return DH_FWK_SUCCESS;
    }
    int32_t ret = UnRegisterHdfListener();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UnRegisterHdfListener failed, ret: %{public}d.", ret);
    }
    ret = UnLoadDevice();
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("UnLoadDevice failed, ret: %{public}d.", ret);
    }
    hdfLoadRef_--;
    DHLOGI("UnLoad camera hdf impl end!");
    return DH_FWK_SUCCESS;
}

void DCameraHdfOperate::ResetRefCount()
{
    DHLOGI("Reset reference count for dcamera.");
    std::unique_lock<std::mutex> loadRefLocker(hdfLoadRefMutex_);
    hdfLoadRef_ = 0;
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
        return ERR_DH_FWK_LOAD_HDF_TIMEOUT;
    }
    return DH_FWK_SUCCESS;
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
        return ERR_DH_FWK_LOAD_HDF_TIMEOUT;
    }
    return DH_FWK_SUCCESS;
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
    OHOS::sptr<IServiceManager> servMgr = IServiceManager::Get();
    OHOS::sptr<IDeviceManager> devmgr = IDeviceManager::Get();
    if (servMgr == nullptr || devmgr == nullptr) {
        DHLOGE("get hdi service manager or device manager failed!");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    OHOS::sptr<IServStatListener> listener = MakeServStatListener();
    if (servMgr->RegisterServiceStatusListener(listener, DEVICE_CLASS_CAMERA) != 0) {
        DHLOGE("RegisterServiceStatusListener failed!");
        return ERR_DH_FWK_REGISTER_HDF_LISTENER_FAIL;
    }
    DHLOGI("Load camera service.");
    int32_t ret = devmgr->LoadDevice(CAMERA_SERVICE_NAME);
    if (ret != HDF_SUCCESS && ret != HDF_ERR_DEVICE_BUSY) {
        DHLOGE("Load camera service failed!");
        servMgr->UnregisterServiceStatusListener(listener);
        return ERR_DH_FWK_LOAD_HDF_FAIL;
    }
    if (WaitLoadCameraService() != DH_FWK_SUCCESS) {
        DHLOGE("Wait load camera service failed!");
        servMgr->UnregisterServiceStatusListener(listener);
        return ERR_DH_FWK_LOAD_HDF_TIMEOUT;
    }
    ret = devmgr->LoadDevice(PROVIDER_SERVICE_NAME);
    if (ret != HDF_SUCCESS && ret != HDF_ERR_DEVICE_BUSY) {
        DHLOGE("Load camera provider service failed!");
        servMgr->UnregisterServiceStatusListener(listener);
        return ERR_DH_FWK_LOAD_HDF_FAIL;
    }
    if (WaitLoadProviderService() != DH_FWK_SUCCESS) {
        DHLOGE("Wait load camera provider service failed!");
        servMgr->UnregisterServiceStatusListener(listener);
        return ERR_DH_FWK_LOAD_HDF_TIMEOUT;
    }
    if (servMgr->UnregisterServiceStatusListener(listener) != 0) {
        DHLOGE("UnregisterServiceStatusListener failed!");
    }
    DHLOGI("LoadDevice for camera end!");
    return DH_FWK_SUCCESS;
}

int32_t DCameraHdfOperate::UnLoadDevice()
{
    DHLOGI("UnLoadDevice for camera begin!");
    OHOS::sptr<IDeviceManager> devmgr = IDeviceManager::Get();
    if (devmgr == nullptr) {
        DHLOGE("get hdi device manager failed!");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t ret = devmgr->UnloadDevice(CAMERA_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload camera service failed, ret: %{public}d", ret);
    }
    ret = devmgr->UnloadDevice(PROVIDER_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload provider service failed, ret: %d", ret);
    }
    cameraServStatus_.store(CAMERA_INVALID_VALUE);
    providerServStatus_.store(CAMERA_INVALID_VALUE);
    DHLOGI("UnLoadDevice for camera end!");
    return DH_FWK_SUCCESS;
}

int32_t DCameraHdfOperate::RegisterHdfListener()
{
    DHLOGI("RegisterHdfListener for camera begin!");
    sptr<IDCameraProvider> camHdiProvider = IDCameraProvider::Get(PROVIDER_SERVICE_NAME);
    if (camHdiProvider == nullptr) {
        DHLOGE("Get hdi camera provider failed.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    if (fwkDCameraHdfCallback_ == nullptr) {
        fwkDCameraHdfCallback_ = new FwkDCameraHdfCallback();
        if (fwkDCameraHdfCallback_ == nullptr) {
            DHLOGE("Create FwkDCameraHdfCallback failed.");
            return ERR_DH_FWK_POINTER_IS_NULL;
        }
    }
    int32_t ret = camHdiProvider->RegisterCameraHdfListener("DHFWK", fwkDCameraHdfCallback_);
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Call hdf proxy RegisterCameraHdfListener failed, ret: %{public}d.", ret);
        return ret;
    }
    DHLOGI("RegisterHdfListener for camera end!");
    return DH_FWK_SUCCESS;
}

int32_t DCameraHdfOperate::UnRegisterHdfListener()
{
    DHLOGI("UnRegisterHdfListener for camera begin!");
    sptr<IDCameraProvider> camHdiProvider = IDCameraProvider::Get(PROVIDER_SERVICE_NAME);
    if (camHdiProvider == nullptr) {
        DHLOGE("Get hdi camera provider failed.");
        return ERR_DH_FWK_POINTER_IS_NULL;
    }
    int32_t ret = camHdiProvider->UnRegisterCameraHdfListener("DHFWK");
    if (ret != DH_FWK_SUCCESS) {
        DHLOGE("Call hdf proxy UnRegisterCameraHdfListener failed, ret: %{public}d.", ret);
        return ret;
    }
    DHLOGI("UnRegisterHdfListener for camera end!");
    return DH_FWK_SUCCESS;
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
    return DH_FWK_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
