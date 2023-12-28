/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <cstring>
#include <thread>
#include <securec.h>

#include "socket.h"

constexpr int32_t DH_SUCCESS = 0;
int Socket(SocketInfo info)
{
    (void)info;
    return DH_SUCCESS;
}

int Listen(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    (void)socket;
    (void)qos;
    (void)qosCount;
    (void)listener;
    return DH_SUCCESS;
}

int Bind(int32_t socket, const QosTV qos[], uint32_t qosCount, const ISocketListener *listener)
{
    (void)socket;
    (void)qos;
    (void)qosCount;
    (void)listener;
    return DH_SUCCESS;
}

void Shutdown(int32_t socket)
{
    (void)socket;
}

int SendBytes(int32_t socket, const void *data, uint32_t len)
{
    (void)socket;
    (void)data;
    (void)len;
    return DH_SUCCESS;
}

int SendMessage(int32_t socket, const void *data, uint32_t len)
{
    (void)socket;
    (void)data;
    (void)len;
    return DH_SUCCESS;
}

int SendStream(int32_t socket, const StreamData *data, const StreamData *ext, const StreamFrameInfo *param)
{
    (void)socket;
    (void)data;
    (void)ext;
    (void)param;
    return DH_SUCCESS;
}

int SendFile(int32_t socket, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    (void)socket;
    (void)sFileList;
    (void)dFileList;
    (void)fileCnt;
    return DH_SUCCESS;
}

int32_t EvaluateQos(const char *peerNetworkId, TransDataType dataType, const QosTV *qos, uint32_t qosCount)
{
    (void)peerNetworkId;
    (void)dataType;
    (void)qos;
    (void)qosCount;
    return DH_SUCCESS;
}