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

#ifndef OHOS_DCAMERA_SOURCE_CONTROLLER_H
#define OHOS_DCAMERA_SOURCE_CONTROLLER_H

#include "icamera_controller.h"

#include "dcamera_index.h"
#include "icamera_channel_listener.h"
#include "dcamera_source_dev.h"
#include "dcamera_source_state_machine.h"
#include "icamera_channel.h"
#include "iremote_object.h"
#include "device_manager.h"
#include "camera_service_proxy.h"

#include "v1_1/id_camera_provider.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceController : public ICameraController,
    public std::enable_shared_from_this<DCameraSourceController> {
public:
    DCameraSourceController(std::string devId, std::string dhId,
        std::shared_ptr<DCameraSourceStateMachine>& stateMachine, std::shared_ptr<DCameraSourceDev>& camDev);
    ~DCameraSourceController() override;
    int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode) override;
    int32_t StopCapture() override;
    int32_t ChannelNeg(std::shared_ptr<DCameraChannelInfo>& info) override;
    int32_t DCameraNotify(std::shared_ptr<DCameraEvent>& events) override;
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings) override;
    int32_t GetCameraInfo(std::shared_ptr<DCameraInfo>& camInfo) override;
    int32_t OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo) override;
    int32_t CloseChannel() override;
    int32_t Init(std::vector<DCameraIndex>& indexs) override;
    int32_t UnInit() override;
    int32_t PauseDistributedHardware(const std::string &networkId) override;
    int32_t ResumeDistributedHardware(const std::string &networkId) override;
    int32_t StopDistributedHardware(const std::string &networkId) override;
    void SetTokenId(uint64_t token) override;

    void OnSessionState(int32_t state, std::string networkId);
    void OnSessionError(int32_t eventType, int32_t eventReason, std::string detail);
    void OnDataReceived(std::vector<std::shared_ptr<DataBuffer>>& buffers);

private:
    void HandleMetaDataResult(std::string& jsonStr);
    void PostChannelDisconnectedEvent();
    int32_t PublishEnableLatencyMsg(const std::string& devId);
    void HandleReceivedData(std::shared_ptr<DataBuffer> &dataBuffer);
    bool CheckAclRight();
    bool GetOsAccountInfo();
    int32_t CheckOsType(const std::string &networkId, bool &isInvalid);
    int32_t ParseValueFromCjson(std::string args, std::string key);
    int32_t AddCameraServiceDeathRecipient();
    class DCameraHdiRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;
    };
    sptr<DCameraHdiRecipient> cameraHdiRecipient_;
    class CameraServiceRecipient : public IRemoteObject::DeathRecipient {
    public:
        CameraServiceRecipient(std::shared_ptr<DCameraSourceController> sourceContrlPtr);
        ~CameraServiceRecipient() override = default;
        void OnRemoteDied(const wptr<IRemoteObject> &remote) override;

    private:
        std::weak_ptr<DCameraSourceController> sourceContrlPtr_;
    };
    sptr<CameraServiceRecipient> cameraServiceRecipient_;

private:
    std::string devId_;
    std::string dhId_;
    std::shared_ptr<ICameraChannel> channel_;
    std::shared_ptr<ICameraChannelListener> listener_;
    std::vector<DCameraIndex> indexs_;
    std::shared_ptr<DCameraSourceStateMachine> stateMachine_;
    std::weak_ptr<DCameraSourceDev> camDev_;
    int32_t channelState_;
    sptr<IDCameraProvider> camHdiProvider_;
    sptr<IRemoteObject> remote_;
    sptr<CameraStandard::ICameraService> cameraServiceProxy_;
    std::shared_ptr<DCameraSourceController> controller_;

    bool isInit;
    const std::string SESSION_FLAG = "control";

    static constexpr uint8_t CHANNEL_REL_SECONDS = 5;
    const size_t DATABUFF_MAX_SIZE = 100 * 1024 * 1024;
    std::atomic<bool> isChannelConnected_ = false;
    std::mutex channelMtx_;
    std::condition_variable channelCond_;
    std::string accountId_ = "";
    int32_t userId_ = -1;
    std::string srcDevId_ = "";
    uint64_t tokenId_ = 0;
};

class DeviceInitCallback : public DmInitCallback {
    void OnRemoteDied() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOURCE_CONTROLLER_H
