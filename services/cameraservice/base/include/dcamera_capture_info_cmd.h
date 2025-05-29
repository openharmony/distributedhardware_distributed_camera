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

#ifndef OHOS_DCAMERA_CAPTURE_INFO_H
#define OHOS_DCAMERA_CAPTURE_INFO_H

#include "distributed_camera_constants.h"
#include "cJSON.h"
#include "v1_1/dcamera_types.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class DCameraCaptureInfo {
public:
    int32_t width_;
    int32_t height_;
    int32_t format_;
    int32_t dataspace_;
    bool isCapture_;
    DCEncodeType encodeType_;
    DCStreamType streamType_;
    std::vector<std::shared_ptr<DCameraSettings>> captureSettings_;
};

class DCameraCaptureInfoCmd {
public:
    std::string type_;
    std::string dhId_;
    std::string command_;
    std::vector<std::shared_ptr<DCameraCaptureInfo>> value_;
    int32_t sceneMode_;
    int32_t userId_;
    uint64_t tokenId_;
    std::string accountId_;

public:
    int32_t Marshal(std::string& jsonStr);
    int32_t Unmarshal(const std::string& jsonStr);

private:
    int32_t UmarshalValue(cJSON* rootValue);
    int32_t UmarshalSettings(cJSON* valueJson, std::shared_ptr<DCameraCaptureInfo>& captureInfo);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_CAPTURE_INFO_H
