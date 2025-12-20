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

#include "dcamera_source_dev.h"

#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "dcamera_hitrace_adapter.h"
#include "dcamera_radar.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

#include "dcamera_channel_info_cmd.h"
#include "dcamera_info_cmd.h"
#include "dcamera_provider_callback_impl.h"
#include "dcamera_source_controller.h"
#include "dcamera_source_input.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_allconnect_manager.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceDev::DCameraSourceDev(std::string devId, std::string dhId,
    std::shared_ptr<ICameraStateListener>& stateLisener) : devId_(devId), dhId_(dhId), stateListener_(stateLisener)
{
    DHLOGI("DCameraSourceDev Construct devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    memberFuncMap_[DCAMERA_EVENT_REGIST] = &DCameraSourceDev::NotifyRegisterResult;
    memberFuncMap_[DCAMERA_EVENT_UNREGIST] = &DCameraSourceDev::NotifyRegisterResult;
    memberFuncMap_[DCAMERA_EVENT_OPEN] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_CLOSE] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_CONFIG_STREAMS] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_RELEASE_STREAMS] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_START_CAPTURE] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_STOP_CAPTURE] = &DCameraSourceDev::NotifyHalResult;
    memberFuncMap_[DCAMERA_EVENT_UPDATE_SETTINGS] = &DCameraSourceDev::NotifyHalResult;

    eventResultMap_[DCAMERA_EVENT_OPEN] = DCAMERA_EVENT_OPEN_CHANNEL_ERROR;
    eventResultMap_[DCAMERA_EVENT_CLOSE] = DCAMERA_EVENT_CLOSE_CHANNEL_ERROR;
    eventResultMap_[DCAMERA_EVENT_CONFIG_STREAMS] = DCAMERA_EVENT_CONFIG_STREAMS_ERROR;
    eventResultMap_[DCAMERA_EVENT_RELEASE_STREAMS] = DCAMERA_EVENT_RELEASE_STREAMS_ERROR;
    eventResultMap_[DCAMERA_EVENT_START_CAPTURE] = DCAMERA_EVENT_START_CAPTURE_ERROR;
    eventResultMap_[DCAMERA_EVENT_STOP_CAPTURE] = DCAMERA_EVENT_STOP_CAPTURE_ERROR;
    eventResultMap_[DCAMERA_EVENT_UPDATE_SETTINGS] = DCAMERA_EVENT_UPDATE_SETTINGS_ERROR;
}

DCameraSourceDev::~DCameraSourceDev()
{
    DHLOGI("DCameraSourceDev Delete devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    srcDevEventHandler_ = nullptr;
    hdiCallback_ = nullptr;
    input_ = nullptr;
    controller_ = nullptr;
    stateMachine_ = nullptr;
    stateListener_ = nullptr;

    eventResultMap_.clear();
    memberFuncMap_.clear();
    DHLOGI("DCameraSourceDev Delete devId %{public}s dhId %{public}s end", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
}

int32_t DCameraSourceDev::InitDCameraSourceDev()
{
    DHLOGI("DCameraSourceDev InitDCameraSourceDev devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    std::shared_ptr<AppExecFwk::EventRunner> runner = AppExecFwk::EventRunner::Create(true);
    srcDevEventHandler_ = std::make_shared<DCameraSourceDev::DCameraSourceDevEventHandler>(
        runner, shared_from_this());
    auto cameraSourceDev = std::shared_ptr<DCameraSourceDev>(shared_from_this());
    stateMachine_ = std::make_shared<DCameraSourceStateMachine>(cameraSourceDev);
    stateMachine_->UpdateState(DCAMERA_STATE_INIT);
    controller_ = std::make_shared<DCameraSourceController>(devId_, dhId_, stateMachine_, cameraSourceDev);
    input_ = std::make_shared<DCameraSourceInput>(devId_, dhId_, cameraSourceDev);
    hdiCallback_ = sptr<DCameraProviderCallbackImpl>(
        new (std::nothrow) DCameraProviderCallbackImpl(devId_, dhId_, cameraSourceDev));
    if (hdiCallback_ == nullptr) {
        DHLOGE("DCameraSourceDev InitDCameraSourceDev failed, hdiCallback is null.");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("DCameraSourceDev InitDCameraSourceDev end devId %{public}s dhId %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
    const std::string& reqId, const EnableParam& param)
{
    DHLOGI("DCameraSourceDev PostTask RegisterDistributedHardware devId %{public}s dhId %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    version_ = param.sinkVersion;
    DCameraIndex index(devId, dhId);
    actualDevInfo_.insert(index);

    std::shared_ptr<DCameraRegistParam> regParam = std::make_shared<DCameraRegistParam>(devId, dhId, reqId,
        param.sinkAttrs, param.sourceAttrs);
    DCameraSourceEvent event(DCAMERA_EVENT_REGIST, regParam);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::UnRegisterDistributedHardware(const std::string devId, const std::string dhId,
    const std::string reqId)
{
    DHLOGI("DCameraSourceDev PostTask UnRegisterDistributedHardware devId %{public}s dhId %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    std::string sinkAttrs;
    std::string sourceAttrs;
    std::shared_ptr<DCameraRegistParam> regParam = std::make_shared<DCameraRegistParam>(devId, dhId, reqId, sinkAttrs,
        sourceAttrs);
    DCameraSourceEvent event(DCAMERA_EVENT_UNREGIST, regParam);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::DCameraNotify(std::string& eventStr)
{
    DHLOGI("DCameraSourceDev PostTask DCameraNotify devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraEventCmd cmd;
    int32_t ret = cmd.Unmarshal(eventStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev DCameraNotify devId %{public}s dhId %{public}s marshal failed, ret: %{public}d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    DCameraSourceEvent event(DCAMERA_EVENT_NOFIFY, cmd.value_);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::OpenSession(DCameraIndex& camIndex)
{
    DHLOGI("DCameraSourceDev PostTask OpenSession devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_OPEN, camIndex);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::CloseSession(DCameraIndex& camIndex)
{
    DHLOGI("DCameraSourceDev PostTask CloseSession devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_CLOSE, camIndex);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ConfigCameraStreams(const std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
{
    DHLOGI("DCameraSourceDev PostTask ConfigStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_CONFIG_STREAMS, streamInfos);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    CHECK_AND_RETURN_RET_LOG(stateListener_ == nullptr, DCAMERA_BAD_VALUE, "stateListener_ is nullptr.");
    stateListener_->OnHardwareStateChanged(devId_, dhId_, DcameraBusinessState::RUNNING);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ReleaseCameraStreams(const std::vector<int>& streamIds)
{
    DHLOGI("DCameraSourceDev PostTask ReleaseStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_RELEASE_STREAMS, streamIds);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::StartCameraCapture(const std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
{
    DHLOGI("DCameraSourceDev PostTask StartCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_START_CAPTURE, captureInfos);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::StopCameraCapture(const std::vector<int>& streamIds)
{
    DHLOGI("DCameraSourceDev PostTask StopCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_STOP_CAPTURE, streamIds);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::UpdateCameraSettings(const std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("DCameraSourceDev PostTask UpdateCameraSettings devId %{public}s dhId %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    DCameraSourceEvent event(DCAMERA_EVENT_UPDATE_SETTINGS, settings);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ProcessHDFEvent(const DCameraHDFEvent& event)
{
    DHLOGI("DCameraSourceDev ProcessHDFEvent devId %{public}s dhId %{public}s event_type %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), event.type_);
    if (event.type_ == EVENT_DCAMERA_FORCE_SWITCH) {
        DCameraSystemSwitchInfo::GetInstance().SetSystemSwitchFlagAndRotation(devId_, true, event.result_);
        DHLOGI("recv force switch event from hdf, result: %{public}d", event.result_);
        return DCAMERA_OK;
    }
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(DCAMERA_EVENT_GET_FULLCAPS);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_PROCESS_HDF_NOTIFY, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::GetFullCaps()
{
    DHLOGI("DCameraSourceDev GetFullCaps enter.");
    if (stateListener_ == nullptr) {
        DHLOGE("DCameraSourceDev DoSyncTrigger, stateListener_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    stateListener_->OnDataSyncTrigger(devId_);
    return DCAMERA_OK;
}

void DCameraSourceDev::DoHicollieProcess()
{
    SetHicollieFlag(true);
}

void DCameraSourceDev::DoProcessData(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<DCameraSourceEvent> eventParam = event->GetSharedObject<DCameraSourceEvent>();
    CHECK_AND_RETURN_LOG(eventParam == nullptr, "eventParam is nullptr.");
    CHECK_AND_RETURN_LOG(stateMachine_ == nullptr, "stateMachine_ is nullptr.");
    int32_t ret = stateMachine_->Execute((*eventParam).GetEventType(), (*eventParam));
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute failed, ret: %{public}d, devId: %{public}s dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    NotifyResult((*eventParam).GetEventType(), (*eventParam), ret);
}

void DCameraSourceDev::DoProcesHDFEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    std::shared_ptr<DCameraSourceEvent> eventParam = event->GetSharedObject<DCameraSourceEvent>();
    CHECK_AND_RETURN_LOG(eventParam == nullptr, "eventParam is nullptr.");
    CHECK_AND_RETURN_LOG(stateMachine_ == nullptr, "stateMachine_ is nullptr.");
    int32_t ret = stateMachine_->Execute((*eventParam).GetEventType(), (*eventParam));
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute failed, ret: %{public}d, devId: %{public}s dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
}

DCameraSourceDev::DCameraSourceDevEventHandler::DCameraSourceDevEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, std::shared_ptr<DCameraSourceDev> srcDevPtr)
    : AppExecFwk::EventHandler(runner), srcDevPtrWPtr_(srcDevPtr)
{
    DHLOGI("Ctor DCameraSourceDevEventHandler.");
}

void DCameraSourceDev::DCameraSourceDevEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    CHECK_AND_RETURN_LOG(event == nullptr, "event is nullptr.");
    uint32_t eventId = event->GetInnerEventId();
    auto srcDevPtr = srcDevPtrWPtr_.lock();
    if (srcDevPtr == nullptr) {
        DHLOGE("Can not get strong self ptr");
        return;
    }
    switch (eventId) {
        case EVENT_HICOLLIE:
            srcDevPtr->DoHicollieProcess();
            break;
        case EVENT_SOURCE_DEV_PROCESS:
            srcDevPtr->DoProcessData(event);
            break;
        case EVENT_PROCESS_HDF_NOTIFY:
            srcDevPtr->DoProcesHDFEvent(event);
            break;
        default:
            DHLOGE("event is undefined, id is %d", eventId);
            break;
    }
}

int32_t DCameraSourceDev::Register(std::shared_ptr<DCameraRegistParam>& param)
{
    DCAMERA_SYNC_TRACE(DCAMERA_REGISTER_CAMERA);
    DHLOGI("DCameraSourceDev Execute Register devId: %{public}s dhId: %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    ReportRegisterCameraEvent(REGIST_CAMERA_EVENT, GetAnonyString(devId_), GetAnonyString(dhId_),
        version_, "execute register event.");
    std::vector<DCameraIndex> actualDevInfo;
    actualDevInfo.assign(actualDevInfo_.begin(), actualDevInfo_.end());
    int32_t ret = controller_->Init(actualDevInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute Register controller init failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    ret = input_->Init();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute Register input init failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        controller_->UnInit();
        return ret;
    }

    sptr<IDCameraProvider> camHdiProvider = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider == nullptr) {
        DHLOGI("Register camHdiProvider is nullptr devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str());
        controller_->UnInit();
        input_->UnInit();
        return DCAMERA_BAD_OPERATE;
    }
    DHBase dhBase;
    dhBase.deviceId_ = param->devId_;
    dhBase.dhId_ = param->dhId_;
    std::string ability;
    ret = ParseEnableParam(param, ability);
    if (ret != DCAMERA_OK) {
        DHLOGE("Parsing param failed.");
    }
    int32_t retHdi = camHdiProvider->EnableDCameraDevice(dhBase, ability, hdiCallback_);
    DHLOGI("DCameraSourceDev Execute Register register hal, ret: %{public}d, devId: %{public}s dhId: %{public}s",
        retHdi, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    if (retHdi != SUCCESS) {
        controller_->UnInit();
        input_->UnInit();
        return DCAMERA_REGIST_HAL_FAILED;
    }
    if (version_ >= SEPARATE_SINK_VERSION) {
        ManageSelectChannel::GetInstance().SetSrcConnect(true);
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ParseEnableParam(std::shared_ptr<DCameraRegistParam>& param, std::string& ability)
{
    cJSON *sinkRootValue = cJSON_Parse(param->sinkParam_.c_str());
    if (sinkRootValue == nullptr) {
        DHLOGE("Input sink ablity info is not json object.");
        return DCAMERA_INIT_ERR;
    }

    cJSON *srcRootValue = cJSON_Parse(param->srcParam_.c_str());
    if (srcRootValue == nullptr) {
        DHLOGE("Input source ablity info is not json object.");
        cJSON_Delete(sinkRootValue);
        return DCAMERA_INIT_ERR;
    }

    cJSON *abilityRootValue = cJSON_CreateObject();
    if (abilityRootValue == nullptr) {
        cJSON_Delete(sinkRootValue);
        cJSON_Delete(srcRootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddItemToObject(abilityRootValue, "SinkAbility", sinkRootValue);
    cJSON_AddItemToObject(abilityRootValue, "SourceCodec", srcRootValue);
    char *jsonstr = cJSON_Print(abilityRootValue);
    if (jsonstr == nullptr) {
        cJSON_Delete(abilityRootValue);
        return DCAMERA_BAD_VALUE;
    }
    ability = jsonstr;
    cJSON_Delete(abilityRootValue);
    cJSON_free(jsonstr);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::UnRegister(std::shared_ptr<DCameraRegistParam>& param)
{
    DCAMERA_SYNC_TRACE(DCAMERA_UNREGISTER_CAMERA);
    DHLOGI("DCameraSourceDev Execute UnRegister devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    ReportRegisterCameraEvent(UNREGIST_CAMERA_EVENT, GetAnonyString(devId_), GetAnonyString(dhId_),
        version_, "execute unregister event.");
    int32_t ret = controller_->UnInit();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute UnRegister controller uninit failed, ret: %{public}d, devId: %{public}s "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }

    ret = input_->UnInit();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute UnRegister input uninit failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }

    sptr<IDCameraProvider> camHdiProvider = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider == nullptr) {
        DHLOGI("UnRegister camHdiProvider is nullptr devId: %{public}s dhId: %{public}s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return DCAMERA_BAD_OPERATE;
    }

    DHBase dhBase;
    dhBase.deviceId_ = param->devId_;
    dhBase.dhId_ = param->dhId_;
    int32_t retHdi = camHdiProvider->DisableDCameraDevice(dhBase);
    DHLOGI("DCameraSourceDev Execute UnRegister unregister hal, ret: %{public}d, devId: %{public}s dhId: %{public}s",
        retHdi, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    if (retHdi != SUCCESS) {
        return DCAMERA_UNREGIST_HAL_FAILED;
    }
    ManageSelectChannel::GetInstance().SetSrcConnect(false);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::OpenCamera()
{
    DHLOGI("DCameraSourceDev Execute OpenCamera devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    ReportCameraOperaterEvent(OPEN_CAMERA_EVENT, GetAnonyString(devId_), dhId_, "execute open camera event.");
    controller_->SetTokenId(tokenId_);
    std::shared_ptr<DCameraOpenInfo> openInfo = std::make_shared<DCameraOpenInfo>();
    int32_t ret = GetLocalDeviceNetworkId(openInfo->sourceDevId_);
    DcameraRadar::GetInstance().ReportDcameraOpen("GetLocalDeviceNetworkId", CameraOpen::OPEN_CAMERA,
        BizState::BIZ_STATE_START, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev getMyId failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    if (DCameraAllConnectManager::IsInited()) {
        auto resourceReq = DCameraAllConnectManager::GetInstance().BuildResourceRequest();
        ret = DCameraAllConnectManager::GetInstance().ApplyAdvancedResource(devId_, resourceReq.get());
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect apply advanced resource failed, ret: %{public}d, devId: %{public}s",
                ret, GetAnonyString(devId_).c_str());
            return ret;
        }
        ret = DCameraAllConnectManager::GetInstance().PublishServiceState(devId_, dhId_, SCM_PREPARE);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect publish scm prepare failed, ret: %{public}d, devId: %{public}s",
                ret, GetAnonyString(devId_).c_str());
            return ret;
        }
        DHLOGI("DCamera allconnect apply advanced resource and publish scm prepare success");
    }

    DcameraStartAsyncTrace(DCAMERA_OPEN_CHANNEL_CONTROL, DCAMERA_OPEN_CHANNEL_TASKID);
    ret = controller_->OpenChannel(openInfo);
    DcameraRadar::GetInstance().ReportDcameraOpenProgress("OpenChannel", CameraOpen::OPEN_CHANNEL, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute OpenCamera OpenChannel failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        DcameraFinishAsyncTrace(DCAMERA_OPEN_CHANNEL_CONTROL, DCAMERA_OPEN_CHANNEL_TASKID);
        return DCAMERA_OPEN_CONFLICT;
    }

    CHECK_AND_RETURN_RET_LOG(stateListener_ == nullptr, DCAMERA_BAD_VALUE, "stateListener_ is nullptr.");
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::CloseCamera()
{
    DHLOGI("DCameraSourceDev Execute CloseCamera devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    ReportCameraOperaterEvent(CLOSE_CAMERA_EVENT, GetAnonyString(devId_), dhId_, "execute close camera event.");
    int32_t ret = input_->CloseChannel();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute CloseCamera input CloseChannel failed, ret: %{public}d, devId: %{public}s "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    ret = controller_->CloseChannel();
    DcameraRadar::GetInstance().ReportDcameraClose("CloseChannel", CameraClose::CLOSE_CAMERA,
        BizState::BIZ_STATE_END, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute CloseCamera controller CloseChannel failed, ret: %{public}d, devId: "
            "%{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    if (DCameraAllConnectManager::IsInited()) {
        ret = DCameraAllConnectManager::GetInstance().PublishServiceState(devId_, dhId_, SCM_IDLE);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCamera allconnect CloseCamera PublishServiceState failed, ret: %{public}d, devId: %{public}s ",
                ret, GetAnonyString(devId_).c_str());
        }
    }
    CHECK_AND_RETURN_RET_LOG(stateListener_ == nullptr, DCAMERA_BAD_VALUE, "stateListener_ is nullptr.");
    stateListener_->OnHardwareStateChanged(devId_, dhId_, DcameraBusinessState::IDLE);
    DCameraSystemSwitchInfo::GetInstance().SetSystemSwitchFlagAndRotation(devId_, false, 0);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
{
    DHLOGI("DCameraSourceDev Execute ConfigStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    for (auto info : streamInfos) {
        sceneMode_ = info->mode_;
        break;
    }
    int32_t ret = input_->ConfigStreams(streamInfos);
    DcameraRadar::GetInstance().ReportDcameraOpenProgress("ConfigStreams", CameraOpen::CONFIG_STREAMS, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute ConfigStreams ConfigStreams failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::shared_ptr<DCameraChannelInfo> chanInfo = std::make_shared<DCameraChannelInfo>();
    ret = GetLocalDeviceNetworkId(chanInfo->sourceDevId_);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev getLocalNetworkId failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    DCameraChannelDetail continueChInfo(CONTINUE_SESSION_FLAG, CONTINUOUS_FRAME);
    DCameraChannelDetail snapShotChInfo(SNAP_SHOT_SESSION_FLAG, SNAPSHOT_FRAME);
    chanInfo->detail_.push_back(continueChInfo);
    chanInfo->detail_.push_back(snapShotChInfo);

    ret = controller_->ChannelNeg(chanInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev ChannelNeg failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<DCameraIndex> actualDevInfo;
    actualDevInfo.assign(actualDevInfo_.begin(), actualDevInfo_.end());
    ret = input_->OpenChannel(actualDevInfo);
    DcameraRadar::GetInstance().ReportDcameraOpenProgress("intput->OpenChannel", CameraOpen::OPEN_DATA_CHANNEL, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev ChannelNeg OpenChannel failed ret: %{public}d, devId: %{public}s, dhId: %{public}s",
            ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease)
{
    DHLOGI("DCameraSourceDev Execute ReleaseStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = input_->ReleaseStreams(streamIds, isAllRelease);
    DcameraRadar::GetInstance().ReportDcameraCloseProgress("ReleaseStreams", CameraClose::RELEASE_STREAM, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute ReleaseStreams failed ret: %{public}d, devId: %{public}s, dhId: %{public}s",
            ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::ReleaseAllStreams()
{
    DHLOGI("DCameraSourceDev Execute ReleaseAllStreams devId %{public}s dhId %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    int32_t ret = input_->ReleaseAllStreams();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute ReleaseAllStreams failed ret: %{public}d, devId: %{public}s, "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
{
    HitraceAndHisyseventImpl(captureInfos);
    DHLOGI("DCameraSourceDev Execute StartCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = input_->StartCapture(captureInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev input StartCapture failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        DcameraFinishAsyncTrace(DCAMERA_CONTINUE_FIRST_FRAME, DCAMERA_CONTINUE_FIRST_FRAME_TASKID);
        DcameraFinishAsyncTrace(DCAMERA_SNAPSHOT_FIRST_FRAME, DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID);
        return ret;
    }
    std::vector<std::shared_ptr<DCameraCaptureInfo>> captures;
    for (auto iter = captureInfos.begin(); iter != captureInfos.end(); iter++) {
        if ((*iter) == nullptr) {
            DHLOGE("DCameraSourceDev StartCapture captureInfos iter is nullptr");
            continue;
        }
        std::shared_ptr<DCameraCaptureInfo> capture = std::make_shared<DCameraCaptureInfo>();
        capture->width_ = (*iter)->width_;
        capture->height_ = (*iter)->height_;
        capture->format_ = (*iter)->format_;
        capture->dataspace_ = (*iter)->dataspace_;
        capture->isCapture_ = (*iter)->isCapture_;
        capture->encodeType_ = (*iter)->encodeType_;
        capture->streamType_ = (*iter)->type_;
        DHLOGI("StartCapture devId %{public}s dhId %{public}s settings size: %{public}zu w: %{public}d h: %{public}d "
            "fmt: %{public}d isC: %{public}d enc: %{public}d streamT: %{public}d", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), (*iter)->captureSettings_.size(), capture->width_, capture->height_,
            capture->format_, capture->isCapture_ ? 1 : 0, capture->encodeType_, capture->streamType_);
        for (auto settingIter = (*iter)->captureSettings_.begin(); settingIter != (*iter)->captureSettings_.end();
            settingIter++) {
            std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
            setting->type_ = settingIter->type_;
            setting->value_ = settingIter->value_;
            capture->captureSettings_.push_back(setting);
        }
        captures.push_back(capture);
    }

    DHLOGI("startcapture sceneMode_: %{public}d", sceneMode_);
    ret = controller_->StartCapture(captures, sceneMode_);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute StartCapture StartCapture failed, ret: %{public}d, devId: %{public}s dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        DcameraFinishAsyncTrace(DCAMERA_CONTINUE_FIRST_FRAME, DCAMERA_CONTINUE_FIRST_FRAME_TASKID);
        DcameraFinishAsyncTrace(DCAMERA_SNAPSHOT_FIRST_FRAME, DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID);
    }
    return ret;
}

void DCameraSourceDev::HitraceAndHisyseventImpl(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
{
    for (auto iter = captureInfos.begin(); iter != captureInfos.end(); iter++) {
        std::shared_ptr<DCCaptureInfo> capture = *iter;
        EventCaptureInfo eventCaptureInfo = {
            .width_ = capture->width_,
            .height_ = capture->height_,
            .format_ = capture->format_,
            .isCapture_ = capture->isCapture_,
            .encodeType_ = capture->encodeType_,
            .type_ = capture->type_,
        };
        ReportStartCaptureEvent(START_CAPTURE_EVENT, eventCaptureInfo, "execute start capture event.");
        if (capture->type_ == CONTINUOUS_FRAME && capture->isCapture_ == true) {
            DcameraStartAsyncTrace(DCAMERA_CONTINUE_FIRST_FRAME, DCAMERA_CONTINUE_FIRST_FRAME_TASKID);
        } else if (capture->type_ == SNAPSHOT_FRAME && capture->isCapture_ == true) {
            DcameraStartAsyncTrace(DCAMERA_SNAPSHOT_FIRST_FRAME, DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID);
        }
    }
}

int32_t DCameraSourceDev::StopCapture(std::vector<int>& streamIds, bool& isAllStop)
{
    DHLOGI("DCameraSourceDev Execute StopCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = input_->StopCapture(streamIds, isAllStop);
    DcameraRadar::GetInstance().ReportDcameraClose("StopCapture", CameraClose::STOP_CAPTURE,
        BizState::BIZ_STATE_START, ret);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute StopCapture input StopCapture failed, ret: %{public}d, devId: %{public}s "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    if (isAllStop) {
        ret = controller_->StopCapture();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceDev Execute StopCapture controller StopCapture failed, ret: %{public}d, devId: "
                "%{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::StopAllCapture()
{
    DHLOGI("DCameraSourceDev Execute StopAllCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = input_->StopAllCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute StopAllCapture input StopAllCapture failed, ret: %{public}d, devId: "
            "%{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    ret = controller_->StopCapture();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute StopAllCapture controller StopAllCapture failed, ret: %{public}d, devId: "
            "%{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("DCameraSourceDev Execute UpdateSettings devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = input_->UpdateSettings(settings);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute UpdateSettings input UpdateSettings failed, ret: %{public}d, devId: "
            "%{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    ret = controller_->UpdateSettings(settings);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute UpdateSettings controller UpdateSettings failed, ret: %{public}d, "
            "devId: %{public}s dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::CameraEventNotify(std::shared_ptr<DCameraEvent>& events)
{
    DHLOGI("DCameraSourceDev Execute CameraEventNotify devId %{public}s dhId %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    int32_t ret = controller_->DCameraNotify(events);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceDev Execute CameraEventNotify DCameraNotify failed, ret: %{public}d, devId: %{public}s "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

void DCameraSourceDev::NotifyResult(DCAMERA_EVENT eventType, DCameraSourceEvent& event, int32_t result)
{
    DHLOGI("Notify func map, execute %{public}d notify", eventType);
    switch (eventType) {
        case DCAMERA_EVENT_REGIST:
        case DCAMERA_EVENT_UNREGIST:
            NotifyRegisterResult(eventType, event, result);
            break;
        case DCAMERA_EVENT_OPEN:
        case DCAMERA_EVENT_CLOSE:
        case DCAMERA_EVENT_CONFIG_STREAMS:
        case DCAMERA_EVENT_RELEASE_STREAMS:
        case DCAMERA_EVENT_START_CAPTURE:
        case DCAMERA_EVENT_STOP_CAPTURE:
        case DCAMERA_EVENT_UPDATE_SETTINGS:
            NotifyHalResult(eventType, event, result);
            break;
        default:
            DHLOGE("Notify func map not find, execute %{public}d notify, devId: %{public}s dhId: %{public}s", eventType,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
    }
}

void DCameraSourceDev::NotifyRegisterResult(DCAMERA_EVENT eventType, DCameraSourceEvent& event, int32_t result)
{
    std::string data = "";
    std::shared_ptr<DCameraRegistParam> param;
    int32_t ret = event.GetDCameraRegistParam(param);
    if (ret != DCAMERA_OK) {
        return;
    }

    if (stateListener_ == nullptr) {
        DHLOGE("DCameraSourceDev can not get listener");
        return;
    }
    if (eventType == DCAMERA_EVENT_REGIST) {
        stateListener_->OnRegisterNotify(param->devId_, param->dhId_, param->reqId_, result, data);
    } else {
        stateListener_->OnUnregisterNotify(param->devId_, param->dhId_, param->reqId_, result, data);
    }
}

void DCameraSourceDev::NotifyHalResult(DCAMERA_EVENT eventType, DCameraSourceEvent& event, int32_t result)
{
    DHLOGI("DCameraSourceDev NotifyHalResult eventType: %{public}d, result: %{public}d devId: %{public}s dhId: "
        "%{public}s", eventType, result, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    if (result == DCAMERA_OK) {
        return;
    }

    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = DCAMERA_OPERATION;
    auto iter = eventResultMap_.find(eventType);
    if (iter == eventResultMap_.end()) {
        return;
    }
    events->eventResult_ = iter->second;
    CameraEventNotify(events);
    return;
}

int32_t DCameraSourceDev::GetStateInfo()
{
    CHECK_AND_RETURN_RET_LOG(stateMachine_ == nullptr, DCAMERA_BAD_VALUE, "stateMachine_ is nullptr.");
    DHLOGI("GetStateInfo In state %{public}d", stateMachine_->GetCameraState());
    return stateMachine_->GetCameraState();
}

std::string DCameraSourceDev::GetVersion()
{
    return version_;
}

int32_t DCameraSourceDev::OnChannelConnectedEvent()
{
    std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
    camEvent->eventType_ = DCAMERA_MESSAGE;
    camEvent->eventResult_ = DCAMERA_EVENT_CHANNEL_CONNECTED;
    DCameraSourceEvent event(DCAMERA_EVENT_NOFIFY, camEvent);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::OnChannelDisconnectedEvent()
{
    DCameraIndex camIndex(devId_, dhId_);
    std::shared_ptr<DCameraIndex> index = std::make_shared<DCameraIndex>(camIndex);
    DCameraSourceEvent event(DCAMERA_EVENT_CLOSE, camIndex);
    std::shared_ptr<DCameraSourceEvent> eventParam = std::make_shared<DCameraSourceEvent>(event);
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent =
        AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
    camEvent->eventType_ = DCAMERA_MESSAGE;
    camEvent->eventResult_ = DCAMERA_EVENT_CHANNEL_DISCONNECTED;
    DCameraSourceEvent eventNotify(DCAMERA_EVENT_NOFIFY, camEvent);
    eventParam = std::make_shared<DCameraSourceEvent>(eventNotify);
    msgEvent = AppExecFwk::InnerEvent::Get(EVENT_SOURCE_DEV_PROCESS, eventParam, 0);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

int32_t DCameraSourceDev::PostHicollieEvent()
{
    CHECK_AND_RETURN_RET_LOG(srcDevEventHandler_ == nullptr, DCAMERA_BAD_VALUE, "srcDevEventHandler_ is nullptr.");
    AppExecFwk::InnerEvent::Pointer msgEvent = AppExecFwk::InnerEvent::Get(EVENT_HICOLLIE);
    srcDevEventHandler_->SendEvent(msgEvent, 0, AppExecFwk::EventQueue::Priority::IMMEDIATE);
    return DCAMERA_OK;
}

void DCameraSourceDev::SetHicollieFlag(bool flag)
{
    hicollieFlag_.store(flag);
}

bool DCameraSourceDev::GetHicollieFlag()
{
    return hicollieFlag_.load();
}

void DCameraSourceDev::SetTokenId(uint64_t token)
{
    tokenId_ = token;
}

int32_t DCameraSourceDev::UpdateDCameraWorkMode(const WorkModeParam& param)
{
    CHECK_AND_RETURN_RET_LOG(input_ == nullptr, DCAMERA_BAD_VALUE, "input_ is nullptr");
    int32_t ret = input_->UpdateWorkMode(param);
    DHLOGI("update dcamera workmode by input_ done, ret %{public}d", ret);
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
