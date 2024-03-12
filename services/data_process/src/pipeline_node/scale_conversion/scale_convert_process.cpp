/*
 * Copyright (c) 2022-2024 Huawei Device Co., Ltd.
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

#include "scale_convert_process.h"

#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_frame_info.h"

namespace OHOS {
namespace DistributedHardware {
ScaleConvertProcess::~ScaleConvertProcess()
{
    if (isScaleConvert_.load()) {
        DHLOGI("~ScaleConvertProcess : ReleaseProcessNode");
        ReleaseProcessNode();
    }
}

int32_t ScaleConvertProcess::InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
    VideoConfigParams& processedConfig)
{
    DHLOGI("ScaleConvertProcess : InitNode.");
    sourceConfig_ = sourceConfig;
    targetConfig_ = targetConfig;
    processedConfig_ = sourceConfig;
    processedConfig_.SetWidthAndHeight(targetConfig.GetWidth(), targetConfig.GetHeight());
    processedConfig_.SetVideoformat(targetConfig.GetVideoformat());
    processedConfig = processedConfig_;

    if (!IsConvertible(sourceConfig, targetConfig)) {
        DHLOGI("sourceConfig: Videoformat %{public}d Width %{public}d, Height %{public}d is the same as the "
            "targetConfig: Videoformat %{public}d Width %{public}d, Height %{public}d.",
            sourceConfig.GetVideoformat(), sourceConfig.GetWidth(), sourceConfig.GetHeight(),
            targetConfig.GetVideoformat(), targetConfig.GetWidth(), targetConfig.GetHeight());
    }

    isScaleConvert_.store(true);
    return DCAMERA_OK;
}

bool ScaleConvertProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return (sourceConfig_.GetWidth() != targetConfig.GetWidth()) ||
        (sourceConfig_.GetHeight() != targetConfig.GetHeight()) ||
        (sourceConfig_.GetVideoformat() != targetConfig.GetVideoformat());
}

void ScaleConvertProcess::ReleaseProcessNode()
{
    DHLOGI("Start release [%{public}zu] node : ScaleConvertNode.", nodeRank_);
    isScaleConvert_.store(false);

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGI("Release [%{public}zu] node : ScaleConvertNode end.", nodeRank_);
}

int ScaleConvertProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    int64_t startScaleTime = GetNowTimeStampUs();
    DHLOGD("Process data in ScaleConvertProcess.");
    if (!isScaleConvert_.load()) {
        DHLOGE("Scale Convert node occurred error or start release.");
        return DCAMERA_DISABLE_PROCESS;
    }

    if (inputBuffers.empty() || inputBuffers[0] == nullptr) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }
    inputBuffers[0]->frameInfo_.timePonit.startScale = startScaleTime;

    if (!IsConvertible(sourceConfig_, processedConfig_)) {
        DHLOGD("The target resolution: %{public}dx%{public}d format: %{public}d is the same as the source "
            "resolution: %{public}dx%{public}d format: %{public}d",
            processedConfig_.GetWidth(), processedConfig_.GetHeight(), processedConfig_.GetVideoformat(),
            sourceConfig_.GetWidth(), sourceConfig_.GetHeight(), sourceConfig_.GetVideoformat());
        return ConvertDone(inputBuffers);
    }

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    if ((GetImageUnitInfo(srcImgInfo, inputBuffers[0]) != DCAMERA_OK) || !CheckScaleProcessInputInfo(srcImgInfo)) {
        DHLOGE("ScaleConvertProcess : srcImgInfo error.");
        return DCAMERA_BAD_VALUE;
    }

    size_t dstBuffSize = 0;
    CalculateBuffSize(dstBuffSize);
    std::shared_ptr<DataBuffer> dstBuf = std::make_shared<DataBuffer>(dstBuffSize);
    ImageUnitInfo dstImgInfo = { processedConfig_.GetVideoformat(), processedConfig_.GetWidth(),
        processedConfig_.GetHeight(), processedConfig_.GetWidth(), processedConfig_.GetHeight(),
        processedConfig_.GetWidth() * processedConfig_.GetHeight(), dstBuf->Size(), dstBuf };
    if (ScaleConvert(srcImgInfo, dstImgInfo) != DCAMERA_OK) {
        DHLOGE("ScaleConvertProcess : Scale convert failed.");
        return DCAMERA_BAD_OPERATE;
    }

    dstBuf->frameInfo_ = inputBuffers[0]->frameInfo_;
    dstBuf->SetInt32("Videoformat", static_cast<int32_t>(processedConfig_.GetVideoformat()));
    dstBuf->SetInt32("alignedWidth", processedConfig_.GetWidth());
    dstBuf->SetInt32("alignedHeight", processedConfig_.GetHeight());
    dstBuf->SetInt32("width", processedConfig_.GetWidth());
    dstBuf->SetInt32("height", processedConfig_.GetHeight());

    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    outputBuffers.push_back(dstBuf);
    return ConvertDone(outputBuffers);
}

void ScaleConvertProcess::CalculateBuffSize(size_t& dstBuffSize)
{
    if (processedConfig_.GetVideoformat() == Videoformat::RGBA_8888) {
        dstBuffSize = static_cast<size_t>(processedConfig_.GetWidth() * processedConfig_.GetHeight() *
            RGB32_MEMORY_COEFFICIENT);
    } else {
        dstBuffSize = static_cast<size_t>(
            processedConfig_.GetWidth() * processedConfig_.GetHeight() * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    }
}

int32_t ScaleConvertProcess::GetImageUnitInfo(ImageUnitInfo& imgInfo, const std::shared_ptr<DataBuffer>& imgBuf)
{
    if (imgBuf == nullptr) {
        DHLOGE("GetImageUnitInfo failed, imgBuf is nullptr.");
        return DCAMERA_BAD_VALUE;
    }

    bool findErr = true;
    int32_t colorFormat = 0;
    findErr = findErr && imgBuf->FindInt32("Videoformat", colorFormat);
    if (!findErr) {
        DHLOGE("GetImageUnitInfo failed, Videoformat is null.");
        return DCAMERA_NOT_FOUND;
    }
    if (colorFormat != static_cast<int32_t>(Videoformat::YUVI420) &&
        colorFormat != static_cast<int32_t>(Videoformat::NV12) &&
        colorFormat != static_cast<int32_t>(Videoformat::NV21)) {
        DHLOGE("GetImageUnitInfo failed, colorFormat %{public}d are not supported.", colorFormat);
        return DCAMERA_NOT_FOUND;
    }
    imgInfo.colorFormat = static_cast<Videoformat>(colorFormat);
    findErr = findErr && imgBuf->FindInt32("width", imgInfo.width);
    findErr = findErr && imgBuf->FindInt32("height", imgInfo.height);
    findErr = findErr && imgBuf->FindInt32("alignedWidth", imgInfo.alignedWidth);
    findErr = findErr && imgBuf->FindInt32("alignedHeight", imgInfo.alignedHeight);
    if (!findErr) {
        DHLOGE("GetImageUnitInfo failed, width %{public}d, height %{public}d, alignedWidth %{public}d, "
            "alignedHeight %{public}d.", imgInfo.width, imgInfo.height, imgInfo.alignedWidth, imgInfo.alignedHeight);
        return DCAMERA_NOT_FOUND;
    }

    imgInfo.chromaOffset = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight);
    imgInfo.imgSize = imgBuf->Size();
    imgInfo.imgData = imgBuf;

    DHLOGD("ScaleConvertProcess imgBuf info : Videoformat %{public}d, alignedWidth %{public}d, alignedHeight "
        "%{public}d, width %{public}d, height %{public}d, chromaOffset %{public}zu, imgSize %{public}zu.",
        imgInfo.colorFormat, imgInfo.alignedWidth, imgInfo.alignedHeight,
        imgInfo.width, imgInfo.height, imgInfo.chromaOffset, imgInfo.imgSize);
    return DCAMERA_OK;
}

bool ScaleConvertProcess::CheckScaleProcessInputInfo(const ImageUnitInfo& srcImgInfo)
{
    return srcImgInfo.colorFormat == Videoformat::YUVI420 &&
        srcImgInfo.width == sourceConfig_.GetWidth() &&
        srcImgInfo.height == sourceConfig_.GetHeight() &&
        IsCorrectImageUnitInfo(srcImgInfo);
}

bool ScaleConvertProcess::CheckScaleConvertInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (srcImgInfo.imgData == nullptr || dstImgInfo.imgData == nullptr) {
        DHLOGE("The imgData of srcImgInfo or the imgData of dstImgInfo are null!");
        return false;
    }

    if (!IsCorrectImageUnitInfo(srcImgInfo)) {
        DHLOGE("srcImginfo fail: width %{public}d, height %{public}d, alignedWidth %{public}d, alignedHeight "
            "%{public}d, chromaOffset %{public}zu, imgSize %{public}zu.", srcImgInfo.width, srcImgInfo.height,
            srcImgInfo.alignedWidth, srcImgInfo.alignedHeight, srcImgInfo.chromaOffset, srcImgInfo.imgSize);
        return false;
    }

    if (!IsCorrectImageUnitInfo(dstImgInfo)) {
        DHLOGE("dstImginfo fail: width %{public}d, height %{public}d, alignedWidth %{public}d, alignedHeight "
            "%{public}d, chromaOffset %{public}zu, imgSize %{public}zu.", dstImgInfo.width, dstImgInfo.height,
            dstImgInfo.alignedWidth, dstImgInfo.alignedHeight, dstImgInfo.chromaOffset, dstImgInfo.imgSize);
        return false;
    }

    if ((dstImgInfo.width == srcImgInfo.alignedWidth) && (dstImgInfo.height == srcImgInfo.alignedHeight) &&
        (dstImgInfo.colorFormat == srcImgInfo.colorFormat)) {
        DHLOGE("Comparison ImgInfo fail: dstwidth %{public}d, dstheight %{public}d, dstColorFormat %{public}d, "
            "srcAlignedWidth %{public}d, srcAlignedHeight %{public}d, srcColorFormat %{public}d.",
            dstImgInfo.width, dstImgInfo.height, dstImgInfo.colorFormat,
            srcImgInfo.alignedWidth, srcImgInfo.alignedHeight, srcImgInfo.colorFormat);
        return false;
    }

    return true;
}

bool ScaleConvertProcess::IsCorrectImageUnitInfo(const ImageUnitInfo& imgInfo)
{
    size_t expectedImgSize = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight *
        YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t expectedChromaOffset = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight);
    return (imgInfo.width <= imgInfo.alignedWidth && imgInfo.height <= imgInfo.alignedHeight &&
        imgInfo.imgSize >= expectedImgSize && imgInfo.chromaOffset == expectedChromaOffset);
}

int32_t ScaleConvertProcess::ScaleConvert(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo)
{
    DHLOGD("Scale convert start.");
    if (!CheckScaleConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("CheckScaleConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    std::shared_ptr<DataBuffer> dstBuf =
        std::make_shared<DataBuffer>(dstImgInfo.width * dstImgInfo.height * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    int32_t ret = ConvertResolution(srcImgInfo, dstImgInfo, dstBuf);
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert I420 scale failed.");
        return ret;
    }

    if (processedConfig_.GetVideoformat() == Videoformat::NV21) {
        ret = ConvertFormatToNV21(srcImgInfo, dstImgInfo, dstBuf);
    } else if (processedConfig_.GetVideoformat() == Videoformat::RGBA_8888) {
        ret = ConvertFormatToRGBA(srcImgInfo, dstImgInfo, dstBuf);
    }
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert I420 to format: %{public}d failed.", processedConfig_.GetVideoformat());
        return ret;
    }

    DHLOGD("Scale convert end.");
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertResolution(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
    std::shared_ptr<DataBuffer>& dstBuf)
{
    if ((srcImgInfo.width == dstImgInfo.width) && (srcImgInfo.height == dstImgInfo.height)) {
        dstBuf = srcImgInfo.imgData;
        DHLOGD("Convert I420 Scale: srcImgInfo is the same as dstImgInfo");
        return DCAMERA_OK;
    }

    DHLOGD("Convert I420 Scale: format=%{public}d, width=[%{public}d, %{public}d], height=[%{public}d, %{public}d]",
        srcImgInfo.colorFormat, srcImgInfo.width, srcImgInfo.alignedWidth, srcImgInfo.height, srcImgInfo.alignedHeight);
    int srcSizeY = srcImgInfo.width * srcImgInfo.height;
    int srcSizeUV = (static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(srcImgInfo.height) >> MEMORY_RATIO_UV);
    uint8_t *srcDataY = srcImgInfo.imgData->Data();
    uint8_t *srcDataU = srcImgInfo.imgData->Data() + srcSizeY;
    uint8_t *srcDataV = srcImgInfo.imgData->Data() + srcSizeY + srcSizeUV;

    int dstSizeY = dstImgInfo.width * dstImgInfo.height;
    int dstSizeUV = (static_cast<uint32_t>(dstImgInfo.width) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(dstImgInfo.height) >> MEMORY_RATIO_UV);
    uint8_t *dstDataY = dstBuf->Data();
    uint8_t *dstDataU = dstBuf->Data() + dstSizeY;
    uint8_t *dstDataV = dstBuf->Data() + dstSizeY + dstSizeUV;

    auto converter = ConverterHandle::GetInstance().GetHandle();
    CHECK_AND_RETURN_RET_LOG(converter.I420Scale == nullptr, DCAMERA_BAD_VALUE, "converter is invalid.");
    int32_t ret = converter.I420Scale(
        srcDataY, srcImgInfo.width,
        srcDataU, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        srcDataV, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        srcImgInfo.width, srcImgInfo.height,
        dstDataY, dstImgInfo.width,
        dstDataU, static_cast<uint32_t>(dstImgInfo.width) >> MEMORY_RATIO_UV,
        dstDataV, static_cast<uint32_t>(dstImgInfo.width) >> MEMORY_RATIO_UV,
        dstImgInfo.width, dstImgInfo.height,
        OpenSourceLibyuv::FilterMode::kFilterNone);
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert I420 scale failed.");
        return DCAMERA_BAD_VALUE;
    }

    srcImgInfo.width = dstImgInfo.width;
    srcImgInfo.height = dstImgInfo.height;
    srcImgInfo.alignedWidth = dstImgInfo.alignedWidth;
    srcImgInfo.alignedHeight = dstImgInfo.alignedHeight;
    srcImgInfo.chromaOffset = static_cast<size_t>(srcImgInfo.alignedWidth * srcImgInfo.alignedHeight);
    srcImgInfo.imgSize = dstBuf->Size();

    DHLOGD("Convert I420 scale success.");
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertFormatToNV21(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
    std::shared_ptr<DataBuffer>& dstBuf)
{
    if (srcImgInfo.colorFormat == dstImgInfo.colorFormat) {
        DHLOGD("Convert format to NV21 srcImgInfo format is the same as dstImgInfo format");
        return DCAMERA_OK;
    }

    DHLOGD("Convert I420 to NV21: format=%{public}d, width=[%{public}d, %{public}d], height=[%{public}d, %{public}d]",
        srcImgInfo.colorFormat, srcImgInfo.width, srcImgInfo.alignedWidth, srcImgInfo.height,
        srcImgInfo.alignedHeight);
    int srcSizeY = srcImgInfo.width * srcImgInfo.height;
    int srcSizeUV = (static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(srcImgInfo.height) >> MEMORY_RATIO_UV);
    uint8_t *srcDataY = dstBuf->Data();
    uint8_t *srcDataU = dstBuf->Data() + srcSizeY;
    uint8_t *srcDataV = dstBuf->Data() + srcSizeY + srcSizeUV;

    int dstSizeY = dstImgInfo.width * dstImgInfo.height;
    uint8_t *dstDataY = dstImgInfo.imgData->Data();
    uint8_t *dstDataUV = dstImgInfo.imgData->Data() + dstSizeY;

    auto converter = ConverterHandle::GetInstance().GetHandle();
    CHECK_AND_RETURN_RET_LOG(converter.I420ToNV21 == nullptr, DCAMERA_BAD_VALUE, "converter is invalid.");
    int32_t ret = converter.I420ToNV21(
        srcDataY, srcImgInfo.width,
        srcDataU, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        srcDataV, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        dstDataY, dstImgInfo.width,
        dstDataUV, dstImgInfo.width,
        dstImgInfo.width, dstImgInfo.height);
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert I420 to NV21 failed.");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGD("Convert I420 to NV21 success.");
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertFormatToRGBA(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo,
    std::shared_ptr<DataBuffer>& dstBuf)
{
    if (srcImgInfo.colorFormat == dstImgInfo.colorFormat) {
        DHLOGD("Convert format to RGBA srcImgInfo format is the same as dstImgInfo format");
        return DCAMERA_OK;
    }

    DHLOGI("Convert I420 to RGBA: format=%{public}d, width=[%{public}d, %{public}d], height=[%{public}d, %{public}d]",
        srcImgInfo.colorFormat, srcImgInfo.width, srcImgInfo.alignedWidth, srcImgInfo.height, srcImgInfo.alignedHeight);
    int srcSizeY = srcImgInfo.width * srcImgInfo.height;
    int srcSizeUV = (static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV) *
                    (static_cast<uint32_t>(srcImgInfo.height) >> MEMORY_RATIO_UV);
    uint8_t *srcDataY = dstBuf->Data();
    uint8_t *srcDataU = dstBuf->Data() + srcSizeY;
    uint8_t *srcDataV = dstBuf->Data() + srcSizeY + srcSizeUV;

    uint8_t *dstDataRGBA = dstImgInfo.imgData->Data();
    auto converter = ConverterHandle::GetInstance().GetHandle();
    CHECK_AND_RETURN_RET_LOG(converter.I420ToRGBA == nullptr, DCAMERA_BAD_VALUE, "converter is invalid.");
    int32_t ret = converter.I420ToRGBA(
        srcDataY, srcImgInfo.width,
        srcDataU, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        srcDataV, static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV,
        dstDataRGBA, dstImgInfo.width * RGB32_MEMORY_COEFFICIENT,
        dstImgInfo.width, dstImgInfo.height);
    if (ret != DCAMERA_OK) {
        DHLOGE("Convert I420 to RGBA failed.");
        return DCAMERA_BAD_VALUE;
    }

    DHLOGD("Convert I420 to RGBA success.");
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    int64_t finishScaleTime = GetNowTimeStampUs();
    DHLOGD("ScaleConvertProcess : Convert Done.");
    if (outputBuffers.empty()) {
        DHLOGE("The received data buffer is empty.");
        return DCAMERA_BAD_VALUE;
    }
    outputBuffers[0]->frameInfo_.timePonit.finishScale = finishScaleTime;

    if (nextDataProcess_ != nullptr) {
        DHLOGD("Send to the next node of the scale convert for processing.");
        int32_t err = nextDataProcess_->ProcessData(outputBuffers);
        if (err != DCAMERA_OK) {
            DHLOGE("Some node after the scale convert processes failed.");
        }
        return err;
    }

    DHLOGD("The current node is the last noed, and output the processed video buffer.");
    std::shared_ptr<DCameraPipelineSource> targetPipelineSource = callbackPipelineSource_.lock();
    if (targetPipelineSource == nullptr) {
        DHLOGE("callbackPipelineSource_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    targetPipelineSource->OnProcessedVideoBuffer(outputBuffers[0]);
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
