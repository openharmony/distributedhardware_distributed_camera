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
    DHLOGI("DCameraClient Constructor dhId: %{public}s", GetAnonyString(dhId).c_str());
    if (dhId.size() >= CAMERA_ID_PREFIX.size()) {
        cameraId_ = dhId.substr(CAMERA_ID_PREFIX.size());
    }
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
    DHLOGI("Init cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
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
    uint64_t listSize = static_cast<uint64_t>(cameraList.size());
    DHLOGI("Init camera size: %{public}" PRIu64, listSize);
    for (auto& info : cameraList) {
        if (info != nullptr && info->GetID() == cameraId_) {
            DHLOGI("Init cameraInfo get id: %{public}s", GetAnonyString(info->GetID()).c_str());
            cameraInfo_ = info;
            break;
        }
    }
    if (cameraInfo_ == nullptr) {
        DHLOGE("Init cameraInfo is null");
        return DCAMERA_BAD_VALUE;
    }

    isInit_ = true;
    DHLOGI("Init %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::UnInit()
{
    DHLOGI("UnInit cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    if (cameraManager_ != nullptr) {
        DHLOGI("UnInit unregister cameraManager callback");
        cameraManager_->SetCallback(nullptr);
    }

    isInit_ = false;
    cameraInfo_ = nullptr;
    cameraManager_ = nullptr;
    DHLOGI("UnInit %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::UpdateSettings(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("UpdateSettings cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    for (auto& setting : settings) {
        if (setting == nullptr) {
            DHLOGE("UpdateSettings setting is null");
            continue;
        }
        switch (setting->type_) {
            case UPDATE_METADATA: {
                DHLOGI("UpdateSettings %{public}s update metadata settings", GetAnonyString(cameraId_).c_str());
                std::string dcSettingValue = setting->value_;
                std::string metadataStr = Base64Decode(dcSettingValue);
                FindCameraMetadata(metadataStr);

                if (cameraInput_ == nullptr) {
                    DHLOGE("UpdateSettings %{public}s cameraInput is null", GetAnonyString(cameraId_).c_str());
                    UpdateSettingCache(metadataStr);
                    return DCAMERA_OK;
                }

                int32_t ret = ((sptr<CameraStandard::CameraInput> &)cameraInput_)->SetCameraSettings(metadataStr);
                if (ret != DCAMERA_OK) {
                    DHLOGE("UpdateSettings %{public}s update metadata settings failed, ret: %{public}d",
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
    DHLOGI("UpdateSettings %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

void DCameraClient::UpdateSettingCache(const std::string& metadataStr)
{
    if (cameraMetadatas_.size() == DCAMERA_MAX_METADATA_SIZE) {
        DHLOGE("UpdateSettingCache %{public}s camera metadata oversize",
            GetAnonyString(cameraId_).c_str());
        cameraMetadatas_.pop();
    }
    cameraMetadatas_.push(metadataStr);
}

void DCameraClient::FindCameraMetadata(const std::string& metadataStr)
{
    std::shared_ptr<Camera::CameraMetadata> cameraMetadata = Camera::MetadataUtils::DecodeFromString(metadataStr);
    CHECK_AND_RETURN_LOG(cameraMetadata == nullptr, "FindCameraMetadata get cameraMetadata is null");
    camera_metadata_item_t focusItem;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_FOCUS_MODE, &focusItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata focus mode: %{public}d", focusItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %{public}s find focus mode failed, ret: %{public}d",
            GetAnonyString(cameraId_).c_str(), ret);
    }

    camera_metadata_item_t exposureItem;
    ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_EXPOSURE_MODE, &exposureItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata exposure mode: %{public}d", exposureItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %{public}s find exposure mode failed, ret: %{public}d",
            GetAnonyString(cameraId_).c_str(), ret);
    }

    camera_metadata_item_t stabilityItem;
    ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_VIDEO_STABILIZATION_MODE, &stabilityItem);
    if (ret == CAM_META_SUCCESS) {
        DHLOGI("FindCameraMetadata stabilization mode: %{public}d", stabilityItem.data.u8[0]);
    } else {
        DHLOGE("FindCameraMetadata %{public}s find stabilization mode failed, ret: %{public}d",
            GetAnonyString(cameraId_).c_str(), ret);
    }
}

int32_t DCameraClient::StartCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
    sptr<Surface>& surface, int32_t sceneMode)
{
    DHLOGI("StartCapture (legacy entry) cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    int32_t ret = PrepareCapture(captureInfos, sceneMode);
    if (ret != DCAMERA_OK) {
        DHLOGE("StartCapture failed during PrepareCapture phase, raw ret: %{public}d", ret);
        return ret;
    }
    ret = CommitCapture(surface);
    if (ret != DCAMERA_OK) {
        DHLOGE("StartCapture failed during CommitCapture phase, raw ret: %{public}d", ret);
        return ret;
    }
    return DCAMERA_OK;
}

int32_t DCameraClient::PrepareCapture(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos, int32_t sceneMode)
{
    DHLOGI("PrepareCapture cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    int32_t ret = DCAMERA_OK;
    if (photoOutput_ == nullptr && previewOutput_ == nullptr) {
        ret = ConfigCaptureSession(captureInfos, sceneMode);
    }
    captureInfosCache_ = captureInfos;

    return CameraServiceErrorType(ret);
}

int32_t DCameraClient::CommitCapture(sptr<Surface>& surface)
{
    DHLOGI("CommitCapture cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    if (surface == nullptr) {
        DHLOGE("CommitCapture: input surface is null.");
        return DCAMERA_BAD_VALUE;
    }
    previewSurface_ = surface;
    GetFpsRanges();
    int32_t ret = DCAMERA_OK;
    if (photoOutput_ == nullptr && previewOutput_ == nullptr) {
        ret = CreateCaptureOutput(captureInfosCache_);
        if (ret != DCAMERA_OK) {
            DHLOGE("CommitCapture: CreateCaptureOutput failed, ret: %{public}d", ret);
            return CameraServiceErrorType(ret);
        }
        ret = ConfigCaptureSessionInner();
        if (ret != DCAMERA_OK) {
            DHLOGE("CommitCapture: ConfigCaptureSessionInner failed, ret: %{public}d", ret);
            return CameraServiceErrorType(ret);
        }
    }
    for (auto& info : captureInfosCache_) {
        if (info == nullptr) {
            continue;
        }
        if ((info->streamType_ == CONTINUOUS_FRAME) || (!info->isCapture_)) {
            continue;
        }
        int32_t innerRet = StartCaptureInner(info);
        if (innerRet != DCAMERA_OK) {
            DHLOGE("CommitCapture failed during StartCaptureInner, streamType: %d, ret: %{public}d",
                info->streamType_, innerRet);
            ret = innerRet;
        }
    }
    return CameraServiceErrorType(ret);
}

void DCameraClient::GetFpsRanges()
{
    for (auto& info : captureInfosCache_) {
        if (info == nullptr || info->streamType_ != CONTINUOUS_FRAME) {
            continue;
        }
        std::vector<std::shared_ptr<DCameraSettings>> captureSettings = info->captureSettings_;
        std::string metadataSetting;
        for (const auto& setting : captureSettings) {
            if (setting->type_ == UPDATE_METADATA) {
                DHLOGI("update metadata settings");
                metadataSetting = setting->value_;
            }
        }

        if (metadataSetting.empty()) {
            DHLOGE("no metadata settings to update");
            continue;
        }

        std::string metadataStr = Base64Decode(metadataSetting);
        std::shared_ptr<Camera::CameraMetadata> cameraMetadata = Camera::MetadataUtils::DecodeFromString(metadataStr);
        camera_metadata_item_t fpsItem;
        int32_t val = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_FPS_RANGES, &fpsItem);
        if (val == CAM_META_SUCCESS) {
            uint32_t fpscount = fpsItem.count;
            DHLOGI("GetFpsRanges: find fps ranges fpscount %{public}d", fpscount);
            CHECK_AND_RETURN_LOG(fpscount != DCAMERA_FPS_SIZE,
                "GetFpsRanges: find fps ranges fpscount: %{public}d is not DCAMERA_FPS_SIZE.", fpscount);
            std::vector<int32_t> fpsRanges;
            for (uint32_t i = 0; i < fpscount; i++) {
                fpsRanges.push_back(fpsItem.data.i32[i]);
            }
            fpsRanges_ = fpsRanges;
        } else {
            DHLOGI("GetFpsRanges: find fps ranges failed, ret: %{public}d", val);
        }
    }
}

int32_t DCameraClient::CameraServiceErrorType(const int32_t errorType)
{
    if (errorType == CameraStandard::CameraErrorCode::SERVICE_FATL_ERROR) {
        return DCAMERA_ALLOC_ERROR;
    } else if (errorType == CameraStandard::CameraErrorCode::CONFLICT_CAMERA) {
        return DCAMERA_DEVICE_BUSY;
    }
    return errorType;
}

int32_t DCameraClient::StopCapture()
{
    DHLOGI("StopCapture cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    fpsRanges_.clear();
    StopOutput();

    if (cameraInput_ != nullptr) {
        DHLOGI("StopCapture %{public}s release cameraInput", GetAnonyString(cameraId_).c_str());
        int32_t ret = cameraInput_->Close();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture cameraInput Close failed, cameraId: %{public}s, ret: %{public}d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        ret = cameraInput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture cameraInput Release failed, cameraId: %{public}s, ret: %{public}d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        cameraInput_ = nullptr;
    }
    ReleaseCaptureSession();

    if (previewSurface_ != nullptr) {
        DHLOGI("StopCapture %s previewsurface unregister consumer listener",
            GetAnonyString(cameraId_).c_str());
        previewSurface_ = nullptr;
    }

    if (photoSurface_ != nullptr) {
        DHLOGI("StopCapture %{public}s photosurface unregister consumer listener",
            GetAnonyString(cameraId_).c_str());
        int32_t ret = photoSurface_->UnregisterConsumerListener();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture %{public}s photosurface unregister consumer listener failed, ret: %{public}d",
                GetAnonyString(cameraId_).c_str(), ret);
        }
        photoListener_ = nullptr;
        photoSurface_ = nullptr;
    }

    DHLOGI("StopCapture %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

void DCameraClient::StopOutput()
{
    if (previewOutput_ != nullptr) {
        DHLOGI("StopCapture %{public}s stop previewOutput", GetAnonyString(cameraId_).c_str());
        int32_t ret = ((sptr<CameraStandard::PreviewOutput> &)previewOutput_)->Stop();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture videoOutput stop failed, cameraId: %{public}s, ret: %{public}d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        DHLOGI("StopCapture %{public}s release previewOutput", GetAnonyString(cameraId_).c_str());
        ret = previewOutput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture previewOutput Release failed, cameraId: %{public}s, ret: %{public}d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        previewOutput_ = nullptr;
    }

    if (photoOutput_ != nullptr) {
        DHLOGI("StopCapture %{public}s release photoOutput", GetAnonyString(cameraId_).c_str());
        int32_t ret = photoOutput_->Release();
        if (ret != DCAMERA_OK) {
            DHLOGE("StopCapture photoOutput Release failed, cameraId: %{public}s, ret: %{public}d",
                   GetAnonyString(cameraId_).c_str(), ret);
        }
        photoOutput_ = nullptr;
    }
}

void DCameraClient::ReleaseCaptureSession()
{
    if (captureSession_ == nullptr) {
        return;
    }
    DHLOGI("StopCapture %{public}s stop captureSession", GetAnonyString(cameraId_).c_str());
    int32_t ret = captureSession_->Stop();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopCapture captureSession stop failed, cameraId: %{public}s, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
    }
    DHLOGI("StopCapture %{public}s release captureSession", GetAnonyString(cameraId_).c_str());
    ret = captureSession_->Release();
    if (ret != DCAMERA_OK) {
        DHLOGE("StopCapture captureSession Release failed, cameraId: %{public}s, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
    }
    captureSession_ = nullptr;
}

int32_t DCameraClient::SetStateCallback(std::shared_ptr<StateCallback>& callback)
{
    DHLOGI("SetStateCallback cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    if (callback == nullptr) {
        DHLOGE("SetStateCallback %{public}s unregistering state callback", GetAnonyString(cameraId_).c_str());
    }
    stateCallback_ = callback;
    return DCAMERA_OK;
}

int32_t DCameraClient::SetResultCallback(std::shared_ptr<ResultCallback>& callback)
{
    DHLOGI("SetResultCallback cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    if (callback == nullptr) {
        DHLOGE("SetResultCallback %{public}s unregistering result callback", GetAnonyString(cameraId_).c_str());
    }
    resultCallback_ = callback;
    return DCAMERA_OK;
}
int32_t DCameraClient::ConfigCaptureSession(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos,
    int32_t sceneMode)
{
    DHLOGI("ConfigCaptureSession cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    CHECK_AND_RETURN_RET_LOG(cameraManager_ == nullptr, DCAMERA_BAD_VALUE, "cameraManager is null.");

    int rv = cameraManager_->CreateCameraInput(cameraInfo_, &((sptr<CameraStandard::CameraInput> &)cameraInput_));
    if (rv != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %{public}s create cameraInput failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    CHECK_AND_RETURN_RET_LOG(cameraInput_ == nullptr, DCAMERA_BAD_VALUE, "cameraInput is null.");

    int32_t rc = ((sptr<CameraStandard::CameraInput> &)cameraInput_)->Open();
    if (rc != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession cameraInput_ Open failed, cameraId: %{public}s, ret: %{public}d",
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
            DHLOGE("ConfigCaptureSession %{public}s set camera settings failed, ret: %{public}d",
                GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
        cameraMetadatas_.pop();
    }

    captureSession_ = cameraManager_->CreateCaptureSession(static_cast<CameraStandard::SceneMode>(sceneMode));
    if (captureSession_ == nullptr) {
        DHLOGE("ConfigCaptureSession %{public}s create captureSession failed",
               GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    std::shared_ptr<DCameraSessionCallback> sessionCallback = std::make_shared<DCameraSessionCallback>(stateCallback_);
    captureSession_->SetFocusCallback(sessionCallback);
    captureSession_->SetCallback(sessionCallback);

    DHLOGI("ConfigCaptureSession %{public}s finished pre-configuration.", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::ConfigCaptureSessionInner()
{
    CHECK_AND_RETURN_RET_LOG(captureSession_ == nullptr, DCAMERA_BAD_VALUE, "captureSession is null.");
    int32_t ret = captureSession_->BeginConfig();
    CHECK_AND_RETURN_RET_LOG(ret != DCAMERA_OK, ret, "%{public}s BeginConfig failed, ret: %{public}d",
        GetAnonyString(cameraId_).c_str(), ret);

    ret = captureSession_->AddInput(cameraInput_);
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %{public}s add cameraInput to captureSession failed, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    if (photoOutput_ != nullptr) {
        ret = captureSession_->AddOutput(photoOutput_);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession %{public}s add photoOutput to captureSession failed, ret: %{public}d",
                   GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
    }

    if (previewOutput_ != nullptr) {
        ret = captureSession_->AddOutput(previewOutput_);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession %{public}s add previewOutput to captureSession failed, ret: %{public}d",
                   GetAnonyString(cameraId_).c_str(), ret);
            return ret;
        }
    }

    ret = captureSession_->CommitConfig();
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %{public}s commit captureSession failed, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }

    ret = captureSession_->Start();
    if (ret != DCAMERA_OK) {
        DHLOGE("ConfigCaptureSession %{public}s start captureSession failed, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
    }

    if (previewOutput_ != nullptr && fpsRanges_.size() == DCAMERA_FPS_SIZE && fpsRanges_[0] > 0 && fpsRanges_[1] > 0) {
        fpsRanges_[0] = (fpsRanges_[0] <= DCAMERA_MAX_FPS) ? fpsRanges_[0] : DCAMERA_MAX_FPS;
        fpsRanges_[1] = (fpsRanges_[1] <= DCAMERA_MAX_FPS) ? fpsRanges_[1] : DCAMERA_MAX_FPS;
        ret = ((sptr<CameraStandard::PreviewOutput> &)previewOutput_)->SetFrameRate(fpsRanges_[0], fpsRanges_[1]);
        if (ret != DCAMERA_OK) {
            DHLOGE("ConfigCaptureSession set frame rate failed, ret: %{public}d", ret);
        }
        DHLOGI("SetFrameRate minfps: %{public}d, maxfps: %{public}d", fpsRanges_[0], fpsRanges_[1]);
    }

    DHLOGI("ConfigCaptureSession %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::CreateCaptureOutput(std::vector<std::shared_ptr<DCameraCaptureInfo>>& captureInfos)
{
    if (captureInfos.empty()) {
        DHLOGE("CreateCaptureOutput no capture info, cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }

    for (auto& info : captureInfos) {
        if (info == nullptr) {
            DHLOGE("CreateCaptureOutput info is null");
            continue;
        }
        if (info->streamType_ == SNAPSHOT_FRAME) {
            int32_t ret = CreatePhotoOutput(info);
            if (ret != DCAMERA_OK) {
                DHLOGE("CreateCaptureOutput %{public}s create photo output failed, ret: %{public}d",
                       GetAnonyString(cameraId_).c_str(), ret);
                return ret;
            }
        } else if (info->streamType_ == CONTINUOUS_FRAME) {
            int32_t ret = CreatePreviewOutput(info);
            if (ret != DCAMERA_OK) {
                DHLOGE("CreateCaptureOutput %{public}s create video output failed, ret: %{public}d",
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
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "CreatePhotoOutput info is null");
    DHLOGI("CreatePhotoOutput dhId: %{public}s, width: %{public}d, height: %{public}d, format: %{public}d, stream: "
        "%{public}d, isCapture: %{public}d", GetAnonyString(cameraId_).c_str(), info->width_, info->height_,
        info->format_, info->streamType_, info->isCapture_);
    photoSurface_ = IConsumerSurface::Create();
    CHECK_AND_RETURN_RET_LOG(photoSurface_ == nullptr, DCAMERA_BAD_VALUE,
        "CreatePhotoOutput create photo surface failed");
    photoListener_ = sptr<DCameraPhotoSurfaceListener>(
        new DCameraPhotoSurfaceListener(photoSurface_, resultCallback_));
    photoSurface_->RegisterConsumerListener((sptr<IBufferConsumerListener> &)photoListener_);
    CameraStandard::CameraFormat photoFormat = ConvertToCameraFormat(info->format_);
    CameraStandard::Size photoSize = {info->width_, info->height_};
    CameraStandard::Profile photoProfile(photoFormat, photoSize);
    sptr<IBufferProducer> bp = photoSurface_->GetProducer();
    int32_t rv = cameraManager_->CreatePhotoOutput(
        photoProfile, bp, &((sptr<CameraStandard::PhotoOutput> &)photoOutput_));
    if (rv != DCAMERA_OK || photoOutput_ == nullptr) {
        DHLOGE("CreatePhotoOutput %{public}s create photo output failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraPhotoCallback> photoCallback = std::make_shared<DCameraPhotoCallback>(stateCallback_);
    ((sptr<CameraStandard::PhotoOutput> &)photoOutput_)->SetCallback(photoCallback);
    DHLOGI("CreatePhotoOutput %{public}s success", GetAnonyString(cameraId_).c_str());
    return DCAMERA_OK;
}

int32_t DCameraClient::CreatePreviewOutput(std::shared_ptr<DCameraCaptureInfo>& info)
{
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "CreatePreviewOutput info is null");
    DHLOGI("CreatePreviewOutput dhId: %{public}s, width: %{public}d, height: %{public}d, format: %{public}d, stream:"
        " %{public}d, isCapture: %{public}d", GetAnonyString(cameraId_).c_str(), info->width_, info->height_,
        info->format_, info->streamType_, info->isCapture_);
    CameraStandard::CameraFormat previewFormat = ConvertToCameraFormat(info->format_);
    CameraStandard::Size previewSize = {info->width_, info->height_};
    CameraStandard::Profile previewProfile(previewFormat, previewSize);
    int32_t rv = cameraManager_->CreatePreviewOutput(
        previewProfile, previewSurface_, &((sptr<CameraStandard::PreviewOutput> &)previewOutput_));
    if (rv != DCAMERA_OK) {
        DHLOGE("CreatePreviewOutput %{public}s create preview output failed", GetAnonyString(cameraId_).c_str());
        return DCAMERA_BAD_VALUE;
    }
    auto previewCallback = std::make_shared<DCameraPreviewCallback>(stateCallback_);
    ((sptr<CameraStandard::PreviewOutput> &)previewOutput_)->SetCallback(previewCallback);
    DHLOGI("CreatePreviewOutput %{public}s success", GetAnonyString(cameraId_).c_str());
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
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "StartCaptureInner info is null");
    if (info->streamType_ != SNAPSHOT_FRAME) {
        DHLOGE("StartCaptureInner unknown stream type");
        return DCAMERA_BAD_VALUE;
    }
    return StartPhotoOutput(info);
}

int32_t DCameraClient::StartPhotoOutput(std::shared_ptr<DCameraCaptureInfo>& info)
{
    CHECK_AND_RETURN_RET_LOG(info == nullptr, DCAMERA_BAD_VALUE, "StartPhotoOutput info is null");
    DHLOGI("StartPhotoOutput cameraId: %{public}s", GetAnonyString(cameraId_).c_str());
    if (photoOutput_ == nullptr) {
        DHLOGE("StartPhotoOutput photoOutput is null");
        return DCAMERA_BAD_VALUE;
    }

    std::vector<std::shared_ptr<DCameraSettings>> captureSettings = info->captureSettings_;
    std::string metadataSetting;
    for (const auto& setting : captureSettings) {
        if (setting->type_ == UPDATE_METADATA) {
            DHLOGI("StartPhotoOutput %{public}s update metadata settings", GetAnonyString(cameraId_).c_str());
            metadataSetting = setting->value_;
        }
    }

    if (metadataSetting.empty()) {
        DHLOGE("StartPhotoOutput no metadata settings to update");
        int32_t ret = ((sptr<CameraStandard::PhotoOutput> &)photoOutput_)->Capture();
        if (ret != DCAMERA_OK) {
            DHLOGE("StartPhotoOutput %{public}s photoOutput capture failed, ret: %{public}d",
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
        DHLOGE("StartPhotoOutput %{public}s photoOutput capture failed, ret: %{public}d",
            GetAnonyString(cameraId_).c_str(), ret);
        return ret;
    }
    return DCAMERA_OK;
}

void DCameraClient::SetPhotoCaptureRotation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    CHECK_AND_RETURN_LOG(cameraMetadata == nullptr, "SetPhotoCaptureRotation param cameraMetadata is null");
    CHECK_AND_RETURN_LOG(photoCaptureSetting == nullptr,
        "SetPhotoCaptureRotation param photoCaptureSetting is null");
    uint32_t rotationCount = 1;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_ORIENTATION, &item);
    if ((ret == CAM_META_SUCCESS) && (rotationCount == item.count)) {
        CameraStandard::PhotoCaptureSetting::RotationConfig rotation =
            static_cast<CameraStandard::PhotoCaptureSetting::RotationConfig>(item.data.i32[0]);
        photoCaptureSetting->SetRotation(rotation);
        DHLOGI("SetPhotoCaptureRotation %{public}s photo capture settings set %{public}d rotation: %{public}d",
            GetAnonyString(cameraId_).c_str(), item.count, rotation);
    }
}

void DCameraClient::SetPhotoCaptureQuality(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    CHECK_AND_RETURN_LOG(cameraMetadata == nullptr, "SetPhotoCaptureRotation param cameraMetadata is null");
    CHECK_AND_RETURN_LOG(photoCaptureSetting == nullptr,
        "SetPhotoCaptureRotation param photoCaptureSetting is null");
    uint32_t qualityCount = 1;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_QUALITY, &item);
    if ((ret == CAM_META_SUCCESS) && (qualityCount == item.count)) {
        bool status = CameraStandard::AddOrUpdateMetadata(photoCaptureSetting->GetCaptureMetadataSetting()->get(),
            OHOS_JPEG_QUALITY, &item.data.u8[0], item.count);
        DHLOGI("SetPhotoCaptureQuality %{public}s photo capture settings set %{public}d quality: %{public}d"
            "status: %{public}d", GetAnonyString(cameraId_).c_str(), item.count, item.data.u8[0], status);
    }
}

void DCameraClient::SetPhotoCaptureLocation(const std::shared_ptr<Camera::CameraMetadata>& cameraMetadata,
    std::shared_ptr<CameraStandard::PhotoCaptureSetting>& photoCaptureSetting)
{
    CHECK_AND_RETURN_LOG(cameraMetadata == nullptr, "SetPhotoCaptureRotation param cameraMetadata is null");
    CHECK_AND_RETURN_LOG(photoCaptureSetting == nullptr,
        "SetPhotoCaptureRotation param photoCaptureSetting is null");
    uint32_t locationCount = 3;
    camera_metadata_item_t item;
    int32_t ret = Camera::FindCameraMetadataItem(cameraMetadata->get(), OHOS_JPEG_GPS_COORDINATES, &item);
    if ((ret == CAM_META_SUCCESS) && (locationCount == item.count)) {
        int32_t latitudeIndex = 0;
        int32_t longitudeIndex = 1;
        int32_t altitudeIndex = 2;
        std::shared_ptr<CameraStandard::Location> location = std::make_shared<CameraStandard::Location>();
        location->latitude = item.data.d[latitudeIndex];
        location->longitude = item.data.d[longitudeIndex];
        location->altitude = item.data.d[altitudeIndex];
        photoCaptureSetting->SetLocation(location);
        DHLOGI("SetPhotoCaptureLocation %{public}s photo capture settings set %{public}d location: "
            "latitude=%{public}s, longitude=%{public}s, altitude=%{public}s", GetAnonyString(cameraId_).c_str(),
            item.count, GetAnonyString(std::to_string(item.data.d[latitudeIndex])).c_str(),
            GetAnonyString(std::to_string(item.data.d[longitudeIndex])).c_str(),
            GetAnonyString(std::to_string(item.data.d[altitudeIndex])).c_str());
    }
}

int32_t DCameraClient::PauseCapture()
{
    if (captureSession_ == nullptr) {
        DHLOGE("PauseCapture captureSession_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = captureSession_->Stop();
    if (ret != DCAMERA_OK) {
        DHLOGE("PauseCapture captureSession stop failed, cameraId: %{public}s, ret: %{public}d",
               GetAnonyString(cameraId_).c_str(), ret);
    }
    return ret;
}

int32_t DCameraClient::ResumeCapture()
{
    if (captureSession_ == nullptr) {
        DHLOGE("ResumeCapture captureSession_ is nullptr.");
        return DCAMERA_BAD_VALUE;
    }
    int32_t ret = captureSession_->Start();
    if (ret != DCAMERA_OK) {
        DHLOGE("ResumeCapture captureSession Start failed, cameraId: %{public}s, ret: %{public}d",
            GetAnonyString(cameraId_).c_str(), ret);
    }
    return ret;
}
} // namespace DistributedHardware
} // namespace OHOS