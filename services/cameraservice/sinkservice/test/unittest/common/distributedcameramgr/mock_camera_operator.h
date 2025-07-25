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

#ifndef OHOS_MOCK_CAMERA_OPERATOR_H
#define OHOS_MOCK_CAMERA_OPERATOR_H

#include "distributed_camera_errno.h"
#include "icamera_operator.h"
#include <mutex>
#include <condition_variable>

namespace OHOS {
namespace DistributedHardware {
extern std::string g_operatorStr;
class MockCameraOperator : public ICameraOperator {
public:
    explicit MockCameraOperator()
    {
    }

    ~MockCameraOperator()
    {
    }

    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<int> asyncOperationState = {0};
    bool stopCaptureFinished = false;
    const int32_t commitCaptureState = 1;
    const int32_t stopCaptureState = 2;
    void ResetAsyncState()
    {
        asyncOperationState = 0;
        stopCaptureFinished = false;
    }

    int32_t Init()
    {
        if (g_operatorStr == "test026") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t UnInit()
    {
        if (g_operatorStr == "test020") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
    {
        if (g_operatorStr == "test015") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t OpenCamera(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
    {
        return DCAMERA_OK;
    }

    int32_t CloseCamera()
    {
        return DCAMERA_OK;
    }

    int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
        sptr<Surface>& surface, int32_t sceneMode)
    {
        if (g_operatorStr == "test022") {
            return DCAMERA_ALLOC_ERROR;
        } else if (g_operatorStr == "test023") {
            return DCAMERA_DEVICE_BUSY;
        }
        return DCAMERA_OK;
    }

    int32_t PrepareCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode)
    {
        if (g_operatorStr == "test_prepare_fail") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t CommitCapture(sptr<Surface>& surface)
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            asyncOperationState = commitCaptureState;
        }
        cv_.notify_one();
        return DCAMERA_OK;
    }

    int32_t StopCapture()
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            asyncOperationState = stopCaptureState;
            stopCaptureFinished = true;  // for test_011
        }
        cv_.notify_one();
        return DCAMERA_OK;
    }

    int32_t SetStateCallback(std::shared_ptr<StateCallback>& callback)
    {
        return DCAMERA_OK;
    }

    int32_t SetResultCallback(std::shared_ptr<ResultCallback>& callback)
    {
        return DCAMERA_OK;
    }

    int32_t PauseCapture()
    {
        if (g_operatorStr == "test029") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t ResumeCapture()
    {
        if (g_operatorStr == "test030") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_CAMERA_OPERATOR_H
