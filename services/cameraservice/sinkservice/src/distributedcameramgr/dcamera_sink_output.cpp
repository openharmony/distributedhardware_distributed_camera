/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "dcamera_sink_output.h"

#include "anonymous_string.h"
#include "dcamera_channel_sink_impl.h"
#include "dcamera_client.h"
#include "dcamera_sink_data_process.h"
#include "dcamera_sink_output_channel_listener.h"
#include "dcamera_sink_output_result_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSinkOutput::DCameraSinkOutput(const std::string& dhId, std::shared_ptr<ICameraOperator>& cameraOperator)
    : dhId_(dhId), operator_(cameraOperator)
{
    DHLOGI("DCameraSinkOutput Constructor dhId: %{public}s", GetAnonyString(dhId_).c_str());
    isInit_ = false;
}

DCameraSinkOutput::~DCameraSinkOutput()
{
    if (isInit_) {
        UnInit();
    }
}

int32_t DCameraSinkOutput::Init()
{
    CHECK_AND_RETURN_RET_LOG(operator_ == nullptr, DCAMERA_BAD_VALUE, "operator_ is null");
    DHLOGI("Init dhId: %{public}s", GetAnonyString(dhId_).c_str());
    auto output = std::shared_ptr<DCameraSinkOutput>(shared_from_this());
    std::shared_ptr<ResultCallback> resultCallback = std::make_shared<DCameraSinkOutputResultCallback>(output);
    operator_->SetResultCallback(resultCallback);

    InitInner(CONTINUOUS_FRAME);
    InitInner(SNAPSHOT_FRAME);
    isInit_ = true;
    DHLOGI("Init %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

void DCameraSinkOutput::InitInner(DCStreamType type)
{
    std::shared_ptr<ICameraChannel> channel = std::make_shared<DCameraChannelSinkImpl>();
    std::shared_ptr<ICameraSinkDataProcess> dataProcess = std::make_shared<DCameraSinkDataProcess>(dhId_, channel);
    dataProcess->Init();
    dataProcesses_.emplace(type, dataProcess);
    channels_.emplace(type, channel);
    sessionState_.emplace(type, DCAMERA_CHANNEL_STATE_DISCONNECTED);
}

int32_t DCameraSinkOutput::UnInit()
{
    DHLOGI("UnInit dhId: %{public}s", GetAnonyString(dhId_).c_str());
    channels_.clear();
    dataProcesses_.clear();
    sessionState_.clear();
    isInit_ = false;
    DHLOGI("UnInit %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSinkOutput::OpenChannel(std::shared_ptr<DCameraChannelInfo>& info)
{
    DHLOGI("OpenChannel dhId: %{public}s", GetAnonyString(dhId_).c_str());
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "OpenChannel info is null");
    std::map<DCStreamType, DCameraSessionMode> modeMaps;
    modeMaps.emplace(CONTINUOUS_FRAME, DCAMERA_SESSION_MODE_VIDEO);
    modeMaps.emplace(SNAPSHOT_FRAME, DCAMERA_SESSION_MODE_JPEG);
    std::vector<DCameraIndex> indexs;
    indexs.push_back(DCameraIndex(info->sourceDevId_, dhId_));
    for (auto iter = info->detail_.begin(); iter != info->detail_.end(); iter++) {
        if (sessionState_[iter->streamType_] != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
            DHLOGE("wrong state, sessionState: %{public}d", sessionState_[iter->streamType_]);
            return DCAMERA_OK;
        }
        auto iterCh = channels_.find(iter->streamType_);
        if (iterCh == channels_.end()) {
            continue;
        }
        auto output = std::shared_ptr<DCameraSinkOutput>(shared_from_this());
        std::shared_ptr<ICameraChannelListener> channelListener =
            std::make_shared<DCameraSinkOutputChannelListener>(iter->streamType_, output);
        int32_t ret = iterCh->second->CreateSession(indexs, iter->dataSessionFlag_, modeMaps[iter->streamType_],
            channelListener);
        if (ret != DCAMERA_OK) {
            DHLOGE("channel create session failed, dhId: %{public}s, ret: %{public}d",
                   GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkOutput::CloseChannel()
{
    DHLOGI("CloseChannel dhId: %{public}s", GetAnonyString(dhId_).c_str());
    auto iterCon = channels_.find(CONTINUOUS_FRAME);
    if (iterCon != channels_.end()) {
        int32_t ret = DCAMERA_OK;
        CHECK_AND_RETURN_RET_LOG(iterCon->second == nullptr, DCAMERA_BAD_VALUE,
            "CloseChannel continuous channel is null");
        ret = iterCon->second->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSinkOutput UnInit release continue session failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }
        sessionState_[CONTINUOUS_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;
    }

    auto iterSnap = channels_.find(SNAPSHOT_FRAME);
    if (iterSnap != channels_.end()) {
        int32_t ret = DCAMERA_OK;
        CHECK_AND_RETURN_RET_LOG(iterSnap->second == nullptr, DCAMERA_BAD_VALUE,
            "CloseChannel snapshot channel is null");
        ret = iterSnap->second->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSinkOutput UnInit release snapshot session failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }
        sessionState_[SNAPSHOT_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkOutput::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    DHLOGI("StartCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    for (auto& info : captureInfos) {
        if (info == nullptr) {
            DHLOGE("StartCapture info is null");
            continue;
        }
        if (dataProcesses_.find(info->streamType_) == dataProcesses_.end()) {
            DHLOGE("has no data process, streamType: %{public}d", info->streamType_);
            break;
        }
        int32_t ret = dataProcesses_[info->streamType_]->StartCapture(info);
        if (ret != DCAMERA_OK) {
            DHLOGE("StartCapture failed, dhId: %{public}s, ret: %{public}d", GetAnonyString(dhId_).c_str(), ret);
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSinkOutput::StopCapture()
{
    DHLOGI("StopCapture dhId: %{public}s", GetAnonyString(dhId_).c_str());
    auto iterCon = dataProcesses_.find(CONTINUOUS_FRAME);
    if (iterCon != dataProcesses_.end()) {
        DHLOGI("StopCapture %{public}s continuous frame stop capture", GetAnonyString(dhId_).c_str());
        CHECK_AND_RETURN_RET_LOG(iterCon->second == nullptr, DCAMERA_BAD_VALUE,
            "StopCapture continuous data process is null");
        int32_t ret = iterCon->second->StopCapture();
        if (ret != DCAMERA_OK) {
            DHLOGE("continuous data process stop capture failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }
    }

    auto iterSnap = dataProcesses_.find(SNAPSHOT_FRAME);
    if (iterSnap != dataProcesses_.end()) {
        DHLOGI("StopCapture %{public}s snapshot frame stop capture", GetAnonyString(dhId_).c_str());
        CHECK_AND_RETURN_RET_LOG(iterSnap->second == nullptr, DCAMERA_BAD_VALUE,
            "StopCapture snapshot data process is null");
        int32_t ret = iterSnap->second->StopCapture();
        if (ret != DCAMERA_OK) {
            DHLOGE("snapshot data process stop capture failed, dhId: %{public}s, ret: %{public}d",
                GetAnonyString(dhId_).c_str(), ret);
        }
    }
    DHLOGI("StopCapture %{public}s success", GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

void DCameraSinkOutput::OnVideoResult(std::shared_ptr<DataBuffer>& buffer)
{
    if (sessionState_[CONTINUOUS_FRAME] != DCAMERA_CHANNEL_STATE_CONNECTED) {
        DHLOGE("OnVideoResult dhId: %{public}s, channel state: %{public}d",
               GetAnonyString(dhId_).c_str(), sessionState_[CONTINUOUS_FRAME]);
        return;
    }
    if (dataProcesses_.find(CONTINUOUS_FRAME) == dataProcesses_.end()) {
        DHLOGE("OnVideoResult %{public}s has no continuous data process", GetAnonyString(dhId_).c_str());
        return;
    }
    CHECK_AND_RETURN_LOG(dataProcesses_[CONTINUOUS_FRAME] == nullptr,
        "OnVideoResult continuous data process is null");
    dataProcesses_[CONTINUOUS_FRAME]->FeedStream(buffer);
}

void DCameraSinkOutput::OnPhotoResult(std::shared_ptr<DataBuffer>& buffer)
{
    if (dataProcesses_.find(SNAPSHOT_FRAME) == dataProcesses_.end()) {
        DHLOGE("OnPhotoResult %{public}s has no snapshot data process", GetAnonyString(dhId_).c_str());
        return;
    }
    CHECK_AND_RETURN_LOG(dataProcesses_[SNAPSHOT_FRAME] == nullptr,
        "OnPhotoResult snapshot data process is null");
    dataProcesses_[SNAPSHOT_FRAME]->FeedStream(buffer);
}

void DCameraSinkOutput::OnSessionState(DCStreamType type, int32_t state)
{
    DHLOGI("OnSessionState dhId: %{public}s, stream type: %{public}d, state: %{public}d",
           GetAnonyString(dhId_).c_str(), type, state);
    sessionState_[type] = state;
    switch (state) {
        case DCAMERA_CHANNEL_STATE_CONNECTING: {
            DHLOGI("channel is connecting, dhId: %{public}s, stream type: %{public}d",
                   GetAnonyString(dhId_).c_str(), type);
            break;
        }
        case DCAMERA_CHANNEL_STATE_CONNECTED: {
            DHLOGI("channel is connected, dhId: %{public}s, stream type: %{public}d",
                   GetAnonyString(dhId_).c_str(), type);
            break;
        }
        case DCAMERA_CHANNEL_STATE_DISCONNECTED: {
            DHLOGI("channel is disconnected, dhId: %{public}s, stream type: %{public}d",
                   GetAnonyString(dhId_).c_str(), type);
            break;
        }
        default: {
            DHLOGE("OnSessionState %{public}s unknown session state", GetAnonyString(dhId_).c_str());
            break;
        }
    }
}

void DCameraSinkOutput::OnSessionError(DCStreamType type, int32_t eventType, int32_t eventReason, std::string detail)
{
    DHLOGI("OnSessionError dhId: %{public}s, stream type: %{public}d, eventType: %{public}d, eventReason: "
        "%{public}d, detail: %{public}s", GetAnonyString(dhId_).c_str(), type, eventType, eventReason, detail.c_str());
}

void DCameraSinkOutput::OnDataReceived(DCStreamType type, std::vector<std::shared_ptr<DataBuffer>>& dataBuffers)
{
}

int32_t DCameraSinkOutput::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    if (dataProcesses_[CONTINUOUS_FRAME] == nullptr) {
        DHLOGD("GetProperty: continuous frame is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    return dataProcesses_[CONTINUOUS_FRAME]->GetProperty(propertyName, propertyCarrier);
}
} // namespace DistributedHardware
} // namespace OHOS