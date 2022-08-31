/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_SINK_SERVICE_IPC_H
#define OHOS_DCAMERA_SINK_SERVICE_IPC_H

#include "idistributed_camera_source.h"

#include <map>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSinkServiceIpc {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSinkServiceIpc);

public:
    void Init();
    void UnInit();
    sptr<IDistributedCameraSource> GetSourceRemoteCamSrv(const std::string& deviceId);
    void OnSourceRemoteCamSrvDied(const wptr<IRemoteObject>& remote);
    void DeleteSourceRemoteCamSrv(const std::string& deviceId);

private:
    DCameraSinkServiceIpc();
    ~DCameraSinkServiceIpc();
    void ClearSourceRemoteCamSrv();

    class SourceRemoteRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    };
    sptr<SourceRemoteRecipient> sourceRemoteRecipient_;
    std::map<std::string, sptr<IDistributedCameraSource>> remoteSources_;
    std::mutex sourceRemoteCamSrvLock_;

    bool isInit_;
    std::mutex initCamSrvLock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SINK_SERVICE_IPC_H
