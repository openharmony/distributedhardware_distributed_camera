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

#ifndef OHOS_DCAMERA_SINK_DEV_H
#define OHOS_DCAMERA_SINK_DEV_H

#include <cstdint>
#include <memory>
#include <string>

#include "icamera_controller.h"
#include "icamera_sink_access_control.h"
#include "idcamera_sink_callback.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkDev {
public:
    explicit DCameraSinkDev(const std::string& dhId, const sptr<IDCameraSinkCallback> &sinkCallback);
    ~DCameraSinkDev();

    int32_t Init();
    int32_t UnInit();
    int32_t SubscribeLocalHardware(const std::string& parameters);
    int32_t UnsubscribeLocalHardware();
    int32_t StopCapture();
    int32_t ChannelNeg(std::string& channelInfo);
    int32_t GetCameraInfo(std::string& cameraInfo);
    int32_t OpenChannel(std::string& openInfo);
    int32_t CloseChannel();
    std::string GetDhid();
    int32_t PauseDistributedHardware(const std::string &networkId);
    int32_t ResumeDistributedHardware(const std::string &networkId);
    int32_t StopDistributedHardware(const std::string &networkId);
    void SetTokenId(uint64_t token);

private:
    bool isInit_;
    std::string dhId_;
    std::shared_ptr<ICameraController> controller_;
    std::shared_ptr<ICameraSinkAccessControl> accessControl_;
    sptr<IDCameraSinkCallback> sinkCallback_;
    uint64_t tokenId_ = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_DEV_H