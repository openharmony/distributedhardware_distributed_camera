/*
* Copyright (c) 2024 Huawei Device Co., Ltd.
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


#ifndef DCAMERA_COLLABORATION_MANAGER_CAPI_H
#define DCAMERA_COLLABORATION_MANAGER_CAPI_H

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum DCameraCollaborationHardwareType {
    SCM_UNKNOWN_TYPE = 0,
    SCM_DISPLAY = 1,
    SCM_MIC = 2,
    SCM_SPEAKER = 3,
    SCM_CAMERA = 4,
} DCameraCollaborationHardwareType;

typedef enum DCameraCollaborationBussinessStatus {
    SCM_IDLE = 1,
    SCM_PREPARE = 2,
    SCM_CONNECTING = 3,
    SCM_CONNECTED = 4
} DCameraCollaborationBussinessStatus;

typedef enum DCameraCollaborationResultCode {
    PASS = 1004720001,
    REJECT = 1004720002
} DCameraCollaborationResultCode;

typedef struct DCameraCollaboration_HardwareRequestInfo {
    DCameraCollaborationHardwareType hardWareType;
    bool canShare;
} DCameraCollaborationHardwareRequestInfo;

typedef struct DCameraCollaborationCommunicationRequestInfo {
    int32_t minBandwidth;
    int32_t maxLatency;
    int32_t minLatency;
    int32_t maxWaitTime;
    const char *dataType;
} DCameraCollaborationCommunicationRequestInfo;

typedef struct DCameraCollaborationResourceRequestInfoSets {
    uint32_t remoteHardwareListSize;
    DCameraCollaboration_HardwareRequestInfo *remoteHardwareList;
    uint32_t localHardwareListSize;
    DCameraCollaboration_HardwareRequestInfo *localHardwareList;
    DCameraCollaborationCommunicationRequestInfo *communicationRequest;
} DCameraCollaborationResourceRequestInfoSets;

typedef struct DCameraCollaborationCallback {
    int32_t (*onStop)(const char *peerNetworkId);
    int32_t (*applyResult)(int32_t errorcode, int32_t result, const char *reason);
} DCameraCollaborationCallback;

typedef struct DCameraCollaborationApi {
    int32_t (*dCameraCollaborationPublishServiceState)(const char *peerNetworkId, const char *serviceName,
        const char *extraInfo, DCameraCollaborationBussinessStatus state);
    int32_t (*dCameraCollaborationApplyAdvancedResource)(const char *peerNetworkId, const char *serviceName,
        DCameraCollaborationResourceRequestInfoSets *resourceRequest,
        DCameraCollaborationCallback *callback);
    int32_t (*dCameraCollaborationRegisterLifecycleCallback)(const char *serviceName,
        DCameraCollaborationCallback *callback);
    int32_t (*dCameraCollaborationUnRegisterLifecycleCallback)(const char *serviceName);
} DCameraCollaborationApi;

int32_t DCameraCollaborationExport(DCameraCollaborationApi *exportapi);

#ifdef __cplusplus
}
#endif

#endif // DCAMERA_COLLABORATION_MANAGER_CAPI_H