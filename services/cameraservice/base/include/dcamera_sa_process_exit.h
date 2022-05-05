/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef DCAMERA_SA_PROCESS_EXIT_H
#define DCAMERA_SA_PROCESS_EXIT_H

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSAExitState {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSAExitState);

public:
    void checkSAProcessState();
    void setSinkProcessState();
    void setSourceProcessState();
private:
    typedef enum {
        DCAMERA_SA_EXIT_STATE_START = 0,
        DCAMERA_SA_EXIT_STATE_STOP = 1
    } DCameraSAProcessState;
    DCameraSAExitState();
    ~DCameraSAExitState();
    DCameraSAProcessState sinkSAProcessState_ = DCAMERA_SA_EXIT_STATE_START;
    DCameraSAProcessState sourceSAProcessState_ = DCAMERA_SA_EXIT_STATE_START;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // DCAMERA_SA_EXIT_STATE_H
