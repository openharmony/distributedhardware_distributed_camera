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

#ifndef OHOS_FEEDING_SMOOTHER_LISTENER_H
#define OHOS_FEEDING_SMOOTHER_LISTENER_H
#include "ifeedable_data_producer.h"
#include "ifeedable_data.h"
#include <memory>

namespace OHOS {
namespace DistributedHardware {
class FeedingSmootherListener {
public:
    FeedingSmootherListener(std::weak_ptr<IFeedableDataProducer> producer)
        : producer_(producer) {}
    ~FeedingSmootherListener() = default;

public:
    int32_t OnSmoothFinished(const std::shared_ptr<IFeedableData>& data);
private:
    std::weak_ptr<IFeedableDataProducer> producer_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_FEEDING_SMOOTHER_LISTENER_H