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

#include "anonymous_string.h"
#include "dcamera_channel_source_impl.h"
#include "dcamera_hitrace_adapter.h"
#include "dcamera_frame_info.h"
#include "dcamera_source_data_process.h"
#include "dcamera_source_event.h"
#include "dcamera_source_input.h"
#include "dcamera_source_input_channel_listener.h"
#include "dcamera_softbus_latency.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceInput::DCameraSourceInput(std::string devId, std::string dhId, std::shared_ptr<DCameraSourceDev>& camDev)
    : devId_(devId), dhId_(dhId), camDev_(camDev), isInit(false)
{
    DHLOGI("DCameraSourceInput Constructor devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
}

DCameraSourceInput::~DCameraSourceInput()
{
    DHLOGI("DCameraSourceInput Destructor devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (isInit) {
        UnInit();
    }
}

int32_t DCameraSourceInput::ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
{
    uint64_t infoSize = static_cast<uint64_t>(streamInfos.size());
    DHLOGI("DCameraSourceInput ConfigStreams devId %{public}s dhId %{public}s, size: %{public}" PRIu64,
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), infoSize);
    int32_t ret = ReleaseAllStreams();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ConfigStreams ReleaseAllStreams failed %{public}d devId: %{public}s, dhId: "
            "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    if (streamInfos.empty()) {
        return DCAMERA_OK;
    }

    std::vector<std::shared_ptr<DCStreamInfo>> snapStreams;
    std::vector<std::shared_ptr<DCStreamInfo>> continueStreams;
    for (auto iter = streamInfos.begin(); iter != streamInfos.end(); iter++) {
        std::shared_ptr<DCStreamInfo> streamInfo = *iter;
        DHLOGI("DCameraSourceInput ConfigStreams devId: %{public}s, dhId: %{public}s, streamId: %{public}d, width: "
            "%{public}d, height: %{public}d, format: %{public}d, dataspace: %{public}d, encodeType:%{public}d "
            "streamType: %{public}d", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), streamInfo->streamId_, streamInfo->width_, streamInfo->height_,
            streamInfo->format_, streamInfo->dataspace_, streamInfo->encodeType_, streamInfo->type_);
        if (streamInfo->type_ == CONTINUOUS_FRAME) {
            continueStreams.push_back(streamInfo);
        }
        if (streamInfo->type_ == SNAPSHOT_FRAME) {
            snapStreams.push_back(streamInfo);
        }
    }

    do {
        ret = dataProcess_[CONTINUOUS_FRAME]->ConfigStreams(continueStreams);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput ConfigStreams continue failed %{public}d devId %{public}s dhId %{public}s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
        }
        ret = dataProcess_[SNAPSHOT_FRAME]->ConfigStreams(snapStreams);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput ConfigStreams snapshot failed %{public}d devId %{public}s dhId %{public}s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
        }
    } while (0);

    if (ret != DCAMERA_OK) {
        ReleaseAllStreams();
    }
    return ret;
}

int32_t DCameraSourceInput::ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease)
{
    DHLOGI("DCameraSourceInput ReleaseStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->ReleaseStreams(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseStreams continue stream ReleaseStreams ret: %{public}d, devId: %{public}s,"
            " dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    ret = dataProcess_[SNAPSHOT_FRAME]->ReleaseStreams(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseStreams snapshot stream ReleaseStreams ret: %{public}d, devId: %{public}s, "
            "dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<int32_t> continueStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(continueStreamIds);
    std::vector<int32_t> snapStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(snapStreamIds);
    if (continueStreamIds.empty() && snapStreamIds.empty()) {
        isAllRelease = true;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos)
{
    DHLOGI("DCameraSourceInput StartCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    for (auto iter = captureInfos.begin(); iter != captureInfos.end(); iter++) {
        int32_t ret = DCAMERA_OK;
        for (auto iterSet = (*iter)->streamIds_.begin(); iterSet != (*iter)->streamIds_.end(); iterSet++) {
            DHLOGI("DCameraSourceInput StartCapture devId %{public}s dhId %{public}s StartCapture id: %{public}d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), *iterSet);
        }
        uint64_t idSize = static_cast<uint64_t>((*iter)->streamIds_.size());
        DHLOGI("DCameraSourceInput StartCapture Inner devId %{public}s dhId %{public}s streamType: %{public}d "
            "idSize: %{public}" PRIu64" isCap: %{public}d", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), (*iter)->type_, idSize, (*iter)->isCapture_ ? 1 : 0);
        ret = dataProcess_[(*iter)->type_]->StartCapture(*iter);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput StartCapture ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::StopCapture(std::vector<int>& streamIds, bool& isAllStop)
{
    DHLOGI("DCameraSourceInput StopCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->StopCapture(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput StopCapture continue ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    int32_t size = dataProcess_[CONTINUOUS_FRAME]->GetProducerSize();
    if (size == 0) {
        isAllStop = true;
        std::vector<int> snapStreamIds;
        dataProcess_[SNAPSHOT_FRAME]->GetAllStreamIds(snapStreamIds);
        ret = dataProcess_[SNAPSHOT_FRAME]->StopCapture(snapStreamIds);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput StopCapture snapshot ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::OpenChannel(std::vector<DCameraIndex>& indexs)
{
    DHLOGI("DCameraSourceInput OpenChannel devId %{public}s dhId %{public}s continue state: %{public}d, snapshot "
        "state: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        channelState_[CONTINUOUS_FRAME], channelState_[SNAPSHOT_FRAME]);
    if (channelState_[CONTINUOUS_FRAME] == DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = EstablishContinuousFrameSession(indexs);
        if (ret != DCAMERA_OK) {
            DHLOGE("esdablish continuous frame failed ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    if (channelState_[SNAPSHOT_FRAME] == DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = EstablishSnapshotFrameSession(indexs);
        if (ret != DCAMERA_OK) {
            DHLOGE("esdablish snapshot frame failed ret: %{public}d,"
                "devId: %{public}s, dhId: %{public}s", ret, GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::CloseChannel()
{
    DHLOGI("DCameraSourceInput CloseChannel devId %{public}s dhId %{public}s continue state: %{public}d, "
        "snapshot state: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        channelState_[CONTINUOUS_FRAME], channelState_[SNAPSHOT_FRAME]);
    if (channelState_[CONTINUOUS_FRAME] != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = DCAMERA_OK;
        ret = channels_[CONTINUOUS_FRAME]->CloseSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput CloseChannel continue stream failed ret: %{public}d, devId: %{public}s, dhId: "
                "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
        channelState_[CONTINUOUS_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;

        ret = channels_[CONTINUOUS_FRAME]->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSourceInput release continue session failed: %{public}d devId %{public}s dhId %{public}s",
                ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
    }

    if (channelState_[SNAPSHOT_FRAME] != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = DCAMERA_OK;
        ret = channels_[SNAPSHOT_FRAME]->CloseSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput CloseChannel snapshot stream failed ret: %{public}d, devId: %{public}s, dhId: "
                "%{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
        channelState_[SNAPSHOT_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;

        ret = channels_[SNAPSHOT_FRAME]->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSourceInput release snapshot session failed: %{public}d devId %{public}s dhId %{public}s",
                ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::Init()
{
    DHLOGI("DCameraSourceInput Init devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    auto input = std::shared_ptr<DCameraSourceInput>(shared_from_this());
    std::shared_ptr<ICameraSourceDataProcess> conDataProcess = std::make_shared<DCameraSourceDataProcess>(devId_, dhId_,
        CONTINUOUS_FRAME);
    std::shared_ptr<ICameraChannel> continueCh = std::make_shared<DCameraChannelSourceImpl>();
    std::shared_ptr<ICameraChannelListener> conListener =
        std::make_shared<DCameraSourceInputChannelListener>(input, CONTINUOUS_FRAME);
    channels_.emplace(CONTINUOUS_FRAME, continueCh);
    listeners_.emplace(CONTINUOUS_FRAME, conListener);
    dataProcess_.emplace(CONTINUOUS_FRAME, conDataProcess);
    channelState_.emplace(CONTINUOUS_FRAME, DCAMERA_CHANNEL_STATE_DISCONNECTED);

    std::shared_ptr<ICameraSourceDataProcess> snapDataProcess = std::make_shared<DCameraSourceDataProcess>(devId_,
        dhId_, SNAPSHOT_FRAME);
    std::shared_ptr<ICameraChannel> snapShotCh = std::make_shared<DCameraChannelSourceImpl>();
    std::shared_ptr<ICameraChannelListener> snapListener =
        std::make_shared<DCameraSourceInputChannelListener>(input, SNAPSHOT_FRAME);
    channels_.emplace(SNAPSHOT_FRAME, snapShotCh);
    listeners_.emplace(SNAPSHOT_FRAME, snapListener);
    dataProcess_.emplace(SNAPSHOT_FRAME, snapDataProcess);
    channelState_.emplace(SNAPSHOT_FRAME, DCAMERA_CHANNEL_STATE_DISCONNECTED);
    isInit = true;
    DHLOGI("DCameraSourceInput Init devId end %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::UnInit()
{
    DHLOGI("DCameraSourceInput UnInit devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    channels_.clear();
    listeners_.clear();
    dataProcess_.clear();
    channelState_.clear();
    isInit = false;
    isChannelConnected_.store(false);
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    return DCAMERA_OK;
}

void DCameraSourceInput::OnSessionState(DCStreamType streamType, int32_t state)
{
    DHLOGI("DCameraSourceInput OnSessionState devId: %{public}s, dhId: %{public}s, streamType: %{public}d, state: "
        "%{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType, state);
    channelState_[streamType] = (DCameraChannelState)state;
    switch (state) {
        case DCAMERA_CHANNEL_STATE_DISCONNECTED: {
            FinshFrameAsyncTrace(streamType);
            DHLOGI("DCameraSourceDev PostTask CloseSession Input OnClose devId %{public}s dhId %{public}s",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            isChannelConnected_.store(false);
            PostChannelDisconnectedEvent();
            break;
        }
        case DCAMERA_CHANNEL_STATE_CONNECTED: {
            FinshFrameAsyncTrace(streamType);
            isChannelConnected_.store(true);
            channelCond_.notify_one();
            DHLOGI("DCameraSourceInput OnSessionState state %{public}d", state);
            break;
        }
        default: {
            DHLOGI("DCameraSourceInput OnSessionState state %{public}d", state);
            break;
        }
    }
}

void DCameraSourceInput::FinshFrameAsyncTrace(DCStreamType streamType)
{
    if (streamType == CONTINUOUS_FRAME) {
        DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
    } else if (streamType == SNAPSHOT_FRAME) {
        DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
    }
}

void DCameraSourceInput::OnSessionError(DCStreamType streamType, int32_t eventType, int32_t eventReason,
    std::string detail)
{
    DHLOGI("DCameraSourceInput OnSessionError devId: %{public}s, dhId: %{public}s, eventType: %{public}d, "
        "eventReason: %{public}d, detail %{public}s", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        eventType, eventReason, detail.c_str());
    (void)streamType;
}

void DCameraSourceInput::OnDataReceived(DCStreamType streamType, std::vector<std::shared_ptr<DataBuffer>>& buffers)
{
    buffers[0]->frameInfo_.offset = DCameraSoftbusLatency::GetInstance().GetTimeSyncInfo(devId_);
    int32_t ret = dataProcess_[streamType]->FeedStream(buffers);
    if (ret != DCAMERA_OK) {
        DHLOGE("OnDataReceived FeedStream %{public}d stream failed ret: %{public}d, devId: %{public}s, "
            "dhId: %{public}s", streamType, ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
}

int32_t DCameraSourceInput::ReleaseAllStreams()
{
    DHLOGI("ReleaseAllStreams devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    std::vector<int> continueStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(continueStreamIds);
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->ReleaseStreams(continueStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("continue stream ReleaseStreams ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<int> snapStreamIds;
    dataProcess_[SNAPSHOT_FRAME]->GetAllStreamIds(snapStreamIds);
    ret = dataProcess_[SNAPSHOT_FRAME]->ReleaseStreams(snapStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("ReleaseAllStreams snapshot stream ReleaseStreams ret: %{public}d, devId: %{public}s, dhId: %{public}s",
            ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::StopAllCapture()
{
    DHLOGI("StopAllCapture devId %{public}s dhId %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    std::vector<int> continueStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(continueStreamIds);
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->StopCapture(continueStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("StopAllCapture continue ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<int> snapStreamIds;
    dataProcess_[SNAPSHOT_FRAME]->GetAllStreamIds(snapStreamIds);
    ret = dataProcess_[SNAPSHOT_FRAME]->StopCapture(snapStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("StopAllCapture snapshot ret: %{public}d, devId: %{public}s, dhId: %{public}s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

void DCameraSourceInput::PostChannelDisconnectedEvent()
{
    std::shared_ptr<DCameraSourceDev> camDev = camDev_.lock();
    if (camDev == nullptr) {
        DHLOGE("DCameraSourceInput PostChannelDisconnectedEvent camDev is nullptr");
        return;
    }
    camDev->OnChannelDisconnectedEvent();
}

int32_t DCameraSourceInput::EstablishContinuousFrameSession(std::vector<DCameraIndex>& indexs)
{
    DcameraStartAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
    int32_t ret = channels_[CONTINUOUS_FRAME]->CreateSession(indexs, CONTINUE_SESSION_FLAG, DCAMERA_SESSION_MODE_VIDEO,
        listeners_[CONTINUOUS_FRAME]);
    if (ret != DCAMERA_OK) {
        DHLOGE("Create Session failed ret: %{public}d,"
            "devId: %{public}s, dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        PostChannelDisconnectedEvent();
        DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
        return ret;
    }
    isChannelConnected_.store(false);
    DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::EstablishSnapshotFrameSession(std::vector<DCameraIndex>& indexs)
{
    DcameraStartAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
    int32_t ret = channels_[SNAPSHOT_FRAME]->CreateSession(indexs, SNAP_SHOT_SESSION_FLAG, DCAMERA_SESSION_MODE_JPEG,
        listeners_[SNAPSHOT_FRAME]);
    if (ret != DCAMERA_OK) {
        DHLOGE("create session failed ret: %{public}d,"
            "devId: %{public}s, dhId: %{public}s", ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        PostChannelDisconnectedEvent();
        DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
        return ret;
    }
    isChannelConnected_.store(false);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
