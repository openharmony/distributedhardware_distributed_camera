/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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
#include "device_security_defines.h"
#include "device_security_info.h"
#include "idistributed_camera_source.h"
#include "ipc_skeleton.h"
#include "dcamera_low_latency.h"
#include <sys/prctl.h>

namespace OHOS {
namespace DistributedHardware {
const int DEFAULT_DEVICE_SECURITY_LEVEL = -1;
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

int32_t DCameraSinkController::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    DHLOGI("StartCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::string accessType = "";
    if ((accessControl_->IsSensitiveSrcAccess(SRC_TYPE)) &&
        (accessControl_->GetAccessControlType(accessType) == DCAMERA_SAME_ACCOUNT)) {
        int32_t ret = StartCaptureInner(captureInfos);
        if (ret == DCAMERA_OK) {
            accessControl_->NotifySensitiveSrc(SRC_TYPE);
        }
        return ret;
    } else {
        std::shared_ptr<std::string> param = std::make_shared<std::string>("");
        std::shared_ptr<std::vector<std::shared_ptr<DCameraCaptureInfo>>> infos =
            std::make_shared<std::vector<std::shared_ptr<DCameraCaptureInfo>>>(captureInfos);
        CHECK_NULL_RETURN(sinkCotrEventHandler_, DCAMERA_BAD_VALUE);
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
    std::lock_guard<std::mutex> autoLock(captureLock_);
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
    if (srcDevId_.empty()) {
        DHLOGE("source deviceId is empty");
        return DCAMERA_BAD_VALUE;
    }

    sptr<IDistributedCameraSource> sourceSA = DCameraSinkServiceIpc::GetInstance().GetSourceRemoteCamSrv(srcDevId_);
    if (sourceSA == nullptr) {
        DHLOGE("sourceSA is null");
        return DCAMERA_BAD_VALUE;
    }

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

    std::string sinkDevId;
    ret = GetLocalDeviceNetworkId(sinkDevId);
    if (ret != DCAMERA_OK) {
        DHLOGE("GetLocalDeviceNetworkId failed, devId: %{public}s, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(sinkDevId).c_str(), GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    ret = sourceSA->DCameraNotify(sinkDevId, dhId_, jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("SourceSA notify failed, srcId: %{public}s, sinkId: %{public}s, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(srcDevId_).c_str(), GetAnonyString(sinkDevId).c_str(),
               GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    DHLOGI("DCameraNotify %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkController::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("UpdateSettings dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (!CheckPermission()) {
        DHLOGE("DCameraSinkController UpdateSettings fail, CheckPermission fail");
        return DCAMERA_WRONG_STATE;
    }
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

int32_t DCameraSinkController::OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo)
{
    DHLOGI("DCameraSinkController OpenChannel Start, dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (!CheckPermission()) {
        DHLOGE("DCameraSinkController OpenChannel fail, CheckPermission fail");
        return DCAMERA_WRONG_STATE;
    }
    if (sessionState_ != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        DHLOGE("wrong state, dhId: %{public}s, sessionState: %{public}d", GetAnonyString(dhId_).c_str(), sessionState_);
        return DCAMERA_WRONG_STATE;
    }
    srcDevId_ = openInfo->sourceDevId_;
    int32_t ret = sinkCallback_->OnNotifyResourceInfo(ResourceEventType::EVENT_TYPE_QUERY_RESOURCE, PAGE_SUBTYPE,
        srcDevId_, isSensitive_, isSameAccount_);
    if (ret != DCAMERA_OK) {
        DHLOGE("Query resource failed, ret: %{public}d", ret);
        return ret;
    }
    DHLOGI("OpenChannel isSensitive: %{public}d, isSameAccout: %{public}d", isSensitive_, isSameAccount_);
    if (isSensitive_ && !isSameAccount_) {
        DHLOGE("Privacy resource must be logged in with the same account.");
        return DCAMERA_BAD_VALUE;
    }

    std::string sinkDevId;
    ret = GetLocalDeviceNetworkId(sinkDevId);
    if (ret != DCAMERA_OK) {
        DHLOGE("GetLocalDeviceNetworkId failed, ret: %{public}d", ret);
        return ret;
    }
    if (isSensitive_ && !CheckDeviceSecurityLevel(srcDevId_, sinkDevId)) {
        DHLOGE("Check device security level failed!");
        return DCAMERA_BAD_VALUE;
    }
    DCameraLowLatency::GetInstance().EnableLowLatency();
    std::vector<DCameraIndex> indexs;
    indexs.push_back(DCameraIndex(srcDevId_, dhId_));
    auto controller = std::shared_ptr<DCameraSinkController>(shared_from_this());
    std::shared_ptr<ICameraChannelListener> listener =
        std::make_shared<DCameraSinkControllerChannelListener>(controller);
    ret = channel_->CreateSession(indexs, SESSION_FLAG, DCAMERA_SESSION_MODE_CTRL, listener);
    if (ret != DCAMERA_OK) {
        DHLOGE("channel create session failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    ret = PullUpPage();
    if (ret != DCAMERA_OK) {
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
    DCameraLowLatency::GetInstance().DisableLowLatency();
    DCameraSinkServiceIpc::GetInstance().DeleteSourceRemoteCamSrv(srcDevId_);
    srcDevId_.clear();
    if (channel_ == nullptr) {
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
    sessionState_ = DCAMERA_CHANNEL_STATE_DISCONNECTED;
    if (isPageStatus_.load()) {
        bool isSensitive = false;
        bool isSameAccout = false;
        ret = sinkCallback_->OnNotifyResourceInfo(ResourceEventType::EVENT_TYPE_CLOSE_PAGE, PAGE_SUBTYPE, srcDevId_,
            isSensitive, isSameAccout);
        if (ret != DCAMERA_OK) {
            DHLOGE("close page failed, ret: %{public}d", ret);
        }
    }
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
    DHLOGI("DCameraSinkController Init %{public}s success", GetAnonyString(dhId_).c_str());
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
    CHECK_AND_RETURN_LOG(event, "event is nullptr.");
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
        default:
            DHLOGE("event is undefined, id is %d", eventId);
            break;
    }
}

void DCameraSinkController::ProcessFrameTrigger(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGD("Receive frame trigger event then start process data in sink controller.");
    std::shared_ptr<std::string> param = event->GetSharedObject<std::string>();
    accessControl_->TriggerFrame(*param);
}

void DCameraSinkController::ProcessPostAuthorization(const AppExecFwk::InnerEvent::Pointer &event)
{
    DHLOGD("Receive post authorization event then start process data in sink controller.");
    std::shared_ptr<std::vector<std::shared_ptr<DCameraCaptureInfo>>> captureInfos =
        event->GetSharedObject<std::vector<std::shared_ptr<DCameraCaptureInfo>>>();
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

void DCameraSinkController::OnSessionState(int32_t state)
{
    DHLOGI("DCameraSinkController::OnSessionState dhId: %{public}s, state: %{public}d",
        GetAnonyString(dhId_).c_str(), state);
    sessionState_ = state;
    switch (state) {
        case DCAMERA_CHANNEL_STATE_CONNECTING:
            DHLOGI("channel is connecting");
            break;
        case DCAMERA_CHANNEL_STATE_CONNECTED:
            DHLOGI("channel is connected");
            break;
        case DCAMERA_CHANNEL_STATE_DISCONNECTED:
            DHLOGI("channel is disconnected");
            std::thread([this]() {
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
            }).detach();
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
    DHLOGI("StartCaptureInner dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::lock_guard<std::mutex> autoLock(captureLock_);
    int32_t ret = output_->StartCapture(captureInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("output start capture failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_ERROR, std::string("output start capture failed"));
        return ret;
    }
    PropertyCarrier carrier;
    ret = output_->GetProperty(SURFACE, carrier);
    if (ret != DCAMERA_OK) {
        DHLOGD("StartCaptureInner: get property fail.");
        return DCAMERA_BAD_VALUE;
    }
    ret = operator_->StartCapture(captureInfos, carrier.surface_);
    if (ret != DCAMERA_OK) {
        DHLOGE("camera client start capture failed, dhId: %{public}s, ret: %{public}d",
            GetAnonyString(dhId_).c_str(), ret);
        if (ret == DCAMERA_ALLOC_ERROR) {
            DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_NO_PERMISSION,
                               std::string("operator start capture permission denial."));
        } else if (ret == DCAMERA_DEVICE_BUSY) {
            DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_DEVICE_IN_USE,
                               std::string("operator start capture in used."));
        }
        return ret;
    }

    DCameraNotifyInner(DCAMERA_MESSAGE, DCAMERA_EVENT_CAMERA_SUCCESS, std::string("operator start capture success"));
    DHLOGI("DCameraSinkController::StartCaptureInner %{public}s success", GetAnonyString(dhId_).c_str());
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
        int ret = captureInfoCmd.Unmarshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("Capture Info Unmarshal failed, dhId: %{public}s ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
        return StartCapture(captureInfoCmd.value_);
    } else if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_UPDATE_METADATA) == 0)) {
        DCameraMetadataSettingCmd metadataSettingCmd;
        int ret = metadataSettingCmd.Unmarshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("Metadata Setting Unmarshal failed, dhId: %{public}s ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
        return UpdateSettings(metadataSettingCmd.value_);
    }
    return DCAMERA_BAD_VALUE;
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
    int32_t level = 0;
    ret = GetDeviceSecurityLevelValue(info, &level);
    DHLOGD("Get device security level, level is %{public}d", level);
    FreeDeviceSecurityInfo(info);
    info = nullptr;
    if (ret != SUCCESS) {
        DHLOGE("Get device security level failed %{public}d", ret);
        return DEFAULT_DEVICE_SECURITY_LEVEL;
    }
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

bool DCameraSinkController::CheckPermission()
{
    DHLOGI("DCameraSinkController CheckPermission Start");
    auto uid = IPCSkeleton::GetCallingUid();
    return uid == DCAMERA_UID;
}

void DeviceInitCallback::OnRemoteDied()
{
    DHLOGI("DeviceInitCallback OnRemoteDied");
}
} // namespace DistributedHardware
} // namespace OHOS