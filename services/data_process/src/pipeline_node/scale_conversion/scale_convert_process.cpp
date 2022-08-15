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

#include "scale_convert_process.h"

#include "distributed_hardware_log.h"
#include "distributed_camera_errno.h"

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
    processedConfig = processedConfig_;

    if (!IsConvertible(sourceConfig, targetConfig)) {
        DHLOGI("sourceConfig: Videoformat %d Width %d, Height %d, targetConfig: Videoformat %d Width %d, Height %d.",
            sourceConfig.GetVideoformat(), sourceConfig.GetWidth(), sourceConfig.GetHeight(),
            targetConfig.GetVideoformat(), targetConfig.GetWidth(), targetConfig.GetHeight());
        isScaleConvert_.store(true);
        return DCAMERA_OK;
    }

    int32_t ret = av_image_alloc(srcData_, srcLineSize_, sourceConfig_.GetWidth(), sourceConfig_.GetHeight(),
        GetAVPixelFormat(sourceConfig_.GetVideoformat()), SOURCE_ALIGN);
    if (ret < DCAMERA_OK) {
        DHLOGE("Could not allocate source image.");
        return DCAMERA_BAD_VALUE;
    }

    dstBuffSize_ = av_image_alloc(dstData_, dstLineSize_, processedConfig_.GetWidth(), processedConfig_.GetHeight(),
        GetAVPixelFormat(processedConfig_.GetVideoformat()), TARGET_ALIGN);
    if (dstBuffSize_ < DCAMERA_OK) {
        DHLOGE("Could not allocate destination image.");
        return DCAMERA_BAD_VALUE;
    }

    swsContext_ = sws_getContext(sourceConfig_.GetWidth(), sourceConfig_.GetHeight(),
        GetAVPixelFormat(sourceConfig_.GetVideoformat()), processedConfig_.GetWidth(), processedConfig_.GetHeight(),
        GetAVPixelFormat(processedConfig_.GetVideoformat()), SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
    if (swsContext_ == nullptr) {
        DHLOGE("Create SwsContext failed.");
        return DCAMERA_BAD_VALUE;
    }

    isScaleConvert_.store(true);
    return DCAMERA_OK;
}

bool ScaleConvertProcess::IsConvertible(const VideoConfigParams& sourceConfig, const VideoConfigParams& targetConfig)
{
    return (sourceConfig_.GetWidth() != targetConfig.GetWidth()) ||
        (sourceConfig_.GetHeight() != targetConfig.GetHeight());
}

void ScaleConvertProcess::ReleaseProcessNode()
{
    DHLOGI("Start release [%d] node : ScaleConvertNode.", nodeRank_);
    isScaleConvert_.store(false);

    {
        std::lock_guard<std::mutex> autoLock(scaleMutex_);
        if (swsContext_ != nullptr) {
            av_freep(&srcData_[0]);
            av_freep(&dstData_[0]);
            sws_freeContext(swsContext_);
            swsContext_ = nullptr;
        }
    }

    if (nextDataProcess_ != nullptr) {
        nextDataProcess_->ReleaseProcessNode();
        nextDataProcess_ = nullptr;
    }
    DHLOGI("Release [%d] node : ScaleConvertNode end.", nodeRank_);
}

int ScaleConvertProcess::ProcessData(std::vector<std::shared_ptr<DataBuffer>>& inputBuffers)
{
    DHLOGD("Process data in ScaleConvertProcess.");
    if (!isScaleConvert_.load()) {
        DHLOGE("Scale Convert node occurred error or start release.");
        return DCAMERA_DISABLE_PROCESS;
    }

    if (inputBuffers.empty() || inputBuffers[0] == nullptr) {
        DHLOGE("The input data buffers is empty.");
        return DCAMERA_BAD_VALUE;
    }

    if (!IsConvertible(sourceConfig_, processedConfig_)) {
        DHLOGD("The target resolution: %dx%d is the same as the source resolution: %dx%d",
            processedConfig_.GetWidth(), processedConfig_.GetHeight(),
            sourceConfig_.GetWidth(), sourceConfig_.GetHeight());
        return ConvertDone(inputBuffers);
    }

    int64_t timeStamp = 0;
    if (!(inputBuffers[0]->FindInt64("timeUs", timeStamp))) {
        DHLOGE("ScaleConvertProcess : Find inputBuffer timeStamp failed.");
        return DCAMERA_BAD_VALUE;
    }

    ImageUnitInfo srcImgInfo {Videoformat::YUVI420, 0, 0, 0, 0, 0, 0, nullptr};
    if ((GetImageUnitInfo(srcImgInfo, inputBuffers[0]) != DCAMERA_OK) || !CheckScaleProcessInputInfo(srcImgInfo)) {
        DHLOGE("ScaleConvertProcess : srcImgInfo error.");
        return DCAMERA_BAD_VALUE;
    }

    std::shared_ptr<DataBuffer> dstBuf = std::make_shared<DataBuffer>(dstBuffSize_);
    ImageUnitInfo dstImgInfo = { processedConfig_.GetVideoformat(), processedConfig_.GetWidth(),
        processedConfig_.GetHeight(), processedConfig_.GetWidth(), processedConfig_.GetHeight(),
        processedConfig_.GetWidth() * processedConfig_.GetHeight(), dstBuf->Size(), dstBuf->Data() };
    if (ScaleConvert(srcImgInfo, dstImgInfo) != DCAMERA_OK) {
        DHLOGE("ScaleConvertProcess : Scale convert failed.");
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
    return ConvertDone(outputBuffers);
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
    DHLOGD("ScaleConvertProcess imgBuf info : Videoformat %d, alignedWidth %d, alignedHeight %d, width %d, height %d" +
        ", chromaOffset %d, imgSize %d.", imgInfo.colorFormat, imgInfo.width, imgInfo.height, imgInfo.alignedWidth,
        imgInfo.alignedHeight, imgInfo.chromaOffset, imgInfo.imgSize);
    return DCAMERA_OK;
}

bool ScaleConvertProcess::CheckScaleProcessInputInfo(const ImageUnitInfo& srcImgInfo)
{
    return srcImgInfo.colorFormat == sourceConfig_.GetVideoformat() &&
        srcImgInfo.alignedWidth == sourceConfig_.GetWidth() &&
        srcImgInfo.alignedHeight == sourceConfig_.GetHeight() &&
        IsCorrectImageUnitInfo(srcImgInfo);
}

bool ScaleConvertProcess::CheckScaleConvertInfo(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    if (srcImgInfo.imgData == nullptr || dstImgInfo.imgData == nullptr) {
        DHLOGE("The imgData of srcImgInfo or the imgData of dstImgInfo are null!");
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

    if ((dstImgInfo.width == srcImgInfo.alignedWidth) && (dstImgInfo.height == srcImgInfo.alignedHeight)) {
        DHLOGE("Comparison ImgInfo fail: dstwidth %d, dstheight %d, srcAlignedWidth %d, srcAlignedHeight %d.",
            dstImgInfo.width, dstImgInfo.height, srcImgInfo.alignedWidth, srcImgInfo.alignedHeight);
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

int32_t ScaleConvertProcess::ScaleConvert(const ImageUnitInfo& srcImgInfo, const ImageUnitInfo& dstImgInfo)
{
    DHLOGD("ScaleConvertProcess : Scale convert start.");
    if (!CheckScaleConvertInfo(srcImgInfo, dstImgInfo)) {
        DHLOGE("ScaleConvertProcess : CheckScaleConvertInfo failed.");
        return DCAMERA_BAD_VALUE;
    }

    std::lock_guard<std::mutex> autoLock(scaleMutex_);
    switch (GetAVPixelFormat(srcImgInfo.colorFormat)) {
        case AV_PIX_FMT_YUV420P: {
            int32_t ret = CopyYUV420SrcData(srcImgInfo);
            if (ret != DCAMERA_OK) {
                DHLOGE("ScaleConvertProcess::ScaleConvert copy yuv420p src data failed.");
                return ret;
            }
            break;
        }
        case AV_PIX_FMT_NV12: {
            int32_t ret = CopyNV12SrcData(srcImgInfo);
            if (ret != DCAMERA_OK) {
                DHLOGE("ScaleConvertProcess::ScaleConvert copy nv12 src data failed.");
                return ret;
            }
            break;
        }
        case AV_PIX_FMT_NV21: {
            int32_t ret = CopyNV21SrcData(srcImgInfo);
            if (ret != DCAMERA_OK) {
                DHLOGE("ScaleConvertProcess::ScaleConvert copy nv21 src data failed.");
                return ret;
            }
            break;
        }
        default:
            DHLOGE("Unknown pixel format not support.");
            return DCAMERA_BAD_VALUE;
    }

    sws_scale(swsContext_, (const uint8_t * const *)srcData_, srcLineSize_, 0, srcImgInfo.alignedHeight,
        dstData_, dstLineSize_);
    int32_t ret = memcpy_s(dstImgInfo.imgData, dstImgInfo.imgSize, dstData_[0], dstBuffSize_);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::ScaleConvert copy dst image info failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::CopyYUV420SrcData(const ImageUnitInfo& srcImgInfo)
{
    int32_t ret = memcpy_s(srcData_[0], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.imgData, srcImgInfo.alignedWidth * srcImgInfo.alignedHeight);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyYUV420SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    ret = memcpy_s(srcData_[1], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_YUV,
        srcImgInfo.imgData + srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_YUV);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyYUV420SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    ret = memcpy_s(srcData_[2], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_YUV,
        srcImgInfo.imgData + srcImgInfo.alignedWidth * srcImgInfo.alignedHeight +
        srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_YUV,
        srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_YUV);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyYUV420SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::CopyNV12SrcData(const ImageUnitInfo& srcImgInfo)
{
    int32_t ret = memcpy_s(srcData_[0], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.imgData, srcImgInfo.alignedWidth * srcImgInfo.alignedHeight);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyNV12SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    ret = memcpy_s(srcData_[1], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_NV,
        srcImgInfo.imgData + srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_NV);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyNV12SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::CopyNV21SrcData(const ImageUnitInfo& srcImgInfo)
{
    int32_t ret = memcpy_s(srcData_[0], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.imgData, srcImgInfo.alignedWidth * srcImgInfo.alignedHeight);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyNV21SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }

    ret = memcpy_s(srcData_[1], srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_NV,
        srcImgInfo.imgData + srcImgInfo.alignedWidth * srcImgInfo.alignedHeight,
        srcImgInfo.alignedWidth * srcImgInfo.alignedHeight / MEMORY_RATIO_NV);
    if (ret != EOK) {
        DHLOGE("ScaleConvertProcess::CopyNV21SrcData memory copy failed, ret = %d", ret);
        return DCAMERA_MEMORY_OPT_ERROR;
    }
    return DCAMERA_OK;
}

int32_t ScaleConvertProcess::ConvertDone(std::vector<std::shared_ptr<DataBuffer>>& outputBuffers)
{
    DHLOGD("ScaleConvertProcess : Convert Done.");
    if (outputBuffers.empty()) {
        DHLOGE("The received data buffer is empty.");
        return DCAMERA_BAD_VALUE;
    }
    
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
        default:
            format = AVPixelFormat::AV_PIX_FMT_YUV420P;
            break;
    }
    return format;
}
} // namespace DistributedHardware
} // namespace OHOS
