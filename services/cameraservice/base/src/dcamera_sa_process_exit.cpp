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

#include "dcamera_sa_process_exit.h"

#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DCameraSAProcessExit);
DCameraSAProcessExit::~DCameraSAProcessExit()
{
    DHLOGI("DCameraSAProcessExit Delete");
}

void DCameraSAProcessExit::setSinkProcessState()
{
    DHLOGI("release sink success.");
    sourceSAProcessState_ = DCAMERA_SA_EXIT_STATE_STOP;
}

void DCameraSAProcessExit::setSourceProcessState()
{
    DHLOGI("release source success.");
    sinkSAProcessState_ = DCAMERA_SA_EXIT_STATE_STOP;
}

void DCameraSAProcessExit::checkSAProcessState()
{
    if (sourceSAProcessState_ == DCAMERA_SA_EXIT_STATE_START || sinkSAProcessState_ == DCAMERA_SA_EXIT_STATE_START) {
        return;
    }
    sourceSAProcessState_ = DCAMERA_SA_EXIT_STATE_START;
    sinkSAProcessState_ = DCAMERA_SA_EXIT_STATE_START;
    DHLOGI("exit sa process.");
    exit(0);
}
} // namespace DistributedHardware
} // namespace OHOS