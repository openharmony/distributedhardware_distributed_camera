/*
 * Copyright (c) 2021-2024 Huawei Device Co., Ltd.
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

#ifndef OHOS_DCAMERA_HANDLER_H
#define OHOS_DCAMERA_HANDLER_H

#include "ihardware_handler.h"

#include <set>

#include "camera_info.h"
#include "camera_input.h"
#include "camera_manager.h"
#include "camera_output_capability.h"
#include "single_instance.h"
#include "v1_1/dcamera_types.h"
#include "cJSON.h"

namespace OHOS {
namespace DistributedHardware {
using namespace OHOS::HDI::DistributedCamera::V1_1;

class DCameraHandler : public IHardwareHandler {
DECLARE_SINGLE_INSTANCE_BASE(DCameraHandler);
public:
    int32_t Initialize() override;
    std::vector<DHItem> QueryMeta() override;
    std::vector<DHItem> Query() override;
    std::map<std::string, std::string> QueryExtraInfo() override;
    bool IsSupportPlugin() override;
    void RegisterPluginListener(std::shared_ptr<PluginListener> listener) override;
    void UnRegisterPluginListener() override;

    std::vector<std::string> GetCameras();

private:
    DCameraHandler() = default;
    ~DCameraHandler();

private:
    const int32_t INVALID_FORMAT = -1;
    int32_t CreateMetaDHItem(sptr<CameraStandard::CameraDevice>& info, DHItem& item);
    int32_t CreateDHItem(sptr<CameraStandard::CameraDevice>& info, DHItem& item);
    int32_t CreateAVCodecList(cJSON* root);
    std::string GetCameraPosition(CameraStandard::CameraPosition position);
    void ProcessProfile(const DCStreamType type, std::map<std::string, std::list<std::string>>& formatMap,
        std::map<std::string, std::list<std::string>>& fpsMap, std::vector<CameraStandard::Profile>& profileList,
        std::set<int32_t>& formatSet);
    void ConfigFormatphoto(const DCStreamType type, cJSON* root,
        std::vector<CameraStandard::Profile>& profileList);
    void ConfigFormatvideo(const DCStreamType type, cJSON* root,
        std::vector<CameraStandard::Profile>& profileList);
    bool IsValid(const DCStreamType type, const CameraStandard::Size& size);
    int32_t CovertToDcameraFormat(CameraStandard::CameraFormat format);
    int32_t CreateMeatdataStr(sptr<CameraStandard::CameraDevice>& info, cJSON *root);

    sptr<CameraStandard::CameraManager> cameraManager_;
    std::shared_ptr<PluginListener> pluginListener_;
};

#ifdef __cplusplus
extern "C" {
#endif // __cplusplus
__attribute__((visibility("default"))) IHardwareHandler* GetHardwareHandler();
#ifdef __cplusplus
}
#endif // __cplusplus
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_HANDLER_H