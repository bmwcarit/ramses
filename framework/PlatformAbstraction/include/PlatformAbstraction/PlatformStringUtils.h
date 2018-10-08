//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMSTRINGUTILS_H
#define RAMSES_PLATFORMSTRINGUTILS_H

#include <stdarg.h>

#include <ramses-capu/os/StringUtils.h>
#include <PlatformAbstraction/PlatformTypes.h>

namespace ramses_internal
{
    class PlatformStringUtils
    {
    public:
        static void   Copy(Char* dst   , UInt32 dstSize, const Char* src);
        static UInt32 StrLen(const Char* str);
        static bool   StrEmpty(const Char* str);
        static bool   StrEqual(const Char* str1, const Char* str2);

        static Int32  Compare(const Char* str1, const Char* str2);
        /**
         * Compare str1 with str2, but only up to the specified compare length.
         * @param   str1    The first compare string.
         * @param   str2    The second compare string.
         * @retval  <0  If str1 is less than str2.
         * @retval  0   If str1 is equal to str2 (limited to the specified compare length).
         * @retval  >0  If str1 is greater than str2.
         */
        static Int32  Compare(const Char* str1, const Char* str2, const UInt32 compareLength);
        static Int32  Sprintf(Char* buffer, size_t buffer_size, const Char* format, ...);
        static Int32  VSprintf(Char* buffer, size_t buffer_size, const Char* format, va_list argPtr);

    protected:
    private:
    };

    inline
    void PlatformStringUtils::Copy(Char* dst, UInt32 dstSize, const Char* src)
    {
        ramses_capu::StringUtils::Strncpy(dst, dstSize, src);
    }

    inline
    UInt32 PlatformStringUtils::StrLen(const Char* str)
    {
        return static_cast<UInt32>(ramses_capu::StringUtils::Strlen(str));
    }

    inline
    bool PlatformStringUtils::StrEqual(const Char* str1, const Char* str2)
    {
        return Compare(str1, str2) == 0;
    }

    inline
    bool PlatformStringUtils::StrEmpty(const char* str)
    {
        return *str == '\0';
    }

    inline
    Int32 PlatformStringUtils::Compare(const Char* str1, const Char* str2)
    {
        return static_cast<Int32>(ramses_capu::StringUtils::Strcmp(str1, str2));
    }

    inline Int32 PlatformStringUtils::Compare(const Char* str1, const Char* str2, const UInt32 compareLength)
    {
        // strncmp currently not present in capu, therefore directly called from here
        return (static_cast<Int32>(::strncmp(str1, str2, compareLength)));
    }

    inline
    Int32 PlatformStringUtils::Sprintf(Char* buffer, size_t buffer_size, const Char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        const Int32 numberCharacters = ramses_capu::StringUtils::Vsprintf(buffer, buffer_size, format, argptr);
        va_end(argptr);
        return numberCharacters;
    }

    inline
    Int32 PlatformStringUtils::VSprintf(Char* buffer, size_t buffer_size, const Char* format, va_list argPtr)
    {
        return ramses_capu::StringUtils::Vsprintf(buffer, buffer_size, format, argPtr);
    }
}

#endif
