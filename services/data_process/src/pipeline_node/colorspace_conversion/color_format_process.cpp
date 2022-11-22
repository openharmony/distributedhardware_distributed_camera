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

#include "color_format_process.h"

#include "distributed_hardware_log.h"

#include "distributed_camera_errno.h"

namespace OHOS {
namespace DistributedHardware {
ColorFormatProcess::~ColorFormatProcess()
{
    if (isColorFormatProcess_.load()) {
        DHLOGD("~ColorFormatProcess : ReleaseProcessNode.");
        ReleaseProcessNode();
    }
}

int32_t ColorFormatProcess::InitNode(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig,
    VideoConfigParams& processedConfig)
{
    DHLOGD("ColorFormatProcess : InitNode.");
    if (!IsConvertible(sourceConfig, targetConfig)) {
        DHLOGE("sourceConfig: Videoformat %d Width %d, Height %d, targetConfig: Videoformat %d Width %d, Height %d.",
            sourceConfig.GetVideoformat(), sourceConfig.GetWidth(), sourceConfig.GetHeight(),
            targetConfig.GetVideoformat(), targetConfig.GetWidth(), targetConfig.GetHeight());
        return DCAMERA_BAD_TYPE;
    }

    sourceConfig_ = sourceConfig;
    targetConfig_ = targetConfig;
    processedConfig_ = sourceConfig;

    if (sourceConfig_.GetVideoformat() != targetConfig_.GetVideoformat()) {
        processedConfig_.SetVideoformat(targetConfig_.GetVideoformat());
    }

    processedConfig = processedConfig_;
    isColorFormatProcess_.store(true);
    return DCAMERA_OK;
}

bool ColorFormatProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return ((sourceConfig.GetVideoformat() == targetConfig.GetVideoformat() ||
        (sourceConfig.GetVideoformat() == Videoformat::NV12 && targetConfig.GetVideoformat() == Videoformat::NV21)) &&
        sourceConfig.GetWidth() == targetConfig.GetWidth() && sourceConfig.GetHeight() == targetConfig.GetHeight());
}

void ColorFormatProcess::ReleaseProcessNode()
{
    DHLOGD("Start release [%d] node : ColorFormatNode.", nodeRank_);
    isColorFormatProcess_.store(false);

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGD("Release [%d] node : ColorFormatNode end.", nodeRank_);
}

int32_t ColorFormatProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    DHLOGD("Process data in ColorFormatProcess.");
    if (inputBuffers.empty() || inputBuffers[0] == nullptr) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (sourceConfig_.GetVideoformat() == processedConfig_.GetVideoformat()) {
        DHLOGD("The target Video Format : %d is the same as the source Video Format : %d.",
            sourceConfig_.GetVideoformat(), processedConfig_.GetVideoformat());
        return ColorFormatDone(inputBuffers);
    }

    int64_t timeStamp = 0;
    if (!(inputBuffers[0]->FindInt64("timeUs", timeStamp))) {
        DHLOGE("ColorConvertProcess : Find inputBuffer timeStamp failed.");
        return DCAMERA_BAD_VALUE;
    }

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    if (GetImageUnitInfo(srcImgInfo, inputBuffers[0]) != DCAMERA_OK || !CheckColorProcessInputInfo(srcImgInfo)) {
        DHLOGE("ColorConvertProcess : srcImgInfo error.");
        return DCAMERA_BAD_VALUE;
    }

    size_t dstBufsize = static_cast<size_t>(sourceConfig_.GetWidth() * sourceConfig_.GetHeight() *
        YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    std::shared_ptr<DataBuffer> dstBuf = std::make_shared<DataBuffer>(dstBufsize);
    ImageUnitInfo dstImgInfo = { processedConfig_.GetVideoformat(), processedConfig_.GetWidth(),
        processedConfig_.GetHeight(), processedConfig_.GetWidth(), processedConfig_.GetHeight(),
        processedConfig_.GetWidth() * processedConfig_.GetHeight(), dstBuf->Size(), dstBuf->Data() };
    if (ColorConvertByColorFormat(srcImgInfo, dstImgInfo) != DCAMERA_OK) {
        DHLOGE("ColorConvertProcess : ColorConvertByColorFormat failed.");
        return DCAMERA_BAD_OPERATE;
    }

    dstBuf->SetInt64("timeUs", timeStamp);
    dstBuf->SetInt32("Videoformat", static_cast<int32_t>(processedConfig_.GetVideoformat()));
    dstBuf->SetInt32("alignedWidth", processedConfig_.GetWidth());
    dstBuf->SetInt32("alignedHeight", processedConfig_.GetHeight());
    dstBuf->SetInt32("width", processedConfig_.GetWidth());
    dstBuf->SetInt32("height", processedConfig_.GetHeight());

    std::vector<std::shared_ptr<DataBuffer>> outputBuffers;
    outputBuffers.push_back(dstBuf);
    return ColorFormatDone(outputBuffers);
}

int32_t ColorFormatProcess::GetImageUnitInfo(ImageUnitInfo& imgInfo, const std::shared_ptr<DataBuffer>& imgBuf)
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
        DHLOGE("GetImageUnitInfo failed, colorFormat %d are not supported.", colorFormat);
        return DCAMERA_NOT_FOUND;
    }
    imgInfo.colorFormat = static_cast<Videoformat>(colorFormat);
    findErr = findErr && imgBuf->FindInt32("width", imgInfo.width);
    findErr = findErr && imgBuf->FindInt32("height", imgInfo.height);
    findErr = findErr && imgBuf->FindInt32("alignedWidth", imgInfo.alignedWidth);
    findErr = findErr && imgBuf->FindInt32("alignedHeight", imgInfo.alignedHeight);
    if (!findErr) {
        DHLOGE("GetImageUnitInfo failed, width %d, height %d, alignedWidth %d, alignedHeight %d.",
            imgInfo.width, imgInfo.height, imgInfo.alignedWidth, imgInfo.alignedHeight);
        return DCAMERA_NOT_FOUND;
    }

    imgInfo.chromaOffset = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight);
    imgInfo.imgSize = imgBuf->Size();
    imgInfo.imgData = imgBuf->Data();
    if (imgInfo.imgData == nullptr) {
        DHLOGE("Get the imgData of the imgBuf failed.");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGD("ColorFormatProcess imgBuf info : Videoformat %d, alignedWidth %d, alignedHeight %d, width %d, height %d," +
        " chromaOffset %d, imgSize %d.", imgInfo.colorFormat, imgInfo.width, imgInfo.height, imgInfo.alignedWidth,
        imgInfo.alignedHeight, imgInfo.chromaOffset, imgInfo.imgSize);
    return DCAMERA_OK;
}

bool ColorFormatProcess::CheckColorProcessInputInfo(const ImageUnitInfo& srcImgInfo)
{
    return srcImgInfo.colorFormat == sourceConfig_.GetVideoformat() &&
        srcImgInfo.alignedWidth == sourceConfig_.GetWidth() &&
        srcImgInfo.alignedHeight == sourceConfig_.GetHeight() &&
        IsCorrectImageUnitInfo(srcImgInfo);
}

bool ColorFormatProcess::CheckColorConvertInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (srcImgInfo.imgData == nullptr || dstImgInfo.imgData == nullptr) {
        DHLOGE("The imgData of srcImgInfo or the imgData of dstImgInfo are null!");
        return false;
    }
    if (srcImgInfo.colorFormat != Videoformat::NV12 && dstImgInfo.colorFormat != Videoformat::NV21) {
        DHLOGE("CopyInfo error : srcImgInfo colorFormat %d, dstImgInfo colorFormat %d.",
            srcImgInfo.colorFormat, dstImgInfo.colorFormat);
        return false;
    }

    if (!IsCorrectImageUnitInfo(srcImgInfo)) {
        DHLOGE("srcImginfo fail: width %d, height %d, alignedWidth %d, alignedHeight %d, chromaOffset %lld, " +
            "imgSize %lld.", srcImgInfo.width, srcImgInfo.height, srcImgInfo.alignedWidth, srcImgInfo.alignedHeight,
            srcImgInfo.chromaOffset, srcImgInfo.imgSize);
        return false;
    }
    if (!IsCorrectImageUnitInfo(dstImgInfo)) {
        DHLOGE("dstImginfo fail: width %d, height %d, alignedWidth %d, alignedHeight %d, chromaOffset %lld, " +
            "imgSize %lld.", dstImgInfo.width, dstImgInfo.height, dstImgInfo.alignedWidth, dstImgInfo.alignedHeight,
            dstImgInfo.chromaOffset, dstImgInfo.imgSize);
        return false;
    }

    if (dstImgInfo.width > srcImgInfo.alignedWidth || dstImgInfo.height > srcImgInfo.alignedHeight) {
        DHLOGE("Comparison ImgInfo fail: dstwidth %d, dstheight %d, srcAlignedWidth %d, srcAlignedHeight %d.",
            dstImgInfo.width, dstImgInfo.height, srcImgInfo.alignedWidth, srcImgInfo.alignedHeight);
        return false;
    }
    return true;
}

bool ColorFormatProcess::IsCorrectImageUnitInfo(const ImageUnitInfo& imgInfo)
{
    size_t expectedImgSize = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight *
                                                 YUV_BYTES_PER_PIXEL / Y2UV_RATIO);
    size_t expectedChromaOffset = static_cast<size_t>(imgInfo.alignedWidth * imgInfo.alignedHeight);
    return (imgInfo.width <= imgInfo.alignedWidth && imgInfo.height <= imgInfo.alignedHeight &&
        imgInfo.imgSize >= expectedImgSize && imgInfo.chromaOffset == expectedChromaOffset);
}

/**
* @brief Separate a row of srcUVPlane into half a row of dstUPlane and half a row of dstVPlane. For example,
* converts the UVPlane memory arrangement of NV12 to the UV memory arrangement of YUVI420. Note that the
* stride and width of the dstImage must be the same.
*/
void ColorFormatProcess::SeparateUVPlaneByRow(const uint8_t *srcUVPlane, uint8_t *dstUPlane, uint8_t *dstVPlane,
    int32_t srcHalfWidth)
{
    int32_t memoryOffset0 = 0;
    int32_t memoryOffset1 = 1;
    int32_t memoryOffset2 = 2;
    int32_t memoryOffset3 = 3;
    int32_t perSeparatebytes = 4;
    for (int32_t x = 0; x < srcHalfWidth - 1; x += memoryOffset2) {
        dstUPlane[x] = srcUVPlane[memoryOffset0];
        dstUPlane[x + memoryOffset1] = srcUVPlane[memoryOffset2];
        dstVPlane[x] = srcUVPlane[memoryOffset1];
        dstVPlane[x + memoryOffset1] = srcUVPlane[memoryOffset3];
        srcUVPlane += perSeparatebytes;
    }
    if (static_cast<uint32_t>(srcHalfWidth) & 1) {
        dstUPlane[srcHalfWidth - 1] = srcUVPlane[memoryOffset0];
        dstVPlane[srcHalfWidth - 1] = srcUVPlane[memoryOffset1];
    }
}

int32_t ColorFormatProcess::SeparateNV12UVPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (!CheckColorConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ColorFormatProcess : CheckColorConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    uint8_t *srcUVPlane = srcImgInfo.imgData + srcImgInfo.chromaOffset;
    int32_t srcUVStride = srcImgInfo.alignedWidth;
    uint8_t *dstUPlane = dstImgInfo.imgData + dstImgInfo.chromaOffset;
    int32_t dstUStride = dstImgInfo.alignedWidth / Y2UV_RATIO;
    uint8_t *dstVPlane = dstUPlane + (dstImgInfo.chromaOffset / Y2UV_RATIO) / Y2UV_RATIO;
    int32_t dstVStride = dstImgInfo.alignedWidth / Y2UV_RATIO;
    int32_t width = srcImgInfo.width / Y2UV_RATIO;
    int32_t height = srcImgInfo.height / Y2UV_RATIO;
    DHLOGD("srcUVStride %d, dstUStride %d, dstVStride %d, src half width %d, src half height %d.",
        srcUVStride, dstUStride, dstVStride, width, height);

    /* Negative height means invert the image. */
    if (height < 0) {
        height = -height;
        dstUPlane = dstUPlane + (height - 1) * dstUStride;
        dstVPlane = dstVPlane + (height - 1) * dstVStride;
        dstUStride = -dstUStride;
        dstVStride = -dstVStride;
    }
    /* No black border of srcImage and dstImage, and the strides of srcImage and dstImage are equal. */
    if (srcUVStride == width * Y2UV_RATIO && dstUStride == width && dstVStride == width) {
        SeparateUVPlaneByRow(srcUVPlane, dstUPlane, dstVPlane, width * height);
        return DCAMERA_OK;
    }
    /* Black borders exist in srcImage or dstImage. */
    for (int32_t y = 0; y < height; ++y) {
        SeparateUVPlaneByRow(srcUVPlane, dstUPlane, dstVPlane, width);
        dstUPlane += dstUStride;
        dstVPlane += dstVStride;
        srcUVPlane += srcUVStride;
    }
    return DCAMERA_OK;
}

