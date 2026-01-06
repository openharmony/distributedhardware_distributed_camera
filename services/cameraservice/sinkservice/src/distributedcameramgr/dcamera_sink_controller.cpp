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

#include "dcamera_sink_controller.h"

#include <securec.h>
#include <thread>
#include <chrono>

#include "anonymous_string.h"
#include "dcamera_channel_sink_impl.h"
#include "dcamera_client.h"
#include "dcamera_metadata_setting_cmd.h"
#include "dcamera_protocol.h"
#include "dcamera_utils_tools.h"

#include "dcamera_sink_access_control.h"
#include "dcamera_sink_controller_channel_listener.h"
#include "dcamera_sink_controller_state_callback.h"
#include "dcamera_sink_output.h"
#include "dcamera_sink_service_ipc.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#ifdef DEVICE_SECURITY_LEVEL_ENABLE
#include "device_security_defines.h"
#include "device_security_info.h"
#endif
#include "ffrt_inner.h"
#include "idistributed_camera_source.h"
#include "ipc_skeleton.h"
#include "dcamera_low_latency.h"
#ifdef OS_ACCOUNT_ENABLE
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#endif
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
const int DEFAULT_DEVICE_SECURITY_LEVEL = -1;
#ifdef SECURITY_LEVEL_CHECK_ENABLE
constexpr int32_t MINIMUM_SECURITY_LEVEL = 3;
#endif
const std::string PAGE_SUBTYPE = "camera";

DCameraSinkController::DCameraSinkController(std::shared_ptr<ICameraSinkAccessControl>& accessControl,
    const sptr<IDCameraSinkCallback> &sinkCallback)
    : isInit_(false), sessionState_(DCAMERA_CHANNEL_STATE_DISCONNECTED), accessControl_(accessControl),
      sinkCallback_(sinkCallback)
{
}

DCameraSinkController::~DCameraSinkController()
{
    if (isInit_) {
        UnInit();
    }
}

