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

#ifndef OHOS_DISTRIBUTED_CAMERA_SOURCE_SERVICE_H
#define OHOS_DISTRIBUTED_CAMERA_SOURCE_SERVICE_H

#include <memory>
#include <mutex>
#include <map>
#include <thread>
#include <vector>

#include "system_ability.h"
#include "ipc_object_stub.h"

#include "dcamera_source_hidumper.h"
#include "dcamera_index.h"
#include "dcamera_service_state_listener.h"
#include "dcamera_source_dev.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_source_stub.h"
#include "distributed_hardware_fwk_kit.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSourceService : public SystemAbility, public DistributedCameraSourceStub {
DECLARE_SYSTEM_ABILITY(DistributedCameraSourceService);

public:
    DistributedCameraSourceService(int32_t saId, bool runOnCreate);
    ~DistributedCameraSourceService() override = default;

    int32_t InitSource(const std::string& params, const sptr<IDCameraSourceCallback>& callback) override;
    int32_t ReleaseSource() override;
    int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& reqId, const EnableParam& param) override;
    int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const std::string& reqId) override;
    int32_t DCameraNotify(const std::string& devId, const std::string& dhId, std::string& events) override;
    int Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    static void GetDumpInfo(CameraDumpInfo& camDump);

    static void CamDevInsert(DCameraIndex& index, std::shared_ptr<DCameraSourceDev>& camDev);
    static std::shared_ptr<DCameraSourceDev> GetCamDevByIndex(DCameraIndex& index);
    static void CamDevErase(DCameraIndex& index);
    static uint32_t GetCamDevNum();
    void StartHicollieThread();
    int32_t UpdateDistributedHardwareWorkMode(const std::string& devId, const std::string& dhId,
        const WorkModeParam& param) override;

protected:
    void OnStart() override;
    void OnStop() override;
    DISALLOW_COPY_AND_MOVE(DistributedCameraSourceService);

private:
    bool Init();
    int32_t LoadDCameraHDF();
    int32_t UnLoadCameraHDF();
    std::string GetCodecInfo();
    std::shared_ptr<DistributedHardwareFwkKit> GetDHFwkKit();

    bool registerToService_ = false;
    DCameraServiceState state_ = DCameraServiceState::DCAMERA_SRV_STATE_NOT_START;

    std::shared_ptr<ICameraStateListener> listener_;
    std::string sourceVer_;
    const size_t MAX_CAMERAS_NUMBER = 32;
    const size_t DUMP_MAX_SIZE = 10 * 1024;

    static std::map<DCameraIndex, std::shared_ptr<DCameraSourceDev>> camerasMap_;
    static std::mutex camDevMutex_;
    std::mutex listenerMutex_;
    std::thread hicollieThread_;
    std::atomic<bool> isHicollieRunning_ = false;
    std::shared_ptr<DistributedHardwareFwkKit> dHFwkKit_;
    std::mutex dHFwkKitMutex_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_SOURCE_SERVICE_H
