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

#include "dcamera_channel_info_cmd.h"
#include "cJSON.h"
#include "distributed_camera_constants.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
int32_t DCameraChannelInfoCmd::Marshal(std::string& jsonStr)
{
    cJSON *rootValue = cJSON_CreateObject();
    if (rootValue == nullptr) {
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddStringToObject(rootValue, "Type", type_.c_str());
    cJSON_AddStringToObject(rootValue, "dhId", dhId_.c_str());
    cJSON_AddStringToObject(rootValue, "Command", command_.c_str());

    CHECK_AND_FREE_RETURN_RET_LOG(value_ == nullptr, DCAMERA_BAD_VALUE, rootValue, "value_ is nullptr");

    cJSON *channelInfo = cJSON_CreateObject();
    if (channelInfo == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddStringToObject(channelInfo, "SourceDevId", value_->sourceDevId_.c_str());
    cJSON_AddItemToObject(rootValue, "Value", channelInfo);

    cJSON *details = cJSON_CreateArray();
    if (details == nullptr) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    cJSON_AddItemToObject(channelInfo, "Detail", details);
    for (auto iter = value_->detail_.begin(); iter != value_->detail_.end(); iter++) {
        cJSON *detail = cJSON_CreateObject();
        if (detail == nullptr) {
            cJSON_Delete(rootValue);
            return DCAMERA_BAD_VALUE;
        }
        cJSON_AddStringToObject(detail, "DataSessionFlag", iter->dataSessionFlag_.c_str());
        cJSON_AddNumberToObject(detail, "StreamType", iter->streamType_);
        cJSON_AddItemToArray(details, detail);
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

int32_t DCameraChannelInfoCmd::Unmarshal(const std::string& jsonStr)
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
    cJSON *sourceDevId = cJSON_GetObjectItemCaseSensitive(valueJson, "SourceDevId");
    if (sourceDevId == nullptr || !cJSON_IsString(sourceDevId) || (sourceDevId->valuestring == nullptr)) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    std::shared_ptr<DCameraChannelInfo> channelInfo = std::make_shared<DCameraChannelInfo>();
    channelInfo->sourceDevId_ = sourceDevId->valuestring;
    cJSON *details = cJSON_GetObjectItemCaseSensitive(valueJson, "Detail");
    if (details == nullptr || !cJSON_IsArray(details) || cJSON_GetArraySize(details) == 0) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    if (UnmarshalDetails(details, channelInfo) != DCAMERA_OK) {
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    }
    value_ = channelInfo;
    cJSON_Delete(rootValue);
    return DCAMERA_OK;
}
       
int32_t DCameraChannelInfoCmd::UnmarshalDetails(cJSON *details, std::shared_ptr<DCameraChannelInfo> channelInfo)
{
    CHECK_AND_RETURN_RET_LOG(details == nullptr, DCAMERA_BAD_VALUE, "details is nullptr");
    CHECK_AND_RETURN_RET_LOG(channelInfo == nullptr, DCAMERA_BAD_VALUE, "channelInfo is nullptr");
    cJSON *detail = nullptr;
    cJSON_ArrayForEach(detail, details) {
        cJSON *dataSessionFlag = cJSON_GetObjectItemCaseSensitive(detail, "DataSessionFlag");
        cJSON *streamType = cJSON_GetObjectItemCaseSensitive(detail, "StreamType");
        if (dataSessionFlag == nullptr || !cJSON_IsString(dataSessionFlag) ||
            (dataSessionFlag->valuestring == nullptr)) {
            return DCAMERA_BAD_VALUE;
        }
        if (streamType == nullptr || !cJSON_IsNumber(streamType)) {
            return DCAMERA_BAD_VALUE;
        }
        DCameraChannelDetail channelDetail;
        channelDetail.dataSessionFlag_ = dataSessionFlag->valuestring;
        channelDetail.streamType_ = static_cast<DCStreamType>(streamType->valueint);
        channelInfo->detail_.push_back(channelDetail);
    }
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS
