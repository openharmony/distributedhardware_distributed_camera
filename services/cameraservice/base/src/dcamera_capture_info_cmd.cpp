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

#include "dcamera_capture_info_cmd.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraCaptureInfoCmd::Marshal(std::string& jsonStr)
{
    cJSON *rootValue = cJSON_CreateObject();
    CHECK_NULL_RETURN((rootValue == nullptr), DCAMERA_BAD_VALUE);

    cJSON_AddStringToObject(rootValue, "Type", type_.c_str());
    cJSON_AddStringToObject(rootValue, "dhId", dhId_.c_str());
    cJSON_AddStringToObject(rootValue, "Command", command_.c_str());

    cJSON *captureInfos = cJSON_CreateArray();
    CHECK_NULL_FREE_RETURN(captureInfos, DCAMERA_BAD_VALUE, rootValue);
    cJSON_AddItemToObject(rootValue, "Value", captureInfos);
    for (auto iter = value_.begin(); iter != value_.end(); iter++) {
        std::shared_ptr<DCameraCaptureInfo> capture = *iter;
        CHECK_NULL_FREE_RETURN(capture, DCAMERA_BAD_VALUE, rootValue);
        cJSON *captureInfo = cJSON_CreateObject();
        CHECK_NULL_FREE_RETURN(captureInfo, DCAMERA_BAD_VALUE, rootValue);
        cJSON_AddItemToArray(captureInfos, captureInfo);
        cJSON_AddNumberToObject(captureInfo, "Width", capture->width_);
        cJSON_AddNumberToObject(captureInfo, "Height", capture->height_);
        cJSON_AddNumberToObject(captureInfo, "Format", capture->format_);
        cJSON_AddNumberToObject(captureInfo, "DataSpace", capture->dataspace_);
        cJSON_AddBoolToObject(captureInfo, "IsCapture", capture->isCapture_);
        cJSON_AddNumberToObject(captureInfo, "EncodeType", capture->encodeType_);
        cJSON_AddNumberToObject(captureInfo, "StreamType", capture->streamType_);
        cJSON *captureSettings = cJSON_CreateArray();
        CHECK_NULL_FREE_RETURN(captureSettings, DCAMERA_BAD_VALUE, rootValue);
        cJSON_AddItemToObject(captureInfo, "CaptureSettings", captureSettings);
        for (auto settingIter = capture->captureSettings_.begin();
            settingIter != capture->captureSettings_.end(); settingIter++) {
            cJSON *captureSetting = cJSON_CreateObject();
            CHECK_NULL_FREE_RETURN(captureSetting, DCAMERA_BAD_VALUE, rootValue);
            cJSON_AddNumberToObject(captureSetting, "SettingType", (*settingIter)->type_);
            cJSON_AddStringToObject(captureSetting, "SettingValue", (*settingIter)->value_.c_str());
            cJSON_AddItemToArray(captureSettings, captureSetting);
        }
    }
    cJSON_AddNumberToObject(rootValue, "mode", sceneMode_);
    cJSON_AddNumberToObject(rootValue, "userId", userId_);
    cJSON_AddNumberToObject(rootValue, "tokenId", tokenId_);
    cJSON_AddStringToObject(rootValue, "accountId", accountId_.c_str());

    char *data = cJSON_Print(rootValue);
    if (data == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    jsonStr = std::string(data);
    cJSON_Delete(rootValue);
    cJSON_free(data);
    return DCAMERA_OK;
}

int32_t DCameraCaptureInfoCmd::Unmarshal(const std::string& jsonStr)
{
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    CHECK_NULL_RETURN((rootValue == nullptr), DCAMERA_BAD_VALUE);

    cJSON *type = cJSON_GetObjectItemCaseSensitive(rootValue, "Type");
    CHECK_AND_FREE_RETURN_RET_LOG((type == nullptr || !cJSON_IsString(type) || (type->valuestring == nullptr)),
        DCAMERA_BAD_VALUE, rootValue, "type parse fail.");
    type_ = type->valuestring;

    cJSON *dhId = cJSON_GetObjectItemCaseSensitive(rootValue, "dhId");
    CHECK_AND_FREE_RETURN_RET_LOG((dhId == nullptr || !cJSON_IsString(dhId) || (dhId->valuestring == nullptr)),
        DCAMERA_BAD_VALUE, rootValue, "dhId parse fail.");
    dhId_ = dhId->valuestring;

    cJSON *command = cJSON_GetObjectItemCaseSensitive(rootValue, "Command");
    if (command == nullptr || !cJSON_IsString(command) || (command->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    command_ = command->valuestring;

    int32_t ret = UmarshalValue(rootValue);

    cJSON *mode = cJSON_GetObjectItemCaseSensitive(rootValue, "mode");
    if (mode == nullptr || !cJSON_IsNumber(mode)) {
        sceneMode_ = 0;
    } else {
        sceneMode_ = mode->valueint;
    }
    cJSON *userId = cJSON_GetObjectItemCaseSensitive(rootValue, "userId");
    if (userId == nullptr || !cJSON_IsNumber(userId)) {
        userId_ = -1;
    } else {
        userId_ = userId->valueint;
    }
    cJSON *tokenId = cJSON_GetObjectItemCaseSensitive(rootValue, "tokenId");
    if (tokenId == nullptr || !cJSON_IsNumber(tokenId)) {
        tokenId_ = 0;
    } else {
        tokenId_ = static_cast<uint64_t>(tokenId->valueint);
    }
    cJSON *accountId = cJSON_GetObjectItemCaseSensitive(rootValue, "accountId");
    if (accountId == nullptr || !cJSON_IsString(accountId) || (accountId->valuestring == nullptr)) {
        accountId_ = "";
    } else {
        accountId_ = accountId->valuestring;
    }
    cJSON_Delete(rootValue);
    return ret;
}

int32_t DCameraCaptureInfoCmd::UmarshalValue(cJSON *rootValue)
{
    cJSON *valueJson = cJSON_GetObjectItemCaseSensitive(rootValue, "Value");
    if (valueJson == nullptr || !cJSON_IsArray(valueJson)) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *capInfo = nullptr;
    cJSON_ArrayForEach(capInfo, valueJson) {
        std::shared_ptr<DCameraCaptureInfo> captureInfo = std::make_shared<DCameraCaptureInfo>();
        cJSON *width = cJSON_GetObjectItemCaseSensitive(capInfo, "Width");
        CHECK_NULL_RETURN((width == nullptr || !cJSON_IsNumber(width)), DCAMERA_BAD_VALUE);
        captureInfo->width_ = width->valueint;

        cJSON *height = cJSON_GetObjectItemCaseSensitive(capInfo, "Height");
        CHECK_NULL_RETURN((height == nullptr || !cJSON_IsNumber(height)), DCAMERA_BAD_VALUE);
        captureInfo->height_ = height->valueint;

        cJSON *format = cJSON_GetObjectItemCaseSensitive(capInfo, "Format");
        CHECK_NULL_RETURN((format == nullptr || !cJSON_IsNumber(format)), DCAMERA_BAD_VALUE);
        captureInfo->format_ = format->valueint;

        cJSON *dataSpace = cJSON_GetObjectItemCaseSensitive(capInfo, "DataSpace");
        CHECK_NULL_RETURN((dataSpace == nullptr || !cJSON_IsNumber(dataSpace)), DCAMERA_BAD_VALUE);
        captureInfo->dataspace_ = dataSpace->valueint;

        cJSON *isCapture = cJSON_GetObjectItemCaseSensitive(capInfo, "IsCapture");
        CHECK_NULL_RETURN((isCapture == nullptr || !cJSON_IsBool(isCapture)), DCAMERA_BAD_VALUE);
        captureInfo->isCapture_ = cJSON_IsTrue(isCapture);

        cJSON *encodeType = cJSON_GetObjectItemCaseSensitive(capInfo, "EncodeType");
        CHECK_NULL_RETURN((encodeType == nullptr || !cJSON_IsNumber(encodeType)), DCAMERA_BAD_VALUE);
        captureInfo->encodeType_ = static_cast<DCEncodeType>(encodeType->valueint);

        cJSON *streamType = cJSON_GetObjectItemCaseSensitive(capInfo, "StreamType");
        CHECK_NULL_RETURN((streamType == nullptr || !cJSON_IsNumber(streamType)), DCAMERA_BAD_VALUE);
        captureInfo->streamType_ = static_cast<DCStreamType>(streamType->valueint);

        cJSON *captureSettings = cJSON_GetObjectItemCaseSensitive(capInfo, "CaptureSettings");
        CHECK_NULL_RETURN((captureSettings == nullptr || !cJSON_IsArray(captureSettings)), DCAMERA_BAD_VALUE);
        int32_t ret = UmarshalSettings(captureSettings, captureInfo);
        if (ret != DCAMERA_OK) {
            return ret;
        }
        value_.push_back(captureInfo);
    }
    return DCAMERA_OK;
}

int32_t DCameraCaptureInfoCmd::UmarshalSettings(cJSON *valueJson,
    std::shared_ptr<DCameraCaptureInfo>& captureInfo)
{
    if (captureInfo == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *captureSetting = nullptr;
    cJSON_ArrayForEach(captureSetting, valueJson) {
        cJSON *settingType = cJSON_GetObjectItemCaseSensitive(captureSetting, "SettingType");
        if (settingType == nullptr || !cJSON_IsNumber(settingType)) {
            return DCAMERA_BAD_VALUE;
        }

        cJSON *settingValue = cJSON_GetObjectItemCaseSensitive(captureSetting, "SettingValue");
        if (settingValue == nullptr || !cJSON_IsString(settingValue) ||
            (settingValue->valuestring == nullptr)) {
            return DCAMERA_BAD_VALUE;
        }
        std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
        setting->type_ = static_cast<DCSettingsType>(settingType->valueint);
        setting->value_ = settingValue->valuestring;
        captureInfo->captureSettings_.push_back(setting);
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
