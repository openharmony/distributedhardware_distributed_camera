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

#include "dcamera_sink_hidumper.h"

#include "dcamera_hidumper.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_sink_service.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DcameraSinkHidumper);

namespace {
const std::string ARGS_HELP = "-h";
const std::string ARGS_VERSION_INFO = "--version";
const std::string ARGS_CAMERA_INFO = "--camNum";
const std::string ARGS_START_DUMP = "--startdump";
const std::string ARGS_STOP_DUMP = "--stopdump";
const std::string ARGS_OPENED_INFO = "--opened";

const std::map<std::string, HidumpFlag> ARGS_MAP = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { ARGS_CAMERA_INFO, HidumpFlag::GET_CAMERA_INFO },
    { ARGS_OPENED_INFO, HidumpFlag::GET_OPENED_INFO },
    { ARGS_VERSION_INFO, HidumpFlag::GET_VERSION_INFO },
    { ARGS_START_DUMP, HidumpFlag::START_DUMP },
    { ARGS_STOP_DUMP, HidumpFlag::STOP_DUMP },
};
}

void DcameraSinkHidumper::SetSinkDumpInfo(CameraDumpInfo& camDumpInfo_)
{
    DistributedCameraSinkService::GetCamDumpInfo(camDumpInfo_);
}

bool DcameraSinkHidumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    result.clear();
    int32_t argsSize = static_cast<int32_t>(args.size());
    if (argsSize > DUMP_MAX_SIZE) {
        DHLOGE("DcameraSinkHidumper Dump args.size() is invalid");
        return false;
    }
    DHLOGI("DcameraSinkHidumper Dump args.size():%{public}d.", argsSize);
    if (args.empty()) {
        ShowHelp(result);
        return true;
    } else if (args.size() > 1) {
        ShowIllegalInfomation(result);
        return true;
    }

    for (int32_t i = 0; i < argsSize; i++) {
        DHLOGI("DcameraSinkHidumper Dump args[%{public}d]: %{public}s.", i, args.at(i).c_str());
    }

    if (ProcessDump(args[0], result) != DCAMERA_OK) {
        return false;
    }
    return true;
}

int32_t DcameraSinkHidumper::ProcessDump(const std::string& args, std::string& result)
{
    DHLOGI("ProcessDump Dump.");
    HidumpFlag hf = HidumpFlag::UNKNOWN;
    auto operatorIter = ARGS_MAP.find(args);
    if (operatorIter != ARGS_MAP.end()) {
        hf = operatorIter->second;
    }

    if (hf == HidumpFlag::GET_HELP) {
        ShowHelp(result);
        return DCAMERA_OK;
    }
    result.clear();
    SetSinkDumpInfo(camDumpInfo_);
    int32_t ret = DCAMERA_BAD_VALUE;
    switch (hf) {
        case HidumpFlag::GET_CAMERA_INFO: {
            ret = GetLocalCameraNumber(result);
            break;
        }
        case HidumpFlag::GET_OPENED_INFO: {
            ret = GetOpenedCameraInfo(result);
            break;
        }
        case HidumpFlag::GET_VERSION_INFO: {
            ret = GetVersionInfo(result);
            break;
        }
        case HidumpFlag::START_DUMP: {
            ret = DcameraHidumper::GetInstance().StartDump();
            result.append("Send dump order ok\n");
            break;
        }
        case HidumpFlag::STOP_DUMP: {
            ret = DcameraHidumper::GetInstance().StopDump();
            result.append("Send stop dump order ok\n");
            break;
        }
        default: {
            ret = ShowIllegalInfomation(result);
            break;
        }
    }

    return ret;
}

int32_t DcameraSinkHidumper::GetLocalCameraNumber(std::string& result)
{
    DHLOGI("GetLocalCameraNumber Dump.");
    result.append("CameraNumber: ")
          .append(std::to_string(camDumpInfo_.camNumber));
    return DCAMERA_OK;
}

int32_t DcameraSinkHidumper::GetOpenedCameraInfo(std::string& result)
{
    DHLOGI("GetOpenedCameraInfo Dump.");
    result.append("OpenedCamera:\n");
    std::vector<std::string> camIds = camDumpInfo_.camIds;
    for (size_t i = 0; i < camIds.size(); i++) {
        result.append(camIds[i]);
        result.append("\n");
    }
    return DCAMERA_OK;
}

int32_t DcameraSinkHidumper::GetVersionInfo(std::string& result)
{
    DHLOGI("GetVersionInfo Dump.");
    result.append("CameraVersion: ")
          .append(camDumpInfo_.version);
    return DCAMERA_OK;
}

void DcameraSinkHidumper::ShowHelp(std::string& result)
{
    DHLOGI("ShowHelp Dump.");
    result.append("Usage:dump  <command> [options]\n")
        .append("Description:\n")
        .append("-h           ")
        .append(": show help\n")
        .append("--version    ")
        .append(": dump camera version in the system\n")
        .append("--camNum     ")
        .append(": dump local camera numbers in the system\n")
        .append("--opened     ")
        .append(": dump the opened camera in the system\n")
        .append("--startdump  ")
        .append(": dump camera data in /data/data/dcamera\n")
        .append("--stopdump   ")
        .append(": stop dump camera data\n");
}

int32_t DcameraSinkHidumper::ShowIllegalInfomation(std::string& result)
{
    DHLOGI("ShowIllegalInfomation Dump.");
    result.append("unknown command, -h for help.");
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS