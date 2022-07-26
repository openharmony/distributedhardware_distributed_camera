/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include <hdf_io_service_if.h>

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraHdfOperate);

void DCameraHdfServStatListener::OnReceive(const ServiceStatus& status)
{
    DHLOGI("service status on receive");
    if (status.serviceName == CAMERA_SERVICE_NAME || status.serviceName == PROVIDER_SERVICE_NAME) {
        callback_(status);
    }
}

int32_t DCameraHdfOperate::LoadDcameraHDFImpl()
{
    if (cameraServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START &&
        providerServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGI("service has already start");
        return DCAMERA_OK;
    }
    servMgr_ = IServiceManager::Get();
    devmgr_ = IDeviceManager::Get();
    if (servMgr_ == nullptr || devmgr_ == nullptr) {
        DHLOGE("get hdi service manager or device manager failed!");
        return DCAMERA_BAD_VALUE;
    }

    ::OHOS::sptr<IServStatListener> listener =
        new DCameraHdfServStatListener(DCameraHdfServStatListener::StatusCallback([&](const ServiceStatus& status) {
            DHLOGI("LoadCameraService service status callback, serviceName: %s, status: %d",
                status.serviceName.c_str(), status.status);
            std::unique_lock<std::mutex> lock(hdfOperateMutex_);
            if (status.serviceName == CAMERA_SERVICE_NAME) {
                cameraServStatus_ = status.status;
                hdfOperateCon_.notify_one();
            } else if (status.serviceName == PROVIDER_SERVICE_NAME) {
                providerServStatus_ = status.status;
                hdfOperateCon_.notify_one();
            }
        }));
    if (servMgr_->RegisterServiceStatusListener(listener, DEVICE_CLASS_CAMERA) != 0) {
        DHLOGE("RegisterServiceStatusListener failed!");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGI("Load camera service.");
    if (devmgr_->LoadDevice(CAMERA_SERVICE_NAME) != 0) {
        DHLOGE("Load camera service failed!");
        return DCAMERA_BAD_OPERATE;
    }
    if (WaitLoadCameraService() != DCAMERA_OK) {
        return DCAMERA_BAD_OPERATE;
    }

    DHLOGI("Load provider service.");
    if (devmgr_->LoadDevice(PROVIDER_SERVICE_NAME) != 0) {
        DHLOGE("Load provider service failed!");
        return DCAMERA_BAD_OPERATE;
    }
    if (WaitLoadProviderService() != DCAMERA_OK) {
        return DCAMERA_BAD_OPERATE;
    }

    if (servMgr_->UnregisterServiceStatusListener(listener) != 0) {
        DHLOGE("UnregisterServiceStatusListener failed!");
    }
    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::WaitLoadCameraService()
{
    DHLOGI("wait Load camera service.");
    std::unique_lock<std::mutex> lock(hdfOperateMutex_);
    hdfOperateCon_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME), [this] {
        return (this->cameraServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    });

    if (cameraServStatus_ != OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGE("wait load cameraService failed, status %d", cameraServStatus_);
        return DCAMERA_BAD_OPERATE;
    }

    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::WaitLoadProviderService()
{
    DHLOGI("wait Load provider service.");
    std::unique_lock<std::mutex> lock(hdfOperateMutex_);
    hdfOperateCon_.wait_for(lock, std::chrono::milliseconds(WAIT_TIME), [this] {
        return (this->providerServStatus_ == OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START);
    });

    if (providerServStatus_ != OHOS::HDI::ServiceManager::V1_0::SERVIE_STATUS_START) {
        DHLOGE("wait load providerService failed, status %d", providerServStatus_);
        return DCAMERA_BAD_OPERATE;
    }

    return DCAMERA_OK;
}

int32_t DCameraHdfOperate::UnLoadDcameraHDFImpl()
{
    DHLOGI("UnLoadCameraHDFImpl begin!");
    devmgr_ = IDeviceManager::Get();
    if (devmgr_ == nullptr) {
        DHLOGE("get hdi device manager failed!");
        return DCAMERA_BAD_VALUE;
    }

    int32_t ret = devmgr_->UnloadDevice(CAMERA_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload camera service failed, ret: %d", ret);
    }
    ret = devmgr_->UnloadDevice(PROVIDER_SERVICE_NAME);
    if (ret != 0) {
        DHLOGE("Unload provider service failed, ret: %d", ret);
    }
    cameraServStatus_ = INVALID_VALUE;
    providerServStatus_ = INVALID_VALUE;
    DHLOGI("UnLoadCameraHDFImpl end!");
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS