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

#ifndef RAMSES_CAPU_UNIXBASED_STRINGUTILS_H
#define RAMSES_CAPU_UNIXBASED_STRINGUTILS_H

#include "ramses-capu/Config.h"
#include <string.h>
#include <stdarg.h>

namespace ramses_capu
{
    namespace posix
    {
        class StringUtils
        {
        public:
            static void Strncpy(char* dst, const uint_t dstSize, const char* src);
            static uint_t Strnlen( const char* str, uint_t maxlen);
            static int32_t Sprintf(char* buffer, const uint_t bufferSize, const char* format, ...);
            static int32_t Vsprintf(char* buffer, const uint_t bufferSize, const char* format, va_list values);
            static int32_t Vscprintf(const char* format, va_list values);
            static uint_t Strlen(const char* str);
            static int_t Strcmp(const char* str1, const char* str2);
            static int_t LastIndexOf(const char* str, const char ch);
            static int_t IndexOf(const char* str, const char ch, const uint_t offset = 0);
            static int_t IndexOf(const char* str, const char* str2, const uint_t offset = 0);
            static bool StartsWith(const char* str, const char* prefix);
        };

        inline
        void
        StringUtils::Strncpy(char* dst, uint_t dstSize, const char* src)
        {
            // reproduce behaviour of strncpy_s
            strncpy(dst, src, dstSize - 1);
            dst[dstSize - 1] = 0;
        }

        inline
        uint_t
        StringUtils::Strnlen(const char* str, uint_t maxlen)
        {
            if (str == 0)
            {
                return 0;
            }
            return strnlen(str, maxlen);
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
        int_t
        StringUtils::Strcmp(const char* str1, const char* str2)
        {
            return strcmp(str1, str2);
        }

        inline
        int32_t
        StringUtils::Vsprintf(char* buffer, uint_t bufferSize, const char* format, va_list values)
        {
            return vsnprintf(buffer, bufferSize, format, values);
        }

        inline
        int32_t
        StringUtils::Vscprintf(const char* format, va_list values)
        {
            char c;
            return vsnprintf(&c, 1, format, values);
        }

        inline
        int32_t
        StringUtils::Sprintf(char* buffer, uint_t bufferSize, const char* format, ...)
        {
            va_list argptr;
            va_start(argptr, format);
            int32_t result = StringUtils::Vsprintf(buffer, bufferSize, format, argptr);
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
            const char* pos = strchr(str +  offset, ch);
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
#endif // RAMSES_CAPU_UNIXBASED_STRINGUTILS_H
