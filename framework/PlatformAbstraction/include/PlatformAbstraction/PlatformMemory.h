//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMMEMORY_H
#define RAMSES_PLATFORMMEMORY_H

#include <PlatformAbstraction/PlatformTypes.h>
#include <cstring>

namespace ramses_internal
{
    namespace PlatformMemory
    {
        inline
        void Set(void* dst, int32_t val, size_t size)
        {
            if(size > 0)
                std::memset(dst, val, size);
        }

        inline
        void Copy(void* dst, const void* src, size_t size)
        {
            if(size > 0)
                std::memcpy(dst, src, size);
        }

        inline
        int32_t Compare(const void* mem1, const void* mem2, size_t num)
        {
            if(num > 0)
                return std::memcmp(mem1, mem2, num);
            return 0;
        }
    };
}

#endif
