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

#ifndef OHOS_DCAMERA_SOURCE_INPUT_H
#define OHOS_DCAMERA_SOURCE_INPUT_H

#include "icamera_channel.h"
#include "icamera_channel_listener.h"
#include "icamera_input.h"
#include "icamera_source_data_process.h"

#include "dcamera_source_dev.h"
#include "distributed_camera_errno.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceInput : public ICameraInput,
    public std::enable_shared_from_this<DCameraSourceInput> {
public:
    DCameraSourceInput(std::string devId, std::string dhId, std::shared_ptr<DCameraSourceDev>& camDev);
    ~DCameraSourceInput() override;

    int32_t ConfigStreams(std::vector<std::shared_ptr<DCStreamInfo>>& streamInfos) override;
    int32_t ReleaseStreams(std::vector<int>& streamIds, bool& isAllRelease) override;
    int32_t ReleaseAllStreams() override;
    int32_t StartCapture(std::vector<std::shared_ptr<DCCaptureInfo>>& captureInfos) override;
    int32_t StopCapture(std::vector<int>& streamIds, bool& isAllStop) override;
    int32_t StopAllCapture() override;
    int32_t OpenChannel(std::vector<DCameraIndex>& indexs) override;
    int32_t CloseChannel() override;
    int32_t Init() override;
    int32_t UnInit() override;
    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings) override;
    int32_t UpdateWorkMode(const WorkModeParam& param) override;

    void OnSessionState(DCStreamType streamType, int32_t state);
    void OnSessionError(DCStreamType streamType, int32_t eventType, int32_t eventReason, std::string detail);
    void OnDataReceived(DCStreamType streamType, std::vector<std::shared_ptr<DataBuffer>>& buffers);

private:
    void FinshFrameAsyncTrace(DCStreamType streamType);
    void PostChannelDisconnectedEvent();
    int32_t EstablishContinuousFrameSession(std::vector<DCameraIndex>& indexs);
    int32_t EstablishSnapshotFrameSession(std::vector<DCameraIndex>& indexs);
    int32_t WaitForOpenChannelCompletion(bool needWait);

private:
    std::map<DCStreamType, std::shared_ptr<ICameraChannel>> channels_;
    std::map<DCStreamType, std::shared_ptr<ICameraChannelListener>> listeners_;
    std::map<DCStreamType, std::shared_ptr<ICameraSourceDataProcess>> dataProcess_;
    std::map<DCStreamType, DCameraChannelState> channelState_;
    std::string devId_;
    std::string dhId_;
    std::weak_ptr<DCameraSourceDev> camDev_;

    bool isInit = false;

    static constexpr uint8_t CHANNEL_REL_SECONDS = 5;
    std::atomic<bool> isChannelConnected_ = false;
    std::mutex channelMtx_;
    std::condition_variable channelCond_;

    static constexpr std::chrono::seconds TIMEOUT_3_SEC = std::chrono::seconds(3);
    std::atomic<bool> isOpenChannelFinished_ = false;
    std::mutex isOpenChannelMtx_;
    std::condition_variable isOpenChannelCond_;
    std::atomic<int32_t> continuousFrameResult_ = DCAMERA_OK;
    std::atomic<int32_t> snapshotFrameResult_ = DCAMERA_OK;
    std::shared_ptr<AppExecFwk::EventRunner> runner_ = AppExecFwk::EventRunner::Create(true);
    std::shared_ptr<AppExecFwk::EventHandler> handler_ = std::make_shared<AppExecFwk::EventHandler>(runner_);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOURCE_INPUT_H
