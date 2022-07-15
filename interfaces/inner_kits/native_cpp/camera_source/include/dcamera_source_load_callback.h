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
#ifndef OHOS_DCAMERA_SOURCE_LOAD_CALLBACK_H
#define OHOS_DCAMERA_SOURCE_LOAD_CALLBACK_H

#include <cstdint>
#include <string>

#include "refbase.h"
#include "system_ability_load_callback_stub.h"
namespace OHOS { class IRemoteObject; }

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceLoadCallback : public SystemAbilityLoadCallbackStub {
public:
    explicit DCameraSourceLoadCallback(const std::string& params);
    void OnLoadSystemAbilitySuccess(int32_t systemAbilityId, const sptr<IRemoteObject>& remoteObject) override;
    void OnLoadSystemAbilityFail(int32_t systemAbilityId) override;
private:
    std::string params_;
};
}
}
#endif