/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_MOCK_DCAMERA_SINK_DATA_PROCESS_H
#define OHOS_MOCK_DCAMERA_SINK_DATA_PROCESS_H

#include <cstdint>
#include <memory>

#include "event_handler.h"

#include "data_buffer.h"
#include "data_process_listener.h"
#include "dcamera_capture_info_cmd.h"
#include "dcamera_index.h"
#include "icamera_channel.h"
#include "icamera_channel_listener.h"
#include "icamera_sink_data_process.h"

namespace OHOS {
namespace DistributedHardware {
class MockDCameraSinkDataProcess : public ICameraSinkDataProcess,
    public std::enable_shared_from_this<MockDCameraSinkDataProcess> {
public:
    explicit MockDCameraSinkDataProcess(const std::shared_ptr<ICameraChannel>& channel)
    {
    }

    ~MockDCameraSinkDataProcess()
    {
    }

    int32_t StartCapture(std::shared_ptr<DCameraCaptureInfo>& captureInfo)
    {
        return DCAMERA_OK;
    }
    int32_t StopCapture()
    {
        return DCAMERA_OK;
    }
    int32_t FeedStream(std::shared_ptr<DataBuffer>& dataBuffer)
    {
        return DCAMERA_OK;
    }
    void Init()
    {
    }
    void OnProcessedVideoBuffer(const std::shared_ptr<DataBuffer>& videoResult)
    {
    }
    void OnError(DataProcessErrorType errorType)
    {
    }
    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
    {
        return DCAMERA_OK;
    }
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_MOCK_DCAMERA_SINK_DATA_PROCESS_H
