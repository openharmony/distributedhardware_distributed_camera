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

#ifndef OHOS_DISTRIBUTED_CAMERA_SOURCE_STUB_H
#define OHOS_DISTRIBUTED_CAMERA_SOURCE_STUB_H

#include <map>
#include "iremote_stub.h"

#include "idistributed_camera_source.h"

namespace OHOS {
namespace DistributedHardware {
class DistributedCameraSourceStub : public IRemoteStub<IDistributedCameraSource> {
public:
    DistributedCameraSourceStub();
    virtual ~DistributedCameraSourceStub();
    int32_t OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    int32_t InitSourceInner(MessageParcel &data, MessageParcel &reply);
    int32_t ReleaseSourceInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterDistributedHardwareInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateDCameraWorkModeInner(MessageParcel &data, MessageParcel &reply);
    int32_t DCameraNotifyInner(MessageParcel &data, MessageParcel &reply);
    bool CheckRegParams(const std::string& devId, const std::string& dhId,
        const std::string& reqId, const EnableParam& param);
    bool CheckUnregParams(const std::string& devId, const std::string& dhId, const std::string& reqId);
    bool CheckNotifyParams(const std::string& devId, const std::string& dhId, std::string& events);
    bool HasEnableDHPermission();
    bool CheckUpdateParams(const std::string& devId, const std::string& dhId, const WorkModeParam& param);

    using DCameraFunc = int32_t (DistributedCameraSourceStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, DCameraFunc> memberFuncMap_;
    const size_t PARAM_MAX_SIZE = 50 * 1024 * 1024;
    const size_t DID_MAX_SIZE = 256;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DISTRIBUTED_CAMERA_SOURCE_STUB_H
