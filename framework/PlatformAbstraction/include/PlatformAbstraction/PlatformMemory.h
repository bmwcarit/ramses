//  -------------------------------------------------------------------------
//  Copyright (C) 2009-2010 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_PLATFORMMEMORY_H
#define RAMSES_PLATFORMMEMORY_H

#include <ramses-capu/os/Memory.h>
#include <PlatformAbstraction/PlatformTypes.h>

namespace ramses_internal
{
    class PlatformMemory
    {
    public:

        static void* Set(void* dst, Int32 val, size_t size);

        static void* Copy(void* dst, const void* src, UInt size);

        static Int32 Compare(const void* mem1, const void* mem2, UInt num);
    };

    inline
    void* PlatformMemory::Set(void* dst, Int32 val, size_t size)
    {
        ramses_capu::Memory::Set(dst, val, size);
        return dst;
    }

    inline
    void* PlatformMemory::Copy(void* dst, const void* src, UInt size)
    {
        ramses_capu::Memory::Copy(dst, src, size);
        return dst;
    }

    inline
    Int32 PlatformMemory::Compare(const void* mem1, const void* mem2, UInt num)
    {
        return ramses_capu::Memory::Compare(mem1, mem2, num);
    }
}

#endif
