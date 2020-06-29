//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMSTRINGUTILS_H
#define RAMSES_PLATFORMSTRINGUTILS_H

#include <cstring>

namespace ramses_internal
{
    class PlatformStringUtils
    {
    public:
        static void Copy(char* dst, size_t dstSize, const char* src);
    };

    inline
    void PlatformStringUtils::Copy(char* dst, size_t dstSize, const char* src)
    {
        // NOTE: disable gcc9 -Wstringop-truncation that warns about potentially missing
        //       null-termination after strncpy but we ensure termination
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 9
        #pragma GCC diagnostic push
        #pragma GCC diagnostic ignored "-Wstringop-truncation"
#endif
        std::strncpy(dst, src, dstSize - 1);
#if defined(__GNUC__) && !defined(__clang__) && __GNUC__ >= 9
        #pragma GCC diagnostic pop
#endif
        // ensure terminating \0
        dst[dstSize - 1] = 0;
    }
}

#endif
