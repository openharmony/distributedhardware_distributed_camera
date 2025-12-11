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

#include "dcamera_stream_data_process_producer.h"

#include <chrono>
#include <securec.h>

#include "anonymous_string.h"
#include "dcamera_buffer_handle.h"
#include "dcamera_hidumper.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include <sys/prctl.h>
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {
DCameraStreamDataProcessProducer::DCameraStreamDataProcessProducer(std::string devId, std::string dhId,
    int32_t streamId, DCStreamType streamType)
    : devId_(devId), dhId_(dhId), streamId_(streamId), streamType_(streamType), eventHandler_(nullptr),
    camHdiProvider_(nullptr), workModeParam_(-1, 0, 0, false)
{
    DHLOGI("DCameraStreamDataProcessProducer Constructor devId %{public}s dhId %{public}s streamType: %{public}d "
        "streamId: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    state_ = DCAMERA_PRODUCER_STATE_STOP;
    interval_ = DCAMERA_PRODUCER_ONE_MINUTE_MS / DCAMERA_PRODUCER_FPS_DEFAULT;
    photoCount_ = COUNT_INIT_NUM;
    syncRunning_.store(false);
    isFirstFrame_.store(true);
}

DCameraStreamDataProcessProducer::~DCameraStreamDataProcessProducer()
{
    DHLOGI("DCameraStreamDataProcessProducer Destructor devId %{public}s dhId %{public}s state: %{public}d streamType"
        ": %{public}d streamId: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), state_,
        streamType_, streamId_);
    if (state_ == DCAMERA_PRODUCER_STATE_START) {
        Stop();
    }
}

void DCameraStreamDataProcessProducer::Start()
{
    DHLOGI("DCameraStreamDataProcessProducer Start producer devId: %{public}s dhId: %{public}s streamType: %{public}d "
        "streamId: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_);
    camHdiProvider_ = IDCameraProvider::Get(HDF_DCAMERA_EXT_SERVICE);
    if (camHdiProvider_ == nullptr) {
        DHLOGE("camHdiProvider_ is null.");
    }
    state_ = DCAMERA_PRODUCER_STATE_START;
    if (streamType_ == CONTINUOUS_FRAME) {
        eventThread_ = std::thread([this]() { this->StartEvent(); });
        std::unique_lock<std::mutex> lock(eventMutex_);
        eventCon_.wait(lock, [this] {
            return eventHandler_ != nullptr;
        });
        smoother_ = std::make_unique<DCameraFeedingSmoother>();
        smootherListener_ = std::make_shared<FeedingSmootherListener>(shared_from_this());
        smoother_->RegisterListener(smootherListener_);
        smoother_->StartSmooth();

        // Start the audio and video synchronization thread
        syncRunning_.store(true);
        syncThread_ = std::thread([this]() { this->SyncVideoThread(); });
    } else {
        producerThread_ = std::thread([this]() { this->LooperSnapShot(); });
    }
}

void DCameraStreamDataProcessProducer::Stop()
{
    DHLOGI("DCameraStreamDataProcessProducer Stop devId: %{public}s dhId: %{public}s streamType: %{public}d "
        "streamId: %{public}d state: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        streamType_, streamId_, state_);
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        state_ = DCAMERA_PRODUCER_STATE_STOP;
    }
    if (streamType_ == CONTINUOUS_FRAME) {
        if (smoother_ != nullptr) {
            smoother_->StopSmooth();
            smoother_ = nullptr;
        }
        smootherListener_ = nullptr;
        {
            std::lock_guard<std::mutex> lock(eventMutex_);
            if ((eventHandler_ != nullptr) && (eventHandler_->GetEventRunner() != nullptr)) {
                eventHandler_->GetEventRunner()->Stop();
            }
            eventThread_.join();
            eventHandler_ = nullptr;
        }
        // Stop the audio and video synchronization thread
        if (syncMem_ != nullptr) {
            syncMem_->UnmapAshmem();
            syncMem_->CloseAshmem();
            syncMem_ = nullptr;
        }
        syncRunning_.store(false);
        syncBufferCond_.notify_all();
        if (syncThread_.joinable()) {
            syncThread_.join();
        }
    } else {
        producerCon_.notify_one();
        producerThread_.join();
    }
    camHdiProvider_ = nullptr;
    DHLOGI("DCameraStreamDataProcessProducer Stop end devId: %{public}s dhId: %{public}s streamType: %{public}d "
        "streamId: %{public}d state: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(),
        streamType_, streamId_, state_);
}

void DCameraStreamDataProcessProducer::FeedStream(const std::shared_ptr<DataBuffer>& buffer)
{
    CHECK_AND_RETURN_LOG(buffer == nullptr, "buffer is nullptr.");
    buffer->frameInfo_.timePonit.startSmooth = GetNowTimeStampUs();
    {
        std::lock_guard<std::mutex> lock(bufferMutex_);
        uint64_t buffersSize = static_cast<uint64_t>(buffer->Size());
        DHLOGD("DCameraStreamDataProcessProducer FeedStream devId %{public}s dhId %{public}s streamId: %{public}d "
            "streamType: %{public}d streamSize: %{public}" PRIu64, GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str(), streamId_, streamType_, buffersSize);
        if (buffers_.size() >= DCAMERA_PRODUCER_MAX_BUFFER_SIZE) {
            buffersSize = static_cast<uint64_t>(buffer->Size());
            DHLOGD("DCameraStreamDataProcessProducer FeedStream OverSize devId %{public}s dhId %{public}s streamType: "
                "%{public}d streamSize: %{public}" PRIu64, GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str(), streamType_, buffersSize);
            buffers_.pop_front();
        }
        if (streamType_ == SNAPSHOT_FRAME) {
            buffers_.push_back(buffer);
            producerCon_.notify_one();
        }
    }
    CHECK_AND_RETURN_LOG(smoother_ == nullptr, "smoother_ is null.");
    if (streamType_ == CONTINUOUS_FRAME) {
        smoother_->PushData(buffer);
    }
}

void DCameraStreamDataProcessProducer::StartEvent()
{
    prctl(PR_SET_NAME, SOURCE_START_EVENT.c_str());
    auto runner = AppExecFwk::EventRunner::Create(false);
    {
        std::lock_guard<std::mutex> lock(eventMutex_);
        eventHandler_ = std::make_shared<AppExecFwk::EventHandler>(runner);
    }
    eventCon_.notify_one();
    runner->Run();
}

void DCameraStreamDataProcessProducer::LooperSnapShot()
{
    std::string name = PRODUCER + std::to_string(streamType_);
    prctl(PR_SET_NAME, name.c_str());
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;
    while (state_ == DCAMERA_PRODUCER_STATE_START) {
        std::shared_ptr<DataBuffer> buffer = nullptr;
        {
            std::unique_lock<std::mutex> lock(bufferMutex_);
            producerCon_.wait(lock, [this] {
                return (!buffers_.empty() || state_ == DCAMERA_PRODUCER_STATE_STOP);
            });
            if (state_ == DCAMERA_PRODUCER_STATE_STOP) {
                continue;
            }
            if (!buffers_.empty()) {
                DHLOGI("LooperSnapShot producer get buffer devId: %{public}s dhId: %{public}s streamType: %{public}d "
                    "streamId: %{public}d state: %{public}d",
                    GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
                buffer = buffers_.front();
            }
        }
        if (buffer == nullptr) {
            DHLOGI("LooperSnapShot producer get buffer failed devId: %{public}s dhId: %{public}s streamType:"
                " %{public}d streamId: %{public}d state: %{public}d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
            continue;
        }
#ifdef DUMP_DCAMERA_FILE
    std::string name =
        "SourceCapture_streamId(" + std::to_string(streamId_) + ")_" + std::to_string(photoCount_++) + ".jpg";
    if (DcameraHidumper::GetInstance().GetDumpFlag() && (IsUnderDumpMaxSize(DUMP_PHOTO_PATH, name) == DCAMERA_OK)) {
        DumpBufferToFile(DUMP_PHOTO_PATH, name, buffer->Data(), buffer->Size());
    }
#endif
        int32_t ret = FeedStreamToDriver(dhBase, buffer);
        if (ret != DCAMERA_OK) {
            std::this_thread::sleep_for(std::chrono::milliseconds(DCAMERA_PRODUCER_RETRY_SLEEP_MS));
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(bufferMutex_);
            buffers_.pop_front();
        }
    }
    DHLOGI("LooperSnapShot producer end devId: %s dhId: %s streamType: %{public}d streamId: %{public}d state: "
        "%{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamType_, streamId_, state_);
}

int32_t DCameraStreamDataProcessProducer::FeedStreamToDriver(const DHBase& dhBase,
    const std::shared_ptr<DataBuffer>& buffer)
{
    if (camHdiProvider_ == nullptr) {
        DHLOGI("camHdiProvider is nullptr");
        return DCAMERA_BAD_VALUE;
    }
    DCameraBuffer sharedMemory;
    int32_t ret = camHdiProvider_->AcquireBuffer(dhBase, streamId_, sharedMemory);
    if (ret != SUCCESS) {
        DHLOGE("AcquireBuffer devId: %{public}s dhId: %{public}s streamId: %{public}d ret: %{public}d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }
    do {
        ret = CheckSharedMemory(sharedMemory, buffer);
        if (ret != DCAMERA_OK) {
            DHLOGE("CheckSharedMemory failed devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str());
            break;
        }
        sharedMemory.bufferHandle_->GetBufferHandle()->virAddr =
            DCameraMemoryMap(sharedMemory.bufferHandle_->GetBufferHandle());
        if (sharedMemory.bufferHandle_->GetBufferHandle()->virAddr == nullptr) {
            DHLOGE("mmap failed devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
                GetAnonyString(dhId_).c_str());
            break;
        }
        ret = memcpy_s(sharedMemory.bufferHandle_->GetBufferHandle()->virAddr, sharedMemory.size_, buffer->Data(),
            buffer->Size());
        if (ret != EOK) {
            DHLOGE("memcpy_s devId: %s dhId: %s streamId: %{public}d bufSize: %zu, addressSize: %{public}d",
                GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_,
                buffer->Size(), sharedMemory.size_);
            break;
        }
        sharedMemory.size_ = buffer->Size();
    } while (0);
    ret = camHdiProvider_->ShutterBuffer(dhBase, streamId_, sharedMemory);
    if (sharedMemory.bufferHandle_ != nullptr) {
        DCameraMemoryUnmap(sharedMemory.bufferHandle_->GetBufferHandle());
    }
    if (ret != SUCCESS) {
        DHLOGE("ShutterBuffer devId: %{public}s dhId: %{public}s streamId: %{public}d ret: %{public}d",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_, ret);
        return DCAMERA_BAD_OPERATE;
    }
    return ret;
}

int32_t DCameraStreamDataProcessProducer::CheckSharedMemory(const DCameraBuffer& sharedMemory,
    const std::shared_ptr<DataBuffer>& buffer)
{
    if (sharedMemory.bufferHandle_ == nullptr || sharedMemory.bufferHandle_->GetBufferHandle() == nullptr) {
        DHLOGE("bufferHandle read failed devId: %{public}s dhId: %{public}s", GetAnonyString(devId_).c_str(),
            GetAnonyString(dhId_).c_str());
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    if (buffer->Size() > sharedMemory.size_) {
        uint64_t buffersSize = static_cast<uint64_t>(buffer->Size());
        DHLOGE("sharedMemory devId: %{public}s dhId: %{public}s streamId: %{public}d bufSize: %{public}" PRIu64
            ", addressSize: %{public}d", GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str(), streamId_,
            buffersSize, sharedMemory.size_);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    return DCAMERA_OK;
}

void DCameraStreamDataProcessProducer::OnSmoothFinished(const std::shared_ptr<IFeedableData>& data)
{
    std::shared_ptr<DataBuffer> buffer = std::reinterpret_pointer_cast<DataBuffer>(data);
    CHECK_AND_RETURN_LOG(buffer == nullptr, "buffer is nullptr.");
    DHBase dhBase;
    dhBase.deviceId_ = devId_;
    dhBase.dhId_ = dhId_;
#ifdef DUMP_DCAMERA_FILE
    if (DcameraHidumper::GetInstance().GetDumpFlag() &&
        (IsUnderDumpMaxSize(DUMP_PATH, TO_DISPLAY) == DCAMERA_OK)) {
        DumpBufferToFile(DUMP_PATH, TO_DISPLAY, buffer->Data(), buffer->Size());
    }
#endif
    {
        // Check if audio-video synchronization is enabled
        std::lock_guard<std::mutex> lock(workModeParamMtx_);
        DHLOGD("OnSmoothFinished rawTime: %{public}" PRIu64 ", isAVsync: %{public}d",
            buffer->frameInfo_.rawTime, workModeParam_.isAVsync);
        if (workModeParam_.isAVsync) {
            WritePtsAndAddBuffer(buffer);
            return;
        }
    }
    auto feedFunc = [this, dhBase, buffer]() {
        FeedStreamToDriver(dhBase, buffer);
    };
    std::lock_guard<std::mutex> lock(eventMutex_);
    if (eventHandler_ != nullptr) {
        eventHandler_->PostTask(feedFunc);
    }
}

void DCameraStreamDataProcessProducer::WritePtsAndAddBuffer(const std::shared_ptr<DataBuffer>& buffer)
{
    bool ret = false;
    if (syncMem_ == nullptr) {
        syncMem_ = sptr<Ashmem>(new (std::nothrow) Ashmem(workModeParam_.fd, workModeParam_.sharedMemLen));
        CHECK_AND_RETURN_LOG(syncMem_ == nullptr, "SyncVideoFrame: syncMem_ is nullptr");

        ret = syncMem_->MapReadAndWriteAshmem();
        CHECK_AND_RETURN_LOG(!ret, "SyncVideoFrame: MapReadAndWriteAshmem failed");
    }
    auto syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
    SyncSharedData *readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    CHECK_AND_RETURN_LOG(readSyncSharedData == nullptr, "read SyncData failed");
    // get voliate lock
    while (!readSyncSharedData->lock) {
        DHLOGI("readSyncSharedData->lock is false");
        syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
        readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    }

    readSyncSharedData->lock = 0;
    ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_LOG(!ret, "write sync data failed!");
    readSyncSharedData->video_current_pts = static_cast<uint64_t>(buffer->frameInfo_.rawTime);
    readSyncSharedData->video_update_clock = static_cast<uint64_t>(GetNowTimeStampUs());
    readSyncSharedData->reset = false;
    readSyncSharedData->lock = 1;
    ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_LOG(!ret, "write sync data failed!");
    std::lock_guard<std::mutex> lock(syncBufferMutex_);
    if (syncBufferQueue_.size() >= DCAMERA_MAX_SYNC_BUFFER_SIZE) {
        DHLOGI("Sync buffer full, drop oldest frame, streamId: %{public}d", streamId_);
        syncBufferQueue_.pop_front();
    }
    syncBufferQueue_.push_back(buffer);
    syncBufferCond_.notify_one(); // Notify the synchronization thread to process
    return;
}

void DCameraStreamDataProcessProducer::SyncVideoThread()
{
    DHLOGI("SyncVideoThread started for streamId: %{public}d", streamId_);
    const std::chrono::milliseconds FRAME_INTERVAL(DCAMERA_SYNC_TIME_INTERVAL); // 33ms per frame
    std::chrono::steady_clock::time_point nextScheduleTime;

    while (syncRunning_.load()) {
        std::shared_ptr<DataBuffer> buffer = nullptr;

        // Wait for video frames in the sync queue
        bool shouldBreak = WaitForVideoFrame(buffer);
        if (shouldBreak) {
            break;
        }

        if (buffer == nullptr) {
            continue;
        }

        // Record the start time when the first frame is sent
        if (isFirstFrame_.load()) {
            nextScheduleTime = std::chrono::steady_clock::now();
            isFirstFrame_.store(false);
            DHLOGI("First frame timestamp recorded, streamId: %{public}d", streamId_);
        }

        // Synchronization frame judgment
        uint64_t videoPtsUs = static_cast<uint64_t>(buffer->frameInfo_.rawTime);
        int32_t syncResult = SyncVideoFrame(videoPtsUs);
        if (syncResult == 1) {
            // Synchronization successful, sending video frame
            DHBase dhBase;
            dhBase.deviceId_ = devId_;
            dhBase.dhId_ = dhId_;

            int32_t ret = FeedStreamToDriver(dhBase, buffer);
            if (ret != DCAMERA_OK) {
                DHLOGE("FeedStreamToDriver failed, ret: %{public}d, streamId: %{public}d", ret, streamId_);
            } else {
                // FeedStreamToDriver success video_update_clock
                UpdateVideoClock(videoPtsUs);
            }
            nextScheduleTime += FRAME_INTERVAL; // Perform timed scheduling after successful transmission
            std::this_thread::sleep_until(nextScheduleTime);
        } else if (syncResult == 0) {
            {
                std::lock_guard<std::mutex> lock(syncBufferMutex_);
                syncBufferQueue_.push_front(buffer);
            }

            // Perform timed scheduling after successful transmission
            nextScheduleTime += FRAME_INTERVAL;
            std::this_thread::sleep_until(nextScheduleTime);
        } else {
            // Video frame is too late, discard directly and process next frame immediately
            continue;
        }
    }

    DHLOGI("SyncVideoThread exited for streamId: %{public}d", streamId_);
}

bool DCameraStreamDataProcessProducer::WaitForVideoFrame(std::shared_ptr<DataBuffer>& buffer)
{
    std::unique_lock<std::mutex> lock(syncBufferMutex_);
    syncBufferCond_.wait(lock, [this] {
        return !syncBufferQueue_.empty() || !syncRunning_.load();
    });

    if (!syncRunning_.load()) {
        return true; // exit
    }

    if (!syncBufferQueue_.empty() && syncBufferQueue_.size() >= DCAMERA_SYNC_WATERMARK) {
        buffer = syncBufferQueue_.front();
        syncBufferQueue_.pop_front();
    }
    
    return false; // don't exit
}

int32_t DCameraStreamDataProcessProducer::SyncVideoFrame(uint64_t videoPtsUs)
{
    int64_t videoPts = static_cast<int64_t>(videoPtsUs / DCAMERA_NS_TO_MS); // us -> ms
    int64_t audioPts = 0; // get audio timestamp from shared memory
    int64_t audioUpdatePts = 0; // audio update time
    float audioSpeed = 1.0f; // audio playback speed factor
    bool ret = false;
    CHECK_AND_RETURN_RET_LOG(syncMem_ == nullptr, DCAMERA_BAD_VALUE, "SyncVideoFrame: syncMem_ is nullptr.");
    auto syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
    SyncSharedData *readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    CHECK_AND_RETURN_RET_LOG(readSyncSharedData == nullptr, DCAMERA_BAD_VALUE, "read SyncData failed");
    // get voliate lock
    while (!readSyncSharedData->lock) {
        DHLOGI("readSyncSharedData->lock is false");
        syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
        readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    }
    readSyncSharedData->lock = 0;
    ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_RET_LOG(!ret, DCAMERA_BAD_VALUE, "write sync data failed!");
    audioPts = static_cast<int64_t>(readSyncSharedData->audio_current_pts / DCAMERA_US_TO_MS); // us -> ms
    audioUpdatePts = static_cast<int64_t>(readSyncSharedData->audio_update_clock / DCAMERA_US_TO_MS); // us -> ms
    audioSpeed = readSyncSharedData->audio_speed;
    readSyncSharedData->lock = 1;
    ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_RET_LOG(!ret, DCAMERA_BAD_VALUE, "write sync data failed!");

    int64_t currentTime = static_cast<int64_t>(GetNowTimeStampMs());
    int64_t estimatedPts = static_cast<int64_t>(audioPts + (currentTime - audioUpdatePts) * audioSpeed);

    int64_t diff = static_cast<int64_t>(estimatedPts - videoPts); // calculate audio-video time difference
    DHLOGD("SyncCheck: videoPts=%{public}" PRIu64 ", estimatedPts=%{public}" PRIu64 ", diff=%{public}" PRId64 "ms",
           videoPts, estimatedPts, diff);

    if (diff > DCAMERA_TIME_DIFF_MAX) {
        int32_t queueSize = static_cast<int32_t>(syncBufferQueue_.size());
        DHLOGI("SyncVideoFrame::late (diff=%{public}" PRId64 "ms, videoPts=%{public}" PRId64
            "ms, queueSize:%{public}d), skip this frame.", diff, videoPts, queueSize);
        // Drop if there is still data in the queue, play the last frame directly
        return (queueSize > 0) ? -1 : 1;
    } else if (diff < DCAMERA_TIME_DIFF_MIN) {
        DHLOGI("SyncVideoFrame::early (diff=%{public}" PRId64 "ms, videoPts=%{public}" PRId64 "ms),"
            " wait for next scheduling.", diff, videoPts);
        return 0;
    } else {
        DHLOGD("SyncVideoFrame::Video frame in sync range, will be sent. diff=%{public}" PRId64 "ms", diff);
        return 1;
    }
}

void DCameraStreamDataProcessProducer::UpdateVideoClock(uint64_t videoPtsUs)
{
    {
        std::lock_guard<std::mutex> lock(workModeParamMtx_);
        if (!workModeParam_.isAVsync) {
            return;
        }
    }

    CHECK_AND_RETURN_LOG(syncMem_ == nullptr, "UpdateVideoClock: syncMem_ is nullptr.");

    auto syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
    SyncSharedData *readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    CHECK_AND_RETURN_LOG(readSyncSharedData == nullptr, "read SyncData failed");
    while (!readSyncSharedData->lock) {
        syncData = syncMem_->ReadFromAshmem(workModeParam_.sharedMemLen, 0);
        readSyncSharedData = reinterpret_cast<SyncSharedData *>(const_cast<void *>(syncData));
    }
    readSyncSharedData->lock = 0;
    bool ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_LOG(!ret, "write sync data failed!");
    readSyncSharedData->video_current_pts = videoPtsUs;
    readSyncSharedData->video_update_clock = static_cast<uint64_t>(GetNowTimeStampUs());
    readSyncSharedData->reset = false;
    readSyncSharedData->lock = 1;
    ret = syncMem_->WriteToAshmem(static_cast<void *>(readSyncSharedData), sizeof(SyncSharedData), 0);
    CHECK_AND_RETURN_LOG(!ret, "write sync data failed!");
    DHLOGD("Video update clock updated to: %" PRIu64 " us", readSyncSharedData->video_update_clock);
}

void DCameraStreamDataProcessProducer::UpdateProducerWorkMode(const WorkModeParam& param)
{
    DHLOGI("begin update producer workmode");
    if (!param.isAVsync) {
        if (syncMem_ != nullptr) {
            syncMem_->UnmapAshmem();
            syncMem_->CloseAshmem();
            syncMem_ = nullptr;
            DHLOGI("syncMem_ release success.");
        }
    }
    {
        std::lock_guard<std::mutex> lock(workModeParamMtx_);
        workModeParam_ = param;
    }
    DHLOGI("update producer workmode success");
}
} // namespace DistributedHardware
} // namespace OHOS
