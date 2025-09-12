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

#include "softbusonsinksessionopened_fuzzer.h"
#include "dcamera_softbus_adapter.h"
#include "fuzzer/FuzzedDataProvider.h"
#include <string>
#include <vector>
#include <array>

namespace OHOS {
namespace DistributedHardware {
const TransDataType VALID_DATA_TYPES[] = {
    DATA_TYPE_MESSAGE,  /**< Message */
    DATA_TYPE_BYTES,        /**< Bytes */
    DATA_TYPE_FILE,         /**< File */
    DATA_TYPE_RAW_STREAM,   /**< Raw data stream */
    DATA_TYPE_VIDEO_STREAM, /**< Video data stream*/
    DATA_TYPE_AUDIO_STREAM, /**< Audio data stream*/
    DATA_TYPE_SLICE_STREAM, /**< Video slice stream*/
    DATA_TYPE_BUTT,
};

const ShutdownReason VALID_SHUTDOWN_REASONS[] = {
    SHUTDOWN_REASON_UNKNOWN,       /**< Shutdown for unknown reason */
    SHUTDOWN_REASON_LOCAL,         /**< Shutdown by local process */
    SHUTDOWN_REASON_PEER,          /**< Shutdown by peer process */
    SHUTDOWN_REASON_LNN_CHANGED,   /**< Shutdown for LNN changed */
    SHUTDOWN_REASON_CONN_CHANGED,  /**< Shutdown for CONN Changed */
    SHUTDOWN_REASON_TIMEOUT,       /**< Shutdown for timeout */
    SHUTDOWN_REASON_SEND_FILE_ERR, /**< Shutdown for sending file error */
    SHUTDOWN_REASON_RECV_FILE_ERR, /**< Shutdown for receiving file error */
    SHUTDOWN_REASON_RECV_DATA_ERR, /**< Shutdown for receiving data error */
    SHUTDOWN_REASON_UNEXPECTED,    /**< Shutdown for unexpected reason */
    SHUTDOWN_REASON_SERVICE_DIED,  /**< Shutdown for death service */
    SHUTDOWN_REASON_LNN_OFFLINE,   /**< Shutdown for offline */
    SHUTDOWN_REASON_LINK_DOWN,     /**< Shutdown for link down */
};

void SoftbusOnSinkSessionOpenedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);
    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    const int maxLen = 32;
    std::string peerSessionNameStr = fdp.ConsumeRandomLengthString(maxLen);
    std::vector<char> nameBuf(peerSessionNameStr.begin(), peerSessionNameStr.end());
    nameBuf.push_back('\0');

    std::string peerDevIdStr = fdp.ConsumeRandomLengthString(maxLen);
    std::vector<char> networkId_buf(peerDevIdStr.begin(), peerDevIdStr.end());
    networkId_buf.push_back('\0');

    std::string pkgNameStr = fdp.ConsumeRandomLengthString(maxLen);
    std::vector<char> pkgNameBuf(pkgNameStr.begin(), pkgNameStr.end());
    pkgNameBuf.push_back('\0');

    TransDataType dataType = fdp.PickValueInArray(VALID_DATA_TYPES);

    PeerSocketInfo socketInfo = {
        .name = nameBuf.data(),
        .networkId = networkId_buf.data(),
        .pkgName = pkgNameBuf.data(),
        .dataType = dataType,
    };

    DCameraSoftbusAdapter::GetInstance().SinkOnBind(sessionId, socketInfo);

    ShutdownReason shutdownReason = fdp.PickValueInArray(VALID_SHUTDOWN_REASONS);
    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, shutdownReason);
    
    size_t remainingSize = fdp.remaining_bytes();
    std::vector<uint8_t> remainingBytes = fdp.ConsumeBytes<uint8_t>(remainingSize);

    if (!remainingBytes.empty()) {
        uint32_t remainingLen = static_cast<uint32_t>(remainingBytes.size());
        if (fdp.ConsumeBool()) {
            DCameraSoftbusAdapter::GetInstance().SinkOnBytes(sessionId, remainingBytes.data(), remainingLen);
        } else {
            DCameraSoftbusAdapter::GetInstance().SinkOnMessage(sessionId, remainingBytes.data(), remainingLen);
        }
    }
}
}
}

/* Fuzzer entry point */
extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    /* Run your code on data */
    OHOS::DistributedHardware::SoftbusOnSinkSessionOpenedFuzzTest(data, size);
    return 0;
}