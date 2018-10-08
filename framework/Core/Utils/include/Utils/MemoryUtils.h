//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_MEMORYUTILS_H
#define RAMSES_MEMORYUTILS_H

#include "PlatformAbstraction/PlatformTypes.h"

namespace ramses_internal
{
    class MemoryUtils
    {
    public:
        template<typename T>
        static Bool AreAllBytesZero(const T* elements, UInt32 elementCount)
        {
            assert(elementCount > 0u);

            const Byte* data = reinterpret_cast<const Byte*>(elements);
            const UInt32 dataSize = sizeof(T) * elementCount;

            return (*data == 0) && (PlatformMemory::Compare(data, data + 1u, dataSize - 1u) == 0);
        }
    };
}

#endif
