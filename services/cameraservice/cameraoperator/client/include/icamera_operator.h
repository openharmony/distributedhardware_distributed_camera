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

#ifndef OHOS_ICAMERA_OPERATOR_H
#define OHOS_ICAMERA_OPERATOR_H

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "data_buffer.h"
#include "dcamera_capture_info_cmd.h"
#include "dcamera_event_cmd.h"
#include "v1_1/dcamera_types.h"

#include "distributed_camera_errno.h"
#include "surface.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;
class StateCallback {
public:
    StateCallback() = default;
    virtual ~StateCallback() = default;

    virtual void OnStateChanged(std::shared_ptr<DCameraEvent>& event) = 0;
    virtual void OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings) = 0;
};

class ResultCallback {
public:
    ResultCallback() = default;
    virtual ~ResultCallback() = default;

    virtual void OnPhotoResult(std::shared_ptr<DataBuffer>& buffer) = 0;
    virtual void OnVideoResult(std::shared_ptr<DataBuffer>& buffer) = 0;
};

class ICameraOperator {
public:
    ICameraOperator() = default;
    virtual ~ICameraOperator() = default;

    virtual int32_t Init() = 0;
    virtual int32_t UnInit() = 0;
    virtual int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings) = 0;
    virtual int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
        sptr<Surface>& surface, int32_t sceneMode) = 0;
    virtual int32_t PrepareCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode)
    {
        return DCAMERA_BAD_OPERATE;
    }
    virtual int32_t CommitCapture(sptr<Surface>& surface)
    {
        return DCAMERA_BAD_OPERATE;
    }
    virtual int32_t StopCapture() = 0;
    virtual int32_t SetStateCallback(std::shared_ptr<StateCallback>& callback) = 0;
    virtual int32_t SetResultCallback(std::shared_ptr<ResultCallback>& callback) = 0;
    virtual int32_t PauseCapture() = 0;
    virtual int32_t ResumeCapture() = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_ICAMERA_OPERATOR_H
