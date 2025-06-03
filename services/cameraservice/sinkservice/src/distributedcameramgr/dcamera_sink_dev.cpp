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

#include "dcamera_sink_dev.h"

#include "anonymous_string.h"
#include "dcamera_channel_info_cmd.h"
#include "dcamera_info_cmd.h"
#include "dcamera_protocol.h"
#include "dcamera_sink_access_control.h"
#include "dcamera_sink_controller.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkDev::DCameraSinkDev(const std::string& dhId, const sptr<IDCameraSinkCallback> &sinkCallback)
    : dhId_(dhId), sinkCallback_(sinkCallback)
{
    DHLOGI("DCameraSinkDev Constructor dhId: %{public}s", GetAnonyString(dhId_).c_str());
    isInit_ = false;
}

DCameraSinkDev::~DCameraSinkDev()
{
    if (isInit_) {
        UnInit();
    }
}

int32_t DCameraSinkDev::Init()
{
    DHLOGI("Init dhId: %{public}s", GetAnonyString(dhId_).c_str());
    accessControl_ = std::make_shared<DCameraSinkAccessControl>();
    controller_ = std::make_shared<DCameraSinkController>(accessControl_, sinkCallback_);
    controller_->SetTokenId(tokenId_);
    DCameraIndex index("", dhId_);
    std::vector<DCameraIndex> indexs;
    indexs.push_back(index);
    int32_t ret = controller_->Init(indexs);
    if (ret != DCAMERA_OK) {
        DHLOGE("init controller failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    isInit_ = true;
    DHLOGI("DCameraSinkDev Init %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkDev::UnInit()
{
    if (controller_ != nullptr) {
        int32_t ret = controller_->UnInit();
        if (ret != DCAMERA_OK) {
            DHLOGE("release controller failed, dhId: %{public}s, ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
        }
    }
    isInit_ = false;
    return DCAMERA_OK;
}

int32_t DCameraSinkDev::SubscribeLocalHardware(const std::string& parameters)
{
    DHLOGI("enter");
    (void)parameters;
    return DCAMERA_OK;
}

int32_t DCameraSinkDev::UnsubscribeLocalHardware()
{
    DHLOGI("enter");
    return DCAMERA_OK;
}

int32_t DCameraSinkDev::StopCapture()
{
    DHLOGI("StopCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    CHECK_AND_RETURN_RET_LOG(controller_ == nullptr, DCAMERA_BAD_VALUE, "controller_ is null.");
    return controller_->StopCapture();
}

int32_t DCameraSinkDev::ChannelNeg(std::string& channelInfo)
{
    DHLOGI("ChannelNeg dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (channelInfo.empty()) {
        DHLOGE("channelInfo is empty");
        return DCAMERA_BAD_VALUE;
    }

    DCameraChannelInfoCmd channelInfoCmd;
    int32_t ret = channelInfoCmd.Unmarshal(channelInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("channelInfo unmarshal failed, dhId: %{public}s, ret: %{public}d",
               GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(controller_ == nullptr, DCAMERA_BAD_VALUE, "controller_ is null.");
    return controller_->ChannelNeg(channelInfoCmd.value_);
}

int32_t DCameraSinkDev::GetCameraInfo(std::string& cameraInfo)
{
    DHLOGI("GetCameraInfo dhId: %{public}s", GetAnonyString(dhId_).c_str());
    std::shared_ptr<DCameraInfo> info = std::make_shared<DCameraInfo>();
    CHECK_AND_RETURN_RET_LOG(controller_ == nullptr, DCAMERA_BAD_VALUE, "controller_ is null.");
    int32_t ret = controller_->GetCameraInfo(info);
    if (ret != DCAMERA_OK) {
        DHLOGE("get state failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }

    DCameraInfoCmd cameraInfoCmd;
    cameraInfoCmd.type_ = DCAMERA_PROTOCOL_TYPE_MESSAGE;
    cameraInfoCmd.dhId_ = dhId_;
    cameraInfoCmd.command_ = DCAMERA_PROTOCOL_CMD_GET_INFO;
    cameraInfoCmd.value_ = info;
    ret = cameraInfoCmd.Marshal(cameraInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("cameraInfoCmd marshal failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    DHLOGI("GetCameraInfo %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkDev::OpenChannel(std::string& openInfo)
{
    DHLOGI("DCameraSinkDev OpenChannel Begin, dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (openInfo.empty()) {
        DHLOGE("openInfo is empty");
        return DCAMERA_BAD_VALUE;
    }

    DCameraOpenInfoCmd cmd;
    int32_t ret = cmd.Unmarshal(openInfo);
    if (ret != DCAMERA_OK) {
        DHLOGE("openInfo unmarshal failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
        return ret;
    }
    CHECK_AND_RETURN_RET_LOG(controller_ == nullptr, DCAMERA_BAD_VALUE, "controller_ is null.");
    return controller_->OpenChannel(cmd.value_);
}

int32_t DCameraSinkDev::CloseChannel()
{
    DHLOGI("CloseChannel dhId: %{public}s", GetAnonyString(dhId_).c_str());
    CHECK_AND_RETURN_RET_LOG(controller_ == nullptr, DCAMERA_BAD_VALUE, "controller_ is null.");
    return controller_->CloseChannel();
}

std::string DCameraSinkDev::GetDhid()
{
    return GetAnonyString(dhId_);
}

int32_t DCameraSinkDev::PauseDistributedHardware(const std::string &networkId)
{
    DHLOGI("Pause distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }
    if (controller_ == nullptr) {
        DHLOGE("controller_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }

    return controller_->PauseDistributedHardware(networkId);
}

int32_t DCameraSinkDev::ResumeDistributedHardware(const std::string &networkId)
{
    DHLOGI("Resume distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }
    if (controller_ == nullptr) {
        DHLOGE("controller_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }

    return controller_->ResumeDistributedHardware(networkId);
}

int32_t DCameraSinkDev::StopDistributedHardware(const std::string &networkId)
{
    DHLOGI("Stop distributed hardware dhId: %{public}s", GetAnonyString(dhId_).c_str());
    if (networkId.empty()) {
        DHLOGE("networkId is empty");
        return DCAMERA_BAD_VALUE;
    }
    if (controller_ == nullptr) {
        DHLOGE("controller_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }

    return controller_->StopDistributedHardware(networkId);
}

void DCameraSinkDev::SetTokenId(uint64_t token)
{
    tokenId_ = token;
}
} // namespace DistributedHardware
} // namespace OHOS