/**
* @brief Combine half a row of srcUPlane and half a row of srcVPlane into a row of dstUVPlane. For example,
* converts the UVPlane memory arrangement of YUVI420 to the UV memory arrangement of NV12. Note that the
* stride and width of the srcImage must be the same.
*/
void ColorFormatProcess::CombineUVPlaneByRow(const uint8_t *srcUPlane, const uint8_t *srcVPlane, uint8_t *dstUVPlane,
    int32_t dstHalfWidth)
{
    int32_t memoryOffset0 = 0;
    int32_t memoryOffset1 = 1;
    int32_t memoryOffset2 = 2;
    int32_t memoryOffset3 = 3;
    int32_t perCombinebytes = 4;
    for (int32_t x = 0; x < dstHalfWidth - 1; x += memoryOffset2) {
        dstUVPlane[memoryOffset0] = srcUPlane[x];
        dstUVPlane[memoryOffset1] = srcVPlane[x];
        dstUVPlane[memoryOffset2] = srcUPlane[x + memoryOffset1];
        dstUVPlane[memoryOffset3] = srcVPlane[x + memoryOffset1];
        dstUVPlane += perCombinebytes;
    }
    if (static_cast<uint32_t>(dstHalfWidth) & 1) {
        dstUVPlane[memoryOffset0] = srcUPlane[dstHalfWidth - 1];
        dstUVPlane[memoryOffset1] = srcVPlane[dstHalfWidth - 1];
    }
}

int32_t ColorFormatProcess::CombineNV12UVPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (!CheckColorConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ColorFormatProcess : CheckColorConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    uint8_t *srcVPlane = srcImgInfo.imgData + srcImgInfo.chromaOffset;
    int32_t srcVStride = srcImgInfo.alignedWidth / Y2UV_RATIO;
    uint8_t *srcUPlane = srcVPlane + (srcImgInfo.chromaOffset / Y2UV_RATIO) / Y2UV_RATIO;
    int32_t srcUStride = srcImgInfo.alignedWidth / Y2UV_RATIO;
    uint8_t *dstUVPlane = dstImgInfo.imgData + dstImgInfo.chromaOffset;
    int32_t dstUVStride = dstImgInfo.alignedWidth;
    int32_t width = dstImgInfo.width / Y2UV_RATIO;
    int32_t height = dstImgInfo.height / Y2UV_RATIO;
    DHLOGD("srcUStride %d, srcVStride %d, dstUVStride %d, dst half width %d, dst half height %d.",
        srcUStride, srcVStride, dstUVStride, width, height);

    /* Negative height means invert the image. */
    if (height < 0) {
        height = -height;
        dstUVPlane = dstUVPlane + (height - 1) * dstUVStride;
        dstUVStride = -dstUVStride;
    }
    /* No black border of srcImage and dstImage, and the strides of srcImage and dstImage are equal. */
    if (srcUStride == width && srcVStride == width && dstUVStride == width * Y2UV_RATIO) {
        CombineUVPlaneByRow(srcUPlane, srcVPlane, dstUVPlane, width * height);
        return DCAMERA_OK;
    }
    /* Black borders exist in srcImage or dstImage. */
    for (int32_t y = 0; y < height; ++y) {
        CombineUVPlaneByRow(srcUPlane, srcVPlane, dstUVPlane, width);
        srcUPlane += srcUStride;
        srcVPlane += srcVStride;
        dstUVPlane += dstUVStride;
    }
    return DCAMERA_OK;
}

int32_t ColorFormatProcess::CopyYPlane(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (!CheckColorConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ColorFormatProcess : CheckColorConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t totalCopyYPlaneSize = dstImgInfo.alignedWidth * dstImgInfo.height;
    if (srcImgInfo.alignedWidth == dstImgInfo.width && dstImgInfo.alignedWidth == dstImgInfo.width) {
        /* No black border of srcImage and dstImage, and the strides of srcImage and dstImage are equal. */
        errno_t err = memcpy_s(dstImgInfo.imgData, totalCopyYPlaneSize, srcImgInfo.imgData, totalCopyYPlaneSize);
        if (err != EOK) {
            DHLOGE("ColorConvert : memcpy_s CopyYPlaner failed by Coalesce rows.");
            return DCAMERA_MEMORY_OPT_ERROR;
        }
    } else {
        /* Black borders exist in srcImage or dstImage. */
        int32_t srcDataOffset = 0;
        int32_t dstDataOffset = 0;
        for (int32_t yh = 0; yh < dstImgInfo.height; yh++) {
            errno_t err = memcpy_s(dstImgInfo.imgData + dstDataOffset, totalCopyYPlaneSize - dstDataOffset,
                srcImgInfo.imgData + srcDataOffset, dstImgInfo.width);
            if (err != EOK) {
                DHLOGE("memcpy_s YPlane in line[%d] failed.", yh);
                return DCAMERA_MEMORY_OPT_ERROR;
            }
            dstDataOffset += dstImgInfo.alignedWidth;
            srcDataOffset += srcImgInfo.alignedWidth;
        }
        DHLOGD("ColorConvert :get valid yplane OK, srcImgInfo: alignedWidth %d, width %d, height %d. " +
            "dstImgInfo: alignedWidth %d, width %d, height %d. dstDataOffset %d, srcDataOffset %d.",
            srcImgInfo.alignedWidth, srcImgInfo.width, srcImgInfo.height, dstImgInfo.alignedWidth,
            dstImgInfo.width, dstImgInfo.height, dstDataOffset, srcDataOffset);
    }
    return DCAMERA_OK;
}

int32_t ColorFormatProcess::ColorConvertNV12ToNV21(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (!CheckColorConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ColorFormatProcess : CheckColorConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t err = CopyYPlane(srcImgInfo, dstImgInfo);
    if (err != DCAMERA_OK) {
        DHLOGE("ColorConvertNV12ToNV21 : CopyYPlane failed.");
        return err;
    }

    std::shared_ptr<DataBuffer> tempPlaneYUV = std::make_shared<DataBuffer>(dstImgInfo.imgSize);
    ImageUnitInfo tempImgInfo = dstImgInfo;
    tempImgInfo.imgData = tempPlaneYUV->Data();
    SeparateNV12UVPlane(srcImgInfo, tempImgInfo);
    CombineNV12UVPlane(tempImgInfo, dstImgInfo);
    return DCAMERA_OK;
}

int32_t ColorFormatProcess::ColorConvertNV12ToI420(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (!CheckColorConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ColorFormatProcess : CheckColorConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    int32_t err = CopyYPlane(srcImgInfo, dstImgInfo);
    if (err != DCAMERA_OK) {
        DHLOGE("ColorConvertNV12ToNV21 : CopyYPlane failed.");
        return err;
    }

    SeparateNV12UVPlane(srcImgInfo, dstImgInfo);
    return DCAMERA_OK;
}

int32_t ColorFormatProcess::ColorConvertByColorFormat(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    int32_t ret;
    switch (srcImgInfo.colorFormat) {
        case Videoformat::NV12:
            switch (dstImgInfo.colorFormat) {
                case Videoformat::NV21:
                    ret = ColorConvertNV12ToNV21(srcImgInfo, dstImgInfo);
                    break;
                case Videoformat::YUVI420:
                    ret = ColorConvertNV12ToI420(srcImgInfo, dstImgInfo);
                    break;
                default:
                    DHLOGE("Unsupport ColorConvert %d to %d.", srcImgInfo.colorFormat, dstImgInfo.colorFormat);
                    return DCAMERA_BAD_OPERATE;
            }
            break;
        case Videoformat::NV21:
        case Videoformat::YUVI420:
        case Videoformat::RGBA_8888:
            DHLOGE("Unsupport ColorConvert %d to %d.", srcImgInfo.colorFormat, dstImgInfo.colorFormat);
            return DCAMERA_BAD_OPERATE;
    }
    return ret;
}

int32_t ColorFormatProcess::ColorFormatDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    DHLOGD("ColorFormat Done.");
    if (outputBuffers.empty()) {
        DHLOGE("The received data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (nextDataProcess_ != nullptr) {
        DHLOGD("Send to the next node of the decoder for processing.");
        int32_t err = nextDataProcess_->ProcessData(outputBuffers);
        if (err != DCAMERA_OK) {
            DHLOGE("Someone node after the decoder processes failed.");
        }
        return err;
    }
    DHLOGD("The current node is the last node, and Output the processed video buffer");
    std::shared_ptr<DCameraPipelineSource> targetPipelineSource = callbackPipelineSource_.lock();
    if (targetPipelineSource == nullptr) {
        DHLOGE("callbackPipelineSource_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    targetPipelineSource->OnProcessedVideoBuffer(outputBuffers[0]);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
