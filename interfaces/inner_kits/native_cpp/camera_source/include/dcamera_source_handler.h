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

#ifndef OHOS_DCAMERA_SOURCE_HANDLER_H
#define OHOS_DCAMERA_SOURCE_HANDLER_H

#include <mutex>
#include <condition_variable>

#include "iremote_proxy.h"
#include "iremote_broker.h"
#include "refbase.h"

#include "dcamera_source_callback.h"
#include "idistributed_hardware_source.h"
#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceHandler : public IDistributedHardwareSource {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSourceHandler);
public:
    int32_t InitSource(const std::string& params) override;
    int32_t ReleaseSource() override;
    int32_t RegisterDistributedHardware(const std::string& devId, const std::string& dhId,
        const EnableParam& param, std::shared_ptr<RegisterCallback> callback) override;
    int32_t UnregisterDistributedHardware(const std::string& devId, const std::string& dhId,
        std::shared_ptr<UnregisterCallback> callback) override;
    int32_t ConfigDistributedHardware(const std::string& devId, const std::string& dhId, const std::string& key,
        const std::string& value) override;
    void RegisterDistributedHardwareStateListener(std::shared_ptr<DistributedHardwareStateListener> listener) override;
    void UnregisterDistributedHardwareStateListener() override;
    void RegisterDataSyncTriggerListener(std::shared_ptr<DataSyncTriggerListener> listener) override;
    void UnregisterDataSyncTriggerListener() override;
    int32_t LoadDistributedHDF(std::shared_ptr<HdfDeathCallback> callback) override;
    int32_t UnLoadDistributedHDF() override;
    void SetSAState();
    int32_t UpdateDistributedHardwareWorkMode(const std::string& devId, const std::string& dhId,
        const WorkModeParam& param) override;
private:
    typedef enum {
        DCAMERA_SA_STATE_STOP = 0,
        DCAMERA_SA_STATE_START = 1,
    } DCameraSAState;
    DCameraSourceHandler();
    ~DCameraSourceHandler();
    void FinishStartSA(const std::string &params);
    void FinishStartSAFailed(const int32_t systemAbilityId);
private:
    std::mutex optLock_;
    sptr<DCameraSourceCallback> callback_;
    std::condition_variable producerCon_;
    std::mutex producerMutex_;
    DCameraSAState state_ = DCAMERA_SA_STATE_STOP;
    friend class DCameraSourceLoadCallback;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
__attribute__((visibility("default"))) IDistributedHardwareSource *GetSourceHardwareHandler();
#ifdef __cplusplus
}
#endif // __cplusplus
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOURCE_HANDLER_H
