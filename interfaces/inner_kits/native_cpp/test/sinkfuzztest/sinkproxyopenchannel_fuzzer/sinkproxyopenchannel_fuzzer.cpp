/*
 * Copyright (c) 2022-2025 Huawei Device Co., Ltd.
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

#include "sinkproxyopenchannel_fuzzer.h"

#include <cstddef>
#include <cstdint>

#include "distributed_camera_constants.h"
#include "distributed_camera_sink_proxy.h"
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "fuzzer/FuzzedDataProvider.h"

namespace OHOS {
namespace DistributedHardware {
void SinkProxyOpenChannelFuzzTest(const uint8_t* data, size_t size)
{
    FuzzedDataProvider fdp(data, size);

    std::string dhId = fdp.ConsumeRandomLengthString();
    std::string openInfo = fdp.ConsumeRemainingBytesAsString();

    sptr<ISystemAbilityManager> samgr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (samgr == nullptr) {
        return;
    }
    sptr<IRemoteObject> remoteObject = samgr->GetSystemAbility(DISTRIBUTED_HARDWARE_CAMERA_SINK_SA_ID);
    std::shared_ptr<DistributedCameraSinkProxy> dCSinkProxy =
        std::make_shared<DistributedCameraSinkProxy>(remoteObject);

    dCSinkProxy->OpenChannel(dhId, openInfo);
}
}
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    OHOS::DistributedHardware::SinkProxyOpenChannelFuzzTest(data, size);
    return 0;
}