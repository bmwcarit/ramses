//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/MemoryBlob.h"
#include "Utils/CompressedMemoryBlob.h"
#include "Utils/LZ4CompressionUtils.h"
#include "PlatformAbstraction/PlatformMemory.h"

namespace ramses_internal
{
    MemoryBlob::MemoryBlob(UInt32 byteSize)
        : m_data(byteSize)
    {
    }

    MemoryBlob::MemoryBlob(const void* data, UInt32 byteSize)
        : m_data(byteSize, reinterpret_cast<const UInt8*>(data))
    {
    }

    MemoryBlob::MemoryBlob(const CompressedMemoryBlob& compressedMemoryBlob)
        : m_data(compressedMemoryBlob.getDecompressedSize())
    {
        if (!LZ4CompressionUtils::decompress(m_data,
                                             compressedMemoryBlob.getRawData(),
                                             compressedMemoryBlob.size()))
        {
            m_data = HeapArray<UInt8>();
        }
    }
}
