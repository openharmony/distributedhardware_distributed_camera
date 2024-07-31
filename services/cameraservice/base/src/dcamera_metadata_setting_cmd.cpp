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

#include "dcamera_metadata_setting_cmd.h"
#include "cJSON.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraMetadataSettingCmd::Marshal(std::string& jsonStr)
{
    cJSON *rootValue = cJSON_CreateObject();
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddStringToObject(rootValue, "Type", type_.c_str());
    cJSON_AddStringToObject(rootValue, "dhId", dhId_.c_str());
    cJSON_AddStringToObject(rootValue, "Command", command_.c_str());

    cJSON *settings = cJSON_CreateArray();
    if (settings == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddItemToObject(rootValue, "Value", settings);
    for (auto iter = value_.begin(); iter != value_.end(); iter++) {
        if ((*iter) == nullptr) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        cJSON *setting = cJSON_CreateObject();
        if (setting == nullptr) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        cJSON_AddNumberToObject(setting, "SettingType", (*iter)->type_);
        cJSON_AddStringToObject(setting, "SettingValue", (*iter)->value_.c_str());
        cJSON_AddItemToArray(settings, setting);
    }
    
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

int32_t DCameraMetadataSettingCmd::Unmarshal(const std::string& jsonStr)
{
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *type = cJSON_GetObjectItemCaseSensitive(rootValue, "Type");
    if (type == nullptr || !cJSON_IsString(type) || (type->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    type_ = type->valuestring;
    cJSON *dhId = cJSON_GetObjectItemCaseSensitive(rootValue, "dhId");
    if (dhId == nullptr || !cJSON_IsString(dhId) || (dhId->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    dhId_ = dhId->valuestring;
    cJSON *command = cJSON_GetObjectItemCaseSensitive(rootValue, "Command");
    if (command == nullptr || !cJSON_IsString(command) || (command->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    command_ = command->valuestring;
    cJSON *settings = cJSON_GetObjectItemCaseSensitive(rootValue, "Value");
    if (settings == nullptr || !cJSON_IsArray(settings) || cJSON_GetArraySize(settings) == 0) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON *subSetting = nullptr;
    cJSON_ArrayForEach(subSetting, settings) {
        cJSON *settingType = cJSON_GetObjectItemCaseSensitive(subSetting, "SettingType");
        cJSON *settingValue = cJSON_GetObjectItemCaseSensitive(subSetting, "SettingValue");
        if (settingType == nullptr || !cJSON_IsNumber(settingType)) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        if (settingValue == nullptr || !cJSON_IsString(settingValue) || (settingValue->valuestring == nullptr)) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        std::shared_ptr<DCameraSettings> setting = std::make_shared<DCameraSettings>();
        setting->type_ = (DCSettingsType)settingType->valueint;
        setting->value_ = settingValue->valuestring;
        value_.push_back(setting);
    }
    cJSON_Delete(rootValue);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
