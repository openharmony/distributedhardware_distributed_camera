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

#ifndef OHOS_DCAMERA_HDF_OPERATE_H
#define OHOS_DCAMERA_HDF_OPERATE_H

#include <atomic>
#include <condition_variable>
#include <mutex>

#include "idistributed_hardware_source.h"
#include "iservstat_listener_hdi.h"
#include "idevmgr_hdi.h"
#include "iservmgr_hdi.h"
#include "single_instance.h"
#include "v1_1/id_camera_provider.h"

namespace OHOS {
namespace DistributedHardware {
using OHOS::HDI::DeviceManager::V1_0::IDeviceManager;
using OHOS::HDI::ServiceManager::V1_0::IServiceManager;
using OHOS::HDI::ServiceManager::V1_0::IServStatListener;
using OHOS::HDI::ServiceManager::V1_0::ServiceStatus;
using OHOS::HDI::ServiceManager::V1_0::ServStatListenerStub;
using OHOS::HDI::DistributedCamera::V1_1::IDCameraProvider;
using OHOS::HDI::DistributedCamera::V1_1::IDCameraHdfCallback;
using OHOS::HDI::DistributedCamera::V1_1::DCameraHDFEvent;
const std::string CAMERA_SERVICE_NAME = "distributed_camera_service";
const std::string PROVIDER_SERVICE_NAME = "distributed_camera_provider_service";
const std::string HDF_LISTENER_SERVICE_NAME = "DHFWK";
constexpr uint16_t CAMERA_INVALID_VALUE = 0xffff;
constexpr int32_t CAMERA_WAIT_TIME = 2000;

class FwkDCameraHdfCallback;
class HdfDeathRecipient : public IRemoteObject::DeathRecipient {
public:
    void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
};
class DCameraHdfOperate {
DECLARE_SINGLE_INSTANCE(DCameraHdfOperate);

public:
    int32_t LoadDcameraHDFImpl(std::shared_ptr<HdfDeathCallback> callback);
    int32_t UnLoadDcameraHDFImpl();
    void OnHdfHostDied();

private:
    int32_t WaitLoadCameraService();
    int32_t WaitLoadProviderService();
    OHOS::sptr<IServStatListener> MakeServStatListener();
    int32_t LoadDevice();
    int32_t UnLoadDevice();
    int32_t RegisterHdfListener();
    int32_t UnRegisterHdfListener();
    int32_t AddHdfDeathBind();
    int32_t RemoveHdfDeathBind();
    int32_t MakeFwkDCameraHdfCallback();

private:
    OHOS::sptr<IDeviceManager> devmgr_;
    OHOS::sptr<IServiceManager> servMgr_;
    sptr<IDCameraProvider> camHdiProvider_;
    std::mutex fwkDCameraHdfCallbackMutex_;
    OHOS::sptr<FwkDCameraHdfCallback> fwkDCameraHdfCallback_;
    std::atomic<uint16_t> cameraServStatus_ = CAMERA_INVALID_VALUE;
    std::atomic<uint16_t> providerServStatus_ = CAMERA_INVALID_VALUE;
    std::condition_variable hdfOperateCon_;
    std::mutex hdfOperateMutex_;
    std::shared_ptr<HdfDeathCallback> hdfDeathCallback_;
    sptr<HdfDeathRecipient> hdfDeathRecipient_ = sptr<HdfDeathRecipient>(new HdfDeathRecipient());
};

class DCameraHdfServStatListener : public OHOS::HDI::ServiceManager::V1_0::ServStatListenerStub {
public:
    using StatusCallback = std::function<void(const ServiceStatus &)>;
    explicit DCameraHdfServStatListener(StatusCallback callback) : callback_(std::move(callback))
    {
    }
    ~DCameraHdfServStatListener() override = default;
    void OnReceive(const ServiceStatus& status) override;

private:
    StatusCallback callback_;
};

class FwkDCameraHdfCallback : public IDCameraHdfCallback {
protected:
    int32_t NotifyEvent(int32_t devId, const DCameraHDFEvent& event) override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HDF_OPERATE_H