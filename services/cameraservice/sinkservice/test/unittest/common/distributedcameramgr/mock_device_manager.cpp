/*
 * Copyright (C) 2026 Huawei Device Co., Ltd.
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

#include "mock_device_manager.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::DistributedHardware;

int32_t DeviceManagerImpl::GetTrustedDeviceList(const std::string &pkgName, const std::string &extra,
    std::vector<DmDeviceInfo> &deviceList)
{
    if (IDeviceManager::deviceMgrMock == nullptr) {
        return 0;
    }
    return IDeviceManager::deviceMgrMock->GetTrustedDeviceList(pkgName, extra, deviceList);
}

int32_t DeviceManagerImpl::InitDeviceManager(const std::string &pkgName,
    std::shared_ptr<DmInitCallback> dmInitCallback)
{
    if (IDeviceManager::deviceMgrMock == nullptr) {
        return 0;
    }
    return IDeviceManager::deviceMgrMock->InitDeviceManager(pkgName, dmInitCallback);
}

int32_t DeviceManagerImpl::GetUdidByNetworkId(const std::string &pkgName, const std::string &nextworkId,
    std::string &udid)
{
    if (IDeviceManager::deviceMgrMock == nullptr) {
        return 0;
    }
    return IDeviceManager::deviceMgrMock->GetUdidByNetworkId(pkgName, nextworkId, udid);
}
