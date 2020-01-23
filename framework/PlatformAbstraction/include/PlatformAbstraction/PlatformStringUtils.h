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
    };

    inline
    void PlatformStringUtils::Copy(Char* dst, UInt32 dstSize, const Char* src)
    {
        ramses_capu::StringUtils::Strncpy(dst, dstSize, src);
    }
}

#endif
