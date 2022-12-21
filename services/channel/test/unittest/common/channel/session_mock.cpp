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

#include "session.h"

constexpr int32_t DH_SUCCESS = 0;
constexpr int32_t DH_ERROR = -1;
constexpr int32_t MOCK_SESSION_ID = 1;
static ISessionListener listener_;
static char g_peerDeviceId_[CHAR_ARRAY_SIZE + 1];
static char g_peerSessionName_[CHAR_ARRAY_SIZE + 1];
static char g_mySessionName_[CHAR_ARRAY_SIZE + 1];
int CreateSessionServer(const char *pkgName, const char *sessionName, const ISessionListener *listener)
{
    (void)pkgName;
    std::cout << "CreateSessionServer start sessionName:" << sessionName << std::endl;
    std::string tmpstr = sessionName;
    std::string testSessionName = "sourcetest024";
    if (tmpstr == testSessionName) {
        return DH_ERROR;
    }
    if (tmpstr.size() <= 0) {
        std::cout << "CreateSessionServer sessionName is empty." << std::endl;
        return DH_ERROR;
    }
    if (listener == nullptr) {
        std::cout << "CreateSessionServer listener is null." << std::endl;
        return DH_ERROR;
    }
    strcpy_s(g_mySessionName_, tmpstr.size() + 1, tmpstr.c_str());
    listener_.OnBytesReceived = listener->OnBytesReceived;
    listener_.OnMessageReceived = listener->OnMessageReceived;
    listener_.OnSessionClosed = listener->OnSessionClosed;
    listener_.OnSessionOpened = listener->OnSessionOpened;
    listener_.OnStreamReceived = listener->OnStreamReceived;
    return DH_SUCCESS;
}

int RemoveSessionServer(const char *pkgName, const char *sessionName)
{
    (void)pkgName;
    (void)sessionName;
    return DH_SUCCESS;
}

int OpenSession(const char *mySessionName, const char *peerSessionName, const char *peerDeviceId, const char *groupId,
    const SessionAttribute *attr)
{
    (void)mySessionName;
    (void)groupId;
    (void)attr;
    if (strlen(peerSessionName) <= 0) {
        return DH_ERROR;
    }
    if (strlen(peerDeviceId) <= 0) {
        return DH_ERROR;
    }
    strncpy_s(g_peerSessionName_, CHAR_ARRAY_SIZE + 1, peerSessionName, CHAR_ARRAY_SIZE);
    strncpy_s(g_peerDeviceId_, DEVICE_ID_SIZE_MAX + 1, peerDeviceId, DEVICE_ID_SIZE_MAX);
    std::thread thd(OpenSessionResult);
    thd.detach();
    return MOCK_SESSION_ID;
}

void OpenSessionResult()
{
    listener_.OnSessionOpened(MOCK_SESSION_ID, DH_SUCCESS);
}

void CloseSession(int sessionId)
{
    (void)sessionId;
}

int SendBytes(int sessionId, const void *data, unsigned int len)
{
    (void)sessionId;
    (void)data;
    (void)len;
    return DH_SUCCESS;
}

int SendMessage(int sessionId, const void *data, unsigned int len)
{
    (void)sessionId;
    (void)data;
    (void)len;
    return DH_SUCCESS;
}

int SendStream(int sessionId, const StreamData *data, const StreamData *ext, const FrameInfo *param)
{
    (void)sessionId;
    (void)data;
    (void)ext;
    (void)param;
    return DH_SUCCESS;
}

int GetMySessionName(int sessionId, char *sessionName, unsigned int len)
{
    (void)sessionId;
    (void)len;
    strncpy_s(sessionName, CHAR_ARRAY_SIZE + 1, g_mySessionName_, CHAR_ARRAY_SIZE);
    return DH_SUCCESS;
}

int GetPeerSessionName(int sessionId, char *sessionName, unsigned int len)
{
    (void)sessionId;
    (void)len;
    std::string tmpstr = sessionName;
    std::string peerSessName = "sinktest0027";
    if (tmpstr == peerSessName) {
        return DH_ERROR;
    }
    strncpy_s(sessionName, CHAR_ARRAY_SIZE + 1, g_peerSessionName_, CHAR_ARRAY_SIZE);
    return DH_SUCCESS;
}

int GetPeerDeviceId(int sessionId, char *devId, unsigned int len)
{
    (void)len;
    int32_t mySessionId = 27;
    if (sessionId == mySessionId) {
        return DH_ERROR;
    }
    strncpy_s(devId, DEVICE_ID_SIZE_MAX + 1, g_peerDeviceId_, DEVICE_ID_SIZE_MAX);
    return DH_SUCCESS;
}

int GetSessionSide(int sessionId)
{
    (void)sessionId;
    return DH_SUCCESS;
}

int SetFileReceiveListener(const char *pkgName, const char *sessionName, const IFileReceiveListener *recvListener,
    const char *rootDir)
{
    (void)pkgName;
    (void)sessionName;
    (void)recvListener;
    (void)rootDir;
    return DH_SUCCESS;
}

int SetFileSendListener(const char *pkgName, const char *sessionName, const IFileSendListener *sendListener)
{
    (void)pkgName;
    (void)sessionName;
    (void)sendListener;
    return DH_SUCCESS;
}

int SendFile(int sessionId, const char *sFileList[], const char *dFileList[], uint32_t fileCnt)
{
    (void)sessionId;
    (void)sFileList;
    (void)dFileList;
    (void)fileCnt;
    return DH_SUCCESS;
}