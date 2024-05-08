/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

const int32_t MAXWIDTHSIZE = 65535;

DCameraHandler::~DCameraHandler()
{
    DHLOGI("~DCameraHandler");
}

int32_t DCameraHandler::Initialize()
{
    DHLOGI("start");
    cameraManager_ = CameraStandard::CameraManager::GetInstance();
    if (cameraManager_ == nullptr) {
        DHLOGE("cameraManager getInstance failed");
        return DCAMERA_INIT_ERR;
    }
    std::shared_ptr<DCameraManagerCallback> cameraMgrCallback = std::make_shared<DCameraManagerCallback>();
    cameraManager_->SetCallback(cameraMgrCallback);
    DHLOGI("success");
    return DCAMERA_OK;
}

std::vector<DHItem> DCameraHandler::QueryMeta()
{
    std::vector<DHItem> itemList;
    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    uint64_t listSize = static_cast<uint64_t>(cameraList.size());
    DHLOGI("get %{public}" PRIu64" cameras", listSize);
    if (cameraList.empty()) {
        DHLOGE("no camera device");
        return itemList;
    }
    for (auto& info : cameraList) {
        if (info->GetConnectionType() != CameraStandard::ConnectionType::CAMERA_CONNECTION_BUILT_IN) {
            DHLOGI("connection type: %{public}d", info->GetConnectionType());
            continue;
        }
        DHLOGI("get %{public}s, position: %{public}d, cameraType: %{public}d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType());
        DHItem item;
        if (CreateMetaDHItem(info, item) == DCAMERA_OK) {
            itemList.emplace_back(item);
        }
    }
    listSize = static_cast<uint64_t>(itemList.size());
    DHLOGI("success, get %{public}" PRIu64" items", listSize);
    return itemList;
}

std::vector<DHItem> DCameraHandler::Query()
{
    std::vector<DHItem> itemList;
    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    uint64_t listSize = static_cast<uint64_t>(cameraList.size());
    DHLOGI("get %{public}" PRIu64" cameras", listSize);
    if (cameraList.empty()) {
        DHLOGE("no camera device");
        return itemList;
    }
    for (auto& info : cameraList) {
        if ((info->GetConnectionType() != CameraStandard::ConnectionType::CAMERA_CONNECTION_BUILT_IN)) {
            DHLOGI("connection type: %{public}d", info->GetConnectionType());
            continue;
        }
        DHLOGI("get %{public}s, position: %{public}d, cameraType: %{public}d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType());
        DHItem item;
        if (CreateDHItem(info, item) == DCAMERA_OK) {
            itemList.emplace_back(item);
        }
    }
    listSize = static_cast<uint64_t>(itemList.size());
    DHLOGI("success, get %{public}" PRIu64" items", listSize);
    return itemList;
}

std::map<std::string, std::string> DCameraHandler::QueryExtraInfo()
{
    DHLOGI("enter");
    std::map<std::string, std::string> extraInfo;
    return extraInfo;
}

bool DCameraHandler::IsSupportPlugin()
{
    DHLOGI("enter");
    return false;
}

void DCameraHandler::RegisterPluginListener(std::shared_ptr<PluginListener> listener)
{
    DHLOGI("enter");
    if (listener == nullptr) {
        DHLOGE("DCameraHandler unregistering plugin listener");
    }
    pluginListener_ = listener;
}

void DCameraHandler::UnRegisterPluginListener()
{
    DHLOGI("enter");
    pluginListener_ = nullptr;
}

std::vector<std::string> DCameraHandler::GetCameras()
{
    std::vector<std::string> cameras;
    std::vector<sptr<CameraStandard::CameraDevice>> cameraList = cameraManager_->GetSupportedCameras();
    uint64_t listSize = static_cast<uint64_t>(cameraList.size());
    DHLOGI("get %{public}" PRIu64" cameras", listSize);
    if (cameraList.empty()) {
        DHLOGE("no camera device");
        return cameras;
    }
    for (auto& info : cameraList) {
        sptr<CameraStandard::CameraOutputCapability> capability = cameraManager_->GetSupportedOutputCapability(info);
        if (capability == nullptr) {
            DHLOGI("get supported capability is null");
            continue;
        }
        if (info->GetConnectionType() != CameraStandard::ConnectionType::CAMERA_CONNECTION_BUILT_IN) {
            DHLOGI("connection type: %{public}d", info->GetConnectionType());
            continue;
        }
        DHLOGI("get %{public}s, position: %{public}d, cameraType: %{public}d",
            GetAnonyString(info->GetID()).c_str(), info->GetPosition(), info->GetCameraType());
        std::string dhId = CAMERA_ID_PREFIX + info->GetID();
        cameras.emplace_back(dhId);
    }
    listSize = static_cast<uint64_t>(cameras.size());
    DHLOGI("success, get %{public}" PRIu64" items", listSize);
    return cameras;
}

int32_t DCameraHandler::CreateAVCodecList(cJSON *root)
{
    DHLOGI("Create avCodecList start");
    std::shared_ptr<MediaAVCodec::AVCodecList> avCodecList = MediaAVCodec::AVCodecListFactory::CreateAVCodecList();
    if (avCodecList == nullptr) {
        DHLOGI("Create avCodecList failed");
        return DCAMERA_BAD_VALUE;
    }
    const std::vector<std::string> encoderName = {std::string(MediaAVCodec::CodecMimeType::VIDEO_AVC),
                                                  std::string(MediaAVCodec::CodecMimeType::VIDEO_HEVC)};
    cJSON *array = cJSON_CreateArray();
    if (array == nullptr) {
        DHLOGI("Create arrray failed");
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddItemToObject(root, CAMERA_CODEC_TYPE_KEY.c_str(), array);
    for (auto &coder : encoderName) {
        MediaAVCodec::CapabilityData *capData = avCodecList->GetCapability(coder, true,
            MediaAVCodec::AVCodecCategory::AVCODEC_HARDWARE);
        if (capData == nullptr) {
            DHLOGI("capData is nullptr");
            return DCAMERA_BAD_VALUE;
        }
        std::string mimeType = capData->mimeType;
        cJSON_AddItemToArray(array, cJSON_CreateString(mimeType.c_str()));
        DHLOGI("codec name: %{public}s, mimeType: %{public}s", coder.c_str(), mimeType.c_str());
    }
    return DCAMERA_OK;
}

int32_t DCameraHandler::CreateMetaDHItem(sptr<CameraStandard::CameraDevice>& info, DHItem& item)
{
    std::string id = info->GetID();
    item.dhId = CAMERA_ID_PREFIX + id;
    item.subtype = "camera";
    DHLOGI("camera id: %{public}s", GetAnonyString(id).c_str());

    cJSON *root = cJSON_CreateObject();
    CHECK_AND_RETURN_RET_LOG(root == nullptr, DCAMERA_BAD_VALUE, "Create cJSON object failed");
    cJSON_AddStringToObject(root, CAMERA_METADATA_KEY.c_str(), CAMERA_METADATA_KEY.c_str());
    char *jsonstr = cJSON_Print(root);
    if (jsonstr == nullptr) {
        cJSON_Delete(root);
        return DCAMERA_BAD_VALUE;
    }
    item.attrs = jsonstr;
    cJSON_free(jsonstr);
    cJSON_Delete(root);
    return DCAMERA_OK;
}

int32_t DCameraHandler::CreateDHItem(sptr<CameraStandard::CameraDevice>& info, DHItem& item)
{
    std::string id = info->GetID();
    item.dhId = CAMERA_ID_PREFIX + id;
    item.subtype = "camera";
    DHLOGI("camera id: %{public}s", GetAnonyString(id).c_str());

    cJSON *root = cJSON_CreateObject();
    CHECK_AND_RETURN_RET_LOG(root == nullptr, DCAMERA_BAD_VALUE, "Create cJSON object failed");
    cJSON_AddStringToObject(root, CAMERA_PROTOCOL_VERSION_KEY.c_str(), CAMERA_PROTOCOL_VERSION_VALUE.c_str());
    cJSON_AddStringToObject(root, CAMERA_POSITION_KEY.c_str(), GetCameraPosition(info->GetPosition()).c_str());
    int32_t ret = CreateAVCodecList(root);
    CHECK_AND_FREE_RETURN_RET_LOG(ret != DCAMERA_OK, DCAMERA_BAD_VALUE, root, "CreateAVCodecList failed");
    sptr<CameraStandard::CameraOutputCapability> capability = cameraManager_->GetSupportedOutputCapability(info);
    CHECK_AND_FREE_RETURN_RET_LOG(capability == nullptr, DCAMERA_BAD_VALUE, root, "get supported capability is null");
    std::vector<CameraStandard::Profile> photoProfiles = capability->GetPhotoProfiles();
    ConfigFormatphoto(SNAPSHOT_FRAME, root, photoProfiles);

    std::vector<CameraStandard::Profile> previewProfiles = capability->GetPreviewProfiles();
    ConfigFormatvideo(CONTINUOUS_FRAME, root, previewProfiles);

    sptr<CameraStandard::CameraInput> cameraInput = nullptr;
    int rv = cameraManager_->CreateCameraInput(info, &cameraInput);
    CHECK_AND_FREE_RETURN_RET_LOG(rv != DCAMERA_OK, DCAMERA_BAD_VALUE, root, "create cameraInput failed");

    std::hash<std::string> h;
    std::string abilityString = cameraInput->GetCameraSettings();
    DHLOGI("abilityString hash: %{public}zu, length: %{public}zu", h(abilityString), abilityString.length());

    std::string encodeString = Base64Encode(reinterpret_cast<const unsigned char *>(abilityString.c_str()),
        abilityString.length());
    DHLOGI("encodeString hash: %zu, length: %zu", h(encodeString), encodeString.length());
    cJSON_AddStringToObject(root, CAMERA_METADATA_KEY.c_str(), encodeString.c_str());
    char *jsonstr = cJSON_Print(root);
    if (jsonstr == nullptr) {
        cJSON_Delete(root);
        return DCAMERA_BAD_VALUE;
    }

    item.attrs = jsonstr;
    if (cameraInput->Release() != DCAMERA_OK) {
        DHLOGE("cameraInput Release failed");
    }
    cJSON_free(jsonstr);
    cJSON_Delete(root);
    return DCAMERA_OK;
}

std::string DCameraHandler::GetCameraPosition(CameraStandard::CameraPosition position)
{
    DHLOGI("position: %{public}d", position);
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
            DHLOGE("unknown camera position");
            break;
        }
    }
    DHLOGI("success ret: %{public}s", ret.c_str());
    return ret;
}

void DCameraHandler::ProcessProfile(const DCStreamType type, std::map<std::string, std::list<std::string>>& formatMap,
    std::vector<CameraStandard::Profile>& profileList, std::set<int32_t>& formatSet)
{
    for (auto& profile : profileList) {
        CameraStandard::CameraFormat format = profile.GetCameraFormat();
        CameraStandard::Size picSize = profile.GetSize();
        int32_t dformat = CovertToDcameraFormat(format);
        if (dformat == INVALID_FORMAT) {
            continue;
        }
        formatSet.insert(dformat);
        DHLOGI("width: %{public}d, height: %{public}d, format: %{public}d", picSize.width, picSize.height, dformat);
        std::string formatName = std::to_string(dformat);
        if (IsValid(type, picSize)) {
            std::string resolutionValue = std::to_string(picSize.width) + "*" + std::to_string(picSize.height);
            formatMap[formatName].push_back(resolutionValue);
        }
    }
}

void DCameraHandler::ConfigFormatphoto(const DCStreamType type, cJSON* root,
    std::vector<CameraStandard::Profile>& profileList)
{
    DHLOGI("type: %{public}d, size: %{public}zu", type, profileList.size());
    std::set<int32_t> formatSet;
    cJSON* formatphotoObj = cJSON_CreateObject();
    if (formatphotoObj == nullptr) {
        return;
    }
    cJSON_AddItemToObject(root, CAMERA_FORMAT_PHOTO.c_str(), formatphotoObj);
    std::map<std::string, std::list<std::string>> formatMap;
    ProcessProfile(type, formatMap, profileList, formatSet);
    cJSON* resolutionObj = cJSON_CreateObject();
    if (resolutionObj == nullptr) {
        return;
    }
    for (auto &pair : formatMap) {
        cJSON* array = cJSON_CreateArray();
        cJSON_AddItemToObject(resolutionObj, pair.first.c_str(), array);
        for (auto &value : pair.second) {
            cJSON_AddItemToArray(array, cJSON_CreateString(value.c_str()));
        }
    }
    cJSON_AddItemToObject(formatphotoObj, CAMERA_RESOLUTION_KEY.c_str(), resolutionObj);
    cJSON* array = cJSON_CreateArray();
    if (array == nullptr) {
        return;
    }
    for (auto format : formatSet) {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(format));
    }
    cJSON_AddItemToObject(formatphotoObj, CAMERA_FORMAT_KEY.c_str(), array);
}

