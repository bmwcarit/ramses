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

#ifndef RAMSES_CAPU_WINDOWS_STRINGUTILS_H
#define RAMSES_CAPU_WINDOWS_STRINGUTILS_H

#include <stdarg.h>
#include "ramses-capu/Config.h"
#include <ramses-capu/os/Memory.h>

namespace ramses_capu
{
    namespace os
    {
        class StringUtils
        {
        public:
            static void Strncpy(char* dst, const uint_t dstSize, const char* src);
            static uint_t Strnlen(const char* dst, uint_t maxlen);
            static int32_t Sprintf(char* buffer, const uint_t bufferSize, const char* format, ...);
            static int32_t Vsprintf(char* buffer, const uint_t bufferSize, const char* format, va_list values);
            static int32_t Vscprintf(const char* format, va_list values);
            static uint_t Strlen(const char* str);
            static int_t Strcmp(const char* str1, const char* str2);
            static int_t LastIndexOf(const char* str, const char ch);
            static int_t IndexOf(const char* str, const char ch, const uint_t offset = 0);
            static int_t IndexOf(const char* str, const char* str2, const uint_t offset = 0);
            static bool StartsWith(const char* str, const char* prefix);
        private:
        };

        inline
        void
        StringUtils::Strncpy(char* dst, uint_t dstSize, const char* src)
        {
            strncpy_s(dst, dstSize, src, _TRUNCATE);
        }

        inline
        uint_t
        StringUtils::Strlen(const char* str)
        {
            if (str == 0)
            {
                return 0;
            }
            return strlen(str);
        }

        inline
        uint_t
        StringUtils::Strnlen(const char* str, uint_t maxlen)
        {
            if (str == 0)
            {
                return 0;
            }
#ifdef __MINGW64__
            return strnlen(str, maxlen);
#else
            return strnlen_s(str, maxlen);
#endif
        }

        inline
        int_t
        StringUtils::Strcmp(const char* str1, const char* str2)
        {
            return strcmp(str1, str2);
        }

        inline
        int32_t
        StringUtils::Vsprintf(char* buffer, uint_t bufferSize, const char* format, va_list values)
        {
            return vsprintf_s(buffer, bufferSize, format, values);
        }

        inline
        int32_t
        StringUtils::Vscprintf(const char* format, va_list values)
        {
            return _vscprintf(format, values);
        }

        inline
        int32_t
        StringUtils::Sprintf(char* buffer, uint_t bufferSize, const char* format, ...)
        {
            va_list argptr;
            va_start(argptr, format);
            const int32_t result = StringUtils::Vsprintf(buffer, bufferSize, format, argptr);
            va_end(argptr);
            return result;
        }

        inline
        int_t
        StringUtils::LastIndexOf(const char* str, const char ch)
        {
            if (!str)
            {
                return -1;
            }
            const char* pos = strrchr(str, ch);
            return pos ? pos - str : -1;
        }

        inline
        int_t
        StringUtils::IndexOf(const char* str, const char ch, const uint_t offset)
        {
            if (!str)
            {
                return -1;
            }
            const char* pos = strchr(str + offset, ch);
            return pos ? pos - str : -1;
        }

        inline
        bool
        StringUtils::StartsWith(const char* str, const char* prefix)
        {
            if (!prefix || !str)
            {
                return false;
            }
            return strncmp(str, prefix, strlen(prefix)) == 0;
        }

        inline
        int_t
        StringUtils::IndexOf(const char* str, const char* str2, const uint_t offset)
        {
            const char* start = strstr(str + offset, str2);
            return start ? (start - str) : -1;
        }
    }
}

#endif //RAMSES_CAPU_WINDOWS_STRINGUTILS_H
