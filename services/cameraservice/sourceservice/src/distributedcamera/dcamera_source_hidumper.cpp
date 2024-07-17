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

#include "dcamera_source_hidumper.h"

#include "dcamera_hidumper.h"
#include "distributed_camera_errno.h"
#include "distributed_camera_source_service.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DcameraSourceHidumper);

namespace {
const std::string ARGS_HELP = "-h";
const std::string ARGS_VERSION_INFO = "--version";
const std::string ARGS_REGISTERED_INFO = "--registered";
const std::string ARGS_CURRENTSTATE_INFO = "--curState";
const std::string ARGS_START_DUMP = "--startdump";
const std::string ARGS_STOP_DUMP = "--stopdump";
const std::string STATE_INT = "Init";
const std::string STATE_REGISTERED = "Registered";
const std::string STATE_OPENED = "Opened";
const std::string STATE_CONFIG_STREAM = "ConfigStream";
const std::string STATE_CAPTURE = "Capture";

const std::map<std::string, HidumpFlag> ARGS_MAP = {
    { ARGS_HELP, HidumpFlag::GET_HELP },
    { ARGS_REGISTERED_INFO, HidumpFlag::GET_REGISTERED_INFO },
    { ARGS_CURRENTSTATE_INFO, HidumpFlag::GET_CURRENTSTATE_INFO },
    { ARGS_VERSION_INFO, HidumpFlag::GET_VERSION_INFO },
    { ARGS_START_DUMP, HidumpFlag::START_DUMP },
    { ARGS_STOP_DUMP, HidumpFlag::STOP_DUMP },
};

const std::map<int32_t, std::string> STATE_MAP = {
    { DCAMERA_STATE_INIT_DUMP, STATE_INT },
    { DCAMERA_STATE_REGIST_DUMP, STATE_REGISTERED },
    { DCAMERA_STATE_OPENED_DUMP, STATE_OPENED },
    { DCAMERA_STATE_CONFIG_STREAM_DUMP, STATE_CONFIG_STREAM },
    { DCAMERA_STATE_CAPTURE_DUMP, STATE_CAPTURE },
};
}

void DcameraSourceHidumper::SetSourceDumpInfo(CameraDumpInfo& camDumpInfo_)
{
    DistributedCameraSourceService::GetDumpInfo(camDumpInfo_);
}

bool DcameraSourceHidumper::Dump(const std::vector<std::string>& args, std::string& result)
{
    result.clear();
    int32_t argsSize = static_cast<int32_t>(args.size());
    if (argsSize > DUMP_MAX_SIZE) {
        DHLOGE("DcameraSourceHidumper Dump args.size() is invalid");
        return false;
    }
    DHLOGI("DcameraSourceHidumper Dump args.size():%{public}d.", argsSize);
    if (args.empty()) {
        ShowHelp(result);
        return true;
    } else if (args.size() > 1) {
        ShowIllegalInfomation(result);
        return true;
    }

    for (int32_t i = 0; i < argsSize; i++) {
        DHLOGI("DcameraSourceHidumper Dump args[%{public}d]: %{public}s.", i, args.at(i).c_str());
    }

    if (ProcessDump(args[0], result) != DCAMERA_OK) {
        return false;
    }
    return true;
}

int32_t DcameraSourceHidumper::ProcessDump(const std::string& args, std::string& result)
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
    SetSourceDumpInfo(camDumpInfo_);
    int32_t ret = DCAMERA_BAD_VALUE;
    switch (hf) {
        case HidumpFlag::GET_REGISTERED_INFO: {
            ret = GetRegisteredInfo(result);
            break;
        }
        case HidumpFlag::GET_CURRENTSTATE_INFO: {
            ret = GetCurrentStateInfo(result);
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

int32_t DcameraSourceHidumper::GetRegisteredInfo(std::string& result)
{
    DHLOGI("GetRegisteredInfo Dump.");
    result.append("CameraNumber: ")
          .append(std::to_string(camDumpInfo_.regNumber));
    return DCAMERA_OK;
}

int32_t DcameraSourceHidumper::GetCurrentStateInfo(std::string& result)
{
    DHLOGI("GetCurrentStateInfo Dump.");
    std::map<std::string, int32_t> devState = camDumpInfo_.curState;
    result.append("CameraId\tState\n");
    for (auto it = devState.begin(); it != devState.end(); it++) {
        std::string deviceId("");
        int32_t camState = 0;
        deviceId = it->first;
        camState = it->second;
        DHLOGI("GetCurrentStateInfo camState is %{public}d.", camState);
        auto state = STATE_MAP.find(camState);
        std::string curState("");
        if (state != STATE_MAP.end()) {
            curState = state->second;
        }
        result.append(deviceId)
              .append("\t")
              .append(curState)
              .append("\n");
    }
    return DCAMERA_OK;
}

int32_t DcameraSourceHidumper::GetVersionInfo(std::string& result)
{
    DHLOGI("GetVersionInfo Dump.");
    result.append("CameraVersion: ")
          .append(camDumpInfo_.version);
    return DCAMERA_OK;
}

void DcameraSourceHidumper::ShowHelp(std::string& result)
{
    DHLOGI("ShowHelp Dump.");
    result.append("Usage:dump  <command> [options]\n")
        .append("Description:\n")
        .append("-h           ")
        .append(": show help\n")
        .append("--version    ")
        .append(": dump camera version in the system\n")
        .append("--registered ")
        .append(": dump number of registered cameras in the system\n")
        .append("--curState   ")
        .append(": dump current state of the camera in the system\n")
        .append("--startdump  ")
        .append(": dump camera data in /data/data/dcamera\n")
        .append("--stopdump   ")
        .append(": stop dump camera data\n");
}

int32_t DcameraSourceHidumper::ShowIllegalInfomation(std::string& result)
{
    DHLOGI("ShowIllegalInfomation Dump.");
    result.append("unknown command, -h for help.");
    return DCAMERA_OK;
}
} // namespace DistributedHardware
} // namespace OHOS