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
#include <cstring>
#include <cstdarg>
#include <cstdio>
#include <string>

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
    };

    inline
    void
    StringUtils::Strncpy(char* dst, const uint_t dstSize, const char* src)
    {
        // NOTE: disable gcc9 -Wstringop-truncation that warns about potentially missing
        //       null-termination after strncpy but we ensure termination
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 9
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        strncpy(dst, src, dstSize - 1);
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 9
        #pragma GCC diagnostic pop
#endif
        // ensure terminating \0
        dst[dstSize - 1] = 0;
    }
}
#endif //RAMSES_CAPU_STRINGUTILS_H