void DCameraHandler::ConfigFormatvideo(const DCStreamType type, cJSON* root,
    std::vector<CameraStandard::Profile>& profileList)
{
    DHLOGI("type: %d, size: %{public}zu", type, profileList.size());
    std::set<int32_t> formatSet;
    cJSON* formatpreviewObj = cJSON_CreateObject();
    if (formatpreviewObj == nullptr) {
        return;
    }
    cJSON_AddItemToObject(root, CAMERA_FORMAT_PREVIEW.c_str(), formatpreviewObj);
    std::map<std::string, std::list<std::string>> formatMap;
    ProcessProfile(type, formatMap, profileList, formatSet);
    cJSON* resolutionObj = cJSON_CreateObject();
    if (resolutionObj == nullptr) {
        return;
    }
    for (auto &pair : formatMap) {
        cJSON* array = cJSON_CreateArray();
        cJSON_AddItemToObject(resolutionObj, pair.first.c_str(), array);
        for (auto &value : pair.second) {
            cJSON_AddItemToArray(array, cJSON_CreateString(value.c_str()));
        }
    }
    cJSON_AddItemToObject(formatpreviewObj, CAMERA_RESOLUTION_KEY.c_str(), resolutionObj);
    cJSON* array = cJSON_CreateArray();
    if (array == nullptr) {
        return;
    }
    for (auto format : formatSet) {
        cJSON_AddItemToArray(array, cJSON_CreateNumber(format));
    }
    cJSON_AddItemToObject(formatpreviewObj, CAMERA_FORMAT_KEY.c_str(), array);
    cJSON* formatvideoObj = cJSON_Duplicate(formatpreviewObj, 1);
    cJSON_AddItemToObject(root, CAMERA_FORMAT_VIDEO.c_str(), formatvideoObj);
}

int32_t DCameraHandler::CovertToDcameraFormat(CameraStandard::CameraFormat format)
{
    DHLOGI("format: %{public}d", format);
    int32_t ret = INVALID_FORMAT;
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
        case CameraStandard::CameraFormat::CAMERA_FORMAT_YCBCR_P010:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_YCBCR_P010;
            break;
        case CameraStandard::CameraFormat::CAMERA_FORMAT_YCRCB_P010:
            ret = camera_format_t::OHOS_CAMERA_FORMAT_YCRCB_P010;
            break;
        default:
            DHLOGE("invalid camera format");
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
            if (size.width > MAXWIDTHSIZE) {
                DHLOGE("size width out of range.");
                return ret;
            }
            uint64_t dcResolution = static_cast<uint64_t>(size.width * size.width);
            uint64_t dcMaxResolution = static_cast<uint64_t>(RESOLUTION_MAX_WIDTH_SNAPSHOT *
                                                             RESOLUTION_MAX_HEIGHT_SNAPSHOT);
            uint64_t dcMinResolution = static_cast<uint64_t>(RESOLUTION_MIN_WIDTH *
                                                             RESOLUTION_MIN_HEIGHT);
            ret = (dcResolution >= dcMinResolution) && (dcResolution <= dcMaxResolution);
            break;
        }
        default: {
            DHLOGE("unknown stream type");
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