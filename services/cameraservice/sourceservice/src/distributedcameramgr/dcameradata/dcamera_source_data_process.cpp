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

#include "dcamera_source_data_process.h"

#include "anonymous_string.h"
#include "dcamera_hitrace_adapter.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraSourceDataProcess::DCameraSourceDataProcess(std::string devId, std::string dhId, DCStreamType streamType)
    : devId_(devId), dhId_(dhId), streamType_(streamType), isFirstContStream_(true)
{
    DHLOGI("DCameraSourceDataProcess Constructor devId %{public}s dhId %{public}s streamType %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_);
}

DCameraSourceDataProcess::~DCameraSourceDataProcess()
{
    DHLOGI("DCameraSourceDataProcess Destructor devId %{public}s dhId %{public}s streamType %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_);
    streamProcess_.clear();
    streamIds_.clear();
}

int32_t DCameraSourceDataProcess::FeedStream(std::vector<std::shared_ptr<DataBuffer>>& buffers)
{
    if (isFirstContStream_ && streamType_ == CONTINUOUS_FRAME) {
        DcameraFinishAsyncTrace(DCAMERA_CONTINUE_FIRST_FRAME, DCAMERA_CONTINUE_FIRST_FRAME_TASKID);
        isFirstContStream_ = false;
    } else if (streamType_ == SNAPSHOT_FRAME) {
        DcameraFinishAsyncTrace(DCAMERA_SNAPSHOT_FIRST_FRAME, DCAMERA_SNAPSHOT_FIRST_FRAME_TASKID);
    }
    uint64_t buffersSize = static_cast<uint64_t>(buffers.size());
    if (buffers.size() > DCAMERA_MAX_NUM) {
        DHLOGI("DCameraSourceDataProcess FeedStream devId %{public}s dhId %{public}s size: %{public}" PRIu64
            " over flow", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), buffersSize);
        return DCAMERA_BAD_VALUE;
    }

    auto buffer = *(buffers.begin());
    CHECK_AND_RETURN_RET_LOG(buffer == nullptr, DCAMERA_BAD_VALUE, "DCameraSourceDataProcess FeedStream buffer is "
        "nullptr");
    buffersSize = static_cast<uint64_t>(buffer->Size());
    DHLOGD("DCameraSourceDataProcess FeedStream devId %{public}s dhId %{public}s streamType %{public}d streamSize: "
        "%{public}" PRIu64, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, buffersSize);
    std::lock_guard<std::mutex> autoLock(streamMutex_);
    for (auto iter = streamProcess_.begin(); iter != streamProcess_.end(); iter++) {
        (*iter)->FeedStream(buffer);
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDataProcess::ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos)
{
    uint64_t infoSize = static_cast<uint64_t>(streamInfos.size());
    DHLOGI("DCameraSourceDataProcess ConfigStreams devId %{public}s dhId %{public}s streamType %{public}d size "
        "%{public}" PRIu64, GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, infoSize);
    if (streamInfos.empty()) {
        DHLOGI("DCameraSourceDataProcess ConfigStreams is empty");
        return DCAMERA_OK;
    }
    std::map<DCameraStreamConfig, std::set<int>> streamConfigs;
    for (auto iter = streamInfos.begin(); iter != streamInfos.end(); iter++) {
        std::shared_ptr<DCStreamInfo> streamInfo = *iter;
        if (streamInfo == nullptr) {
            DHLOGE("DCameraSourceDataProcess ConfigStreams streamInfo is nullptr");
            continue;
        }
        DCameraStreamConfig streamConfig(streamInfo->width_, streamInfo->height_, streamInfo->format_,
            streamInfo->dataspace_, streamInfo->encodeType_, streamInfo->type_);
        DHLOGI("DCameraSourceDataProcess ConfigStreams devId %{public}s dhId %{public}s, streamId: %{public}d info: "
            "width: %{public}d, height: %{public}d, format: %{public}d, dataspace: %{public}d, encodeType: "
            "%{public}d streamType: %{public}d", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), streamInfo->streamId_, streamConfig.width_, streamConfig.height_,
            streamConfig.format_, streamConfig.dataspace_, streamConfig.encodeType_, streamConfig.type_);
        if (streamConfigs.find(streamConfig) == streamConfigs.end()) {
            std::set<int> streamIdSet;
            streamConfigs.emplace(streamConfig, streamIdSet);
        }
        streamConfigs[streamConfig].insert(streamInfo->streamId_);
        streamIds_.insert(streamInfo->streamId_);
    }

    for (auto iter = streamConfigs.begin(); iter != streamConfigs.end(); iter++) {
        DHLOGI("DCameraSourceDataProcess ConfigStreams devId %{public}s dhId %{public}s, info: width: %{public}d, "
            "height: %{public}d, format: %{public}d, dataspace: %{public}d, encodeType: %{public}d streamType: "
            "%{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), iter->first.width_,
            iter->first.height_, iter->first.format_, iter->first.dataspace_, iter->first.encodeType_,
            iter->first.type_);

        std::shared_ptr<DCameraStreamDataProcess> streamProcess =
            std::make_shared<DCameraStreamDataProcess>(devId_, dhId_, streamType_);
        std::shared_ptr<DCameraStreamConfig> streamConfig =
            std::make_shared<DCameraStreamConfig>(iter->first.width_, iter->first.height_, iter->first.format_,
            iter->first.dataspace_, iter->first.encodeType_, iter->first.type_);
        streamProcess->ConfigStreams(streamConfig, iter->second);

        streamProcess_.push_back(streamProcess);
    }

    return DCAMERA_OK;
}

int32_t DCameraSourceDataProcess::ReleaseStreams(std::vector<int32_t>& streamIds)
{
    DHLOGI("DCameraSourceDataProcess ReleaseStreams devId %{public}s dhId %{public}s streamType: %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_);
    std::lock_guard<std::mutex> autoLock(streamMutex_);
    std::set<int32_t> streamIdSet(streamIds.begin(), streamIds.end());
    auto iter = streamProcess_.begin();
    while (iter != streamProcess_.end()) {
        (*iter)->ReleaseStreams(streamIdSet);
        std::set<int32_t> processStreamIds;
        (*iter)->GetAllStreamIds(processStreamIds);
        if (processStreamIds.empty()) {
            iter = streamProcess_.erase(iter);
        } else {
            iter++;
        }
    }

    std::string strStreams;
    for (auto iterSet = streamIdSet.begin(); iterSet != streamIdSet.end(); iterSet++) {
        strStreams += (std::to_string(*iterSet) + std::string(" "));
        streamIds_.erase(*iterSet);
    }
    uint64_t processSize = static_cast<uint64_t>(streamProcess_.size());
    DHLOGI("DCameraSourceDataProcess ReleaseStreams devId %{public}s dhId %{public}s streamType: %{public}d "
        "streamProcessSize: %{public}" PRIu64" streams: %{public}s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str(), streamType_, processSize, strStreams.c_str());
    return DCAMERA_OK;
}

int32_t DCameraSourceDataProcess::StartCapture(std::shared_ptr<DCCaptureInfo>& captureInfo)
{
    DHLOGI("DCameraSourceDataProcess StartCapture devId %{public}s dhId %{public}s width: %{public}d, height: "
        "%{public}d, format: %{public}d, isCapture: %{public}d, dataspace: %{public}d, encodeType: %{public}d, "
        "streamType: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        captureInfo->width_, captureInfo->height_, captureInfo->format_, captureInfo->isCapture_,
        captureInfo->dataspace_, captureInfo->encodeType_, captureInfo->type_);
    if (streamType_ == CONTINUOUS_FRAME && captureInfo->isCapture_ == true) {
        isFirstContStream_ = true;
    }

    std::shared_ptr<DCameraStreamConfig> streamConfig =
        std::make_shared<DCameraStreamConfig>(captureInfo->width_, captureInfo->height_, captureInfo->format_,
        captureInfo->dataspace_, captureInfo->encodeType_, captureInfo->type_);
    std::set<int32_t> streamIds(captureInfo->streamIds_.begin(), captureInfo->streamIds_.end());
    for (auto iterSet = streamIds.begin(); iterSet != streamIds.end(); iterSet++) {
        DHLOGI("DCameraSourceDataProcess StartCapture devId %{public}s dhId %{public}s StartCapture id: %{public}d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), *iterSet);
    }
    for (auto iter = streamProcess_.begin(); iter != streamProcess_.end(); iter++) {
        (*iter)->StartCapture(streamConfig, streamIds);
    }
    return DCAMERA_OK;
}

int32_t DCameraSourceDataProcess::StopCapture(std::vector<int32_t>& streamIds)
{
    DHLOGI("DCameraSourceDataProcess StopCapture devId %{public}s dhId %{public}s streamType: %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_);
    std::set<int32_t> streamIdSet(streamIds.begin(), streamIds.end());
    for (auto iterSet = streamIdSet.begin(); iterSet != streamIdSet.end(); iterSet++) {
        DHLOGI("DCameraSourceDataProcess StopCapture devId %{public}s dhId %{public}s stream id: %{public}d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), *iterSet);
    }
    for (auto iter = streamProcess_.begin(); iter != streamProcess_.end(); iter++) {
        (*iter)->StopCapture(streamIdSet);
    }
    if ((streamType_ == CONTINUOUS_FRAME) && (GetProducerSize() == 0)) {
        DestroyPipeline();
    }
    return DCAMERA_OK;
}

void DCameraSourceDataProcess::DestroyPipeline()
{
    DHLOGI("DCameraSourceDataProcess DestroyPipeline devId %{public}s dhId %{public}s streamType: %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_);
    for (auto iter = streamProcess_.begin(); iter != streamProcess_.end(); iter++) {
        (*iter)->DestroyPipeline();
    }
}

int32_t DCameraSourceDataProcess::GetProducerSize()
{
    int32_t ret = 0;
    for (auto iter = streamProcess_.begin(); iter != streamProcess_.end(); iter++) {
        std::shared_ptr<DCameraStreamDataProcess> streamDataProcess = *iter;
        int32_t size = streamDataProcess->GetProducerSize();
        ret += size;
    }
    DHLOGI("DCameraSourceDataProcess GetProducerSize devId %{public}s dhId %{public}s size %{public}d",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), ret);
    return ret;
}

void DCameraSourceDataProcess::GetAllStreamIds(std::vector<int32_t>& streamIds)
{
    streamIds.assign(streamIds_.begin(), streamIds_.end());
}

int32_t DCameraSourceDataProcess::UpdateProducerWorkMode(std::vector<int32_t>& streamIds, const WorkModeParam& param)
{
    DHLOGI("DCameraSourceDataProcess UpdateProducerWorkMode devId %{public}s dhId %{public}s",
        GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
    int32_t ret = DCAMERA_OK;
    for (auto iter = streamProcess_.begin(); iter !=  streamProcess_.end(); iter++) {
        int32_t currentRet = (*iter)->UpdateProducerWorkMode(streamIds, param);
        if (currentRet != DCAMERA_OK) {
            DHLOGE("DCameraSourceDataProcess UpdateProducerWorkMode failed, ret:%{public}d", currentRet);
            ret = currentRet;
        }
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS
