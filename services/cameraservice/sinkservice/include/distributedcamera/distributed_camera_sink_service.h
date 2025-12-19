/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_SINK_SERVICE_H
#define OHOS_DISTRIBUTED_CAMERA_SINK_SERVICE_H

#include <mutex>
#include "system_ability.h"
#include "ipc_object_stub.h"

#include "dcamera_sink_dev.h"
#include "dcamera_sink_hidumper.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_sink_stub.h"
#include "iaccess_listener.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSinkService : public SystemAbility, public DistributedCameraSinkStub {
DECLARE_SYSTEM_ABILITY(DistributedCameraSinkService);

public:
    DistributedCameraSinkService(int32_t saId, bool runOnCreate);
    ~DistributedCameraSinkService() override = default;

    int32_t InitSink(const std::string& params, const sptr<IDCameraSinkCallback> &sinkCallback) override;
    int32_t ReleaseSink() override;
    int32_t SubscribeLocalHardware(const std::string& dhId, const std::string& parameters) override;
    int32_t UnsubscribeLocalHardware(const std::string& dhId) override;
    int32_t StopCapture(const std::string& dhId) override;
    int32_t ChannelNeg(const std::string& dhId, std::string& channelInfo) override;
    int32_t GetCameraInfo(const std::string& dhId, std::string& cameraInfo) override;
    int32_t OpenChannel(const std::string& dhId, std::string& openInfo) override;
    int32_t CloseChannel(const std::string& dhId) override;
    int Dump(int32_t fd, const std::vector<std::u16string>& args) override;
    static void GetCamDumpInfo(CameraDumpInfo& camDump);
    int32_t PauseDistributedHardware(const std::string &networkId) override;
    int32_t ResumeDistributedHardware(const std::string &networkId) override;
    int32_t StopDistributedHardware(const std::string &networkId) override;
    int32_t SetAccessListener(const sptr<IAccessListener> &listener, int32_t timeOut,
        const std::string &pkgName) override;
    int32_t RemoveAccessListener(const std::string &pkgName) override;
    int32_t SetAuthorizationResult(const std::string &requestId, bool granted) override;

protected:
    void OnStart() override;
    void OnStop() override;
    DISALLOW_COPY_AND_MOVE(DistributedCameraSinkService);

private:
    bool IsCurSinkDev(std::shared_ptr<DCameraSinkDev> sinkDevice);

    bool Init();
    void GetCamIds();
    bool registerToService_ = false;
    DCameraServiceState state_ = DCameraServiceState::DCAMERA_SRV_STATE_NOT_START;

    std::string sinkVer_;
    std::mutex mapMutex_;
    std::map<std::string, std::shared_ptr<DCameraSinkDev>> camerasMap_;
    static DistributedCameraSinkService* dcSinkService;
    const size_t DUMP_MAX_SIZE = 10 * 1024;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_SINK_SERVICE_H