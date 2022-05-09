/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_COLOR_FORMAT_PROCESS_H
#define OHOS_COLOR_FORMAT_PROCESS_H

#include "securec.h"

#include "abstract_data_process.h"
#include "data_buffer.h"
#include "dcamera_pipeline_source.h"
#include "image_common_type.h"

namespace OHOS {
namespace DistributedHardware {
class ColorFormatProcess : public AbstractDataProcess {
public:
    explicit ColorFormatProcess(const std::weak_ptr<DCameraPipelineSource>& callbackPipSource)
        : callbackPipelineSource_(callbackPipSource) {}
    ~ColorFormatProcess();

    int32_t InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
        VideoConfigParams& processedConfig) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers) override;
    void ReleaseProcessNode() override;

private:
    bool IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig);
    int32_t GetImageUnitInfo(ImageUnitInfo& imgInfo, const std::shared_ptr<DataBuffer>& imgBuf);
    bool CheckColorProcessInputInfo(const ImageUnitInfo& srcImgInfo);
    bool CheckColorConvertInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    bool IsCorrectImageUnitInfo(const ImageUnitInfo& imgInfo);
    void SeparateUVPlaneByRow(const uint8_t *srcUVPlane, uint8_t *dstUPlane, uint8_t *dstVPlane,
        int32_t srcHalfWidth);
    int32_t SeparateNV12UVPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    void CombineUVPlaneByRow(const uint8_t *srcUPlane, const uint8_t *srcVPlane, uint8_t *dstUVPlane,
        int32_t dstHalfWidth);
    int32_t CombineNV12UVPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t CopyYPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t ColorConvertNV12ToNV21(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t ColorConvertNV12ToI420(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t ColorConvertByColorFormat(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t ColorFormatDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers);

private:
    constexpr static int32_t YUV_BYTES_PER_PIXEL = 3;
    constexpr static int32_t Y2UV_RATIO = 2;

    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
    std::atomic<bool> isColorFormatProcess_ = false;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_COLOR_FORMAT_PROCESS_H
