/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "dcamera_client_demo.h"

using namespace OHOS;
using namespace OHOS::Camera;
using namespace OHOS::CameraStandard;
using namespace OHOS::DistributedHardware;

namespace OHOS {
namespace DistributedHardware {
static const std::map<camera_focus_state_t, FocusCallback::FocusState> METADATA_FOCUS_STATE_MAP = {
    { OHOS_CAMERA_FOCUS_STATE_SCAN, FocusCallback::SCAN },
    { OHOS_CAMERA_FOCUS_STATE_FOCUSED, FocusCallback::FOCUSED },
    { OHOS_CAMERA_FOCUS_STATE_UNFOCUSED, FocusCallback::UNFOCUSED }
};

void DCameraDemoStateCallback::OnMetadataResult(std::vector<std::shared_ptr<DCameraSettings>>& settings)
{
    DHLOGI("DCameraDemoStateCallback::OnMetadataResult");
    for (auto dcSetting : settings) {
        DCSettingsType dcSettingType = dcSetting->type_;
        std::string dcSettingValue = dcSetting->value_;
        DHLOGI("DCameraDemoStateCallback::OnMetadataResult dcSetting type: %d", dcSettingType);

        std::string metadataStr = Base64Decode(dcSettingValue);
        std::shared_ptr<CameraMetadata> cameraMetadata = MetadataUtils::DecodeFromString(metadataStr);
        camera_metadata_item_t item;
        int32_t ret = FindCameraMetadataItem(cameraMetadata->get(), OHOS_CONTROL_FOCUS_STATE, &item);
        if (ret != CAM_META_SUCCESS) {
            DHLOGE("DCameraDemoStateCallback::OnMetadataResult camera find metadata item failed, ret: %d", ret);
            return;
        }

        camera_focus_state_t focusState = static_cast<camera_focus_state_t>(item.data.u8[0]);
        auto iter = METADATA_FOCUS_STATE_MAP.find(focusState);
        if (iter == METADATA_FOCUS_STATE_MAP.end()) {
            DHLOGE("DCameraDemoStateCallback::OnMetadataResult metadata focus state map find focus state failed");
            return;
        }
        DHLOGI("DCameraDemoStateCallback::OnMetadataResult focusState: %d", iter->second);
    }
}

void DCameraDemoPhotoResultCallback::OnPhotoResult(std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGI("DCameraDemoPhotoResultCallback::OnPhotoResult");
    std::cout << "saving photo ..." << std::endl;
    char path[1024] = {0};
    int32_t ret = sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/log/dcamera_photo_%lld.jpg",
        GetNowTimeStampMs());
    if (ret < 0) {
        DHLOGE("DCameraDemoPhotoResultCallback::OnPhotoResult create photo file failed, ret: %d", ret);
        return;
    }

    DHLOGI("DCameraDemoPhotoResultCallback::OnPhotoResult saving photo to file %s", path);
    int32_t fd = open(path, O_RDWR | O_CREAT, FILE_PERMISSIONS_FLAG);
    if (fd == -1) {
        DHLOGE("DCameraDemoPhotoResultCallback::OnPhotoResult open file failed, error: %s", strerror(errno));
        return;
    }

    ret = write(fd, buffer->Data(), buffer->Capacity());
    if (ret == -1) {
        DHLOGE("DCameraDemoPhotoResultCallback::OnPhotoResult write file failed, error: %s", strerror(errno));
    }

    std::cout << "saving photo success" << std::endl;
    close(fd);
    return;
}

void DCameraDemoPreviewResultCallback::OnVideoResult(std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGI("DCameraDemoPreviewResultCallback::OnVideoResult");
    std::cout << "saving preview ..." << std::endl;
    char path[1024] = {0};
    int32_t ret = sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/log/dcamera_preview_%lld.yuv",
        GetNowTimeStampMs());
    if (ret < 0) {
        DHLOGE("DCameraDemoPreviewResultCallback::OnVideoResult create preview file failed, ret: %d", ret);
        return;
    }

    DHLOGI("DCameraDemoPreviewResultCallback::OnVideoResult saving preview to file %s", path);
    int32_t fd = open(path, O_RDWR | O_CREAT, FILE_PERMISSIONS_FLAG);
    if (fd == -1) {
        DHLOGE("DCameraDemoPreviewResultCallback::OnVideoResult open file failed, error: %s", strerror(errno));
        return;
    }

    ret = write(fd, buffer->Data(), buffer->Capacity());
    if (ret == -1) {
        DHLOGE("DCameraDemoPreviewResultCallback::OnVideoResult write file failed, error: %s", strerror(errno));
    }

    std::cout << "saving preview success" << std::endl;
    close(fd);
    return;
}

void DCameraDemoVideoResultCallback::OnVideoResult(std::shared_ptr<DataBuffer>& buffer)
{
    DHLOGI("DCameraDemoVideoResultCallback::OnVideoResult");
    std::cout << "saving video ..." << std::endl;
    char path[1024] = {0};
    int32_t ret = sprintf_s(path, sizeof(path) / sizeof(path[0]), "/data/log/dcamera_video_%lld.yuv",
        GetNowTimeStampMs());
    if (ret < 0) {
        DHLOGE("DCameraDemoVideoResultCallback::OnVideoResult create video file failed, ret: %d", ret);
        return;
    }

    DHLOGI("DCameraDemoVideoResultCallback::OnVideoResult saving video to file %s", path);
    int32_t fd = open(path, O_RDWR | O_CREAT, FILE_PERMISSIONS_FLAG);
    if (fd == -1) {
        DHLOGE("DCameraDemoVideoResultCallback::OnVideoResult open file failed, error: %s", strerror(errno));
        return;
    }

    ret = write(fd, buffer->Data(), buffer->Capacity());
    if (ret == -1) {
        DHLOGE("DCameraDemoVideoResultCallback::OnVideoResult write file failed, error: %s", strerror(errno));
    }

    std::cout << "saving video success" << std::endl;
    close(fd);
    return;
}
} // namespace DistributedHardware
} // namespace OHOS