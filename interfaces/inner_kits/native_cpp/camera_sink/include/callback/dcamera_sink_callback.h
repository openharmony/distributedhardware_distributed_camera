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

#ifndef OHOS_DCAMERA_SINK_CALLBACK_H
#define OHOS_DCAMERA_SINK_CALLBACK_H

#include <vector>
#include <mutex>

#include "dcamera_sink_callback_stub.h"
#include "idistributed_hardware_sink.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkCallback : public DCameraSinkCallbackStub {
public:
    DCameraSinkCallback() = default;
    ~DCameraSinkCallback() override;

    int32_t OnNotifyResourceInfo(const ResourceEventType &type, const std::string &subtype,
        const std::string &networkId, bool &isSensitive, bool &isSameAccout) override;
    int32_t OnHardwareStateChanged(const std::string &devId, const std::string &dhId, int32_t status) override;

    void PushPrivacyResCallback(const std::shared_ptr<PrivacyResourcesListener> &listener);
    void RegisterStateListener(const std::shared_ptr<DistributedHardwareSinkStateListener> listener);
    void UnRegisterStateListener();

private:
    std::mutex privacyResMutex_;
    std::vector<std::shared_ptr<PrivacyResourcesListener>> privacyResCallback_;
    std::shared_ptr<DistributedHardwareSinkStateListener> stateListener_ = nullptr;
    std::mutex stateListenerMtx_;
    const size_t DID_MAX_SIZE = 256;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_CALLBACK_H
