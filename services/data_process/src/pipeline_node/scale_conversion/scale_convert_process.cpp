/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "dcamera_frame_info.h"
#include <cmath>

namespace OHOS {
namespace DistributedHardware {
ScaleConvertProcess::~ScaleConvertProcess()
{
    DumpFileUtil::CloseDumpFile(&dumpFile_);
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

    if (targetConfig.GetVideoformat() == Videoformat::P010) {
        AVPixelFormat sourceFmt = GetAVPixelFormat(sourceConfig.GetVideoformat());
        AVPixelFormat targetFmt = GetAVPixelFormat(targetConfig.GetVideoformat());
        DHLOGI("VideoFormat P010 src fmt: %{public}d -> %{public}d, dst fmt: %{public}d -> %{public}d",
            sourceConfig.GetVideoformat(), sourceFmt, targetConfig.GetVideoformat(), targetFmt);
        swsContext_ = sws_getContext(sourceConfig_.GetWidth(), sourceConfig_.GetHeight(), sourceFmt,
            processedConfig_.GetWidth(), processedConfig_.GetHeight(), targetFmt,
            SWS_FAST_BILINEAR | SWS_FULL_CHR_H_INT, nullptr, nullptr, nullptr);
        CHECK_AND_RETURN_RET_LOG(swsContext_ == nullptr, DCAMERA_MEMORY_OPT_ERROR,
            "Failed to create sws context for P010 conversion");
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

    {
        std::lock_guard<std::mutex> autoLock(scaleMutex_);
        if (swsContext_ != nullptr) {
            sws_freeContext(swsContext_);
            swsContext_ = nullptr;
        }
    }

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGI("Release [%{public}zu] node : ScaleConvertNode end.", nodeRank_);
}

void ScaleConvertProcess::Crop(ImageUnitInfo& sourceConfig, ImageUnitInfo& targetConfig)
{
    const double src_width = static_cast<double>(sourceConfig.width);
    const double src_height = static_cast<double>(sourceConfig.height);
    const double dst_width = static_cast<double>(targetConfig.width);
    const double dst_height = static_cast<double>(targetConfig.height);
    double src_ratio = src_width / src_height;
    double dst_ratio = dst_width / dst_height;
    if (std::abs(src_ratio - dst_ratio) < 1e-6) {
        DHLOGI("Same aspect ratio");
        return;
    }
    int crop_width = 0;
    int crop_height = 0;
    if (src_ratio > dst_ratio) {
        DHLOGI("The source aspect ratio is greater than the target aspect ratio");
        crop_width = static_cast<int>(src_height * dst_width / dst_height);
        crop_height = src_height;
    } else {
        DHLOGI("The source aspect ratio is less than the target aspect ratio");
        crop_width = src_width;
        crop_height = static_cast<int>(src_width * dst_height / dst_width);
    }
    const size_t y_size = static_cast<size_t>(crop_width * crop_height);
    const size_t uv_size = static_cast<size_t>((crop_width / Y2UV_RATIO) * (crop_height / Y2UV_RATIO));
    const size_t total_size = static_cast<size_t>(crop_width * crop_height * YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    std::shared_ptr<DataBuffer> cropBuf = std::make_shared<DataBuffer>(total_size);
    uint8_t* dstY = cropBuf->Data();
    uint8_t* dstU = dstY + y_size;
    uint8_t* dstV = dstU + uv_size;
    CropConvert(sourceConfig, targetConfig, crop_width, crop_height, dstY, dstU, dstV, cropBuf);
}

void ScaleConvertProcess::CropConvert(ImageUnitInfo& sourceConfig, ImageUnitInfo& targetConfig, int crop_width,
    int crop_height, uint8_t* dstY, uint8_t* dstU, uint8_t* dstV, std::shared_ptr<DataBuffer> cropBuf)
{
    const int offsetX = (sourceConfig.width - crop_width) / Y2UV_RATIO;
    const int offsetY = (sourceConfig.height - crop_height) / Y2UV_RATIO;
    uint8_t* srcY = sourceConfig.imgData->Data();
    uint8_t* srcU = srcY + sourceConfig.width * sourceConfig.height;
    uint8_t* srcV = srcU + (sourceConfig.width / Y2UV_RATIO)
        * (sourceConfig.height / Y2UV_RATIO);
    
    for (int y = 0; y < crop_height; ++y) {
        const uint8_t* src_row = srcY + (offsetY + y) * sourceConfig.width;
        uint8_t* dst_row = dstY + y * crop_width;
        errno_t err = memcpy_s(dst_row, crop_width, src_row + offsetX, crop_width);
        if (err != EOK) {
            DHLOGE("memcpy_s failed for Y plane at row %{public}d", y);
            return;
        }
    }
    for (int y = 0; y < crop_height / Y2UV_RATIO; ++y) {
        const uint8_t* src_row = srcU + ((offsetY / Y2UV_RATIO) + y) * (sourceConfig.width / Y2UV_RATIO);
        uint8_t* dst_row = dstU + y * (crop_width / Y2UV_RATIO);
        errno_t err = memcpy_s(dst_row, crop_width / Y2UV_RATIO, src_row + (offsetX / Y2UV_RATIO),
            crop_width / Y2UV_RATIO);
        if (err != EOK) {
            DHLOGE("memcpy_s failed for U plane at row %{public}d", y);
            return;
        }
    }
    for (int y = 0; y < crop_height / Y2UV_RATIO; ++y) {
        const uint8_t* src_row = srcV + ((offsetY / Y2UV_RATIO) + y) * (sourceConfig.width / Y2UV_RATIO);
        uint8_t* dst_row = dstV + y * (crop_width / Y2UV_RATIO);
        errno_t err = memcpy_s(dst_row, crop_width / Y2UV_RATIO, src_row + (offsetX / Y2UV_RATIO),
            crop_width / Y2UV_RATIO);
        if (err != EOK) {
            DHLOGE("memcpy_s failed for V plane at row %{public}d", y);
            return;
        }
    }
    sourceConfig.imgData = cropBuf;
    sourceConfig.width = crop_width;
    sourceConfig.height = crop_height;
    sourceConfig.alignedWidth = crop_width;
    sourceConfig.alignedHeight = crop_height;
    sourceConfig.chromaOffset = static_cast<size_t>(crop_width * crop_height);
    sourceConfig.imgSize = cropBuf->Size();
    DHLOGD("Cropped successfully: %{public}dx%{public}d -> %{public}dx%{public}d, offset (%{public}d,%{public}d)",
           sourceConfig.width, sourceConfig.height, crop_width, crop_height, offsetX, offsetY);
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
    DumpFileUtil::OpenDumpFile(DUMP_SERVER_PARA, DUMP_DCAMERA_AFTER_SCALE_FILENAME, &dumpFile_);

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
    if (targetConfig_.GetIsSystemSwitch()) {
        Crop(srcImgInfo, dstImgInfo);
    }
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

    DumpFileUtil::WriteDumpFile(dumpFile_, static_cast<void *>(dstBuf->Data()), dstBuf->Size());
    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    outputBuffers.push_back(dstBuf);
    return ConvertDone(outputBuffers);
}

void ScaleConvertProcess::CalculateBuffSize(size_t& dstBuffSize)
{
    if (processedConfig_.GetVideoformat() == Videoformat::RGBA_8888) {
        dstBuffSize = static_cast<size_t>(processedConfig_.GetWidth() * processedConfig_.GetHeight() *
            RGB32_MEMORY_COEFFICIENT);
    } else if (processedConfig_.GetVideoformat() == Videoformat::P010) {
        dstBuffSize = static_cast<size_t>(
            processedConfig_.GetWidth() * processedConfig_.GetHeight() * YUV_BYTES_PER_PIXEL);
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
    } else if (targetConfig_.GetVideoformat() == Videoformat::P010) {
        ret = ConvertFormatToP010(srcImgInfo, dstImgInfo);
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
    CHECK_AND_RETURN_RET_LOG((srcImgInfo.imgData == nullptr), DCAMERA_BAD_VALUE, "Data buffer exists null data");
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
    CHECK_AND_RETURN_RET_LOG((dstBuf == nullptr), DCAMERA_BAD_VALUE, "Buffer is null.");
    CHECK_AND_RETURN_RET_LOG((dstImgInfo.imgData == nullptr), DCAMERA_BAD_VALUE, "Image data is null.");
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
    CHECK_AND_RETURN_RET_LOG((dstBuf == nullptr), DCAMERA_BAD_VALUE, "Buffer is null.");
    CHECK_AND_RETURN_RET_LOG((dstImgInfo.imgData == nullptr), DCAMERA_BAD_VALUE, "Image data is null.");
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

int32_t ScaleConvertProcess::ConvertFormatToP010(ImageUnitInfo& srcImgInfo, ImageUnitInfo& dstImgInfo)
{
    CHECK_AND_RETURN_RET_LOG((srcImgInfo.imgData == nullptr), DCAMERA_BAD_VALUE, "Data buffer exists null data");
    int32_t srcSizeY = srcImgInfo.width * srcImgInfo.height;
    int32_t srcSizeUV = (static_cast<uint32_t>(srcImgInfo.width) >> MEMORY_RATIO_UV) *
        (static_cast<uint32_t>(srcImgInfo.height) >> MEMORY_RATIO_UV);
    uint8_t* srcDataY = srcImgInfo.imgData->Data();
    uint8_t* srcDataU = srcImgInfo.imgData->Data() + srcSizeY;
    uint8_t* srcDataV = srcImgInfo.imgData->Data() + srcSizeY + srcSizeUV;
    for (int i = 0; i < srcSizeUV; i++) {
        uint8_t temp = srcDataU[i];
        srcDataU[i] = srcDataV[i];
        srcDataV[i] = temp;
    }
    uint8_t* srcData[3] = { srcDataY, srcDataU, srcDataV };
    int32_t srcLinsize[3] = { srcImgInfo.width, srcImgInfo.width / 2, srcImgInfo.width / 2 };

    int32_t dstWidth = dstImgInfo.width;
    int32_t dstHeight = dstImgInfo.height;
    int32_t dstLinsize[3] = { dstWidth * 2, dstWidth * 2, dstWidth * 2 };

    uint8_t* dstDataY = dstImgInfo.imgData->Data();
    uint8_t* dstDataU = dstImgInfo.imgData->Data() + dstWidth * dstHeight * 2;
    uint8_t* dstDataV = dstImgInfo.imgData->Data() + dstWidth * dstHeight * 2 + dstWidth * dstHeight * 2;

    uint8_t* dstData[3] = { dstDataY, dstDataU, dstDataV };
    int32_t ret = sws_scale(swsContext_, srcData, srcLinsize, 0, srcImgInfo.height, dstData, dstLinsize);
    if (ret < 0) {
        DHLOGE("ScaleConvertProcess::ConvertFormatToP010 sws_scale failed, ret = %{public}d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    int64_t finishScaleTime = GetNowTimeStampUs();
    DHLOGD("ScaleConvertProcess : Convert Done.");
    if (outputBuffers.empty() || outputBuffers[0] == nullptr) {
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

AVPixelFormat ScaleConvertProcess::GetAVPixelFormat(Videoformat colorFormat)
{
    AVPixelFormat format;
    switch (colorFormat) {
        case Videoformat::NV12:
            format = AVPixelFormat::AV_PIX_FMT_NV12;
            break;
        case Videoformat::NV21:
            format = AVPixelFormat::AV_PIX_FMT_NV21;
            break;
        case Videoformat::P010:
            format = AVPixelFormat::AV_PIX_FMT_P010LE;
            break;
        default:
            format = AVPixelFormat::AV_PIX_FMT_YUV420P;
            break;
    }
    return format;
}

int32_t ScaleConvertProcess::GetProperty(const std::string& propertyName, PropertyCarrier& propertyCarrier)
{
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
