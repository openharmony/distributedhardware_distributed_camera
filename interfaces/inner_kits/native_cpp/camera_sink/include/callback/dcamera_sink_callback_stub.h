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

#ifndef OHOS_DCAMERA_SINK_CALLBACK_STUB_H
#define OHOS_DCAMERA_SINK_CALLBACK_STUB_H

#include <map>

#include "iremote_stub.h"
#include "idcamera_sink_callback.h"
#include "refbase.h"
namespace OHOS { class MessageOption; }
namespace OHOS { class MessageParcel; }

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkCallbackStub : public IRemoteStub<IDCameraSinkCallback> {
public:
    DCameraSinkCallbackStub();
    virtual ~DCameraSinkCallbackStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t OnNotifyResourceInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t OnHardwareStateChangedInner(MessageParcel &data, MessageParcel &reply);

    using DCameraFunc = int32_t (DCameraSinkCallbackStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, DCameraFunc> memberFuncMap_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_CALLBACK_STUB_H
