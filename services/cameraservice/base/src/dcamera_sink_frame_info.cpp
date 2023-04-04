/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "nlohmann/json.hpp"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"
using json = nlohmann::json;

namespace OHOS {
namespace DistributedHardware {
void DCameraSinkFrameInfo::Marshal(std::string& jsonStr)
{
    json frameInfo;
    frameInfo[FRAME_INFO_TYPE] = type_;
    frameInfo[FRAME_INFO_INDEX] = index_;
    frameInfo[FRAME_INFO_PTS] = pts_;
    frameInfo[FRAME_INFO_START_ENCODE] = startEncodeT_;
    frameInfo[FRAME_INFO_FINISH_ENCODE] = finishEncodeT_;
    frameInfo[FRAME_INFO_SENDT] = sendT_;
    frameInfo[FRAME_INFO_VERSION] = ver_;
    jsonStr = frameInfo.dump();
}

int32_t DCameraSinkFrameInfo::Unmarshal(const std::string& jsonStr)
{
    json frameInfo = json::parse(jsonStr, nullptr, false);
    if (frameInfo.is_discarded()) {
        DHLOGE("FrameInfo json::parse error.");
        return DCAMERA_BAD_VALUE;
    }

    if (!frameInfo.contains(FRAME_INFO_TYPE) || !frameInfo[FRAME_INFO_TYPE].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    type_ = frameInfo[FRAME_INFO_TYPE].get<std::int8_t>();

    if (!frameInfo.contains(FRAME_INFO_INDEX) || !frameInfo[FRAME_INFO_INDEX].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    index_ = frameInfo[FRAME_INFO_INDEX].get<std::int32_t>();

    if (!frameInfo.contains(FRAME_INFO_PTS) || !frameInfo[FRAME_INFO_PTS].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    pts_ = frameInfo[FRAME_INFO_PTS].get<std::int64_t>();

    if (!frameInfo.contains(FRAME_INFO_START_ENCODE) || !frameInfo[FRAME_INFO_START_ENCODE].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    startEncodeT_ = frameInfo[FRAME_INFO_START_ENCODE].get<std::int64_t>();

    if (!frameInfo.contains(FRAME_INFO_FINISH_ENCODE) || !frameInfo[FRAME_INFO_FINISH_ENCODE].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    finishEncodeT_ = frameInfo[FRAME_INFO_FINISH_ENCODE].get<std::int64_t>();

    if (!frameInfo.contains(FRAME_INFO_SENDT) || !frameInfo[FRAME_INFO_SENDT].is_number_integer()) {
        return DCAMERA_BAD_VALUE;
    }
    sendT_ = frameInfo[FRAME_INFO_SENDT].get<std::int64_t>();

    if (!frameInfo.contains(FRAME_INFO_VERSION) || !frameInfo[FRAME_INFO_VERSION].is_string()) {
        return DCAMERA_BAD_VALUE;
    }
    ver_ = frameInfo[FRAME_INFO_VERSION].get<std::string>();
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS