/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
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

#include "dcamera_client.h"

#include "anonymous_string.h"
#include "camera_util.h"
#include "camera_metadata_operator.h"
#include "dcamera_input_callback.h"
#include "dcamera_manager_callback.h"
#include "dcamera_photo_callback.h"
#include "dcamera_preview_callback.h"
#include "dcamera_session_callback.h"
#include "dcamera_utils_tools.h"
#include "dcamera_video_callback.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"

namespace OHOS {
namespace DistributedHardware {
DCameraClient::DCameraClient(const std::string& dhId)
{
    DHLOGI("DCameraClient Constructor dhId: %s", GetAnonyString(dhId).c_str());
    cameraId_ = dhId.substr(CAMERA_ID_PREFIX.size());
    isInit_ = false;
}

DCameraClient::~DCameraClient()
{
    if (isInit_) {
        UnInit();
    }
}

int32_t DCameraClient::Init()
{
    DHLOGI("Init cameraId: %s", GetAnonyString(cameraId_).c_str());
    cameraManager_ = CameraStandard::CameraManager::GetInstance();
    if (cameraManager_ == nullptr) {
        DHLOGE("Init cameraManager getInstance failed");
        return DCAMERA_BAD_VALUE;
    }
    cameraManager_->SetCallback(std::make_shared<DCameraManagerCallback>());

    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    if (cameraList.empty()) {
        DHLOGE("Init no camera device");
        return DCAMERA_BAD_VALUE;
    }
    DHLOGI("Init camera size: %d", cameraList.size());
    for (auto& info : cameraList) {
        if (info->GetID() == cameraId_) {
            DHLOGI("Init cameraInfo get id: %s", GetAnonyString(info->GetID()).c_str());
            cameraInfo_ = info;
            break;
        }
    }
    if (cameraInfo_ == nullptr) {
        DHLOGE("Init cameraInfo is null");
        return DCAMERA_BAD_VALUE;
    }

    isInit_ = true;
    DHLOGI("Init %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::UnInit()
{
    DHLOGI("UnInit cameraId: %s", GetAnonyString(cameraId_).c_str());
    if (cameraManager_ != nullptr) {
        DHLOGI("UnInit unregister cameraManager callback");
        cameraManager_->SetCallback(nullptr);
    }

    isInit_ = false;
    cameraInfo_ = nullptr;
    cameraManager_ = nullptr;
    DHLOGI("UnInit %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("UpdateSettings cameraId: %s", GetAnonyString(cameraId_).c_str());
    for (auto& setting : settings) {
        switch (setting->type_) {
            case UPDATE_METADATA: {
                DHLOGI("UpdateSettings %s update metadata settings", GetAnonyString(cameraId_).c_str());
                std::string dcSettingValue = setting->value_;
                std::string metadataStr = Base64Decode(dcSettingValue);
                FindCameraMetadata(metadataStr);

                if (cameraInput_ == nullptr) {
                    DHLOGE("UpdateSettings %s cameraInput is null", GetAnonyString(cameraId_).c_str());
                    UpdateSettingCache(metadataStr);
                    return DCAMERA_OK;
                }

                int32_t ret = ((sptr<CameraStandard::CameraInput> &)cameraInput_)->SetCameraSettings(metadataStr);
                if (ret != DCAMERA_OK) {
                    DHLOGE("UpdateSettings %s update metadata settings failed, ret: %d",
                        GetAnonyString(cameraId_).c_str(), ret);
                    return ret;
                }
                break;
            }
            default: {
                DHLOGE("UpdateSettings unknown setting type");
                break;
            }
        }
    }
    DHLOGI("UpdateSettings %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

void DCameraClient::UpdateSettingCache(const std::string& metadataStr)
{
    if (cameraMetadatas_.size() == DCAMERA_MAX_METADATA_SIZE) {
        DHLOGE("UpdateSettingCache %s camera metadata oversize",
            GetAnonyString(cameraId_).c_str());
        cameraMetadatas_.pop();
    }
    cameraMetadatas_.push(metadataStr);
}

void DCameraClient::FindCameraMetadata(const std::string& metadataStr)
{
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = Camera::MetadataUtils::DecodeFromString(metadataStr);
    camera_metadata_item_t focusItem;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_FOCUS_MODE, &focusItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata focus mode: %d", focusItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %s find focus mode failed, ret: %d",
            GetAnonyString(cameraId_).c_str(), ret);
    }

    camera_metadata_item_t exposureItem;
    ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_EXPOSURE_MODE, &exposureItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata exposure mode: %d", exposureItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %s find exposure mode failed, ret: %d",
            GetAnonyString(cameraId_).c_str(), ret);
    }

    camera_metadata_item_t stabilityItem;
    ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_VIDEO_STABILIZATION_MODE, &stabilityItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata stabilization mode: %d", stabilityItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %s find stabilization mode failed, ret: %d",
            GetAnonyString(cameraId_).c_str(), ret);
    }
}

int32_t DCameraClient::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
    sptr<Surface>& surface)
{
    DHLOGI("StartCapture cameraId: %s", GetAnonyString(cameraId_).c_str());
    if ((photoOutput_ == nullptr) && (previewOutput_ == nullptr)) {
        DHLOGI("StartCapture %s config capture session", GetAnonyString(cameraId_).c_str());
        if (surface == nullptr) {
            DHLOGE("StartCapture: input surface is nullptr.");
            return DCAMERA_BAD_VALUE;
        }
        previewSurface_ = surface;
        int32_t ret = ConfigCaptureSession(captureInfos);
        if (ret != DCAMERA_OK) {
            DHLOGE("StartCapture config capture session failed, cameraId: %s, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
            return CameraServiceErrorType(ret);
        }
    }

    for (auto& info : captureInfos) {
        if ((info->streamType_ == CONTINUOUS_FRAME) || (!info->isCapture_)) {
            continue;
        }
        int32_t ret = StartCaptureInner(info);
        if (ret != DCAMERA_OK) {
            DHLOGE("StartCapture failed, cameraId: %s, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
            return CameraServiceErrorType(ret);
        }
    }
    DHLOGI("StartCapture %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::CameraServiceErrorType(const int32_t errorType)
{
    if (errorType == CameraStandard::CamServiceError::CAMERA_ALLOC_ERROR) {
        return DCAMERA_ALLOC_ERROR;
    } else if (errorType == CameraStandard::CamServiceError::CAMERA_DEVICE_BUSY) {
        return DCAMERA_DEVICE_BUSY;
    }
    return errorType;
}

int32_t DCameraClient::StopCapture()
{
    DHLOGI("StopCapture cameraId: %s", GetAnonyString(cameraId_).c_str());
    if (previewOutput_ != nullptr) {
        DHLOGI("StopCapture %s stop previewOutput", GetAnonyString(cameraId_).c_str());
        int32_t ret = ((sptr<CameraStandard::PreviewOutput> &)previewOutput_)->Stop();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture videoOutput stop failed, cameraId: %s, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        DHLOGI("StopCapture %s release previewOutput", GetAnonyString(cameraId_).c_str());
        ret = previewOutput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture previewOutput Release failed, cameraId: %s, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        previewOutput_ = nullptr;
    }

    if (photoOutput_ != nullptr) {
        DHLOGI("StopCapture %s release photoOutput", GetAnonyString(cameraId_).c_str());
        int32_t ret = photoOutput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture photoOutput Release failed, cameraId: %s, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        photoOutput_ = nullptr;
    }

    if (cameraInput_ != nullptr) {
        DHLOGI("StopCapture %s release cameraInput", GetAnonyString(cameraId_).c_str());
        int32_t ret = cameraInput_->Close();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture cameraInput Close failed, cameraId: %s, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        ret = cameraInput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture cameraInput Release failed, cameraId: %s, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        cameraInput_ = nullptr;
    }
    ReleaseCaptureSession();

    if (previewSurface_ != nullptr) {
        DHLOGI("StopCapture %s preview surface unregister consumer listener",
            GetAnonyString(cameraId_).c_str());
        previewSurface_ = nullptr;
    }

    if (photoSurface_ != nullptr) {
        DHLOGI("StopCapture %s photo surface unregister consumer listener",
            GetAnonyString(cameraId_).c_str());
        int32_t ret = photoSurface_->UnregisterConsumerListener();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture %s photo surface unregister consumer listener failed, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        photoListener_ = nullptr;
        photoSurface_ = nullptr;
    }

    DHLOGI("StopCapture %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

void DCameraClient::ReleaseCaptureSession()
{
    if (captureSession_ == nullptr) {
        return;
    }
    DHLOGI("StopCapture %s stop captureSession", GetAnonyString(cameraId_).c_str());
    int32_t ret = captureSession_->Stop();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopCapture captureSession stop failed, cameraId: %s, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
    }
    DHLOGI("StopCapture %s release captureSession", GetAnonyString(cameraId_).c_str());
    ret = captureSession_->Release();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopCapture captureSession Release failed, cameraId: %s, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
    }
    captureSession_ = nullptr;
}

int32_t DCameraClient::SetStateCallback(std::shared_ptr<StateCallback>& callback)
{
    DHLOGI("SetStateCallback cameraId: %s", GetAnonyString(cameraId_).c_str());
    if (callback == nullptr) {
        DHLOGE("SetStateCallback %s unregistering state callback", GetAnonyString(cameraId_).c_str());
    }
    stateCallback_ = callback;
    return DCAMERA_OK;
}

int32_t DCameraClient::SetResultCallback(std::shared_ptr<ResultCallback>& callback)
{
    DHLOGI("SetResultCallback cameraId: %s", GetAnonyString(cameraId_).c_str());
    if (callback == nullptr) {
        DHLOGE("SetResultCallback %s unregistering result callback", GetAnonyString(cameraId_).c_str());
    }
    resultCallback_ = callback;
    return DCAMERA_OK;
}

int32_t DCameraClient::ConfigCaptureSession(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    DHLOGI("ConfigCaptureSession cameraId: %s", GetAnonyString(cameraId_).c_str());
    int rv = cameraManager_->CreateCameraInput(cameraInfo_, &((sptr<CameraStandard::CameraInput> &)cameraInput_));
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s create cameraInput failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    int32_t rc = ((sptr<CameraStandard::CameraInput> &)cameraInput_)->Open();
    if (rc != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession cameraInput_ Open failed, cameraId: %s, ret: %d",
            GetAnonyString(cameraId_).c_str(), rc);
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraInputCallback> inputCallback = std::make_shared<DCameraInputCallback>(stateCallback_);
    ((sptr<CameraStandard::CameraInput> &)cameraInput_)->SetErrorCallback(inputCallback);

    while (!cameraMetadatas_.empty()) {
        std::string metadataStr = cameraMetadatas_.front();
        FindCameraMetadata(metadataStr);
        int32_t ret = ((sptr<CameraStandard::CameraInput> &)cameraInput_)->SetCameraSettings(metadataStr);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession %s set camera settings failed, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
        cameraMetadatas_.pop();
    }

    rv = cameraManager_->CreateCaptureSession(&captureSession_);
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s create captureSession failed",
               GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    std::shared_ptr<DCameraSessionCallback> sessionCallback = std::make_shared<DCameraSessionCallback>(stateCallback_);
    captureSession_->SetFocusCallback(sessionCallback);
    captureSession_->SetCallback(sessionCallback);

    int32_t ret = CreateCaptureOutput(captureInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession create capture output failed, cameraId: %s, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    return ConfigCaptureSessionInner();
}

int32_t DCameraClient::ConfigCaptureSessionInner()
{
    int32_t ret = captureSession_->BeginConfig();
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s config captureSession failed, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    ret = captureSession_->AddInput(cameraInput_);
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s add cameraInput to captureSession failed, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    if (photoOutput_ != nullptr) {
        ret = captureSession_->AddOutput(photoOutput_);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession %s add photoOutput to captureSession failed, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
    }

    if (previewOutput_ != nullptr) {
        ret = captureSession_->AddOutput(previewOutput_);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession %s add previewOutput to captureSession failed, ret: %d",
                   GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
    }

    ret = captureSession_->CommitConfig();
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s commit captureSession failed, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    ret = captureSession_->Start();
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %s start captureSession failed, ret: %d",
               GetAnonyString(cameraId_).c_str(), ret);
    }

    DHLOGI("ConfigCaptureSession %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::CreateCaptureOutput(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    if (captureInfos.empty()) {
        DHLOGE("CreateCaptureOutput no capture info, cameraId: %s", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    for (auto& info : captureInfos) {
        if (info->streamType_ == SNAPSHOT_FRAME) {
            int32_t ret = CreatePhotoOutput(info);
            if (ret != DCAMERA_OK) {
                DHLOGE("CreateCaptureOutput %s create photo output failed, ret: %d",
                       GetAnonyString(cameraId_).c_str(), ret);
                return ret;
            }
        } else if (info->streamType_ == CONTINUOUS_FRAME) {
            int32_t ret = CreatePreviewOutput(info);
            if (ret != DCAMERA_OK) {
                DHLOGE("CreateCaptureOutput %s create video output failed, ret: %d",
                       GetAnonyString(cameraId_).c_str(), ret);
                return ret;
            }
        } else {
            DHLOGE("CreateCaptureOutput unknown stream type");
            return DCAMERA_BAD_VALUE;
        }
    }
    return DCAMERA_OK;
}

int32_t DCameraClient::CreatePhotoOutput(std::shared_ptr<DCameraCaptureInfo>& info)
{
    DHLOGI("CreatePhotoOutput dhId: %s, width: %d, height: %d, format: %d, stream: %d, isCapture: %d",
        GetAnonyString(cameraId_).c_str(), info->width_, info->height_, info->format_, info->streamType_,
        info->isCapture_);
    photoSurface_ = IConsumerSurface::Create();
    photoListener_ = new DCameraPhotoSurfaceListener(photoSurface_, resultCallback_);
    photoSurface_->RegisterConsumerListener((sptr<IBufferConsumerListener> &)photoListener_);
    CameraStandard::CameraFormat photoFormat = ConvertToCameraFormat(info->format_);
    CameraStandard::Size photoSize = {info->width_, info->height_};
    CameraStandard::Profile photoProfile(photoFormat, photoSize);
    sptr<IBufferProducer> bp = photoSurface_->GetProducer();
    int32_t rv = cameraManager_->CreatePhotoOutput(
        photoProfile, bp, &((sptr<CameraStandard::PhotoOutput> &)photoOutput_));
    if (rv != DCAMERA_OK) {
        DHLOGE("CreatePhotoOutput %s create photo output failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraPhotoCallback> photoCallback = std::make_shared<DCameraPhotoCallback>(stateCallback_);
    ((sptr<CameraStandard::PhotoOutput> &)photoOutput_)->SetCallback(photoCallback);
    DHLOGI("CreatePhotoOutput %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::CreatePreviewOutput(std::shared_ptr<DCameraCaptureInfo>& info)
{
    DHLOGI("CreatePreviewOutput dhId: %s, width: %d, height: %d, format: %d, stream: %d, isCapture: %d",
        GetAnonyString(cameraId_).c_str(), info->width_, info->height_, info->format_, info->streamType_,
        info->isCapture_);
    CameraStandard::CameraFormat previewFormat = ConvertToCameraFormat(info->format_);
    CameraStandard::Size previewSize = {info->width_, info->height_};
    CameraStandard::Profile previewProfile(previewFormat, previewSize);
    int32_t rv = cameraManager_->CreatePreviewOutput(
        previewProfile, previewSurface_, &((sptr<CameraStandard::PreviewOutput> &)previewOutput_));
    if (rv != DCAMERA_OK) {
        DHLOGE("CreatePreviewOutput %s create preview output failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    auto previewCallback = std::make_shared<DCameraPreviewCallback>(stateCallback_);
    ((sptr<CameraStandard::PreviewOutput> &)previewOutput_)->SetCallback(previewCallback);
    DHLOGI("CreatePreviewOutput %s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

CameraStandard::CameraFormat DCameraClient::ConvertToCameraFormat(int32_t format)
{
    CameraStandard::CameraFormat ret = CameraStandard::CameraFormat::CAMERA_FORMAT_INVALID;
    DCameraFormat df = static_cast<DCameraFormat>(format);
    switch (df) {
        case DCameraFormat::OHOS_CAMERA_FORMAT_RGBA_8888:
            ret = CameraStandard::CameraFormat::CAMERA_FORMAT_RGBA_8888;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_YCBCR_420_888:
            ret = CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_420_888;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_YCRCB_420_SP:
            ret = CameraStandard::CameraFormat::CAMERA_FORMAT_YUV_420_SP;
            break;
        case DCameraFormat::OHOS_CAMERA_FORMAT_JPEG:
            ret = CameraStandard::CameraFormat::CAMERA_FORMAT_JPEG;
            break;
        default:
            break;
    }
    return ret;
}

int32_t DCameraClient::StartCaptureInner(std::shared_ptr<DCameraCaptureInfo>& info)
{
    if (info->streamType_ != SNAPSHOT_FRAME) {
        DHLOGE("StartCaptureInner unknown stream type");
        return DCAMERA_BAD_VALUE;
    }
    return StartPhotoOutput(info);
}

int32_t DCameraClient::StartPhotoOutput(std::shared_ptr<DCameraCaptureInfo>& info)
{
    DHLOGI("StartPhotoOutput cameraId: %s", GetAnonyString(cameraId_).c_str());
    if (photoOutput_ == nullptr) {
        DHLOGE("StartPhotoOutput photoOutput is null");
        return DCAMERA_BAD_VALUE;
    }

    std::vector<std::shared_ptr<DCameraSettings>> captureSettings = info->captureSettings_;
    std::string metadataSetting;
    for (const auto& setting : captureSettings) {
        if (setting->type_ == UPDATE_METADATA) {
            DHLOGI("StartPhotoOutput %s update metadata settings", GetAnonyString(cameraId_).c_str());
            metadataSetting = setting->value_;
        }
    }

    if (metadataSetting.empty()) {
        DHLOGE("StartPhotoOutput no metadata settings to update");
        int32_t ret = ((sptr<CameraStandard::PhotoOutput> &)photoOutput_)->Capture();
        if (ret != DCAMERA_OK) {
            DHLOGE("StartPhotoOutput %s photoOutput capture failed, ret: %d",
                GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
        return DCAMERA_OK;
    }

    std::string metadataStr = Base64Decode(metadataSetting);
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = Camera::MetadataUtils::DecodeFromString(metadataStr);
    std::shared_ptr<CameraStandard::PhotoCaptureSetting> photoCaptureSetting =
        std::make_shared<CameraStandard::PhotoCaptureSetting>();
    SetPhotoCaptureRotation(cameraMetadata, photoCaptureSetting);
    SetPhotoCaptureQuality(cameraMetadata, photoCaptureSetting);
    SetPhotoCaptureLocation(cameraMetadata, photoCaptureSetting);
    int32_t ret = ((sptr<CameraStandard::PhotoOutput> &)photoOutput_)->Capture(photoCaptureSetting);
    if (ret != DCAMERA_OK) {
        DHLOGE("StartPhotoOutput %s photoOutput capture failed, ret: %d",
            GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }
    return DCAMERA_OK;
}

void DCameraClient::SetPhotoCaptureRotation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    uint32_t rotationCount = 1;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_ORIENTATION, &item);
    if ((ret == CAM_META_SUCCESS) && (rotationCount == item.count)) {
        CameraStandard::PhotoCaptureSetting::RotationConfig rotation =
            static_cast<CameraStandard::PhotoCaptureSetting::RotationConfig>(item.data.i32[0]);
        photoCaptureSetting->SetRotation(rotation);
        DHLOGI("SetPhotoCaptureRotation %s photo capture settings set %d rotation: %d",
            GetAnonyString(cameraId_).c_str(), item.count, rotation);
    }
}

void DCameraClient::SetPhotoCaptureQuality(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    uint32_t qualityCount = 1;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_QUALITY, &item);
    if ((ret == CAM_META_SUCCESS) && (qualityCount == item.count)) {
        CameraStandard::PhotoCaptureSetting::QualityLevel quality =
            static_cast<CameraStandard::PhotoCaptureSetting::QualityLevel>(item.data.u8[0]);
        photoCaptureSetting->SetQuality(quality);
        DHLOGI("SetPhotoCaptureQuality %s photo capture settings set %d quality: %d",
            GetAnonyString(cameraId_).c_str(), item.count, quality);
    }
}

void DCameraClient::SetPhotoCaptureLocation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    uint32_t locationCount = 3;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_GPS_COORDINATES, &item);
    if ((ret == CAM_META_SUCCESS) && (locationCount == item.count)) {
        int32_t latitudeIndex = 0;
        int32_t longitudeIndex = 1;
        int32_t altitudeIndex = 2;
        std::unique_ptr<CameraStandard::Location> location = std::make_unique<CameraStandard::Location>();
        location->latitude = item.data.d[latitudeIndex];
        location->longitude = item.data.d[longitudeIndex];
        location->altitude = item.data.d[altitudeIndex];
        photoCaptureSetting->SetLocation(location);
        DHLOGI("SetPhotoCaptureLocation %s photo capture settings set %d location: " +
            "latitude=%f, longitude=%f, altitude=%f", GetAnonyString(cameraId_).c_str(), item.count,
            item.data.d[latitudeIndex], item.data.d[longitudeIndex], item.data.d[altitudeIndex]);
    }
}
} // namespace DistributedHardware
} // namespace OHOS
