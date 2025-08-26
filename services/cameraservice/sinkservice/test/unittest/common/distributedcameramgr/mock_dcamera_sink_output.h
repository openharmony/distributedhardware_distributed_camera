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

#ifndef OHOS_MOCK_DCAMERA_SINK_OUTPUT_H
#define OHOS_MOCK_DCAMERA_SINK_OUTPUT_H

#include "distributed_camera_errno.h"
#include "icamera_operator.h"
#include "icamera_sink_output.h"
#include "iconsumer_surface.h"

namespace OHOS {
namespace DistributedHardware {
extern std::string g_outputStr;
class MockDCameraSinkOutput : public ICameraSinkOutput {
public:
    explicit MockDCameraSinkOutput(const std::string& dhId, const std::shared_ptr<ICameraOperator>& cameraOperator) {}
    ~MockDCameraSinkOutput() override = default;

    int32_t Init() override
    {
        if (g_outputStr == "test026") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t UnInit() override
    {
        if (g_outputStr == "test019") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos) override
    {
        if (g_outputStr == "ParallelStartCapture_EncoderFail_Test" || g_outputStr == "test021") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t StopCapture() override
    {
        return DCAMERA_OK;
    }

    int32_t OpenChannel(std::shared_ptr<DCameraChannelInfo>& info) override
    {
        return DCAMERA_OK;
    }
    int32_t CloseChannel() override
    {
        if (g_outputStr == "test018") {
            return DCAMERA_BAD_VALUE;
        }
        return DCAMERA_OK;
    }

    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier) override
    {
        if (g_outputStr == "GetPropertyFail_Test") {
            return DCAMERA_BAD_VALUE;
        }
        
        if (propertyName == "surface") {
            sptr<IConsumerSurface> consumerSurface = IConsumerSurface::Create("test_surface");
            sptr<IBufferProducer> producer = consumerSurface->GetProducer();
            sptr<Surface> mockSurface = Surface::CreateSurfaceAsProducer(producer);
            propertyCarrier.surface_ = mockSurface;
        }
        
        return DCAMERA_OK;
    }
};
}
}
#endif