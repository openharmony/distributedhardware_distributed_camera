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

#ifndef OHOS_DCAMERA_SOURCE_SERVICE_IPC_H
#define OHOS_DCAMERA_SOURCE_SERVICE_IPC_H

#include "idistributed_camera_sink.h"

#include <map>

#include "single_instance.h"

namespace OHOS {
namespace DistributedHardware {
class DCameraSourceServiceIpc {
DECLARE_SINGLE_INSTANCE_BASE(DCameraSourceServiceIpc);

public:
    void Init();
    void UnInit();
    sptr<IDistributedCameraSink> GetSinkRemoteCamSrv(const std::string& deviceId);
    void OnSinkRemoteCamSrvDied(const wptr<IRemoteObject>& remote);
    void DeleteSinkRemoteCamSrv(const std::string& deviceId);

private:
    DCameraSourceServiceIpc();
    ~DCameraSourceServiceIpc();
    void ClearSinkRemoteCamSrv();

    class SinkRemoteRecipient : public IRemoteObject::DeathRecipient {
    public:
        void OnRemoteDied(const wptr<IRemoteObject>& remote) override;
    };
    sptr<SinkRemoteRecipient> sinkRemoteRecipient_;
    std::map<std::string, sptr<IDistributedCameraSink>> remoteSinks_;
    std::mutex sinkRemoteCamSrvLock_;

    bool isInit_;
    std::mutex initCamSrvLock_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_SOURCE_SERVICE_IPC_H
