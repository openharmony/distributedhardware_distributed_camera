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

#ifndef OHOS_IDCAMERA_SINK_CALLBACK_H
#define OHOS_IDCAMERA_SINK_CALLBACK_H

#include "iremote_broker.h"
#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
class IDCameraSinkCallback : public OHOS::IRemoteBroker {
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"ohos.distributedhardware.dcamerasinkcallback");
    enum {
        NOTIFY_RESOURCEINFO = 0,
        NOTIFY_STATE_CHANGEINFO = 1,
    };

    virtual ~IDCameraSinkCallback() {}
    virtual int32_t OnNotifyResourceInfo(const ResourceEventType &type, const std::string &subtype,
        const std::string &networkId, bool &isSensitive, bool &isSameAccout) = 0;
    virtual int32_t OnHardwareStateChanged(const std::string &devId, const std::string &dhId, int32_t status) = 0;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_IDCAMERA_SINK_CALLBACK_H
