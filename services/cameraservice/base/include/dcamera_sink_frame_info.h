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

#ifndef OHOS_DCAMERA_SINK_FRAME_INFO_H
#define OHOS_DCAMERA_SINK_FRAME_INFO_H

#include <string>

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkFrameInfo {
public:
    DCameraSinkFrameInfo()
        : type_(-1), index_(-1), pts_(0), startEncodeT_(0), finishEncodeT_(0), sendT_(0), ver_("1.0")
    {}
    ~DCameraSinkFrameInfo() = default;
    int8_t type_;
    int32_t index_;
    int64_t pts_;
    int64_t startEncodeT_;
    int64_t finishEncodeT_;
    int64_t sendT_;
    std::string ver_;
    std::string rawTime_;

public:
    const std::string FRAME_INFO_TYPE = "type";
    const std::string FRAME_INFO_INDEX = "index";
    const std::string FRAME_INFO_PTS = "pts";
    const std::string FRAME_INFO_START_ENCODE = "startEncodeT";
    const std::string FRAME_INFO_FINISH_ENCODE = "finishEncodeT";
    const std::string FRAME_INFO_ENCODET = "encodeT";
    const std::string FRAME_INFO_SENDT = "sendT";
    const std::string FRAME_INFO_VERSION = "ver";
    const std::string RAW_TIME = "rawTime";

public:
    void Marshal(std::string& jsonStr);
    int32_t Unmarshal(const std::string& jsonStr);
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_FRAME_INFO_H
