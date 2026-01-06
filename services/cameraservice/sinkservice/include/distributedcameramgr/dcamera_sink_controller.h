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

#ifndef OHOS_DCAMERA_SINK_CONTROLLER_H
#define OHOS_DCAMERA_SINK_CONTROLLER_H

#include "event_handler.h"
#include "icamera_controller.h"

#include "icamera_channel.h"
#include "icamera_operator.h"
#include "icamera_sink_access_control.h"
#include "icamera_sink_output.h"
#include <mutex>
#include <atomic>
#include "device_manager.h"
#include "device_manager_callback.h"
#include "property_carrier.h"
#include "idcamera_sink_callback.h"

namespace OHOS {
namespace DistributedHardware {
constexpr uint32_t EVENT_FRAME_TRIGGER = 1;
constexpr uint32_t EVENT_AUTHORIZATION = 2;
enum DcameraBusinessState : int32_t {
    UNKNOWN,
    IDLE,
    RUNNING,
    PAUSING
};
class DCameraSinkController : public ICameraController, public std::enable_shared_from_this<DCameraSinkController> {
public:
    explicit DCameraSinkController(std::shared_ptr<ICameraSinkAccessControl>& accessControl,
        const sptr<IDCameraSinkCallback> &sinkCallback);
    ~DCameraSinkController() override;

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

    void OnStateChanged(std::shared_ptr<DCameraEvent>& event);
    void OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings);

    void OnSessionState(int32_t state, std::string networkId);
    void OnSessionError(int32_t eventType, int32_t eventReason, std::string detail);
    void OnDataReceived(std::vector<std::shared_ptr<DataBuffer>>& buffers);

    class DCameraSinkContrEventHandler : public AppExecFwk::EventHandler {
        public:
            DCameraSinkContrEventHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner,
                std::shared_ptr<DCameraSinkController> sinkContrPtr);
            ~DCameraSinkContrEventHandler() override = default;
            void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
            enum {
                EVENT_START_BASE = 100,
                EVENT_ENCODER_PREPARED,
                EVENT_CAMERA_PREPARED,
            };
        private:
            std::weak_ptr<DCameraSinkController> sinkContrWPtr_;
    };

private:
    int32_t StartCaptureInner(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos);
    int32_t DCameraNotifyInner(int32_t type, int32_t result, std::string content);
    int32_t HandleReceivedData(std::shared_ptr<DataBuffer>& dataBuffer);
    void PostAuthorization(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos);
    bool CheckDeviceSecurityLevel(const std::string &srcDeviceId, const std::string &dstDeviceId);
    int32_t GetDeviceSecurityLevel(const std::string &udid);
    std::string GetUdidByNetworkId(const std::string &networkId);
    int32_t PullUpPage();
    void ProcessFrameTrigger(const AppExecFwk::InnerEvent::Pointer &event);
    void ProcessPostAuthorization(const AppExecFwk::InnerEvent::Pointer &event);
    int32_t CreateCtrlSession();
    int32_t CheckSensitive();
    bool CheckAclRight();
    bool IsIdenticalAccount(const std::string &networkId);
    class DCameraSurfaceHolder {
    public:
        DCameraSurfaceHolder(int32_t r, sptr<Surface> s) : result(r), surface(s) {}
        int32_t result;
        sptr<Surface> surface;
    };
    void HandleCaptureError(int32_t errorCode, const std::string& errorMsg);
    void CheckAndCommitCapture();
    std::atomic<bool> isEncoderReady_ {false};
    std::atomic<bool> isCameraReady_ {false};
    int32_t encoderResult_ = DCAMERA_OK;
    int32_t cameraResult_ = DCAMERA_OK;
    sptr<Surface> preparedSurface_ = nullptr;
    std::mutex stateMutex_;
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captureInfosCache_;

    bool isInit_;
    int32_t sessionState_;
    std::mutex autoLock_;
    std::mutex captureLock_;
    std::mutex channelLock_;
    std::string dhId_;
    std::string srcDevId_;
    std::shared_ptr<DCameraSinkContrEventHandler> sinkCotrEventHandler_;
    std::shared_ptr<ICameraChannel> channel_;
    std::shared_ptr<ICameraOperator> operator_;
    std::shared_ptr<ICameraSinkAccessControl> accessControl_;
    std::shared_ptr<ICameraSinkOutput> output_;
    sptr<IDCameraSinkCallback> sinkCallback_;
    std::atomic<bool> isPageStatus_ = false;
    std::shared_ptr<DmInitCallback> initCallback_;
    bool isSensitive_ = false;
    bool isSameAccount_ = false;
    bool isCheckSecLevel_ = false;
    int32_t sceneMode_ = 0;
    int32_t userId_ = -1;
    const int32_t MAX_RETRY_TIMES_ = 3;
    uint64_t tokenId_ = 0;
    uint64_t sinkTokenId_ = 0;
    std::string accountId_ = "";

    const std::string SESSION_FLAG = "control";
    const std::string SRC_TYPE = "camera";
    const size_t DATABUFF_MAX_SIZE = 100 * 1024 * 1024;
    const uint32_t MAX_ID_LENGTH = 256;
    const uint32_t MAX_ONLINE_DEVICE_SIZE = 1024;
    enum DcameraCaptureState : int32_t {
        CAPTURE_IDLE,
        CAPTURE_STARTING,
        CAPTURE_RUNNING,
    };

    std::mutex captureStateMutex_;
    std::condition_variable captureStateCv_;
    DcameraCaptureState captureState_ {CAPTURE_IDLE};
};

class DeviceInitCallback : public DmInitCallback {
    void OnRemoteDied() override;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_CONTROLLER_H
