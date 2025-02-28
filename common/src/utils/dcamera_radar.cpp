/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include "dcamera_radar.h"

#include "hisysevent.h"
#include "securec.h"

#include "anonymous_string.h"
#include "dcamera_hisysevent_adapter.h"
#include "distributed_camera_errno.h"
#include "distributed_hardware_log.h"

namespace OHOS {
namespace DistributedHardware {
IMPLEMENT_SINGLE_INSTANCE(DcameraRadar);

bool DcameraRadar::ReportDcameraInit(const std::string& func, CameraInit bizStage, BizState bizState, int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_INIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC),
            BIZ_STATE, static_cast<int32_t>(bizState));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_INIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            BIZ_STATE, static_cast<int32_t>(bizState),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraInit error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraInitProgress(const std::string& func, CameraInit bizStage, int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_INIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_INIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraInitProgress error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraOpen(const std::string& func, CameraOpen bizStage, BizState bizState, int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_OPEN),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC),
            BIZ_STATE, static_cast<int32_t>(bizState));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_OPEN),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            BIZ_STATE, static_cast<int32_t>(bizState),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraOpen error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraOpenProgress(const std::string& func, CameraOpen bizStage, int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_OPEN),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_OPEN),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraOpenProgress error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraClose(const std::string& func, CameraClose bizStage, BizState bizState,
    int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_CLOSE),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC),
            BIZ_STATE, static_cast<int32_t>(bizState));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_CLOSE),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            BIZ_STATE, static_cast<int32_t>(bizState),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraClose error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraCloseProgress(const std::string& func, CameraClose bizStage, int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_CLOSE),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_CLOSE),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraCloseProgress error, res:%{public}d", res);
        return false;
    }
    return true;
}

bool DcameraRadar::ReportDcameraUnInit(const std::string& func, CameraUnInit bizStage, BizState bizState,
    int32_t errCode)
{
    int32_t res = DCAMERA_OK;
    StageRes stageRes = (errCode == DCAMERA_OK) ? StageRes::STAGE_SUCC : StageRes::STAGE_FAIL;
    if (stageRes == StageRes::STAGE_SUCC) {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_UNINIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_SUCC),
            BIZ_STATE, static_cast<int32_t>(bizState));
    } else {
        res = HiSysEventWrite(
            DISTRIBUTED_CAMERA,
            DISTRIBUTED_CAMERA_BEHAVIOR,
            HiviewDFX::HiSysEvent::EventType::BEHAVIOR,
            ORG_PKG, ORG_PKG_NAME,
            FUNC, func,
            BIZ_SCENE, static_cast<int32_t>(BizScene::CAMERA_UNINIT),
            BIZ_STAGE, static_cast<int32_t>(bizStage),
            STAGE_RES, static_cast<int32_t>(StageRes::STAGE_FAIL),
            BIZ_STATE, static_cast<int32_t>(bizState),
            ERROR_CODE, errCode);
    }
    if (res != DCAMERA_OK) {
        DHLOGE("ReportDcameraUnInit error, res:%{public}d", res);
        return false;
    }
    return true;
}
} // namespace DistributedHardware
} // namespace OHOS