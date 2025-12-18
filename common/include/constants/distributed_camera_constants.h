/*
 * Copyright (c) 2021-2025 Huawei Device Co., Ltd.
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

#ifndef OHOS_DISTRIBUTED_CAMERA_CONSTANTS_H
#define OHOS_DISTRIBUTED_CAMERA_CONSTANTS_H

#include <cstdint>
#include <string>
#include <unistd.h>

namespace OHOS {
namespace DistributedHardware {
typedef enum {
    DCAMERA_SRV_STATE_NOT_START,
    DCAMERA_SRV_STATE_RUNNING
} DCameraServiceState;

typedef enum {
    DCAMERA_AUTHORIZATION_DEFAULT = 0,
    DCAMERA_AUTHORIZATION_AGREE = 1,
    DCAMERA_AUTHORIZATION_REJECT = 2,
    DCAMERA_AUTHORIZATION_TIMEOUT = 3,
} AuthorizationState;

typedef enum {
    DCAMERA_SAME_ACCOUNT = 0,
    DCAMERA_DIFF_ACCOUNT = 1,
    DCAMERA_NO_ACCOUNT = 2,
} AccessControlType;

typedef enum {
    DCAMERA_MESSAGE = 0,
    DCAMERA_OPERATION = 1,
    DCAMERA_SINK_STOP = 2,
    DCAMERE_GETFULLCAP = 3,
} DCameraEventType;

typedef enum {
    DCAMERA_EVENT_CHANNEL_DISCONNECTED = 0,
    DCAMERA_EVENT_CHANNEL_CONNECTED = 1,
    DCAMERA_EVENT_CAMERA_SUCCESS = 2,
    DCAMERA_EVENT_SINK_STOP = 3,

    DCAMERA_EVENT_CAMERA_ERROR = -1,
    DCAMERA_EVENT_OPEN_CHANNEL_ERROR = -2,
    DCAMERA_EVENT_CLOSE_CHANNEL_ERROR = -3,
    DCAMERA_EVENT_CONFIG_STREAMS_ERROR = -4,
    DCAMERA_EVENT_RELEASE_STREAMS_ERROR = -5,
    DCAMERA_EVENT_START_CAPTURE_ERROR = -6,
    DCAMERA_EVENT_STOP_CAPTURE_ERROR = -7,
    DCAMERA_EVENT_UPDATE_SETTINGS_ERROR = -8,
    DCAMERA_EVENT_DEVICE_ERROR = -9,
    DCAMERA_EVENT_DEVICE_PREEMPT = -10,
    DCAMERA_EVENT_DEVICE_IN_USE = -11,
    DCAMERA_EVENT_NO_PERMISSION = -12,
} DCameraEventResult;

typedef enum {
    OHOS_CAMERA_FORMAT_INVALID = 0,
    OHOS_CAMERA_FORMAT_RGBA_8888,
    OHOS_CAMERA_FORMAT_YCBCR_420_888,
    OHOS_CAMERA_FORMAT_YCRCB_420_SP,
    OHOS_CAMERA_FORMAT_YCBCR_420_SP,
    OHOS_CAMERA_FORMAT_JPEG,
    OHOS_CAMERA_FORMAT_YCBCB_P010,
} DCameraFormat;

const uint32_t DCAMERA_MAX_NUM = 1;
const uint32_t DCAMERA_PRODUCER_ONE_MINUTE_MS = 1000;
const uint32_t DCAMERA_PRODUCER_FPS_DEFAULT = 30;
const uint32_t DCAMERA_MAX_RECV_DATA_LEN = 104857600;
const uint16_t DCAMERA_MAX_RECV_EXT_LEN = 65535;
const uint32_t DISTRIBUTED_HARDWARE_CAMERA_SOURCE_SA_ID = 4803;
const uint32_t DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID = 4804;
const std::string SESSION_HEAD = "ohos.dhardware.dcamera_";
const std::string DEVICE_ID_0 = "device/0";
const std::string DEVICE_ID_1 = "device/1";
const std::string SENDER_SESSION_NAME_CONTROL = "_control_sender";
const std::string SENDER_SESSION_NAME_DATA_SNAPSHOT = "_dataSnapshot_sender";
const std::string SENDER_SESSION_NAME_DATA_CONTINUE = "_dataContinue_sender";
const std::string RECEIVER_SESSION_NAME_CONTROL = "_control_receiver";
const std::string RECEIVER_SESSION_NAME_DATA_SNAPSHOT = "_dataSnapshot_receiver";
const std::string RECEIVER_SESSION_NAME_DATA_CONTINUE = "_dataContinue_receiver";
const std::string DCAMERA_PKG_NAME = "ohos.dhardware.dcamera";
const std::string SNAP_SHOT_SESSION_FLAG = "dataSnapshot";
const std::string CONTINUE_SESSION_FLAG = "dataContinue";
const std::string TIME_STAMP_US = "timeStampUs";
const std::string FRAME_TYPE = "frameType";
const std::string INDEX = "index";
const std::string START_ENCODE_TIME_US = "startEncodeT";
const std::string FINISH_ENCODE_TIME_US = "finishEncodeT";
const std::string SEND_TIME_US = "sendT";
const std::string RECV_TIME_US = "recvT";

const std::string DISTRIBUTED_HARDWARE_ID_KEY = "dhID";
const std::string CAMERA_ID_PREFIX = "Camera_";
const std::string CAMERA_PROTOCOL_VERSION_KEY = "ProtocolVer";
const std::string CAMERA_PROTOCOL_VERSION_VALUE = "1.0";
const std::string CAMERA_POSITION_KEY = "Position";
const std::string CAMERA_POSITION_BACK = "BACK";
const std::string CAMERA_POSITION_FRONT = "FRONT";
const std::string CAMERA_POSITION_UNSPECIFIED = "UNSPECIFIED";
const std::string CAMERA_METADATA_KEY = "MetaData";
const std::string CAMERA_CODEC_TYPE_KEY = "CodecType";
const std::string CAMERA_FORMAT_KEY = "OutputFormat";
const std::string CAMERA_FORMAT_PREVIEW = "Preview";
const std::string CAMERA_FORMAT_VIDEO = "Video";
const std::string CAMERA_FORMAT_PHOTO = "Photo";
const std::string CAMERA_RESOLUTION_KEY = "Resolution";
const std::string CAMERA_FPS_KEY = "Fps";
const std::string CAMERA_SURFACE_FORMAT = "CAMERA_SURFACE_FORMAT";
const std::string HDF_DCAMERA_EXT_SERVICE = "distributed_camera_provider_service";
const std::string CAMERA_SUPPORT_MODE = "Mode";
constexpr static int8_t FRAME_HEAD = 0;

const int32_t VALID_OS_TYPE = 10;
const int32_t INVALID_OS_TYPE = -1;
const std::string KEY_OS_TYPE = "OS_TYPE";

const int32_t RESOLUTION_MAX_WIDTH_SNAPSHOT = 4096;
const int32_t RESOLUTION_MAX_HEIGHT_SNAPSHOT = 3072;
const int32_t RESOLUTION_MAX_WIDTH_CONTINUOUS = 1920;
const int32_t RESOLUTION_MAX_HEIGHT_CONTINUOUS = 1080;
const int32_t RESOLUTION_MIN_WIDTH = 320;
const int32_t RESOLUTION_MIN_HEIGHT = 240;
const int32_t DUMP_FILE_MAX_SIZE = 295 * 1024 *1024;
const int32_t HICOLLIE_INTERVAL_TIME_MS = 20 * 1000;
const int32_t HICOLLIE_DELAY_TIME_MS = 5 * 1000;
const size_t HICOLLIE_SLEEP_TIME_US = 5 * 1000 * 1000;

const int32_t DCAMERA_QOS_TYPE_MIN_BW = 40 * 1024 * 1024;
const int32_t DCAMERA_QOS_TYPE_MAX_LATENCY = 8000;
const int32_t DCAMERA_QOS_TYPE_MIN_LATENCY = 2000;

const int32_t CAMERA_META_DATA_ITEM_CAPACITY = 10;
const int32_t CAMERA_META_DATA_DATA_CAPACITY = 100;
const int32_t DCAMERA_UID = 3052;
const int32_t CAMERA_SERVICE_ID = 3008;

const uint32_t DCAMERA_SHIFT_32 = 32;
const uint32_t DCAMERA_SHIFT_24 = 24;
const uint32_t DCAMERA_SHIFT_16 = 16;
const uint32_t DCAMERA_SHIFT_8 = 8;
const uint32_t COUNT_INIT_NUM = 1;

const uint32_t UINT32_SHIFT_MASK_24 = 0xff000000;
const uint32_t UINT32_SHIFT_MASK_16 = 0x00ff0000;
const uint32_t UINT32_SHIFT_MASK_8 = 0x0000ff00;
const uint32_t UINT32_SHIFT_MASK_0 = 0x000000ff;
const uint16_t UINT16_SHIFT_MASK_0 = 0x00ff;

const std::string CHANNEL_DISCONNECTED = "disconnected";
const std::string PRODUCER = "producer";
const std::string REGISTER_SERVICE_NOTIFY = "regSvcNotify";
const std::string SINK_START_EVENT = "sinkStartEvent";
const std::string SOURCE_START_EVENT = "srcStartEvent";
const std::string DECODE_DATA_EVENT = "srcDecEvent";
const std::string PIPELINE_SRC_EVENT = "srcPipeEvent";
const std::string UNREGISTER_SERVICE_NOTIFY = "unregSvcNotify";
const std::string LOOPER_SMOOTH = "looperSmooth";
const std::string DUMP_PATH = "/data/data/dcamera";
const std::string DUMP_PHOTO_PATH = "/data/data/dcamera/photodump";
const std::string TO_DISPLAY = "AfterDecodeToDisplay.yuv";
const std::string SINK_PHOTO = "_SinkPhoto.jpg";
const std::string AFTER_ENCODE = "SinkAfterEncode.h265";
const std::string BEFORE_DECODE = "SourceBeforeDecode.h265";
const std::string CAMERA_HICOLLIE = "CameraHicollie";
const std::string SEPARATE_SINK_VERSION = "2.0";
const std::string START_CAPTURE_SUCC = "operator start capture success";
const std::string CAMERA_SERVICE_DIED = "camera service died";
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_CONSTANTS_H