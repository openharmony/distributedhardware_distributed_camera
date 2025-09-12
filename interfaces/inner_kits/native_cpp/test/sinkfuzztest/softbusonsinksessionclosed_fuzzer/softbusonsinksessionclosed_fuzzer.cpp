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

#include "softbusonsinksessionclosed_fuzzer.h"
#include "dcamera_softbus_adapter.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
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

void SoftbusOnSinkSessionClosedFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    int32_t sessionId = fdp.ConsumeIntegral<int32_t>();
    ShutdownReason reason = fdp.PickValueInArray(VALID_SHUTDOWN_REASONS);

    DCameraSoftbusAdapter::GetInstance().SinkOnShutDown(sessionId, reason);
    DCameraSoftbusAdapter::GetInstance().SourceOnShutDown(sessionId, reason);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SoftbusOnSinkSessionClosedFuzzTest(data, size);
    return 0;
}