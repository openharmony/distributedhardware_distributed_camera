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

#include "dcamera_source_controller.h"

#include <securec.h>
#include <cstdlib>
#include "iservice_registry.h"
#include "iservmgr_hdi.h"
#include "iproxy_broker.h"

#include "anonymous_string.h"
#include "dcamera_capture_info_cmd.h"
#include "dcamera_channel_source_impl.h"
#include "dcamera_hitrace_adapter.h"
#include "dcamera_metadata_setting_cmd.h"
#include "dcamera_protocol.h"
#include "dcamera_radar.h"
#include "dcamera_softbus_latency.h"
#include "dcamera_source_controller_channel_listener.h"
#include "dcamera_source_service_ipc.h"
#include "dcamera_utils_tools.h"
#include "dcamera_hisysevent_adapter.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "idistributed_camera_sink.h"
#include "dcamera_low_latency.h"
#ifdef OS_ACCOUNT_ENABLE
#include "ohos_account_kits.h"
#include "os_account_manager.h"
#endif

namespace OHOS {
namespace DistributedHardware {
DCameraSourceController::DCameraSourceController(std::string devId, std::string dhId,
    std::shared_ptr<DCameraSourceStateMachine>& stateMachine, std::shared_ptr<DCameraSourceDev>& camDev)
    : devId_(devId), dhId_(dhId), stateMachine_(stateMachine), camDev_(camDev),
    channelState_(DCAMERA_CHANNEL_STATE_DISCONNECTED)
{
    DHLOGI("DCameraSourceController create devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    isInit = false;
    cameraHdiRecipient_ = sptr<DCameraHdiRecipient>(new DCameraHdiRecipient());
}

DCameraSourceController::~DCameraSourceController()
{
    DHLOGI("DCameraSourceController delete devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (isInit) {
        UnInit();
    }
    camHdiProvider_ = nullptr;
}

int32_t DCameraSourceController::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
    int32_t sceneMode)
{
    if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
        DHLOGE("StartCapture not support operate %{public}zu camera", indexs_.size());
        return DCAMERA_BAD_OPERATE;
    }

    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    DHLOGI("StartCapture devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    DCameraCaptureInfoCmd cmd;
    cmd.type_ = DCAMERA_PROTOCOL_TYPE_OPERATION;
    cmd.dhId_ = dhId;
    cmd.command_ = DCAMERA_PROTOCOL_CMD_CAPTURE;
    cmd.value_.assign(captureInfos.begin(), captureInfos.end());
    cmd.sceneMode_ = sceneMode;
    cmd.userId_ = userId_;
    cmd.tokenId_ = tokenId_;
    cmd.accountId_ = accountId_;
    std::string jsonStr;
    int32_t ret = cmd.Marshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("Marshal failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    DHLOGI("devId: %{public}s, dhId: %{public}s captureCommand: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str(), cmd.command_.c_str());
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())),
        jsonStr.length());
    if (ret != EOK) {
        DHLOGE("memcpy_s failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(channel_ == nullptr, DCAMERA_BAD_VALUE, "channel_ is null.");
    ret = channel_->SendData(buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("SendData failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    DHLOGI("StartCapture devId: %{public}s, dhId: %{public}s success", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceController::StopCapture()
{
    if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
        DHLOGE("StopCapture not support operate %{public}zu camera", indexs_.size());
        return DCAMERA_BAD_OPERATE;
    }

    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    DHLOGI("StopCapture devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(devId);
        CHECK_AND_RETURN_RET_LOG(camSinkSrv == nullptr, DCAMERA_BAD_VALUE, "stopCapture can not get sink service");
        int32_t ret = camSinkSrv->StopCapture(dhId);
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture failed: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return DCAMERA_BAD_OPERATE;
        }
    } else {
        cJSON *rootValue = cJSON_CreateObject();
        if (rootValue == nullptr) {
            return DCAMERA_BAD_VALUE;
        }
        cJSON_AddStringToObject(rootValue, "Command", DCAMERA_PROTOCOL_CMD_STOP_CAPTURE.c_str());
        char *data = cJSON_Print(rootValue);
        if (data == nullptr) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        std::string jsonStr = std::string(data);
        cJSON_Delete(rootValue);
        cJSON_free(data);
        std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
        int32_t ret = memcpy_s(buffer->Data(), buffer->Capacity(),
            reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())), jsonStr.length());
        CHECK_AND_RETURN_RET_LOG(ret != EOK, ret, "StopCapture memcpy_s failed ret: %{public}d", ret);
        CHECK_AND_RETURN_RET_LOG(channel_ == nullptr, DCAMERA_BAD_VALUE, "channel_ is null.");
        ret = channel_->SendData(buffer);
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture SendData failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
    }
    DHLOGI("StopCapture devId: %{public}s, dhId: %{public}s success", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceController::ChannelNeg(std::shared_ptr<DCameraChannelInfo>& info)
{
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "ChannelNeg info is nullptr");
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
            DHLOGE("ChannelNeg not support operate %{public}zu camera", indexs_.size());
            return DCAMERA_BAD_OPERATE;
        }

        std::string dhId = indexs_.begin()->dhId_;
        std::string devId = indexs_.begin()->devId_;
        DHLOGI("ChannelNeg devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
            GetAnonyString(dhId).c_str());
        sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(devId);
        if (camSinkSrv == nullptr) {
            DHLOGE("can not get service, devId: %{public}s", GetAnonyString(devId).c_str());
            return DCAMERA_BAD_OPERATE;
        }
        DCameraChannelInfoCmd cmd;
        cmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
        cmd.dhId_ = dhId;
        cmd.command_ = DCAMERA_PROTOCOL_CMD_CHAN_NEG;
        cmd.value_ = info;
        std::string jsonStr;
        int32_t ret = cmd.Marshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("Marshal failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
        DHLOGD("devId: %{public}s, dhId: %{public}s channelNegCommand: %{public}s",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str(), cmd.command_.c_str());
        ret = camSinkSrv->ChannelNeg(dhId, jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("ChannelNeg rpc failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
        DHLOGD("DCameraSourceController ChannelNeg devId: %{public}s, dhId: %{public}s success",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceController::DCameraNotify(std::shared_ptr<DCameraEvent>& events)
{
    CHECK_AND_RETURN_RET_LOG(events == nullptr, DCAMERA_BAD_VALUE, "DCameraNotify events is nullptr");
    if (events->eventResult_ == DCAMERA_EVENT_CAMERA_SUCCESS &&
        events->eventContent_ == START_CAPTURE_SUCC) {
        DcameraRadar::GetInstance().ReportDcameraOpen("StartCapture", CameraOpen::START_CAPTURE,
            BizState::BIZ_STATE_END, DCAMERA_OK);
    }

    if (events->eventResult_ == DCAMERA_EVENT_CAMERA_ERROR) {
        DcameraFinishAsyncTrace(DCAMERA_CONTINUE_FIRST_FRAME, DCAMERA_CONTINUE_FIRST_FRAME_TASKID);
        DcameraFinishAsyncTrace(DCAMERA_SNAPSHOT_FIRST_FRAME, DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID);
    }
    if (events->eventResult_ == DCAMERA_EVENT_DEVICE_IN_USE) {
        ReportCameraOperaterEvent(DCAMERA_CONFLICT_RECEIVE_EVENT, GetAnonyString(devId_),
            dhId_, "Source notify device in use event");
    }
    if (camHdiProvider_ == nullptr) {
        DHLOGI("DCameraNotify camHdiProvider is nullptr devId: %{public}s dhId: %{public}s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return DCAMERA_BAD_OPERATE;
    }
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;
    if (events->eventType_ == DCAMERA_SINK_STOP && events->eventResult_ == DCAMERA_EVENT_SINK_STOP) {
        events->eventType_ = DCAMERA_OPERATION;
        events->eventResult_ = DCAMERA_EVENT_DEVICE_ERROR;
    }
    DCameraHDFEvent hdiEvent;
    hdiEvent.type_ = events->eventType_;
    hdiEvent.result_ = events->eventResult_;
    hdiEvent.content_ = events->eventContent_;
    int32_t retHdi = camHdiProvider_->Notify(dhBase, hdiEvent);
    DHLOGI("Nofify hal, ret: %{public}d, devId: %{public}s dhId: %{public}s, type: %{public}d, result: %{public}d, "
        "content: %{public}s", retHdi, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        events->eventType_, events->eventResult_, events->eventContent_.c_str());
    if (retHdi != SUCCESS) {
        return DCAMERA_BAD_OPERATE;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceController::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
        DHLOGE("UpdateSettings not support operate %{public}zu camera", indexs_.size());
        return DCAMERA_BAD_OPERATE;
    }

    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    DHLOGI("UpdateSettings devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    DCameraMetadataSettingCmd cmd;
    cmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
    cmd.dhId_ = dhId;
    cmd.command_ = DCAMERA_PROTOCOL_CMD_UPDATE_METADATA;
    cmd.value_.assign(settings.begin(), settings.end());
    std::string jsonStr;
    int32_t ret = cmd.Marshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGE("Marshal failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    std::shared_ptr<DataBuffer> buffer = std::make_shared<DataBuffer>(jsonStr.length() + 1);
    ret = memcpy_s(buffer->Data(), buffer->Capacity(), reinterpret_cast<uint8_t *>(const_cast<char *>(jsonStr.c_str())),
        jsonStr.length());
    if (ret != EOK) {
        DHLOGE("memcpy_s failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(channel_ == nullptr, DCAMERA_BAD_VALUE, "channel_ is null.");
    ret = channel_->SendData(buffer);
    if (ret != DCAMERA_OK) {
        DHLOGE("SendData failed %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
        return ret;
    }
    DHLOGI("UpdateSettings devId: %{public}s, dhId: %{public}s success", GetAnonyString(devId).c_str(),
        GetAnonyString(dhId).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceController::GetCameraInfo(std::shared_ptr<DCameraInfo>& camInfo)
{
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
            DHLOGE("GetCameraInfo not support operate %{public}zu camera", indexs_.size());
            return DCAMERA_BAD_OPERATE;
        }

        std::string dhId = indexs_.begin()->dhId_;
        std::string devId = indexs_.begin()->devId_;
        DHLOGI("GetCameraInfo devId: %{public}s, dhId: %{public}s", GetAnonyString(devId).c_str(),
            GetAnonyString(dhId).c_str());
        sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(devId);
        if (camSinkSrv == nullptr) {
            DHLOGE("can not get service, devId: %{public}s", GetAnonyString(devId).c_str());
            return DCAMERA_BAD_OPERATE;
        }
        std::string camInfoJson;
        int32_t ret = camSinkSrv->GetCameraInfo(dhId, camInfoJson);
        if (ret != DCAMERA_OK) {
            DHLOGE("GetCameraInfo failed: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
            return ret;
        }
        DCameraInfoCmd cmd;
        ret = cmd.Unmarshal(camInfoJson);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraInfoCmd Unmarshal failed: %{public}d", ret);
            return ret;
        }
        camInfo = cmd.value_;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceController::OpenChannel(std::shared_ptr<DCameraOpenInfo>& openInfo)
{
    CHECK_AND_RETURN_RET_LOG(openInfo == nullptr, DCAMERA_BAD_VALUE, "OpenChannel openInfo is nullptr");
    if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
        DHLOGE("OpenChannel not support operate %{public}zu camera", indexs_.size());
        return DCAMERA_BAD_OPERATE;
    }
    CHECK_AND_RETURN_RET_LOG(AddCameraServiceDeathRecipient() != DCAMERA_OK, DCAMERA_BAD_VALUE,
        "AddCameraServiceDeathRecipient failed");
    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    DHLOGI("DCameraSourceController OpenChannel Start, devId: %{public}s, dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    srcDevId_ = openInfo->sourceDevId_;
    if (!CheckAclRight()) {
        DHLOGE("ACL check failed.");
        return DCAMERA_BAD_OPERATE;
    }
    bool isInvalid = false;
    CHECK_AND_RETURN_RET_LOG(CheckOsType(devId, isInvalid) != DCAMERA_OK && isInvalid, DCAMERA_BAD_VALUE,
        "CheckOsType failed or invalid osType");
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(devId);
        CHECK_AND_RETURN_RET_LOG(camSinkSrv == nullptr, DCAMERA_BAD_OPERATE,
            "DCameraSourceController can not get service, devId: %{public}s", GetAnonyString(devId).c_str());
        std::string jsonStr;
        DCameraOpenInfoCmd cmd;
        cmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
        cmd.dhId_ = dhId;
        cmd.command_ = DCAMERA_PROTOCOL_CMD_OPEN_CHANNEL;
        cmd.value_ = openInfo;
        int32_t ret = cmd.Marshal(jsonStr);
        CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
            "DCameraSourceController Marshal OpenInfo failed %{public}d", ret);
        ret = camSinkSrv->OpenChannel(dhId, jsonStr);
        CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret,
            "DCameraSourceController SA OpenChannel failed %{public}d", ret);
        DHLOGD("DCameraSourceController OpenChannel devId: %{public}s, dhId: %{public}s success",
            GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    }

    std::vector<DCameraIndex> indexs;
    indexs.push_back(DCameraIndex(devId, dhId));
    CHECK_AND_RETURN_RET_LOG(channel_ == nullptr, DCAMERA_BAD_VALUE, "OpenChannel channel_ is null.");
    int32_t ret = channel_->CreateSession(indexs, SESSION_FLAG, DCAMERA_SESSION_MODE_CTRL, listener_);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceController Bind Socket failed, ret: %{public}d", ret);
        PostChannelDisconnectedEvent();
        return ret;
    }
    return PublishEnableLatencyMsg(devId);
}

int32_t DCameraSourceController::ParseValueFromCjson(std::string args, std::string key)
{
    DHLOGD("ParseValueFromCjson");
    cJSON *jParam = cJSON_Parse(args.c_str());
    CHECK_NULL_RETURN(jParam == nullptr, DCAMERA_BAD_VALUE);
    cJSON *retItem = cJSON_GetObjectItemCaseSensitive(jParam, key.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG(retItem == nullptr || !cJSON_IsNumber(retItem),
        DCAMERA_BAD_VALUE, jParam, "Not found key result");
    int32_t ret = retItem->valueint;
    cJSON_Delete(jParam);
    return ret;
}

int32_t DCameraSourceController::CheckOsType(const std::string &networkId, bool &isInvalid)
{
    std::vector<DistributedHardware::DmDeviceInfo> dmDeviceInfoList;
    int32_t errCode = DeviceManager::GetInstance().GetTrustedDeviceList(DCAMERA_PKG_NAME, "", dmDeviceInfoList);
    CHECK_AND_RETURN_RET_LOG(errCode != DCAMERA_OK, DCAMERA_BAD_VALUE,
        "Get device manager trusted device list fail, errCode %{public}d", errCode);
    for (const auto& dmDeviceInfo : dmDeviceInfoList) {
        if (dmDeviceInfo.networkId == networkId) {
            int32_t osType = ParseValueFromCjson(dmDeviceInfo.extraData, KEY_OS_TYPE);
            if (osType != VALID_OS_TYPE && osType != DCAMERA_BAD_VALUE) {
                isInvalid = true;
            }
            DHLOGI("remote found, osType: %{public}d, isInvalid: %{public}d", osType, isInvalid);
            return DCAMERA_OK;
        }
    }
    DHLOGI("remote not found.");
    return DCAMERA_OK;
}

bool DCameraSourceController::CheckAclRight()
{
    if (!GetOsAccountInfo()) {
        return false;
    }
    std::shared_ptr<DmInitCallback> initCallback = std::make_shared<DeviceInitCallback>();
    int32_t ret = DeviceManager::GetInstance().InitDeviceManager(DCAMERA_PKG_NAME, initCallback);
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
        .networkId = devId_,
    };
    DHLOGI("CheckAclRight dmSrcCaller networkId: %{public}s, accountId: %{public}s, devId: %{public}s",
        GetAnonyString(srcDevId_).c_str(), GetAnonyString(accountId_).c_str(), GetAnonyString(devId_).c_str());
    if (DeviceManager::GetInstance().CheckSrcAccessControl(dmSrcCaller, dmDstCallee)) {
        return true;
    }
    return false;
}

bool DCameraSourceController::GetOsAccountInfo()
{
#ifdef OS_ACCOUNT_ENABLE
    std::vector<int32_t> ids;
    int32_t ret = AccountSA::OsAccountManager::QueryActiveOsAccountIds(ids);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK || ids.empty(), false,
        "Get userId from active os accountIds fail, ret: %{public}d", ret);
    userId_ = ids[0];

    AccountSA::OhosAccountInfo osAccountInfo;
    ret = AccountSA::OhosAccountKits::GetInstance().GetOhosAccountInfo(osAccountInfo);
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, false,
        "Get accountId from ohos account info fail, ret: %{public}d", ret);
    accountId_ = osAccountInfo.uid_;
#endif
    return true;
}

int32_t DCameraSourceController::CloseChannel()
{
    if (indexs_.empty() || indexs_.size() > DCAMERA_MAX_NUM) {
        DHLOGE("CloseChannel not support operate %{public}zu camera", indexs_.size());
        return DCAMERA_BAD_OPERATE;
    }
    if (cameraServiceProxy_ != nullptr && cameraServiceProxy_->AsObject() != nullptr) {
        (void)cameraServiceProxy_->AsObject()->RemoveDeathRecipient(cameraServiceRecipient_);
    }
    cameraServiceRecipient_ = nullptr;
    DCameraLowLatency::GetInstance().DisableLowLatency();
    DCameraSoftbusLatency::GetInstance().StopSoftbusTimeSync(devId_);
    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    DHLOGI("DCameraSourceController CloseChannel Start, devId: %{public}s, dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    CHECK_AND_RETURN_RET_LOG(channel_ == nullptr, DCAMERA_BAD_VALUE, "CloseChannel channel_ is null.");
    int32_t ret = channel_->CloseSession();
    if (ret != DCAMERA_OK) {
        DHLOGE("CloseSession failed %{public}d", ret);
    }
    DHLOGI("DCameraSourceController CloseChannel devId: %{public}s, dhId: %{public}s success",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    channelState_ = DCAMERA_CHANNEL_STATE_DISCONNECTED;
    ret = channel_->ReleaseSession();
    if (ret != DCAMERA_OK) {
        DHLOGE("ReleaseSession failed %{public}d", ret);
    }
    if (!ManageSelectChannel::GetInstance().GetSrcConnect()) {
        sptr<IDistributedCameraSink> camSinkSrv = DCameraSourceServiceIpc::GetInstance().GetSinkRemoteCamSrv(devId);
        if (camSinkSrv != nullptr) {
            ret = camSinkSrv->CloseChannel(dhId);
            if (ret != DCAMERA_OK) {
                DHLOGE("DCameraSourceController SA CloseChannel failed %{public}d", ret);
            }
            DCameraSourceServiceIpc::GetInstance().DeleteSinkRemoteCamSrv(devId);
        }
    }
    return ret;
}

int32_t DCameraSourceController::Init(std::vector<DCameraIndex>& indexs)
{
    DHLOGI("DCameraSourceController Init");
    if (indexs.size() > DCAMERA_MAX_NUM) {
        DHLOGE("DCameraSourceController init error");
        return DCAMERA_INIT_ERR;
    }
    camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider_ == nullptr) {
        DHLOGE("camHdiProvider_ is null.");
        return DCAMERA_INIT_ERR;
    }
    remote_ = OHOS::HDI::hdi_objcast<IDCameraProvider>(camHdiProvider_);
    if (remote_ != nullptr) {
        remote_->AddDeathRecipient(cameraHdiRecipient_);
    }

    indexs_.assign(indexs.begin(), indexs.end());
    std::string dhId = indexs_.begin()->dhId_;
    std::string devId = indexs_.begin()->devId_;
    controller_ = std::shared_ptr<DCameraSourceController>(shared_from_this());
    listener_ = std::make_shared<DCameraSourceControllerChannelListener>(controller_);
    channel_ = std::make_shared<DCameraChannelSourceImpl>();
    DHLOGI("DCameraSourceController Init GetProvider end devId: %{public}s, dhId: %{public}s",
        GetAnonyString(devId).c_str(), GetAnonyString(dhId).c_str());
    isInit = true;
    return DCAMERA_OK;
}

int32_t DCameraSourceController::UnInit()
{
    DHLOGI("DCameraSourceController UnInit");
    indexs_.clear();
    isInit = false;
    isChannelConnected_.store(false);
    if (remote_ != nullptr) {
        remote_->RemoveDeathRecipient(cameraHdiRecipient_);
    }
    return DCAMERA_OK;
}

void DCameraSourceController::OnSessionState(int32_t state, std::string networkId)
{
    DHLOGI("DCameraSourceController OnSessionState state %{public}d", state);
    channelState_ = state;
    switch (state) {
        case DCAMERA_CHANNEL_STATE_CONNECTED: {
            DcameraFinishAsyncTrace(DCAMERA_OPEN_CHANNEL_CONTROL, DCAMERA_OPEN_CHANNEL_TASKID);
            isChannelConnected_.store(true);
            CHECK_AND_RETURN_LOG(stateMachine_ == nullptr, "stateMachine_ is nullptr");
            stateMachine_->UpdateState(DCAMERA_STATE_OPENED);
            std::shared_ptr<DCameraSourceDev> camDev = camDev_.lock();
            if (camDev == nullptr) {
                DHLOGE("DCameraSourceController OnSessionState camDev is nullptr");
                break;
            }
            camDev->OnChannelConnectedEvent();
            break;
        }
        case DCAMERA_CHANNEL_STATE_DISCONNECTED: {
            DcameraFinishAsyncTrace(DCAMERA_OPEN_CHANNEL_CONTROL, DCAMERA_OPEN_CHANNEL_TASKID);
            DHLOGI("DCameraSourceDev PostTask Controller CloseSession OnClose devId %{public}s dhId %{public}s",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            isChannelConnected_.store(false);
            PostChannelDisconnectedEvent();
            break;
        }
        default: {
            break;
        }
    }
}

void DCameraSourceController::OnSessionError(int32_t eventType, int32_t eventReason, std::string detail)
{
    DHLOGI("DCameraSourceController OnSessionError devId: %{public}s, dhId: %{public}s, eventType: %{public}d, "
        "eventReason: %{public}d, detail %{public}s", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        eventType, eventReason, detail.c_str());
    return;
}

void DCameraSourceController::OnDataReceived(std::vector<std::shared_ptr<DataBuffer>>& buffers)
{
    for (auto& buffer : buffers) {
        if (buffer->Size() <= 0 || buffer->Size() > DATABUFF_MAX_SIZE) {
            DHLOGI("buffer is invalid");
            return;
        }
        HandleReceivedData(buffer);
    }
}

void DCameraSourceController::HandleReceivedData(std::shared_ptr<DataBuffer>& dataBuffer)
{
    CHECK_AND_RETURN_LOG(dataBuffer == nullptr, "dataBuffer is nullptr");
    DHLOGI("DCameraSourceController::HandleReceivedData dhId: %{public}s", GetAnonyString(dhId_).c_str());
    uint8_t *data = dataBuffer->Data();
    std::string jsonStr(reinterpret_cast<const char *>(data), dataBuffer->Capacity());
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    if (rootValue == nullptr) {
        return;
    }
    cJSON *comvalue = cJSON_GetObjectItemCaseSensitive(rootValue, "Command");
    if (comvalue == nullptr || !cJSON_IsString(comvalue) || (comvalue->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        DHLOGE("parse command failed");
        return;
    }
    std::string command = std::string(comvalue->valuestring);
    cJSON_Delete(rootValue);
    if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_METADATA_RESULT) == 0)) {
        HandleMetaDataResult(jsonStr);
    } else if ((!command.empty()) && (command.compare(DCAMERA_PROTOCOL_CMD_STATE_NOTIFY) == 0)) {
        DCameraEventCmd cmd;
        int32_t ret = cmd.Unmarshal(jsonStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceController Unmarshal failed, ret: %{public}d, devId: %{public}s, "
                "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return;
        }
        DCameraNotify(cmd.value_);
    }
}

void DCameraSourceController::HandleMetaDataResult(std::string& jsonStr)
{
    if (camHdiProvider_ == nullptr) {
        DHLOGI("DCameraSourceController HandleMetaDataResult camHdiProvider is null, devId: %{public}s, "
            "dhId: %{public}s", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return;
    }
    DCameraMetadataSettingCmd cmd;
    int32_t ret = cmd.Unmarshal(jsonStr);
    if (ret != DCAMERA_OK) {
        DHLOGI("DCameraSourceController HandleMetaDataResult failed, ret: %{public}d, devId: %{public}s, "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return;
    }
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;
    for (auto iter = cmd.value_.begin(); iter != cmd.value_.end(); iter++) {
        DCameraSettings setting;
        setting.type_ = (*iter)->type_;
        setting.value_ = (*iter)->value_;
        int32_t retHdi = camHdiProvider_->OnSettingsResult(dhBase, setting);
        DHLOGI("OnSettingsResult hal, ret: %{public}d, devId: %{public}s dhId: %{public}s", retHdi,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
}

int32_t DCameraSourceController::PublishEnableLatencyMsg(const std::string& devId)
{
    DHLOGI("DCameraSourceController PublishEnableLatencyMsg Start,devId: %{public}s", GetAnonyString(devId_).c_str());
    isChannelConnected_.store(false);
    DCameraLowLatency::GetInstance().EnableLowLatency();
    DCameraSoftbusLatency::GetInstance().StartSoftbusTimeSync(devId);
    DHLOGI("DCameraSourceController PublishEnableLatencyMsg End,devId: %{public}s", GetAnonyString(devId_).c_str());
    return DCAMERA_OK;
}

void DCameraSourceController::PostChannelDisconnectedEvent()
{
    std::shared_ptr<DCameraSourceDev> camDev = camDev_.lock();
    if (camDev == nullptr) {
        DHLOGE("DCameraSourceController PostChannelDisconnectedEvent camDev is nullptr");
        return;
    }
    camDev->OnChannelDisconnectedEvent();
}

int32_t DCameraSourceController::PauseDistributedHardware(const std::string &networkId)
{
    return DCAMERA_OK;
}

int32_t DCameraSourceController::ResumeDistributedHardware(const std::string &networkId)
{
    return DCAMERA_OK;
}

int32_t DCameraSourceController::StopDistributedHardware(const std::string &networkId)
{
    return DCAMERA_OK;
}

void DCameraSourceController::DCameraHdiRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGE("Exit the current process.");
    _Exit(0);
}

int32_t DCameraSourceController::AddCameraServiceDeathRecipient()
{
    cameraServiceRecipient_ = sptr<CameraServiceRecipient>(new CameraServiceRecipient(controller_));
    auto samgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    CHECK_AND_RETURN_RET_LOG(samgr == nullptr, DCAMERA_BAD_VALUE, "GetSystemAbilityManager failed.");
    auto object = samgr->CheckSystemAbility(CAMERA_SERVICE_ID);
    CHECK_AND_RETURN_RET_LOG(object == nullptr, DCAMERA_BAD_VALUE,
        "CheckSystemAbility %{public}d failed.", CAMERA_SERVICE_ID);
    cameraServiceProxy_ = iface_cast<CameraStandard::ICameraService>(object);
    CHECK_AND_RETURN_RET_LOG(cameraServiceProxy_ == nullptr, DCAMERA_BAD_VALUE, "Get ICameraService failed.");
    auto remoteObj = cameraServiceProxy_->AsObject();
    CHECK_AND_RETURN_RET_LOG(remoteObj == nullptr, DCAMERA_BAD_VALUE, "Get remote object failed.");
    remoteObj->AddDeathRecipient(cameraServiceRecipient_);
    return DCAMERA_OK;
}

DCameraSourceController::CameraServiceRecipient::CameraServiceRecipient(
    std::shared_ptr<DCameraSourceController> sourceContrlPtr) : sourceContrlPtr_(sourceContrlPtr)
{
    DHLOGI("Ctor CameraServiceRecipient()");
}

void DCameraSourceController::CameraServiceRecipient::OnRemoteDied(const wptr<IRemoteObject> &remote)
{
    DHLOGE("CameraService died, Exit the current process.");
    std::shared_ptr<DCameraEvent> events = std::make_shared<DCameraEvent>();
    events->eventType_ = DCAMERA_MESSAGE;
    events->eventResult_ = DCAMERA_EVENT_DEVICE_ERROR;
    events->eventContent_ = CAMERA_SERVICE_DIED;
    auto sourceContrl = sourceContrlPtr_.lock();
    CHECK_AND_RETURN_LOG(sourceContrl == nullptr, "sourceContrl is nullptr.");
    sourceContrl->DCameraNotify(events);
}

void DCameraSourceController::SetTokenId(uint64_t token)
{
    tokenId_ = token;
}

void DeviceInitCallback::OnRemoteDied()
{
    DHLOGI("DeviceInitCallback OnRemoteDied");
}
} // namespace DistributedHardware
} // namespace OHOS
