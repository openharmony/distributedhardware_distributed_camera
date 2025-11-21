/*
 * Copyright (c) 2022-2023 Huawei Device Co., Ltd.
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

#ifndef OHOS_SCALE_CONVERT_PROCESS_H
#define OHOS_SCALE_CONVERT_PROCESS_H

#include "abstract_data_process.h"

#ifdef __cplusplus
extern "C" {
#endif
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libavutil/parseutils.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
};
#endif

#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <mutex>
#include <securec.h>

#include "dcamera_pipeline_source.h"
#include "image_common_type.h"
#include "dcamera_utils_tools.h"

namespace OHOS {
namespace DistributedHardware {
class ScaleConvertProcess : public AbstractDataProcess {
public:
    explicit ScaleConvertProcess(const std::weak_ptr<DCameraPipelineSource>& callbackPipeSource)
        : callbackPipelineSource_(callbackPipeSource) {}
    ~ScaleConvertProcess() override;

    int32_t InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
        VideoConfigParams& processedConfig) override;
    int32_t ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers) override;
    void ReleaseProcessNode() override;

    int32_t GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier) override;

private:
    bool IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig);
    bool IsCorrectImageUnitInfo(const ImageUnitInfo& imgInfo);
    bool CheckScaleProcessInputInfo(const ImageUnitInfo& srcImgInfo);
    bool CheckScaleConvertInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo);
    int32_t GetImageUnitInfo(ImageUnitInfo& imgInfo, const std::shared_ptr<DataBuffer>& imgBuf);
    int32_t ScaleConvert(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo);
    void Crop(ImageUnitInfo& sourceConfig, ImageUnitInfo& targetConfig);
    void CropConvert(ImageUnitInfo& sourceConfig, ImageUnitInfo& targetConfig, int crop_width,
        int crop_height, uint8_t* dstY, uint8_t* dstU, uint8_t* dstV, std::shared_ptr<DataBuffer> cropBuf);
#ifdef DCAMERA_SUPPORT_FFMPEG
    int32_t CopyYUV420SrcData(const ImageUnitInfo& srcImgInfo);
    int32_t CopyNV12SrcData(const ImageUnitInfo& srcImgInfo);
    int32_t CopyNV21SrcData(const ImageUnitInfo& srcImgInfo);
#else
    int32_t ConvertResolution(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
        std::shared_ptr<DataBuffer>& dstBuf);
    int32_t ConvertFormatToNV21(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
        std::shared_ptr<DataBuffer>& dstBuf);
    int32_t ConvertFormatToRGBA(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
        std::shared_ptr<DataBuffer>& dstBuf);
    int32_t ConvertFormatToP010(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo);
    void CalculateBuffSize(size_t& dstBuffSize);
#endif
    AVPixelFormat GetAVPixelFormat(Videoformat colorFormat);
    int32_t ConvertDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers);

private:
    constexpr static int32_t DATA_LEN = 4;
    constexpr static int32_t MEMORY_RATIO_NV = 2;
    constexpr static int32_t MEMORY_RATIO_YUV = 4;
    constexpr static int32_t SOURCE_ALIGN = 16;
    constexpr static int32_t TARGET_ALIGN = 1;
    constexpr static int32_t YUV_BYTES_PER_PIXEL = 3;
    constexpr static int32_t Y2UV_RATIO = 2;
    constexpr static int32_t RGB32_MEMORY_COEFFICIENT = 4;
    constexpr static uint32_t MEMORY_RATIO_UV = 1;

#ifdef DCAMERA_SUPPORT_FFMPEG
    uint8_t *srcData_[DATA_LEN] = { nullptr };
    uint8_t *dstData_[DATA_LEN] = { nullptr };
    int32_t srcLineSize_[DATA_LEN] = { 0 };
    int32_t dstLineSize_[DATA_LEN] = { 0 };
    int32_t dstBuffSize_ = 0;
#endif
    SwsContext *swsContext_ = nullptr;
    std::mutex scaleMutex_;
    VideoConfigParams sourceConfig_;
    VideoConfigParams targetConfig_;
    VideoConfigParams processedConfig_;
    std::weak_ptr<DCameraPipelineSource> callbackPipelineSource_;
    std::atomic<bool> isScaleConvert_ = false;
    FILE *dumpFile_ = nullptr;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_SCALE_CONVERT_PROCESS_H
