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

#include "dcamera_event_cmd.h"
#include "cJSON.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraEventCmd::Marshal(std::string& jsonStr)
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

    cJSON *event = cJSON_CreateObject();
    if (event == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddNumberToObject(event, "EventType", value_->eventType_);
    cJSON_AddNumberToObject(event, "EventResult", value_->eventResult_);
    cJSON_AddStringToObject(event, "EventContent", value_->eventContent_.c_str());
    cJSON_AddItemToObject(rootValue, "Value", event);
    
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

int32_t DCameraEventCmd::Unmarshal(const std::string& jsonStr)
{
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON *type = cJSON_GetObjectItemCaseSensitive(rootValue, "Type");
    cJSON *dhId = cJSON_GetObjectItemCaseSensitive(rootValue, "dhId");
    if (type == nullptr || !cJSON_IsString(type) || (type->valuestring == nullptr) ||
        dhId == nullptr || !cJSON_IsString(dhId) || (dhId->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    type_ = type->valuestring;
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
    cJSON *eventType = cJSON_GetObjectItemCaseSensitive(valueJson, "EventType");
    if (eventType == nullptr || !cJSON_IsNumber(eventType)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraEvent> event = std::make_shared<DCameraEvent>();
    event->eventType_ = eventType->valueint;
    cJSON *eventResult = cJSON_GetObjectItemCaseSensitive(valueJson, "EventResult");
    if (eventResult == nullptr || !cJSON_IsNumber(eventResult)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    event->eventResult_ = eventResult->valueint;
    cJSON *eventContent = cJSON_GetObjectItemCaseSensitive(valueJson, "EventContent");
    if (eventContent == nullptr || !cJSON_IsString(eventContent) || (eventContent->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    event->eventContent_ = eventContent->valuestring;
    value_ = event;
    cJSON_Delete(rootValue);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
