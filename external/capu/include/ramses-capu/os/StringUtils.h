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

#ifndef RAMSES_CAPU_STRINGUTILS_H
#define RAMSES_CAPU_STRINGUTILS_H

#include "ramses-capu/Config.h"
#include "ramses-capu/Error.h"
#include <ramses-capu/os/PlatformInclude.h>

#include RAMSES_CAPU_PLATFORM_INCLUDE(StringUtils)

namespace ramses_capu
{
    /**
     * Utilities for manipulating string.
     */
    class StringUtils
    {
    public:
        /**
         * Static method to copy a String of length dstSize from src to dst
         * @param dst destination buffer
         * @param dstSize number of chars to be copied
         * @param src source buffer
         */
        static void Strncpy(char* dst, const uint_t dstSize, const char* src);

        /**
         * Static method to write a C string according to the given format into the array pointed by buffer.
         * After the format parameter, the function expects at least as many additional arguments as specified in format.
         * @param buffer which contains the string
         * @param bufferSize size of the buffer
         * @param format the format of the string
         * @return total number of characters written on success, negative number otherwise
         */
        static int32_t Sprintf(char* buffer, const uint_t bufferSize, const char* format, ...);

        /**
         * Static method to write a C string according to the given format into the array pointed by buffer.
         * The arguments specified in the format have to be passed by the values parameter.
         * @param buffer which contains the string
         * @param bufferSize size of the buffer
         * @param format the format of the string
         * @param values arguments for the format
         * @return total number of characters written on success, negative number otherwise
         */
        static int32_t Vsprintf(char* buffer, const uint_t bufferSize, const char* format, va_list values);

        /**
         * Static method to count the bytes according to the given format
         * The arguments specified in the format have to be passed by the values parameter.
         * @param format the format of the string
         * @param values arguments for the format
         */
        static int32_t Vscprintf(const char* format, va_list values);

        /**
         * Static method to return the length of the given String
         * @param str the string
         * @return length of the string
         */
        static uint_t Strlen(const char* str);

        /**
        * Static method to return the length of the given String
        * @param str the string
        * @param maxlen maximum number of characters to search for null terminator
        * @return length of the string
        */
        static uint_t Strnlen(const char* str, uint_t maxlen);

        /**
         * Static method to compare two C strings.
         * @param str1 first string
         * @param str2 second string
         * @return 0 if both strings are equal
         *         > 0 if the first character which does not match has a greater value in str1
         *         < 0 otherwise
         */
        static int_t Strcmp(const char* str1, const char* str2);

        /**
         * Calculates the last index of a given character.
         * @param str the string
         * @param ch the search char.
         * @return The last index of the specified char or -1 if the char was not found.
         */
        static int_t LastIndexOf(const char* str, const char ch);

        /**
         * Calculates the first index of a given character.
         * @param str the string
         * @param ch the search char.
         * @param offset the index from where the search for the character has to be started (default 0).
         * @return The first index of the specified char or -1 if the char was not found.
         */
        static int_t IndexOf(const char* str, const char ch, const uint_t offset = 0);

        /**
         * Calculates the first index of a given string
         * @param string to search in
         * @param string to search for
         * @param offset The index from where the search for the 2nd string has to be started in the 1st string (default 0).
         * @return the first index of the specified string or -1 if the char was not found
         */
        static int_t IndexOf(const char* str, const char* str2, const uint_t offset = 0);

        /**
         * Checks if the given testString starts with the given prefix.
         * @param str The string which is tested.
         * @param prefix The prefix.
         * @return True if the testString starts with the given prefix.
         */
        static bool StartsWith(const char* str, const char* prefix);
    };

    inline
    void
    StringUtils::Strncpy(char* dst, const uint_t dstSize, const char* src)
    {
        ramses_capu::os::StringUtils::Strncpy(dst, dstSize, src);
    }

    inline
    int32_t
    StringUtils::Sprintf(char* buffer, const uint_t bufferSize, const char* format, ...)
    {
        va_list argptr;
        va_start(argptr, format);
        const int32_t result = ramses_capu::os::StringUtils::Vsprintf(buffer, bufferSize, format, argptr);
        va_end(argptr);
        return result;
    }

    inline
    int32_t
    StringUtils::Vsprintf(char* buffer, const uint_t bufferSize, const char* format, va_list values)
    {
        return ramses_capu::os::StringUtils::Vsprintf(buffer, bufferSize, format, values);
    }

    inline
    int32_t
    StringUtils::Vscprintf(const char* format, va_list values)
    {
        return ramses_capu::os::StringUtils::Vscprintf(format, values);
    }

    inline
    uint_t
    StringUtils::Strlen(const char* str)
    {
        return ramses_capu::os::StringUtils::Strlen(str);
    }

    inline
    uint_t
    StringUtils::Strnlen(const char* str, uint_t maxlen)
    {
        return ramses_capu::os::StringUtils::Strnlen(str, maxlen);
    }

    inline
    int_t
    StringUtils::Strcmp(const char* str1, const char* str2)
    {
        return ramses_capu::os::StringUtils::Strcmp(str1, str2);
    }

    inline
    int_t
    StringUtils::LastIndexOf(const char* str, const char ch)
    {
        return ramses_capu::os::StringUtils::LastIndexOf(str, ch);
    }

    inline
    int_t
    StringUtils::IndexOf(const char* str, const char ch, const uint_t offset)
    {
        if (Strlen(str) >= offset)
        {
            return ramses_capu::os::StringUtils::IndexOf(str, ch, offset);
        }
        else
        {
            return -1;
        }
    }

    inline
    bool
    StringUtils::StartsWith(const char* str, const char* prefix)
    {
        return ramses_capu::os::StringUtils::StartsWith(str, prefix);
    }

    inline
    int_t
    StringUtils::IndexOf(const char* str, const char* str2, const uint_t offset)
    {
        if (str && str2 && Strlen(str) >= offset)
        {
            return ramses_capu::os::StringUtils::IndexOf(str, str2, offset);
        }
        else
        {
            return -1;
        }
    }
}
#endif //RAMSES_CAPU_STRINGUTILS_H
