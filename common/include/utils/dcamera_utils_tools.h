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

#ifndef OHOS_DCAMERA_UTILS_TOOL_H
#define OHOS_DCAMERA_UTILS_TOOL_H

#include <atomic>
#include <cstdint>
#include <string>
#include <fstream>
#include <map>
#include <memory>
#include <functional>
#include "single_instance.h"
#include "iaccess_listener.h"

#ifdef DCAMERA_MMAP_RESERVE
#include "image_converter.h"
#endif

namespace OHOS {
namespace DistributedHardware {
const std::string BASE_64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

int32_t GetLocalDeviceNetworkId(std::string& networkId);
int64_t GetNowTimeStampMs();
int64_t GetNowTimeStampUs();
int32_t GetAlignedHeight(int32_t width);
std::string Base64Encode(const unsigned char *toEncode, unsigned int len);
std::string Base64Decode(const std::string& basicString);
void DumpBufferToFile(const std::string& dumpPath, const std::string& fileName, uint8_t *buffer, size_t bufSize);
bool IsBase64(unsigned char c);
int32_t IsUnderDumpMaxSize(const std::string& dumpPath, const std::string& fileName);

#ifdef DCAMERA_MMAP_RESERVE
class ConverterHandle {
    DECLARE_SINGLE_INSTANCE(ConverterHandle);

public:
    void InitConverter();
    void DeInitConverter();
    const OHOS::OpenSourceLibyuv::ImageConverter &GetHandle();

    using DlHandle = void *;
private:
    std::atomic<bool> isInited_ = false;
    DlHandle dlHandler_ = nullptr;
    OHOS::OpenSourceLibyuv::ImageConverter converter_ = {0};
};
#endif

const std::string DUMP_SERVER_PARA = "sys.dcamera.dump.write.enable";
const std::string DUMP_SERVICE_DIR = "/data/local/tmp/";
const std::string DUMP_DCAMERA_AFTER_ENC_FILENAME = "dump_after_enc_dcamsink.h265";
const std::string DUMP_DCAMERA_BEFORE_DEC_FILENAME = "dump_before_dec_dcamsource.h265";
const std::string DUMP_DCAMERA_AFTER_DEC_FILENAME = "dump_after_dec_dcamsource.yuv";
const std::string DUMP_DCAMERA_AFTER_SCALE_FILENAME = "dump_after_scale_dcamsource.yuv";
const std::string DUMP_DCAMERA_AFTER_ROTATE_FILENAME = "dump_after_rotate_dcamsource.yuv";

class DumpFileUtil {
public:
    static void WriteDumpFile(FILE *dumpFile, void *buffer, size_t bufferSize);
    static void CloseDumpFile(FILE **dumpFile);
    static std::map<std::string, std::string> g_lastPara;
    static void OpenDumpFile(std::string para, std::string fileName, FILE **file);
private:
    static FILE *OpenDumpFileInner(std::string para, std::string fileName);
    static void ChangeDumpFileState(std::string para, FILE **dumpFile, std::string fileName);
};

class ManageSelectChannel {
DECLARE_SINGLE_INSTANCE(ManageSelectChannel);

public:
    void SetSrcConnect(bool isSoftbusConnect);
    void SetSinkConnect(bool isSoftbusConnect);
    bool GetSrcConnect();
    bool GetSinkConnect();
private:
    bool isSoftbusConnectSource_ = false;
    bool isSoftbusConnectSink_ = false;
};

class DCameraSystemSwitchItem {
public:
    DCameraSystemSwitchItem() : isSystemSwitch_(false), rotation_(0) {}
    DCameraSystemSwitchItem(bool isSystemSwitch, int32_t rotation)
        : isSystemSwitch_(isSystemSwitch), rotation_(rotation) {}

    bool IsSystemSwitch();
    int32_t GetRotate();
private:
    bool isSystemSwitch_ = false;
    int32_t rotation_ = 0;
};

class DCameraSystemSwitchInfo {
DECLARE_SINGLE_INSTANCE(DCameraSystemSwitchInfo);

public:
    int32_t SetSystemSwitchFlagAndRotation(const std::string& devId, bool isSystemSwitch,
        int32_t rotation);
    bool GetSystemSwitchFlag(std::string devId);
    int32_t GetSystemSwitchRotation(std::string devId);
private:
    std::map<std::string, DCameraSystemSwitchItem> map_ = {};
    std::mutex mtxLock_;
};

class DCameraAccessConfigManager {
DECLARE_SINGLE_INSTANCE(DCameraAccessConfigManager);

public:
    int32_t SetAccessConfig(const sptr<IAccessListener>& listener, int32_t timeOut,
        const std::string& pkgName);
    sptr<IAccessListener> GetAccessListener();
    int32_t GetAccessTimeOut();
    std::string GetAccessPkgName();
    void ClearAccessConfig();
    void SetAuthorizationGranted(const std::string& networkId, bool granted);
    bool IsAuthorizationGranted(const std::string& networkId);
    bool HasAuthorizationDecision(const std::string& networkId);
    void ClearAuthorizationResult(const std::string& networkId);
    void SetCurrentNetworkId(const std::string& networkId);
    std::string GetCurrentNetworkId();
    bool WaitForAuthorizationResult(const std::string& networkId, int32_t timeoutSeconds = 3);
    void ClearAccessConfigByPkgName(const std::string& pkgName);

private:
    std::mutex mtxLock_;
    std::condition_variable authCondVar_;
    sptr<IAccessListener> listener_ = nullptr;
    int32_t timeOut_ = 0;
    std::string pkgName_ = "";
    std::string currentNetworkId_ = "";

    std::map<std::string, bool> authorizationResults_;
};
} // namespace DistributedHardware
} // namespace OHOS
#endif // OHOS_DCAMERA_UTILS_TOOL_H
