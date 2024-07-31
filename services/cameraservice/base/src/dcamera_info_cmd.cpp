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

#include "dcamera_info_cmd.h"
#include "cJSON.h"

#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraInfoCmd::Marshal(std::string& jsonStr)
{
    if (value_ == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *rootValue = cJSON_CreateObject();
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddStringToObject(rootValue, "Type", type_.c_str());
    cJSON_AddStringToObject(rootValue, "dhId", dhId_.c_str());
    cJSON_AddStringToObject(rootValue, "Command", command_.c_str());

    cJSON *info = cJSON_CreateObject();
    if (info == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddNumberToObject(info, "State", value_->state_);
    cJSON_AddItemToObject(rootValue, "Value", info);
    
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

int32_t DCameraInfoCmd::Unmarshal(const std::string& jsonStr)
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
    cJSON *valueJson = cJSON_GetObjectItemCaseSensitive(rootValue, "Value");
    if (valueJson == nullptr || !cJSON_IsObject(valueJson)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON *state = cJSON_GetObjectItemCaseSensitive(valueJson, "State");
    if (state == nullptr || !cJSON_IsNumber(state)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraInfo> info = std::make_shared<DCameraInfo>();
    info->state_ = state->valueint;

    value_ = info;
    cJSON_Delete(rootValue);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
