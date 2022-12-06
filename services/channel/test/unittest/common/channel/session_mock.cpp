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
#include "session.h"

constexpr int32_t DH_SUCCESS = 0;
constexpr int32_t DH_ERROR = -1;
constexpr int32_t MOCK_SESSION_ID = 1;
static ISessionListener listener_;
static char peerDeviceId_[CHAR_ARRAY_SIZE];
static char peerSessionName_[CHAR_ARRAY_SIZE];
static char mySessionName_[CHAR_ARRAY_SIZE];
int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    std::cout << "CreateSessionServer start sessionName:" << sessionName << std::endl;
    std::string tmpstr = sessionName;
    if (tmpstr.size() <= 0) {
        std::cout << "CreateSessionServer sessionName is empty." << std::endl;
        return DH_ERROR;
    }
    if (listener == nullptr) {
        std::cout << "CreateSessionServer listener is null." << std::endl;
        return DH_ERROR;
    }
    strcpy(mySessionName_, tmpstr.c_str());
    listener_.OnBytesReceived = listener->OnBytesReceived;
    listener_.OnMessageReceived = listener->OnMessageReceived;
    listener_.OnSessionClosed = listener->OnSessionClosed;
    listener_.OnSessionOpened = listener->OnSessionOpened;
    listener_.OnStreamReceived = listener->OnStreamReceived;
    return DH_SUCCESS;
}

int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    return DH_SUCCESS;
}

int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerDeviceId, const char *groupId,
    const SessionAttribute *attr)
{
    if (strlen(peerSessionName) <= 0) {
        return DH_ERROR;
    }
    if (strlen(peerDeviceId) <= 0) {
        return DH_ERROR;
    }
    strncpy(peerSessionName_, peerSessionName, CHAR_ARRAY_SIZE);
    strncpy(peerDeviceId_, peerDeviceId, DEVICE_ID_SIZE_MAX);
    std::thread thd(OpenSessionResult);
    thd.detach();
    return MOCK_SESSION_ID;
}

void OpenSessionResult()
{
    listener_.OnSessionOpened(MOCK_SESSION_ID, DH_SUCCESS);
}

void CloseSession(int sessionId) {}

int SendBytes(int sessionId, const void *data, unsigned int len)
{
    return DH_SUCCESS;
}

int SendMessage(int sessionId, const void *data, unsigned int len)
{
    return DH_SUCCESS;
}

int SendStream(int sessionId, const StreamData *data, const StreamData *ext, const FrameInfo *param)
{
    return DH_SUCCESS;
}

int GetMySessionName(int sessionId, char *sessionName, unsigned int len)
{
    strncpy(sessionName, mySessionName_, CHAR_ARRAY_SIZE);
    return DH_SUCCESS;
}

int GetPeerSessionName(int sessionId, char *sessionName, unsigned int len)
{
    strncpy(sessionName, peerSessionName_, CHAR_ARRAY_SIZE);
    return DH_SUCCESS;
}

int GetPeerDeviceId(int sessionId, char *devId, unsigned int len)
{
    strncpy(devId, peerDeviceId_, DEVICE_ID_SIZE_MAX);
    return DH_SUCCESS;
}

int GetSessionSide(int sessionId)
{
    return DH_SUCCESS;
}

int SetFileReceiveListener(const char *pkgName, const char *sessionName, const IFileReceiveListener *recvListener,
    const char *rootDir)
{
    return DH_SUCCESS;
}

int SetFileSendListener(const char *pkgName, const char *sessionName, const IFileSendListener *sendListener)
{
    return DH_SUCCESS;
}

int SendFile(int sessionId, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    return DH_SUCCESS;
}