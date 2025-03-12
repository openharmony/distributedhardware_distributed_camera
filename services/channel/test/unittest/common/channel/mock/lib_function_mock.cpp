/*
 * Copyright (c) 2025 Huawei Device Co., Ltd.
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

#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>

#include "dlfcn_mock.h"

char *realpath(const char *path, char *resolvedPath)
{
    return resolvedPath;
}

void *dlopen(const char *file, int mode)
{
    auto instance = OHOS::DistributedHardware::DlfcnMock::GetOrCreateInstance();
    return instance->DlopenMock(file, mode);
}

int dlclose(void *handle)
{
    auto instance = OHOS::DistributedHardware::DlfcnMock::GetOrCreateInstance();
    return instance->DlcloseMock(handle);
}

void *dlsym(void *__restrict handle, const char *__restrict name)
{
    auto instance = OHOS::DistributedHardware::DlfcnMock::GetOrCreateInstance();
    return instance->DlsymMock(handle, name);
}

namespace OHOS {
namespace DistributedHardware {
std::shared_ptr<DlfcnMock> DlfcnMock::instance_;

std::shared_ptr<DlfcnMock> DlfcnMock::GetOrCreateInstance()
{
    if (!instance_) {
        instance_ = std::make_shared<DlfcnMock>();
    }
    return instance_;
}

void DlfcnMock::ReleaseInstance()
{
    instance_.reset();
    instance_ = nullptr;
}
} // namespace DistributedHardware
} // namespace OHOS
