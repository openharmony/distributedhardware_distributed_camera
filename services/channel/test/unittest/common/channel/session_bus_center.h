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

#ifndef SOFTBUS_BUS_CENTER_H
#define SOFTBUS_BUS_CENTER_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif
constexpr uint32_t MOCK_NETWORK_ID_BUF_LEN = 65;
constexpr uint32_t MOCK_DEVICE_NAME_BUF_LEN = 65;
/**
 * @brief Defines the basic information about a device.
 *
 * @since 1.0
 * @version 1.0
 */
typedef struct {
    char networkId[MOCK_NETWORK_ID_BUF_LEN];    /**< Device ID */
    char deviceName[MOCK_DEVICE_NAME_BUF_LEN];  /**< Device name */
    uint16_t deviceTypeId;
} NodeBasicInfo;

typedef enum {
    LOW_ACCURACY = 0,
    NORMAL_ACCURACY,
    HIGH_ACCURACY,
    SUPER_HIGH_ACCURACY,
    UNAVAIL_ACCURACY = 0xFFFF,
} TimeSyncAccuracy;

typedef enum {
    SHORT_PERIOD = 0,
    NORMAL_PERIOD,
    LONG_PERIOD,
} TimeSyncPeriod;

typedef struct {
    int32_t millisecond;
    int32_t microsecond;
    TimeSyncAccuracy accuracy;
} TimeSyncResult;

typedef enum {
    NODE_SPECIFIC = 0,
    ALL_LNN,
    WRITE_RTC,
} TimeSyncFlag;
typedef struct {
    TimeSyncResult result;
    TimeSyncFlag flag;
    union {
        char targetNetworkId[MOCK_NETWORK_ID_BUF_LEN];
        char masterNetworkId[MOCK_NETWORK_ID_BUF_LEN];
    } target;
} TimeSyncResultInfo;

typedef struct {
    void (*onTimeSyncResult)(const TimeSyncResultInfo *info, int32_t retCode);
} ITimeSyncCb;

int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info);
#ifdef __cplusplus
}
#endif
#endif // SOFTBUS_BUS_CENTER_H