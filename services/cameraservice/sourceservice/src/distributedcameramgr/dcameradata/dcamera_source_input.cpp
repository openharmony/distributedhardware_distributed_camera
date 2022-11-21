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

#include "dcamera_source_input.h"

#include "anonymous_string.h"
#include "dcamera_hitrace_adapter.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

#include "dcamera_channel_source_impl.h"
#include "dcamera_source_data_process.h"
#include "dcamera_source_event.h"
#include "dcamera_source_input_channel_listener.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceInput::DCameraSourceInput(std::string devId, std::string dhId, std::shared_ptr<EventBus>& eventBus)
    : devId_(devId), dhId_(dhId), eventBus_(eventBus), isInit(false)
{
    DHLOGI("DCameraSourceInput Constructor devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
}

DCameraSourceInput::~DCameraSourceInput()
{
    DHLOGI("DCameraSourceInput Destructor devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (isInit) {
        UnInit();
    }
}

int32_t DCameraSourceInput::ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
{
    DHLOGI("DCameraSourceInput ConfigStreams devId %s dhId %s, size: %d", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str(), streamInfos.size());
    int32_t ret = ReleaseAllStreams();
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ConfigStreams ReleaseAllStreams failed %d devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    if (streamInfos.empty()) {
        return DCAMERA_OK;
    }

    std::vector<std::shared_ptr<DCStreamInfo>> snapStreams;
    std::vector<std::shared_ptr<DCStreamInfo>> continueStreams;
    for (auto iter = streamInfos.begin(); iter != streamInfos.end(); iter++) {
        std::shared_ptr<DCStreamInfo> streamInfo = *iter;
        DHLOGI("DCameraSourceInput ConfigStreams devId: %s, dhId: %s, streamId: %d, width: %d, height: %d," +
            "format: %d, dataspace: %d, encodeType:%d streamType: %d", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), streamInfo->streamId_, streamInfo->width_, streamInfo->height_,
            streamInfo->format_, streamInfo->dataspace_, streamInfo->encodeType_, streamInfo->type_);
        switch (streamInfo->type_) {
            case CONTINUOUS_FRAME: {
                continueStreams.push_back(streamInfo);
                break;
            }
            case SNAPSHOT_FRAME: {
                snapStreams.push_back(streamInfo);
                break;
            }
            default:
                break;
        }
    }

    do {
        ret = dataProcess_[CONTINUOUS_FRAME]->ConfigStreams(continueStreams);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput ConfigStreams continue failed %d devId %s dhId %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            break;
        }
        ret = dataProcess_[SNAPSHOT_FRAME]->ConfigStreams(snapStreams);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput ConfigStreams snapshot failed %d devId %s dhId %s", ret,
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
    DHLOGI("DCameraSourceInput ReleaseStreams devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->ReleaseStreams(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseStreams continue stream ReleaseStreams ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    ret = dataProcess_[SNAPSHOT_FRAME]->ReleaseStreams(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseStreams snapshot stream ReleaseStreams ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
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
    DHLOGI("DCameraSourceInput StartCapture devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    for (auto iter = captureInfos.begin(); iter != captureInfos.end(); iter++) {
        int32_t ret = DCAMERA_OK;
        for (auto iterSet = (*iter)->streamIds_.begin(); iterSet != (*iter)->streamIds_.end(); iterSet++) {
            DHLOGI("DCameraSourceInput StartCapture devId %s dhId %s StartCapture id: %d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), *iterSet);
        }
        DHLOGI("DCameraSourceInput StartCapture Inner devId %s dhId %s streamType: %d idSize: %d isCap: %d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), (*iter)->type_, (*iter)->streamIds_.size(),
            (*iter)->isCapture_ ? 1 : 0);
        ret = dataProcess_[(*iter)->type_]->StartCapture(*iter);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput StartCapture ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::StopCapture(std::vector<int>& streamIds, bool& isAllStop)
{
    DHLOGI("DCameraSourceInput StopCapture devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->StopCapture(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput StopCapture continue ret: %d, devId: %s, dhId: %s", ret,
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
            DHLOGE("DCameraSourceInput StopCapture snapshot ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::OpenChannel(std::vector<DCameraIndex>& indexs)
{
    DHLOGI("DCameraSourceInput OpenChannel devId %s dhId %s continue state: %d, snapshot state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), channelState_[CONTINUOUS_FRAME],
        channelState_[SNAPSHOT_FRAME]);
    if (channelState_[CONTINUOUS_FRAME] == DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = DCAMERA_OK;
        DcameraStartAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
        ret = channels_[CONTINUOUS_FRAME]->CreateSession(indexs, CONTINUE_SESSION_FLAG, DCAMERA_SESSION_MODE_VIDEO,
            listeners_[CONTINUOUS_FRAME]);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput CreateSession continue failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
            return ret;
        }

        ret = channels_[CONTINUOUS_FRAME]->OpenSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput OpenChannel continue stream failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_CONTINUE, DCAMERA_OPEN_DATA_CONTINUE_TASKID);
            return ret;
        }
    }

    if (channelState_[SNAPSHOT_FRAME] == DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        DcameraStartAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
        int32_t ret = DCAMERA_OK;
        ret = channels_[SNAPSHOT_FRAME]->CreateSession(indexs, SNAP_SHOT_SESSION_FLAG, DCAMERA_SESSION_MODE_JPEG,
            listeners_[SNAPSHOT_FRAME]);
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput Init CreateSession snapshot failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
            return ret;
        }

        ret = channels_[SNAPSHOT_FRAME]->OpenSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput OpenChannel snapshot stream failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            DcameraFinishAsyncTrace(DCAMERA_OPEN_DATA_SNAPSHOT, DCAMERA_OPEN_DATA_SNAPSHOT_TASKID);
            return ret;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::CloseChannel()
{
    DHLOGI("DCameraSourceInput CloseChannel devId %s dhId %s continue state: %d, snapshot state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), channelState_[CONTINUOUS_FRAME],
        channelState_[SNAPSHOT_FRAME]);
    if (channelState_[CONTINUOUS_FRAME] != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = DCAMERA_OK;
        ret = channels_[CONTINUOUS_FRAME]->CloseSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput CloseChannel continue stream failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
        channelState_[CONTINUOUS_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;

        ret = channels_[CONTINUOUS_FRAME]->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSourceInput release continue session failed: %d devId %s dhId %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
    }

    if (channelState_[SNAPSHOT_FRAME] != DCAMERA_CHANNEL_STATE_DISCONNECTED) {
        int32_t ret = DCAMERA_OK;
        ret = channels_[SNAPSHOT_FRAME]->CloseSession();
        if (ret != DCAMERA_OK) {
            DHLOGE("DCameraSourceInput CloseChannel snapshot stream failed ret: %d, devId: %s, dhId: %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
        channelState_[SNAPSHOT_FRAME] = DCAMERA_CHANNEL_STATE_DISCONNECTED;

        ret = channels_[SNAPSHOT_FRAME]->ReleaseSession();
        if (ret != DCAMERA_OK) {
            DHLOGI("DCameraSourceInput release snapshot session failed: %d devId %s dhId %s", ret,
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::Init()
{
    DHLOGI("DCameraSourceInput Init devId %s dhId %s", GetAnonyString(devId_).c_str(),
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
    DHLOGI("DCameraSourceInput Init devId end %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::UnInit()
{
    DHLOGI("DCameraSourceInput UnInit devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    channels_.clear();
    listeners_.clear();
    dataProcess_.clear();
    channelState_.clear();
    isInit = false;
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    return DCAMERA_OK;
}

void DCameraSourceInput::OnSessionState(DCStreamType streamType, int32_t state)
{
    DHLOGI("DCameraSourceInput OnSessionState devId: %s, dhId: %s, streamType: %d, state: %d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType, state);
    channelState_[streamType] = (DCameraChannelState)state;
    switch (state) {
        case DCAMERA_CHANNEL_STATE_DISCONNECTED: {
            FinshFrameAsyncTrace(streamType);
            DHLOGI("DCameraSourceDev PostTask CloseSession Input OnClose devId %s dhId %s",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
            DCameraIndex camIndex(devId_, dhId_);
            DCameraSourceEvent event(*this, DCAMERA_EVENT_CLOSE, camIndex);
            eventBus_->PostEvent<DCameraSourceEvent>(event);
            std::shared_ptr<DCameraEvent> camEvent = std::make_shared<DCameraEvent>();
            camEvent->eventType_ = DCAMERA_MESSAGE;
            camEvent->eventResult_ = DCAMERA_EVENT_CHANNEL_DISCONNECTED;
            DCameraSourceEvent srcEvent(*this, DCAMERA_EVENT_NOFIFY, camEvent);
            eventBus_->PostEvent<DCameraSourceEvent>(srcEvent);
            break;
        }
        case DCAMERA_CHANNEL_STATE_CONNECTED: {
            FinshFrameAsyncTrace(streamType);
            DHLOGI("DCameraSourceInput OnSessionState state %d", state);
            break;
        }
        default: {
            DHLOGI("DCameraSourceInput OnSessionState state %d", state);
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
    DHLOGI("DCameraSourceInput OnSessionError devId: %s, dhId: %s, eventType: %d, eventReason: %d, detail %s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), eventType, eventReason, detail.c_str());
}

void DCameraSourceInput::OnDataReceived(DCStreamType streamType, std::vector<std::shared_ptr<DataBuffer>>& buffers)
{
    int32_t ret = dataProcess_[streamType]->FeedStream(buffers);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput OnDataReceived FeedStream %d stream failed ret: %d, devId: %s, dhId: %s", streamType,
            ret, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    }
}

int32_t DCameraSourceInput::ReleaseAllStreams()
{
    DHLOGI("DCameraSourceInput ReleaseAllStreams devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    std::vector<int> continueStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(continueStreamIds);
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->ReleaseStreams(continueStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseAllStreams continue stream ReleaseStreams ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<int> snapStreamIds;
    dataProcess_[SNAPSHOT_FRAME]->GetAllStreamIds(snapStreamIds);
    ret = dataProcess_[SNAPSHOT_FRAME]->ReleaseStreams(snapStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput ReleaseAllStreams snapshot stream ReleaseStreams ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceInput::StopAllCapture()
{
    DHLOGI("DCameraSourceInput StopAllCapture devId %s dhId %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    std::vector<int> continueStreamIds;
    dataProcess_[CONTINUOUS_FRAME]->GetAllStreamIds(continueStreamIds);
    int32_t ret = dataProcess_[CONTINUOUS_FRAME]->StopCapture(continueStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput StopAllCapture continue ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }

    std::vector<int> snapStreamIds;
    dataProcess_[SNAPSHOT_FRAME]->GetAllStreamIds(snapStreamIds);
    ret = dataProcess_[SNAPSHOT_FRAME]->StopCapture(snapStreamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraSourceInput StopAllCapture snapshot ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return ret;
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
