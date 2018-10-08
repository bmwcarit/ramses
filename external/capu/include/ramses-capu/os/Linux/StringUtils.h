/*
 * Copyright (C) 2012 BMW Car IT GmbH
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef RAMSES_CAPU_LINUX_STRINGUTILS_H
#define RAMSES_CAPU_LINUX_STRINGUTILS_H

#include <ramses-capu/os/Posix/StringUtils.h>

namespace ramses_capu
{
    namespace os
    {
        class StringUtils: private ramses_capu::posix::StringUtils
        {
        public:
            using ramses_capu::posix::StringUtils::Strncpy;
            using ramses_capu::posix::StringUtils::Strnlen;
            using ramses_capu::posix::StringUtils::Sprintf;
            using ramses_capu::posix::StringUtils::Vsprintf;
            using ramses_capu::posix::StringUtils::Vscprintf;
            using ramses_capu::posix::StringUtils::Strlen;
            using ramses_capu::posix::StringUtils::Strcmp;
            using ramses_capu::posix::StringUtils::LastIndexOf;
            using ramses_capu::posix::StringUtils::IndexOf;
            using ramses_capu::posix::StringUtils::StartsWith;
        };
    }
}
#endif // RAMSES_CAPU_LINUX_STRINGUTILS_H
