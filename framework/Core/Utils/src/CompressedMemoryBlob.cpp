//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/CompressedMemoryBlob.h"
#include "Utils/MemoryBlob.h"


namespace ramses_internal
{
    CompressedMemoryBlob::CompressedMemoryBlob(const UInt8* compressedData, UInt32 compressedByteSize, UInt32 decompressedByteSize)
        : m_decompressedSize(decompressedByteSize)
        , m_compressedSize(compressedByteSize)
        , m_data(compressedByteSize, compressedData)
    {
    }

    CompressedMemoryBlob::CompressedMemoryBlob(UInt32 compressedByteSize, UInt32 decompressedByteSize)
        : m_decompressedSize(decompressedByteSize)
        , m_compressedSize(compressedByteSize)
        , m_data(compressedByteSize)
    {
    }

    CompressedMemoryBlob::CompressedMemoryBlob(const MemoryBlob& memoryBlob, LZ4CompressionUtils::CompressionLevel level)
        : m_decompressedSize(memoryBlob.size())
        , m_compressedSize(0)
        , m_data(LZ4CompressionUtils::compressedSizeBound(m_decompressedSize))
    {
        if (!LZ4CompressionUtils::compress(m_data,
                                           m_compressedSize,
                                           memoryBlob.getRawData(),
                                           m_decompressedSize,
                                           level))
        {
            m_decompressedSize = 0;
            m_compressedSize = 0;
            m_data = HeapArray<UInt8>();
        }
    }
}
