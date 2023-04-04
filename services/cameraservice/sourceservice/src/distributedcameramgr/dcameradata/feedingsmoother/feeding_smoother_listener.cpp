/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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
#include "feeding_smoother_listener.h"
#include "distributed_hardware_log.h"
#include "smoother_constants.h"

namespace OHOS {
namespace DistributedHardware {
int32_t FeedingSmootherListener::OnSmoothFinished(const std::shared_ptr<IFeedableData>& data)
{
    std::shared_ptr<IFeedableDataProducer> producer = producer_.lock();
    if (producer == nullptr) {
        DHLOGE("Producer is nullptr, notify failed.");
        return NOTIFY_FAILED;
    }
    producer->OnSmoothFinished(data);
    return NOTIFY_SUCCESS;
}
} // namespace DistributedHardware
} // namespace OHOS