int32_t DCameraSinkController::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
    int32_t sceneMode)
{
    DHLOGI("StartCapture dhId: %{public}s, mode: %{public}d", GetAnonyString(dhId_).c_str(), sceneMode);
    std::string accessType = "";
    CHECK_AND_RETURN_RET_LOG(accessControl_ == nullptr, DCAMERA_BAD_VALUE, "accessControl_ is null.");
    if ((accessControl_->IsSensitiveSrcAccess(SRC_TYPE)) &&
        (accessControl_->GetAccessControlType(accessType) == DCAMERA_SAME_ACCOUNT)) {
#ifdef SECURITY_LEVEL_CHECK_ENABLE
        std::string sourceUdid = GetUdidByNetworkId(srcDevId_);
        if (sourceUdid.empty()) {
            DHLOGE("source udid is empty");
            return DCAMERA_BAD_VALUE;
        }
        int32_t sourceSecurityLevel = GetDeviceSecurityLevel(sourceUdid);
        DHLOGI("sourceSecurityLevel: %{public}d", sourceSecurityLevel);
        if (sourceSecurityLevel < MINIMUM_SECURITY_LEVEL) {
            return DCameraNotifyInner(DCAMERA_SINK_STOP, DCAMERA_EVENT_NO_PERMISSION,
                std::string("source stop dcamera security less than three"));
        }
#endif
        int32_t ret = StartCaptureInner(captureInfos);
        if (ret == DCAMERA_OK) {
            accessControl_->NotifySensitiveSrc(SRC_TYPE);
        }
        return ret;
    } else {
        std::shared_ptr<std::string> param = std::make_shared<std::string>("");
        std::shared_ptr<std::vector<std::shared_ptr<DCameraCaptureInfo>>> infos =
            std::make_shared<std::vector<std::shared_ptr<DCameraCaptureInfo>>>(captureInfos);
        CHECK_AND_RETURN_RET_LOG(sinkCotrEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "sinkCotrEventHandler_ is null.");
        AppExecFwk::InnerEvent::Pointer triggerEvent =
                AppExecFwk::InnerEvent::Get(EVENT_FRAME_TRIGGER, param, 0);
        AppExecFwk::InnerEvent::Pointer authorizationEvent =
                AppExecFwk::InnerEvent::Get(EVENT_AUTHORIZATION, infos, 0);
        sinkCotrEventHandler_->SendEvent(triggerEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
        sinkCotrEventHandler_->SendEvent(authorizationEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkController::StopCapture()
{
    DHLOGI("StopCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::unique_lock<std::mutex> lock(captureStateMutex_);
    if (!captureStateCv_.wait_for(lock, std::chrono::seconds(MAX_RETRY_TIMES_),
        [this] { return captureState_ != CAPTURE_STARTING; })) {
        DHLOGE("StopCapture timed out waiting for state to change from STARTING. dhId: %{public}s",
               GetAnonyString(dhId_).c_str());
        return DCAMERA_WRONG_STATE;
    }
    if (captureState_ == CAPTURE_IDLE) {
        DHLOGI("StopCapture called when state is already IDLE. dhId: %{public}s", GetAnonyString(dhId_).c_str());
        return DCAMERA_OK;
    }
    captureState_ = CAPTURE_IDLE;
    if (operator_ == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = operator_->StopCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("client stop capture failed, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(dhId_).c_str(), ret);
        DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_ERROR, std::string("operator stop capture failed."));
        return ret;
    }
    if (output_ == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    ret = output_->StopCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("output stop capture failed, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(dhId_).c_str(), ret);
        DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_ERROR, std::string("output stop capture failed"));
        return ret;
    }
    DHLOGI("StopCapture %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::ChannelNeg(std::shared_ptr<DCameraChannelInfo>& info)
{
    DHLOGI("ChannelNeg dhId: %{public}s", GetAnonyString(dhId_).c_str());
    int32_t ret = output_->OpenChannel(info);
    if (ret != DCAMERA_OK) {
        DHLOGE("channel negotiate failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    DHLOGI("ChannelNeg %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::DCameraNotify(std::shared_ptr<DCameraEvent>& events)
{
    DHLOGI("DCameraNotify dhId: %{public}s", GetAnonyString(dhId_).c_str());
    CHECK_AND_RETURN_RET_LOG(srcDevId_.empty(), DCAMERA_BAD_VALUE, "source deviceId is empty");

    DCameraEventCmd eventCmd;
    std::string jsonStr = "";
    eventCmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
    eventCmd.dhId_ = dhId_;
    eventCmd.command_ = DCAMERA_PROTOCOL_CMD_STATE_NOTIFY;
    eventCmd.value_ = events;
    int32_t ret = eventCmd.Marshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraEventCmd marshal failed, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
        ret = memcpy_s(buffer->Data(), buffer->Capacity(),
            reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())), jsonStr.length());
        CHECK_AND_RETURN_RET_LOG(ret != EOK, DCAMERA_BAD_VALUE, "DCameraNotify memcpy_s failed, ret: %{public}d", ret);
        ret = channel_->SendData(buffer);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraNotify channel send data failed, dhId: %{public}s ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
            return DCAMERA_BAD_VALUE;
        }
    } else {
        std::string sinkDevId;
        ret = GetLocalDeviceNetworkId(sinkDevId);
        if (ret != DCAMERA_OK) {
            DHLOGE("GetLocalDeviceNetworkId failed, devId: %{public}s, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(sinkDevId).c_str(), GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }

        sptr<IDistributedCameraSource> sourceSA = DCameraSinkServiceIpc::GetInstance().GetSourceRemoteCamSrv(srcDevId_);
        CHECK_AND_RETURN_RET_LOG(sourceSA == nullptr, DCAMERA_BAD_VALUE, "sourceSA is null");
        ret = sourceSA->DCameraNotify(sinkDevId, dhId_, jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("SourceSA notify failed, srcId: %{public}s, sinkId: %{public}s, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(srcDevId_).c_str(), GetAnonyString(sinkDevId).c_str(),
                GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
    }

    DHLOGI("DCameraNotify %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("UpdateSettings dhId: %{public}s", GetAnonyString(dhId_).c_str());
    int32_t ret = operator_->UpdateSettings(settings);
    if (ret != DCAMERA_OK) {
        DHLOGE("UpdateSettings failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    DHLOGI("UpdateSettings %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::GetCameraInfo(std::shared_ptr<DCameraInfo>& camInfo)
{
    DHLOGI("GetCameraInfo dhId: %{public}s, session state: %{public}d", GetAnonyString(dhId_).c_str(), sessionState_);
    camInfo->state_ = sessionState_;
    return DCAMERA_OK;
}

bool DCameraSinkController::IsIdenticalAccount(const std::string &networkId)
{
    CHECK_AND_RETURN_RET_LOG(networkId.empty() || networkId.length() > MAX_ID_LENGTH, false, "networkId is invalid.");
    DmAuthForm authForm = DmAuthForm::INVALID_TYPE;
    std::vector<DmDeviceInfo> deviceList;
    DeviceManager::GetInstance().GetTrustedDeviceList(DCAMERA_PKG_NAME, "", deviceList);
    CHECK_AND_RETURN_RET_LOG(deviceList.size() == 0 || deviceList.size() > MAX_ONLINE_DEVICE_SIZE,
        false, "DeviceList size is invalid!");
    for (const auto &deviceInfo : deviceList) {
        if (std::string(deviceInfo.networkId) == networkId) {
            authForm = deviceInfo.authForm;
            break;
        }
    }
    CHECK_AND_RETURN_RET_LOG(authForm == DmAuthForm::IDENTICAL_ACCOUNT, true, "account check success.");
    return false;
}

int32_t DCameraSinkController::CheckSensitive()
{
    if (sinkCallback_ == nullptr) {
        DHLOGE("check sensitive callback is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = sinkCallback_->OnNotifyResourceInfo(ResourceEventType::EVENT_TYPE_QUERY_RESOURCE, PAGE_SUBTYPE,
        srcDevId_, isSensitive_, isSameAccount_);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "Query resource failed, ret: %{public}d", ret);
    DHLOGI("OpenChannel isSensitive: %{public}d, isSameAccout: %{public}d", isSensitive_, isSameAccount_);
    if (isSensitive_ && !isSameAccount_) {
        DHLOGE("Privacy resource must be logged in with the same account.");
        return DCAMERA_BAD_VALUE;
    }

    if (isCheckSecLevel_) {
        std::string sinkDevId;
        ret = GetLocalDeviceNetworkId(sinkDevId);
        CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "GetLocalDeviceNetworkId failed, ret: %{public}d", ret);
        if (isSensitive_ && !CheckDeviceSecurityLevel(srcDevId_, sinkDevId)) {
            DHLOGE("Check device security level failed!");
            return DCAMERA_BAD_VALUE;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkController::OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo)
{
    DHLOGI("DCameraSinkController OpenChannel Start, dhId: %{public}s", GetAnonyString(dhId_).c_str());
    ManageSelectChannel::GetInstance().SetSinkConnect(false);
    if (sessionState_ != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        DHLOGE("wrong state, dhId: %{public}s, sessionState: %{public}d", GetAnonyString(dhId_).c_str(), sessionState_);
        return DCAMERA_WRONG_STATE;
    }
    srcDevId_ = openInfo->sourceDevId_;
    std::vector<DCameraIndex> indexs;
    indexs.push_back(DCameraIndex(srcDevId_, dhId_));
    auto controller = std::shared_ptr<DCameraSinkController>(shared_from_this());
    std::shared_ptr<ICameraChannelListener> listener =
        std::make_shared<DCameraSinkControllerChannelListener>(controller);
    int32_t ret = channel_->CreateSession(indexs, SESSION_FLAG, DCAMERA_SESSION_MODE_CTRL, listener);
    if (ret != DCAMERA_OK) {
        DHLOGE("channel create session failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    DHLOGI("DCameraSinkController OpenChannel %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::PullUpPage()
{
    if (isSensitive_) {
        bool isSensitive = false;
        bool isSameAccout = false;
        int32_t ret = sinkCallback_->OnNotifyResourceInfo(ResourceEventType::EVENT_TYPE_PULL_UP_PAGE, PAGE_SUBTYPE,
            srcDevId_, isSensitive, isSameAccout);
        if (ret != DCAMERA_OK) {
            DHLOGE("pull up page failed, ret %{public}d", ret);
            return ret;
        }
        isPageStatus_.store(true);
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkController::CloseChannel()
{
    DHLOGI("DCameraSinkController CloseChannel Start, dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::lock_guard<std::mutex> autoLock(channelLock_);
    if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
        DCameraSinkServiceIpc::GetInstance().DeleteSourceRemoteCamSrv(srcDevId_);
        if (channel_ == nullptr) {
            DHLOGE("DCameraSinkController CloseChannel channel_ is nullptr");
            return DCAMERA_BAD_VALUE;
        }
        int32_t ret = channel_->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSinkController release channel failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }

        ret = output_->CloseChannel();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSinkController CloseChannel failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }
    }
    srcDevId_.clear();
    sessionState_ = DCAMERA_CHANNEL_STATE_DISCONNECTED;
    isPageStatus_.store(false);
    DHLOGI("DCameraSinkController CloseChannel %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::Init(std::vector<DCameraIndex>& indexs)
{
    DHLOGI("DCameraSinkController Init");
    dhId_ = indexs[0].dhId_;
    operator_ = std::make_shared<DCameraClient>(dhId_);
    output_ = std::make_shared<DCameraSinkOutput>(dhId_, operator_);
    int32_t ret = output_->Init();
    if (ret != DCAMERA_OK) {
        DHLOGE("output init failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    auto controller = std::shared_ptr<DCameraSinkController>(shared_from_this());
    std::shared_ptr<StateCallback> stateCallback = std::make_shared<DCameraSinkControllerStateCallback>(controller);
    operator_->SetStateCallback(stateCallback);
    ret = operator_->Init();
    if (ret != DCAMERA_OK) {
        DHLOGE("operator init failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    channel_ = std::make_shared<DCameraChannelSinkImpl>();
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    sinkCotrEventHandler_ =
        std::make_shared<DCameraSinkController::DCameraSinkContrEventHandler>(runner, shared_from_this());
    isInit_ = true;
    initCallback_ = std::make_shared<DeviceInitCallback>();
    ManageSelectChannel::GetInstance().SetSinkConnect(true);
    ret = CreateCtrlSession();
    if (ret != DCAMERA_OK) {
        DHLOGE("CreateCtrlSessiion init failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    std::shared_ptr<DCameraChannelInfo> info = std::make_shared<DCameraChannelInfo>();
    info->sourceDevId_ = "";
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    DCameraChannelDetail snapShotChInfo(SNAP_SHOT_SESSION_FLAG, SNAPSHOT_FRAME);
    info->detail_.push_back(continueChInfo);
    info->detail_.push_back(snapShotChInfo);
    ret = ChannelNeg(info);
    if (ret != DCAMERA_OK) {
        DHLOGE("ChannelNeg init failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    DHLOGI("DCameraSinkController Init %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::CreateCtrlSession()
{
    DHLOGI("DCameraSinkController CreateCtrlSessiion Start, dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (sessionState_ != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        DHLOGE("wrong state, dhId: %{public}s, sessionState: %{public}d", GetAnonyString(dhId_).c_str(), sessionState_);
        return DCAMERA_WRONG_STATE;
    }
    DCameraLowLatency::GetInstance().EnableLowLatency();
    std::vector<DCameraIndex> indexs;
    indexs.push_back(DCameraIndex("", dhId_));
    auto controller = std::shared_ptr<DCameraSinkController>(shared_from_this());
    std::shared_ptr<ICameraChannelListener> listener =
        std::make_shared<DCameraSinkControllerChannelListener>(controller);
    int32_t ret = channel_->CreateSession(indexs, SESSION_FLAG, DCAMERA_SESSION_MODE_CTRL, listener);
    if (ret != DCAMERA_OK) {
        DHLOGE("channel create session failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    DHLOGI("DCameraSinkController CreateCtrlSessiion %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::UnInit()
{
    DHLOGI("DCameraSinkController UnInit dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::lock_guard<std::mutex> autoLock(autoLock_);
    int32_t ret = StopCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSinkController UnInit %{public}s stop capture failed, ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
    }

    ret = CloseChannel();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSinkController UnInit %{public}s close channel failed, ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
    }

    DCameraLowLatency::GetInstance().DisableLowLatency();
    if (ManageSelectChannel::GetInstance().GetSinkConnect()) {
        if (channel_ != nullptr) {
            ret = channel_->ReleaseSession();
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraSinkController release channel failed, dhId: %{public}s, ret: %{public}d",
                    GetAnonyString(dhId_).c_str(), ret);
            }
        }
        if (output_ != nullptr) {
            ret = output_->CloseChannel();
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraSinkController output CloseChannel failed, dhId: %{public}s, ret: %{public}d",
                    GetAnonyString(dhId_).c_str(), ret);
            }
        }
    }

    if (output_ != nullptr) {
        ret = output_->UnInit();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSinkController release output failed, dhId: %{public}s, ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
        }
    }

    if (operator_ != nullptr) {
        ret = operator_->UnInit();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSinkController release operator failed, dhId: %{public}s, ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
        }
    }

    isInit_ = false;
    ManageSelectChannel::GetInstance().SetSinkConnect(false);
    DHLOGI("DCameraSinkController UnInit %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

DCameraSinkController::DCameraSinkContrEventHandler::DCameraSinkContrEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, std::shared_ptr<DCameraSinkController> sinkContrPtr)
    : AppExecFwk::EventHandler(runner), sinkContrWPtr_(sinkContrPtr)
{
    DHLOGI("Ctor DCameraSinkContrEventHandler.");
}

void DCameraSinkController::DCameraSinkContrEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    CHECK_AND_RETURN_LOG(event == nullptr, "event is nullptr.");
    uint32_t eventId = event->GetInnerEventId();
    auto sinkContr = sinkContrWPtr_.lock();
    if (sinkContr == nullptr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_FRAME_TRIGGER:
            sinkContr->ProcessFrameTrigger(event);
            break;
        case EVENT_AUTHORIZATION:
            sinkContr->ProcessPostAuthorization(event);
            break;
        case EVENT_ENCODER_PREPARED: {
            std::shared_ptr<DCameraSurfaceHolder> holder = event->GetSharedObject<DCameraSurfaceHolder>();
            if (holder != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(sinkContr->captureStateMutex_);
                    sinkContr->isEncoderReady_ = true;
                    sinkContr->encoderResult_ = holder->result;
                    sinkContr->preparedSurface_ = holder->surface;
                    sinkContr->CheckAndCommitCapture();
                }
            }
            break;
        }
        case EVENT_CAMERA_PREPARED: {
            int32_t result = event->GetParam();
            std::lock_guard<std::mutex> lock(sinkContr->captureStateMutex_);
            sinkContr->isCameraReady_ = true;
            sinkContr->cameraResult_ = result;
            sinkContr->CheckAndCommitCapture();
            break;
        }
        default:
            DHLOGE("event is undefined, id is %d", eventId);
            break;
    }
}

static bool WaitForAuthorization()
{
    std::string networkId = DCameraAccessConfigManager::GetInstance().GetCurrentNetworkId();
    if (!networkId.empty()) {
        if (!DCameraAccessConfigManager::GetInstance().HasAuthorizationDecision(networkId)) {
            int32_t timeOut = DCameraAccessConfigManager::GetInstance().GetAccessTimeOut();
            bool gotResult = DCameraAccessConfigManager::GetInstance().WaitForAuthorizationResult(networkId, timeOut);
            if (!gotResult) {
                DHLOGE("Authorization timeout, cannot start encoder");
                return false;
            }
        }

        if (!DCameraAccessConfigManager::GetInstance().IsAuthorizationGranted(networkId)) {
            DHLOGE("Authorization denied, cannot start encoder");
            return false;
        }
    }
    return true;
}

void DCameraSinkController::CheckAndCommitCapture()
{
    if (!isEncoderReady_ || !isCameraReady_) {
        DHLOGI("Waiting... EncoderReady: %d, CameraReady: %d", isEncoderReady_.load(), isCameraReady_.load());
        return;
    }

    bool result = WaitForAuthorization();
    if (!result) {
        captureState_ = CAPTURE_IDLE;
        captureStateCv_.notify_all();
        HandleCaptureError(DCAMERA_ALLOC_ERROR, "commit capture no permission.");
        return;
    }

    DHLOGI("Both tasks prepared. Checking results...");
    if (cameraResult_ != DCAMERA_OK || encoderResult_ != DCAMERA_OK) {
        DHLOGE("Preparation failed! Camera ret: %d, Encoder ret: %d", cameraResult_, encoderResult_);
        int32_t finalErrorCode = DCAMERA_OK;
        std::string errorMsg = "unknown preparation error";
        if (cameraResult_ != DCAMERA_OK) {
            finalErrorCode = cameraResult_;
            errorMsg = "camera preparation failed.";
        } else {
            finalErrorCode = encoderResult_;
            errorMsg = "output start capture failed";
        }
        captureState_ = CAPTURE_IDLE;
        captureStateCv_.notify_all();
        HandleCaptureError(finalErrorCode, errorMsg);
        return;
    }
    
    DHLOGI("All preparations successful. Committing capture...");
    int32_t ret = operator_->CommitCapture(preparedSurface_);
    if (ret != DCAMERA_OK) {
        DHLOGE("CommitCapture failed, ret: %d", ret);
        captureState_ = CAPTURE_IDLE;
        captureStateCv_.notify_all();
        HandleCaptureError(ret, "commit capture failed.");
        return;
    }
    captureState_ = CAPTURE_RUNNING;
    captureStateCv_.notify_all();
    
    DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_CAMERA_SUCCESS, START_CAPTURE_SUCC);
    DHLOGI("CheckAndCommitCapture successfully started capture.");
}

void DCameraSinkController::ProcessFrameTrigger(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGD("Receive frame trigger event then start process data in sink controller.");
    std::shared_ptr<std::string> param = event->GetSharedObject<std::string>();
    CHECK_AND_RETURN_LOG(param == nullptr, "ProcessFrameTrigger get param is null");
    accessControl_->TriggerFrame(*param);
}

void DCameraSinkController::ProcessPostAuthorization(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGD("Receive post authorization event then start process data in sink controller.");
    std::shared_ptr<std::vector<std::shared_ptr<DCameraCaptureInfo>>> captureInfos =
        event->GetSharedObject<std::vector<std::shared_ptr<DCameraCaptureInfo>>>();
    CHECK_AND_RETURN_LOG(captureInfos == nullptr, "ProcessPostAuthorization get captureInfos is null");
    PostAuthorization(*captureInfos);
}

void DCameraSinkController::OnStateChanged(std::shared_ptr<DCameraEvent>& event)
{
    DHLOGI("DCameraSinkController::OnStateChanged dhId: %{public}s, result: %{public}d",
           GetAnonyString(dhId_).c_str(), event->eventResult_);
    if (event->eventResult_ == DCAMERA_EVENT_DEVICE_PREEMPT) {
        DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_PREEMPT, std::string("camera device preempted"));
    } else {
        DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_ERROR, std::string("camera error"));
    }
}

void DCameraSinkController::OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("DCameraSinkController::OnMetadataResult dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (settings.empty()) {
        DHLOGE("camera settings is empty");
        return;
    }
    DCameraMetadataSettingCmd cmd;
    cmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
    cmd.dhId_ = dhId_;
    cmd.command_ = DCAMERA_PROTOCOL_CMD_METADATA_RESULT;
    cmd.value_.assign(settings.begin(), settings.end());
    std::string jsonStr;
    int32_t ret = cmd.Marshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("Marshal metadata settings failed, dhId: %{public}s ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
        return;
    }
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())),
        jsonStr.length());
    if (ret != EOK) {
        DHLOGE("memcpy_s failed, dhId: %{public}s ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return;
    }
    ret = channel_->SendData(buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("channel send data failed, dhId: %{public}s ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return;
    }
    DHLOGI("DCameraSinkController::OnMetadataResult dhId: %{public}s success", GetAnonyString(dhId_).c_str());
}

void DCameraSinkController::OnSessionState(int32_t state, std::string networkId)
{
    DHLOGI("DCameraSinkController::OnSessionState dhId: %{public}s, state: %{public}d",
        GetAnonyString(dhId_).c_str(), state);
    sessionState_ = state;
    std::string sinkDevId;
    int32_t val = GetLocalDeviceNetworkId(sinkDevId);
    CHECK_AND_LOG(val != DCAMERA_OK, "GetLocalDeviceNetworkId failed, val: %{public}d", val);
    switch (state) {
        case DCAMERA_CHANNEL_STATE_CONNECTING:
            DHLOGI("channel is connecting");
            break;
        case DCAMERA_CHANNEL_STATE_CONNECTED: {
            DHLOGI("channel is connected");
            if (sinkCallback_ != nullptr) {
                sinkCallback_->OnHardwareStateChanged(sinkDevId, dhId_, DcameraBusinessState::RUNNING);
            }
            if (!ManageSelectChannel::GetInstance().GetSinkConnect()) {
                break;
            }
            srcDevId_ = networkId;
            break;
        }
        case DCAMERA_CHANNEL_STATE_DISCONNECTED:
            DHLOGI("channel is disconnected");
            if (sinkCallback_ != nullptr) {
                sinkCallback_->OnHardwareStateChanged(sinkDevId, dhId_, DcameraBusinessState::IDLE);
            }
            ffrt::submit([this]() {
                DHLOGI("DCameraSinkController::OnSessionState %{public}s new thread session state: %{public}d",
                    GetAnonyString(dhId_).c_str(), sessionState_);
                prctl(PR_SET_NAME, CHANNEL_DISCONNECTED.c_str());
                std::lock_guard<std::mutex> autoLock(autoLock_);
                int32_t ret = CloseChannel();
                if (ret != DCAMERA_OK) {
                    DHLOGE("session state: %{public}d, %{public}s close channel failed, ret: %{public}d",
                        sessionState_, GetAnonyString(dhId_).c_str(), ret);
                }
                ret = StopCapture();
                if (ret != DCAMERA_OK) {
                    DHLOGE("session state: %{public}d, %{public}s stop capture failed, ret: %{public}d",
                        sessionState_, GetAnonyString(dhId_).c_str(), ret);
                }
            });
            break;
        default:
            DHLOGE("unknown session state");
            break;
    }
}

void DCameraSinkController::OnSessionError(int32_t eventType, int32_t eventReason, std::string detail)
{
    DHLOGI("DCameraSinkController::OnSessionError dhId: %{public}s, eventType: %{public}d, eventReason: %{public}d,"
        " detail: %{public}s", GetAnonyString(dhId_).c_str(), eventType, eventReason, detail.c_str());
}

void DCameraSinkController::OnDataReceived(std::vector<std::shared_ptr<DataBuffer>>& buffers)
{
    DHLOGI("OnReceivedData %{public}s control channel receive data", GetAnonyString(dhId_).c_str());
    for (auto& buffer : buffers) {
        if (buffer->Size() <= 0 || buffer->Size() > DATABUFF_MAX_SIZE) {
            DHLOGI("buffer is invalid");
            return;
        }
        HandleReceivedData(buffer);
    }
}

void DCameraSinkController::PostAuthorization(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    DHLOGI("DCameraSinkController::PostAuthorization dhId: %{public}s", GetAnonyString(dhId_).c_str());
    int32_t ret = StartCaptureInner(captureInfos);
    if (ret == DCAMERA_OK) {
        accessControl_->NotifySensitiveSrc(SRC_TYPE);
    }
}

int32_t DCameraSinkController::StartCaptureInner(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    DHLOGI("StartCaptureInner (EventHandler) dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::lock_guard<std::mutex> lock(captureStateMutex_);
    if (captureState_ == CAPTURE_STARTING) {
        DHLOGW("StartCaptureInner called while another start is in progress. Call ignored.");
        return DCAMERA_WRONG_STATE;
    }
    captureState_ = CAPTURE_STARTING;

    isEncoderReady_ = false;
    isCameraReady_ = false;
    encoderResult_ = DCAMERA_OK;
    cameraResult_ = DCAMERA_OK;
    preparedSurface_ = nullptr;
    captureInfosCache_ = captureInfos;
    ffrt::submit([this]() {
        DHLOGI("Output initialization task start.");
        int32_t ret = output_->StartCapture(captureInfosCache_);
        sptr<Surface> surface = nullptr;
        if (ret == DCAMERA_OK) {
            PropertyCarrier carrier;
            if (output_->GetProperty(SURFACE, carrier) == DCAMERA_OK) {
                surface = carrier.surface_;
            } else {
                ret = DCAMERA_BAD_VALUE;
            }
        }
        std::shared_ptr<DCameraSurfaceHolder> holder = std::make_shared<DCameraSurfaceHolder>(ret, surface);
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            DCameraSinkContrEventHandler::EVENT_ENCODER_PREPARED, holder);
        sinkCotrEventHandler_->SendEvent(event);
        }, {}, ffrt::task_attr().name("DCamSinkOutput").qos(ffrt::qos_user_initiated));

    ffrt::submit([this]() {
        DHLOGI("Operator preparation task start.");
        int32_t ret = operator_->PrepareCapture(captureInfosCache_, sceneMode_);
        AppExecFwk::InnerEvent::Pointer event = AppExecFwk::InnerEvent::Get(
            DCameraSinkContrEventHandler::EVENT_CAMERA_PREPARED, ret);
        sinkCotrEventHandler_->SendEvent(event);
        }, {}, ffrt::task_attr().name("DCamOpPrepare").qos(ffrt::qos_user_initiated));

    DHLOGI("StartCaptureInner has dispatched parallel tasks.");
    return DCAMERA_OK;
}

int32_t DCameraSinkController::DCameraNotifyInner(int32_t type, int32_t result, std::string reason)
{
    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = type;
    event->eventResult_ = result;
    event->eventContent_ = reason;
    return DCameraNotify(event);
}

int32_t DCameraSinkController::HandleReceivedData(std::shared_ptr<DataBuffer>& dataBuffer)
{
    DHLOGI("DCameraSinkController::HandleReceivedData dhId: %{public}s", GetAnonyString(dhId_).c_str());
    uint8_t *data = dataBuffer->Data();
    std::string jsonStr(reinterpret_cast<const char *>(data), dataBuffer->Capacity());
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *comvalue = cJSON_GetObjectItemCaseSensitive(rootValue, "Command");
    if (comvalue == nullptr || !cJSON_IsString(comvalue) || (comvalue->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        DHLOGE("parse command failed");
        return DCAMERA_BAD_VALUE;
    }
    std::string command = std::string(comvalue->valuestring);
    cJSON_Delete(rootValue);
    if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_CAPTURE) == 0)) {
        DCameraCaptureInfoCmd captureInfoCmd;
        int32_t ret = captureInfoCmd.Unmarshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("Capture Info Unmarshal failed, dhId: %{public}s ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
        sceneMode_ = captureInfoCmd.sceneMode_;
        userId_ = captureInfoCmd.userId_;
        tokenId_ = captureInfoCmd.tokenId_;
        accountId_ = captureInfoCmd.accountId_;
        if (!CheckAclRight()) {
            DHLOGE("ACL check failed.");
            return DCAMERA_BAD_VALUE;
        }
#ifdef DCAMERA_OPEN_STABILE
        CHECK_AND_RETURN_RET_LOG(!IsIdenticalAccount(srcDevId_), DCAMERA_BAD_VALUE, "Account check failed.");
#endif
        return StartCapture(captureInfoCmd.value_, sceneMode_);
    } else if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_UPDATE_METADATA) == 0)) {
        DCameraMetadataSettingCmd metadataSettingCmd;
        int32_t ret = metadataSettingCmd.Unmarshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("Metadata Setting Unmarshal failed, dhId: %{public}s ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
        return UpdateSettings(metadataSettingCmd.value_);
    } else if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_STOP_CAPTURE) == 0)) {
        return StopCapture();
    }
    return DCAMERA_BAD_VALUE;
}

bool DCameraSinkController::CheckAclRight()
{
    if (userId_ == -1) {
        DHLOGI("Acl check version compatibility processing.");
        return true;
    }
    std::string sinkDevId;
    int32_t ret = GetLocalDeviceNetworkId(sinkDevId);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false, "GetLocalDeviceNetworkId failed, ret: %{public}d", ret);
    int32_t userId = -1;
    std::string accountId = "";
#ifdef OS_ACCOUNT_ENABLE
    std::vector<int32_t> ids;
    ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK || ids.empty(), false,
        "Get userId from active os accountIds fail, ret: %{public}d", ret);
    userId = ids[0];

    AccountSA::OhosAccountInfo osAccountInfo;
    ret = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(osAccountInfo);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false,
        "Get accountId from ohos account info fail, ret: %{public}d", ret);
    accountId = osAccountInfo.uid_;
#endif
    ret = DeviceManager::GetInstance().InitDeviceManager(DCAMERA_PKG_NAME, initCallback_);
    if (ret != DCAMERA_OK) {
        DHLOGE("InitDeviceManager failed ret = %{public}d", ret);
        return false;
    }
    DmAccessCaller dmSrcCaller = {
        .accountId = accountId_,
        .pkgName = DCAMERA_PKG_NAME,
        .networkId = srcDevId_,
        .userId = userId_,
        .tokenId = tokenId_,
    };
    DmAccessCallee dmDstCallee = {
        .accountId = accountId,
        .networkId = sinkDevId,
        .pkgName = DCAMERA_PKG_NAME,
        .userId = userId,
        .tokenId = sinkTokenId_,
    };
    DHLOGI("CheckAclRight srcDevId: %{public}s, accountId: %{public}s, sinkDevId: %{public}s",
        GetAnonyString(srcDevId_).c_str(), GetAnonyString(accountId).c_str(), GetAnonyString(sinkDevId).c_str());
    return DeviceManager::GetInstance().CheckSinkAccessControl(dmSrcCaller, dmDstCallee);
}

int32_t DCameraSinkController::PauseDistributedHardware(const std::string &networkId)
{
    DHLOGI("Pause distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }
    if (operator_ == nullptr) {
        DHLOGE("operator_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = operator_->PauseCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("Pause distributed hardware failed, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
    }
    return ret;
}

int32_t DCameraSinkController::ResumeDistributedHardware(const std::string &networkId)
{
    DHLOGI("Resume distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }
    if (operator_ == nullptr) {
        DHLOGE("operator_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = operator_->ResumeCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("Resume distributed hardware failed, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
    }
    return ret;
}

int32_t DCameraSinkController::StopDistributedHardware(const std::string &networkId)
{
    DHLOGI("Stop distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }

    isPageStatus_.store(false);
    return DCameraNotifyInner(DCAMERA_SINK_STOP, DCAMERA_EVENT_SINK_STOP, std::string("sink stop dcamera business"));
}

bool DCameraSinkController::CheckDeviceSecurityLevel(const std::string &srcDeviceId, const std::string &dstDeviceId)
{
    DHLOGD("CheckDeviceSecurityLevel srcDeviceId %{public}s, dstDeviceId %{public}s.",
        GetAnonyString(srcDeviceId).c_str(), GetAnonyString(dstDeviceId).c_str());
    std::string srcUdid = GetUdidByNetworkId(srcDeviceId);
    if (srcUdid.empty()) {
        DHLOGE("src udid is empty");
        return false;
    }
    std::string dstUdid = GetUdidByNetworkId(dstDeviceId);
    if (dstUdid.empty()) {
        DHLOGE("dst udid is empty");
        return false;
    }
    DHLOGD("CheckDeviceSecurityLevel srcUdid %{public}s, dstUdid %{public}s.",
        GetAnonyString(srcUdid).c_str(), GetAnonyString(dstUdid).c_str());
    int32_t srcDeviceSecurityLevel = GetDeviceSecurityLevel(srcUdid);
    int32_t dstDeviceSecurityLevel = GetDeviceSecurityLevel(dstUdid);
    DHLOGI("srcDeviceSecurityLevel is %{public}d, dstDeviceSecurityLevel is %{public}d.",
        srcDeviceSecurityLevel, dstDeviceSecurityLevel);
    if (srcDeviceSecurityLevel == DEFAULT_DEVICE_SECURITY_LEVEL ||
        srcDeviceSecurityLevel < dstDeviceSecurityLevel) {
        DHLOGE("The device security of source device is lower.");
        return false;
    }
    return true;
}

int32_t DCameraSinkController::GetDeviceSecurityLevel(const std::string &udid)
{
    #ifdef DEVICE_SECURITY_LEVEL_ENABLE
    DeviceIdentify devIdentify;
    devIdentify.length = DEVICE_ID_MAX_LEN;
    int32_t ret = memcpy_s(devIdentify.identity, DEVICE_ID_MAX_LEN, udid.c_str(), DEVICE_ID_MAX_LEN);
    if (ret != 0) {
        DHLOGE("str copy failed %{public}d", ret);
        return DEFAULT_DEVICE_SECURITY_LEVEL;
    }
    DeviceSecurityInfo *info = nullptr;
    ret = RequestDeviceSecurityInfo(&devIdentify, nullptr, &info);
    if (ret != SUCCESS) {
        DHLOGE("Request device security info failed %{public}d", ret);
        FreeDeviceSecurityInfo(info);
        info = nullptr;
        return DEFAULT_DEVICE_SECURITY_LEVEL;
    }
    #endif
    int32_t level = 0;
    #ifdef DEVICE_SECURITY_LEVEL_ENABLE
    ret = GetDeviceSecurityLevelValue(info, &level);
    DHLOGD("Get device security level, level is %{public}d", level);
    FreeDeviceSecurityInfo(info);
    info = nullptr;
    if (ret != SUCCESS) {
        DHLOGE("Get device security level failed %{public}d", ret);
        return DEFAULT_DEVICE_SECURITY_LEVEL;
    }
    #endif
    return level;
}

std::string DCameraSinkController::GetUdidByNetworkId(const std::string &networkId)
{
    if (networkId.empty()) {
        DHLOGE("networkId is empty!");
        return "";
    }
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(DCAMERA_PKG_NAME, initCallback_);
    if (ret != DCAMERA_OK) {
        DHLOGE("InitDeviceManager failed ret = %{public}d", ret);
        return "";
    }
    std::string udid = "";
    ret = DeviceManager::GetInstance().GetUdidByNetworkId(DCAMERA_PKG_NAME, networkId, udid);
    if (ret != DCAMERA_OK) {
        DHLOGE("GetUdidByNetworkId failed ret = %{public}d", ret);
        return "";
    }
    return udid;
}

void DCameraSinkController::SetTokenId(uint64_t token)
{
    sinkTokenId_ = token;
}

void DeviceInitCallback::OnRemoteDied()
{
    DHLOGI("DeviceInitCallback OnRemoteDied");
}

void DCameraSinkController::HandleCaptureError(int32_t errorCode, const std::string& errorMsg)
{
    DHLOGE("Capture failed with error code: %{public}d, message: %{public}s", errorCode, errorMsg.c_str());
    switch (errorCode) {
        case DCAMERA_ALLOC_ERROR:
            DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_NO_PERMISSION,
                               std::string("operator start capture permission denial."));
            break;
        case DCAMERA_DEVICE_BUSY:
            DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_IN_USE,
                               std::string("operator start capture in used."));
            break;
        default:
            DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_ERROR, errorMsg);
            break;
    }
}

} // namespace DistributedHardware
} // namespace OHOS