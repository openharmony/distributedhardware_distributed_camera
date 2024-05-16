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

#ifndef OHOS_DCAMERA_PROVIDER_CALLBACK_IMPL_H
#define OHOS_DCAMERA_PROVIDER_CALLBACK_IMPL_H

#include "v1_1/id_camera_provider_callback.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class DCameraSourceDev;
class DCameraProviderCallbackImpl : public IDCameraProviderCallback {
public:
    DCameraProviderCallbackImpl(std::string devId, std::string dhId, std::shared_ptr<DCameraSourceDev>& sourceDev);
    ~DCameraProviderCallbackImpl() override;

    int32_t OpenSession(const DHBase& dhBase) override;
    int32_t CloseSession(const DHBase& dhBase) override;
    int32_t ConfigureStreams(const DHBase& dhBase, const std::vector<DCStreamInfo>& streamInfos) override;
    int32_t ReleaseStreams(const DHBase& dhBase, const std::vector<int>& streamIds) override;
    int32_t StartCapture(const DHBase& dhBase, const std::vector<DCCaptureInfo>& captureInfos) override;
    int32_t StopCapture(const DHBase& dhBase, const std::vector<int>& streamIds) override;
    int32_t UpdateSettings(const DHBase& dhBase, const std::vector<DCameraSettings>& settings) override;
    int32_t NotifyEvent(const DHBase& dhBase, const DCameraHDFEvent& event) override;

private:
    bool CheckDHBase(const DHBase& dhBase);
    bool CheckStreamInfo(const DCStreamInfo& stream);
    bool CheckCaptureInfo(const DCCaptureInfo& captureInfo);

    std::string devId_;
    std::string dhId_;
    std::weak_ptr<DCameraSourceDev> sourceDev_;
    const size_t PARAM_MAX_SIZE = 50 * 1024 * 1024;
    const size_t DID_MAX_SIZE = 256;
    const int32_t RESOLUTION_MAX_WIDTH = 10000;
    const int32_t RESOLUTION_MAX_HEIGHT = 10000;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_PROVIDER_CALLBACK_IMPL_H
