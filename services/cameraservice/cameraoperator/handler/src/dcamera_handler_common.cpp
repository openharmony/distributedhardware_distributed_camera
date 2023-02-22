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

#include "dcamera_handler.h"

#include <functional>

#include "anonymous_string.h"
#include "avcodec_info.h"
#include "avcodec_list.h"
#include "dcamera_manager_callback.h"
#include "dcamera_utils_tools.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "metadata_utils.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraHandler);

DCameraHandler::~DCameraHandler()
{
    DHLOGI("~DCameraHandlerCommon");
}

int32_t DCameraHandler::Initialize()
{
    DHLOGI("DCameraHandlerCommon::Initialize");
    cameraManager_ = CameraStandard::CameraManager::GetInstance();
    if (cameraManager_ == nullptr) {
        DHLOGE("DCameraHandlerCommon::Initialize cameraManager getInstance failed");
        return DCAMERA_INIT_ERR;
    }
    std::shared_ptr<DCameraManagerCallback> cameraMgrCallback = std::make_shared<DCameraManagerCallback>();
    cameraManager_->SetCallback(cameraMgrCallback);
    DHLOGI("DCameraHandlerCommon::Initialize success");
    return DCAMERA_OK;
}

std::vector<DHItem> DCameraHandler::Query()
{
    std::vector<DHItem> itemList;
    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    DHLOGI("DCameraHandlerCommon::Query get %d cameras", cameraList.size());
    if (cameraList.empty()) {
        DHLOGE("DCameraHandlerCommon::Query no camera device");
        return itemList;
    }
    for (auto& info : cameraList) {
        if (info->GetConnectionType() != CameraStandard::ConnectionType::CAMERA_CONNECTION_BUILT_IN) {
            DHLOGI("DCameraHandlerCommon::Query connection type: %d", info->GetConnectionType());
            continue;
        }
        DHLOGI("DCameraHandlerCommon::Query get %s, position: %d, cameraType: %d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType());
        DHItem item;
        if (CreateDHItem(info, item) == DCAMERA_OK) {
            itemList.emplace_back(item);
        }
    }
    DHLOGI("DCameraHandlerCommon::Query success, get %d items", itemList.size());
    return itemList;
}

std::map<std::string, std::string> DCameraHandler::QueryExtraInfo()
{
    DHLOGI("DCameraHandler::QueryExtraInfo");
    std::map<std::string, std::string> extraInfo;
    return extraInfo;
}

bool DCameraHandler::IsSupportPlugin()
{
    DHLOGI("DCameraHandler::IsSupportPlugin");
    return false;
}

void DCameraHandler::RegisterPluginListener(std::shared_ptr<PluginListener> listener)
{
    DHLOGI("DCameraHandler::RegisterPluginListener");
    if (listener == nullptr) {
        DHLOGE("DCameraHandler unregistering plugin listener");
    }
    pluginListener_ = listener;
}

void DCameraHandler::UnRegisterPluginListener()
{
    DHLOGI("DCameraHandler::UnRegisterPluginListener");
    pluginListener_ = nullptr;
}

std::vector<std::string> DCameraHandler::GetCameras()
{
    std::vector<std::string> cameras;
    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    DHLOGI("DCameraHandlerCommon::GetCameras get %d cameras", cameraList.size());
    if (cameraList.empty()) {
        DHLOGE("DCameraHandlerCommon::GetCameras no camera device");
        return cameras;
    }
    for (auto& info : cameraList) {
        sptr<CameraStandard::CameraOutputCapability> capability = cameraManager_->GetSupportedOutputCapability(info);
        if (capability == nullptr) {
            DHLOGI("DCameraHandlerCommon::GetCameras get supported capability is null");
            continue;
        }
        if (info->GetConnectionType() != CameraStandard::ConnectionType::CAMERA_CONNECTION_BUILT_IN) {
            DHLOGI("DCameraHandlerCommon::GetCameras connection type: %d", info->GetConnectionType());
            continue;
        }
        DHLOGI("DCameraHandlerCommon::GetCameras get %s, position: %d, cameraType: %d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType());
        std::string dhId = CAMERA_ID_PREFIX + info->GetID();
        cameras.emplace_back(dhId);
    }
    DHLOGI("DCameraHandlerCommon::GetCameras success, get %d items", cameras.size());
    return cameras;
}

int32_t DCameraHandler::CreateDHItem(sptr<CameraStandard::CameraDevice>& info, DHItem& item)
{
    std::string id = info->GetID();
    item.dhId = CAMERA_ID_PREFIX + id;
    DHLOGI("DCameraHandlerCommon::CreateDHItem camera id: %s", GetAnonyString(id).c_str());

    Json::Value root;
    root[CAMERA_PROTOCOL_VERSION_KEY] = Json::Value(CAMERA_PROTOCOL_VERSION_VALUE);
    root[CAMERA_POSITION_KEY] = Json::Value(GetCameraPosition(info->GetPosition()));
    root[CAMERA_CODEC_TYPE_KEY].append("OMX_hisi_video_encoder_avc");

    sptr<CameraStandard::CameraInput> cameraInput = nullptr;
    int rv = cameraManager_->CreateCameraInput(info, &cameraInput);
    if (rv != DCAMERA_OK) {
        DHLOGE("DCameraHandlerCommon::CreateDHItem create cameraInput failed");
        return DCAMERA_BAD_VALUE;
    }

    sptr<CameraStandard::CameraOutputCapability> capability = cameraManager_->GetSupportedOutputCapability(info);
    if (capability == nullptr) {
        DHLOGI("DCameraHandlerCommon::CreateDHItem get supported capability is null");
        return DCAMERA_BAD_VALUE;
    }
    std::vector<CameraStandard::Profile> photoProfiles = capability->GetPhotoProfiles();
    ConfigFormatAndResolution(SNAPSHOT_FRAME, root, photoProfiles);

    std::vector<CameraStandard::Profile> previewProfiles = capability->GetPreviewProfiles();
    ConfigFormatAndResolution(CONTINUOUS_FRAME, root, previewProfiles);

    std::hash<std::string> h;
    std::string abilityString = cameraInput->GetCameraSettings();
    DHLOGI("DCameraHandlerCommon::CreateDHItem abilityString hash: %zu, length: %zu",
        h(abilityString), abilityString.length());

    std::string encodeString = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    DHLOGI("DCameraHandlerCommon::CreateDHItem encodeString hash: %zu, length: %zu",
        h(encodeString), encodeString.length());
    root[CAMERA_METADATA_KEY] = Json::Value(encodeString);

    item.attrs = root.toStyledString();
    if (cameraInput->Release() != DCAMERA_OK) {
        DHLOGE("DCameraHandlerCommon::CreateDHItem cameraInput Release failed");
    }
    return DCAMERA_OK;
}

std::string DCameraHandler::GetCameraPosition(CameraStandard::CameraPosition position)
{
    DHLOGI("DCameraHandler::GetCameraPosition position: %d", position);
    std::string ret = "";
    switch (position) {
        case CameraStandard::CameraPosition::CAMERA_POSITION_BACK: {
            ret = CAMERA_POSITION_BACK;
            break;
        }
        case CameraStandard::CameraPosition::CAMERA_POSITION_FRONT: {
            ret = CAMERA_POSITION_FRONT;
            break;
        }
        case CameraStandard::CameraPosition::CAMERA_POSITION_UNSPECIFIED: {
            ret = CAMERA_POSITION_UNSPECIFIED;
            break;
        }
        default: {
            DHLOGE("DCameraHandler::GetCameraPosition unknown camera position");
            break;
        }
    }
    DHLOGI("DCameraHandler::GetCameraPosition success ret: %s", ret.c_str());
    return ret;
}

void DCameraHandler::ConfigFormatAndResolution(const DCStreamType type, Json::Value& root,
    std::vector<CameraStandard::Profile>& profileList)
{
    DHLOGI("DCameraHandler::ConfigFormatAndResolution camera Profile size: %d", profileList.size());
    std::set<int32_t> formatSet;
    for (auto& profile : profileList) {
        CameraStandard::CameraFormat format = profile.GetCameraFormat();
        CameraStandard::Size picSize = profile.GetSize();
        int32_t dformat = CovertToDcameraFormat(format);
        formatSet.insert(dformat);
        DHLOGI("DCameraHandler::ConfigFormatAndResolution, width: %d, height: %d, format: %d",
            picSize.width, picSize.height, dformat);
        std::string formatName = std::to_string(dformat);
        if (IsValid(type, picSize)) {
            std::string resolutionValue = std::to_string(picSize.width) + "*" + std::to_string(picSize.height);
            if (type == SNAPSHOT_FRAME) {
                root[CAMERA_FORMAT_PHOTO][CAMERA_RESOLUTION_KEY][formatName].append(resolutionValue);
            } else if (type == CONTINUOUS_FRAME) {
                root[CAMERA_FORMAT_PREVIEW][CAMERA_RESOLUTION_KEY][formatName].append(resolutionValue);
                root[CAMERA_FORMAT_VIDEO][CAMERA_RESOLUTION_KEY][formatName].append(resolutionValue);
            }
        }
    }

    for (auto format : formatSet) {
        if (type == SNAPSHOT_FRAME) {
            root[CAMERA_FORMAT_PHOTO][CAMERA_FORMAT_KEY].append(format);
        } else if (type == CONTINUOUS_FRAME) {
            root[CAMERA_FORMAT_PREVIEW][CAMERA_FORMAT_KEY].append(format);
            root[CAMERA_FORMAT_VIDEO][CAMERA_FORMAT_KEY].append(format);
        }
    }
}

int32_t DCameraHandler::CovertToDcameraFormat(CameraStandard::CameraFormat format)
{
    DHLOGI("DCameraHandler::CovertToDcameraFormat format: %d", format);
    int32_t ret = -1;
    switch (format) {
        case CameraStandard::CameraFormat::CAMERA_FORMAT_RGBA_8888:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_RGBA_8888;
            break;
        case CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_420_888:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_YCBCR_420_888;
            break;
        case CameraStandard::CameraFormat::CAMERA_FORMAT_YUV_420_SP:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_420_SP;
            break;
        case CameraStandard::CameraFormat::CAMERA_FORMAT_JPEG:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_JPEG;
            break;
        default:
            DHLOGE("DCameraHandler::CovertToDcameraFormat invalid camera format");
            break;
    }
    return ret;
}

bool DCameraHandler::IsValid(const DCStreamType type, const CameraStandard::Size& size)
{
    bool ret = false;
    switch (type) {
        case CONTINUOUS_FRAME: {
            ret = (size.width >= RESOLUTION_MIN_WIDTH) &&
                    (size.height >= RESOLUTION_MIN_HEIGHT) &&
                    (size.width <= RESOLUTION_MAX_WIDTH_CONTINUOUS) &&
                    (size.height <= RESOLUTION_MAX_HEIGHT_CONTINUOUS);
            break;
        }
        case SNAPSHOT_FRAME: {
            ret = (size.width >= RESOLUTION_MIN_WIDTH) &&
                    (size.height >= RESOLUTION_MIN_HEIGHT) &&
                    (size.width <= RESOLUTION_MAX_WIDTH_SNAPSHOT) &&
                    (size.height <= RESOLUTION_MAX_HEIGHT_SNAPSHOT);
            break;
        }
        default: {
            DHLOGE("DCameraHandler::isValid unknown stream type");
            break;
        }
    }
    return ret;
}

IHardwareHandler* GetHardwareHandler()
{
    DHLOGI("DCameraHandler::GetHardwareHandler");
    return &DCameraHandler::GetInstance();
}
} // namespace DistributedHardware
} // namespace OHOS