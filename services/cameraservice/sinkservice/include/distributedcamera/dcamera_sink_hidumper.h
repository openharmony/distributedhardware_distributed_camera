/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_SINK_HIDUMPER_H
#define OHOS_DISTRIBUTED_CAMERA_SINK_HIDUMPER_H

#include <map>
#include <string>
#include <vector>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceDev;
enum class HidumpFlag {
    UNKNOWN = 0,
    GET_HELP,
    GET_CAMERA_INFO,
    GET_OPENED_INFO,
    GET_VERSION_INFO,
    START_DUMP,
    STOP_DUMP,
};

struct CameraDumpInfo {
    std::string version;
    int32_t camNumber;
    std::vector<std::string> camIds;
};

class DcameraSinkHidumper {
DECLARE_SINGLE_INSTANCE_BASE(DcameraSinkHidumper);

public:
    bool Dump(const std::vector<std::string>& args, std::string& result);

private:
    explicit DcameraSinkHidumper() = default;
    ~DcameraSinkHidumper() = default;
    void ShowHelp(std::string& result);
    int32_t ShowIllegalInfomation(std::string& result);
    int32_t ProcessDump(const std::string& args, std::string& result);
    void SetSinkDumpInfo(CameraDumpInfo& camDumpInfo_);

    int32_t GetLocalCameraNumber(std::string& result);
    int32_t GetOpenedCameraInfo(std::string& result);
    int32_t GetVersionInfo(std::string& result);

private:
    CameraDumpInfo camDumpInfo_;
    const int32_t DUMP_MAX_SIZE = 10 * 1024;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_SINK_HIDUMPER_H