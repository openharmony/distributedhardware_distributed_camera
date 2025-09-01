/*
 * Copyright (c) 2023-2025 Huawei Device Co., Ltd.
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

#include "dcamera_sink_frame_info.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {
void DCameraSinkFrameInfo::Marshal(std::string& jsonStr)
{
    cJSON *frameInfo = cJSON_CreateObject();
    if (frameInfo == nullptr) {
        return;
    }
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_TYPE.c_str(), type_);
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_INDEX.c_str(), index_);
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_PTS.c_str(), pts_);
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_START_ENCODE.c_str(), startEncodeT_);
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_FINISH_ENCODE.c_str(), finishEncodeT_);
    cJSON_AddNumberToObject(frameInfo, FRAME_INFO_SENDT.c_str(), sendT_);
    cJSON_AddStringToObject(frameInfo, FRAME_INFO_VERSION.c_str(), ver_.c_str());
    cJSON_AddStringToObject(frameInfo, RAW_TIME.c_str(), rawTime_.c_str());

    char *data = cJSON_Print(frameInfo);
    if (data == nullptr) {
        cJSON_Delete(frameInfo);
        return;
    }
    jsonStr = std::string(data);
    cJSON_Delete(frameInfo);
    cJSON_free(data);
}

int32_t DCameraSinkFrameInfo::Unmarshal(const std::string& jsonStr)
{
    cJSON *rootValue = cJSON_Parse(jsonStr.c_str());
    CHECK_NULL_RETURN((rootValue == nullptr), DCAMERA_BAD_VALUE);

    cJSON *type = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_TYPE.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((type == nullptr || !cJSON_IsNumber(type)),
        DCAMERA_BAD_VALUE, rootValue, "type parse fail.");
    type_ = static_cast<int8_t>(type->valueint);

    cJSON *index = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_INDEX.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((index == nullptr || !cJSON_IsNumber(index)),
        DCAMERA_BAD_VALUE, rootValue, "index parse fail.");
    index_ = static_cast<int32_t>(index->valueint);

    cJSON *pts = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_PTS.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((pts == nullptr || !cJSON_IsNumber(pts)),
        DCAMERA_BAD_VALUE, rootValue, "pts parse fail.");
    pts_ = static_cast<int64_t>(pts->valueint);

    cJSON *startEncode = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_START_ENCODE.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((startEncode == nullptr || !cJSON_IsNumber(startEncode)),
        DCAMERA_BAD_VALUE, rootValue, "startEncode parse fail.");
    startEncodeT_ = static_cast<int64_t>(startEncode->valueint);

    cJSON *finishEncode = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_FINISH_ENCODE.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((finishEncode == nullptr || !cJSON_IsNumber(finishEncode)),
        DCAMERA_BAD_VALUE, rootValue, "finishEncode parse fail.");
    finishEncodeT_ = static_cast<int64_t>(finishEncode->valueint);

    cJSON *sendT = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_SENDT.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((sendT == nullptr || !cJSON_IsNumber(sendT)),
        DCAMERA_BAD_VALUE, rootValue, "sendT parse fail.");
    sendT_ = static_cast<int64_t>(sendT->valueint);

    cJSON *ver = cJSON_GetObjectItemCaseSensitive(rootValue, FRAME_INFO_VERSION.c_str());
    CHECK_AND_FREE_RETURN_RET_LOG((ver == nullptr || !cJSON_IsString(ver)),
        DCAMERA_BAD_VALUE, rootValue, "ver parse fail.");
    ver_ = std::string(ver->valuestring);

    cJSON *rawTime = cJSON_GetObjectItemCaseSensitive(rootValue, RAW_TIME.c_str());
    if (rawTime == nullptr) {
        rawTime_ = "";
    } else if (!cJSON_IsString(rawTime)) {
        DHLOGE("rawTime parse fail.");
        cJSON_Delete(rootValue);
        return DCAMERA_BAD_VALUE;
    } else {
        rawTime_ = std::string(rawTime->valuestring);
    }
    cJSON_Delete(rootValue);
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS