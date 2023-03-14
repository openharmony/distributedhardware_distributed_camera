/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "softbus_bus_center.h"

constexpr int32_t DH_SUCCESS = 0;
int32_t GetLocalNodeDeviceInfo(const char *pkgName, NodeBasicInfo *info)
{
    NodeBasicInfo nodeBasicInfo_ = {
        .networkId = "networkidc08647073e02e7a78f09473aa122ff57fc81c00",
        .deviceName = "f6d4c08647073e02e7a78f09473aa122ff57fc81c00981fcf5be989e7d112591",
        .deviceTypeId = 1};
    (void)pkgName;
    *info = nodeBasicInfo_;
    return DH_SUCCESS;
}

int32_t StartTimeSync(const char *pkgName, const char *targetNetworkId, TimeSyncAccuracy accuracy,
    TimeSyncPeriod period, ITimeSyncCb *cb)
{
    (void)pkgName;
    (void)targetNetworkId;
    (void)accuracy;
    (void)period;
    (void)cb;
    return DH_SUCCESS;
}

int32_t StopTimeSync(const char *pkgName, const char *targetNetworkId)
{
    (void)pkgName;
    (void)targetNetworkId;
    return DH_SUCCESS;
}