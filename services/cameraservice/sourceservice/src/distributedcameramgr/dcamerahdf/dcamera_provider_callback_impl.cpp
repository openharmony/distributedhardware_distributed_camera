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

#include "dcamera_provider_callback_impl.h"

#include "dcamera_index.h"
#include "dcamera_source_dev.h"

#include "anonymous_string.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
DCameraProviderCallbackImpl::DCameraProviderCallbackImpl(std::string devId, std::string dhId,
    std::shared_ptr<DCameraSourceDev>& sourceDev) : devId_(devId), dhId_(dhId), sourceDev_(sourceDev)
{
    DHLOGI("DCameraProviderCallbackImpl create devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
}

DCameraProviderCallbackImpl::~DCameraProviderCallbackImpl()
{
    DHLOGI("DCameraProviderCallbackImpl delete devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
}

int32_t DCameraProviderCallbackImpl::OpenSession(const DHBase& dhBase)
{
    DHLOGI("DCameraProviderCallbackImpl OpenSession devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase)) {
        DHLOGE("DCameraProviderCallbackImpl OpenSession input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl OpenSession failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    DCameraIndex camIndex(devId_, dhId_);
    int32_t ret = sourceDev->OpenSession(camIndex);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl OpenSession failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t DCameraProviderCallbackImpl::CloseSession(const DHBase& dhBase)
{
    DHLOGI("DCameraProviderCallbackImpl CloseSession devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase)) {
        DHLOGE("DCameraProviderCallbackImpl CloseSession input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl CloseSession failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    DCameraIndex camIndex(devId_, dhId_);
    int32_t ret = sourceDev->CloseSession(camIndex);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl CloseSession failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool DCameraProviderCallbackImpl::CheckDHBase(const DHBase& dhBase)
{
    if (dhBase.deviceId_.empty() || dhBase.deviceId_.size() > DID_MAX_SIZE ||
        dhBase.dhId_.empty() || dhBase.dhId_.size() > DID_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl CheckDHBase dhBase is invalid");
        return false;
    }
    return true;
}

int32_t DCameraProviderCallbackImpl::ConfigureStreams(const DHBase& dhBase,
    const std::vector<DCStreamInfo>& streamInfos)
{
    DHLOGI("DCameraProviderCallbackImpl ConfigStreams devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase) || streamInfos.empty() || streamInfos.size() > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl ConfigStreams input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl ConfigStreams failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    std::vector<std::shared_ptr<DCStreamInfo>> streams;
    for (auto iter = streamInfos.begin(); iter != streamInfos.end(); iter++) {
        if (!CheckStreamInfo(*iter)) {
            DHLOGE("DCameraProviderCallbackImpl ConfigStreams streamInfo is invalid");
            return FAILED;
        }
        std::shared_ptr<DCStreamInfo> stream = std::make_shared<DCStreamInfo>();
        stream->streamId_ = iter->streamId_;
        stream->width_ = iter->width_;
        stream->height_ = iter->height_;
        stream->stride_ = iter->stride_;
        stream->format_ = iter->format_;
        stream->dataspace_ = iter->dataspace_;
        stream->encodeType_ = iter->encodeType_;
        stream->type_ = iter->type_;
        streams.push_back(stream);
    }
    int32_t ret = sourceDev->ConfigCameraStreams(streams);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl CloseSession failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool DCameraProviderCallbackImpl::CheckStreamInfo(const DCStreamInfo& stream)
{
    if (stream.streamId_ < 0 || stream.width_ < 0 || stream.height_ < 0 ||
        (stream.width_ * stream.height_ > RESOLUTION_MAX_WIDTH * RESOLUTION_MAX_HEIGHT)) {
        DHLOGE("DCameraProviderCallbackImpl CheckStreamInfo stream is invalid");
        return false;
    }

    if (stream.stride_ < 0 || stream.format_ < 0 || stream.dataspace_ < 0) {
        DHLOGE("DCameraProviderCallbackImpl CheckStreamInfo stream is invalid");
        return false;
    }
    return true;
}

int32_t DCameraProviderCallbackImpl::ReleaseStreams(const DHBase& dhBase, const std::vector<int>& streamIds)
{
    DHLOGI("DCameraProviderCallbackImpl ReleaseStreams devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase) || streamIds.size() > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl ReleaseStreams input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl ReleaseStreams failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    int32_t ret = sourceDev->ReleaseCameraStreams(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl ReleaseStreams failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t DCameraProviderCallbackImpl::StartCapture(const DHBase& dhBase, const std::vector<DCCaptureInfo>& captureInfos)
{
    DHLOGI("DCameraProviderCallbackImpl StartCapture devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase) || captureInfos.empty() || captureInfos.size() > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl StartCapture input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl StartCapture failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }

    std::vector<std::shared_ptr<DCCaptureInfo>> captures;
    for (auto iter = captureInfos.begin(); iter != captureInfos.end(); iter++) {
        if (!CheckCaptureInfo(*iter)) {
            DHLOGE("DCameraProviderCallbackImpl StartCapture captureInfo is invalid");
            return FAILED;
        }
        for (auto item : iter->captureSettings_) {
            if (item.value_.size() > PARAM_MAX_SIZE) {
                DHLOGE("DCameraProviderCallbackImpl StartCapture captureSettings value is too long");
                return FAILED;
            }
        }
        std::shared_ptr<DCCaptureInfo> capture = std::make_shared<DCCaptureInfo>();
        capture->streamIds_.assign(iter->streamIds_.begin(), iter->streamIds_.end());
        capture->width_ = iter->width_;
        capture->height_ = iter->height_;
        capture->stride_ = iter->stride_;
        capture->format_ = iter->format_;
        capture->dataspace_ = iter->dataspace_;
        capture->isCapture_ = iter->isCapture_;
        capture->encodeType_ = iter->encodeType_;
        capture->type_ = iter->type_;
        capture->captureSettings_.assign(iter->captureSettings_.begin(), iter->captureSettings_.end());
        captures.push_back(capture);
    }
    int32_t ret = sourceDev->StartCameraCapture(captures);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl StartCapture failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

bool DCameraProviderCallbackImpl::CheckCaptureInfo(const DCCaptureInfo& captureInfo)
{
    if (sizeof(captureInfo.streamIds_) > PARAM_MAX_SIZE || captureInfo.width_ < 0 || captureInfo.height_ < 0 ||
        (captureInfo.width_ * captureInfo.height_ > RESOLUTION_MAX_WIDTH * RESOLUTION_MAX_HEIGHT)) {
        DHLOGE("DCameraProviderCallbackImpl CheckCaptureInfo captureInfo is invalid");
        return false;
    }

    if (captureInfo.stride_ < 0 || captureInfo.format_ < 0 || captureInfo.dataspace_ < 0 ||
        sizeof(captureInfo.captureSettings_) > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl CheckCaptureInfo captureInfo is invalid");
        return false;
    }
    return true;
}

int32_t DCameraProviderCallbackImpl::StopCapture(const DHBase& dhBase, const std::vector<int>& streamIds)
{
    DHLOGI("DCameraProviderCallbackImpl StopCapture devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase) || streamIds.size() > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl StopCapture input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl StopCapture failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    int32_t ret = sourceDev->StopCameraCapture(streamIds);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl StopCapture failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}

int32_t DCameraProviderCallbackImpl::UpdateSettings(const DHBase& dhBase, const std::vector<DCameraSettings>& settings)
{
    DHLOGI("DCameraProviderCallbackImpl UpdateSettings devId: %s dhId: %s", GetAnonyString(devId_).c_str(),
        GetAnonyString(dhId_).c_str());
    if (!CheckDHBase(dhBase) || settings.size() > PARAM_MAX_SIZE) {
        DHLOGE("DCameraProviderCallbackImpl UpdateSettings input is invalid");
        return FAILED;
    }
    std::shared_ptr<DCameraSourceDev> sourceDev = sourceDev_.lock();
    if (sourceDev == nullptr) {
        DHLOGE("DCameraProviderCallbackImpl UpdateSettings failed, can not get device, devId: %s, dhId: %s",
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }

    std::vector<std::shared_ptr<DCameraSettings>> settingInfos;
    for (auto iter = settings.begin(); iter != settings.end(); iter++) {
        if (iter->value_.size() > PARAM_MAX_SIZE) {
            DHLOGE("DCameraProviderCallbackImpl UpdateSettings value is too long");
            return FAILED;
        }
        std::shared_ptr<DCameraSettings> settingInfo = std::make_shared<DCameraSettings>();
        settingInfo->type_ = iter->type_;
        settingInfo->value_ = iter->value_;
        settingInfos.push_back(settingInfo);
    }
    int32_t ret = sourceDev->UpdateCameraSettings(settingInfos);
    if (ret != DCAMERA_OK) {
        DHLOGE("DCameraProviderCallbackImpl UpdateSettings failed, ret: %d, devId: %s, dhId: %s", ret,
            GetAnonyString(devId_).c_str(), GetAnonyString(dhId_).c_str());
        return FAILED;
    }
    return SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS
