//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------
#ifndef RAMSES_COMPRESSEDMEMORYBLOB_H
#define RAMSES_COMPRESSEDMEMORYBLOB_H

#include "Collections/HeapArray.h"
#include "Utils/LZ4CompressionUtils.h"

namespace ramses_internal
{
    class MemoryBlob;

    /**
    * Container for LZ4 compressed raw byte data
    */
    class CompressedMemoryBlob
    {
    public:
        explicit CompressedMemoryBlob(const UInt8* compressedData, UInt32 compressedByteSize, UInt32 decompressedByteSize);
        explicit CompressedMemoryBlob(UInt32 compressedByteSize, UInt32 decompressedByteSize);

        // see LZ4CompressionUtils::compressWithLZ4BlockAPI for details for compressionLevel
        explicit CompressedMemoryBlob(const MemoryBlob& memoryBlob, LZ4CompressionUtils::CompressionLevel level);
        UInt32       size() const;
        const UInt8* getRawData() const;
        UInt8*       getRawData();

        UInt32       getDecompressedSize() const;

        CompressedMemoryBlob(const CompressedMemoryBlob&) = delete;
        CompressedMemoryBlob& operator=(const CompressedMemoryBlob&) = delete;
        CompressedMemoryBlob(CompressedMemoryBlob&&) = delete;
        CompressedMemoryBlob& operator=(CompressedMemoryBlob&&) = delete;

    private:
        UInt32 m_decompressedSize;
        UInt32 m_compressedSize;
        HeapArray<UInt8> m_data;
    };

    inline
    UInt32 CompressedMemoryBlob::size() const
    {
        return m_compressedSize;
    }

    inline
    const UInt8* CompressedMemoryBlob::getRawData() const
    {
        return m_data.data();
    }

    inline
    UInt8* CompressedMemoryBlob::getRawData()
    {
        return m_data.data();
    }

    inline
    UInt32 CompressedMemoryBlob::getDecompressedSize() const
    {
        return m_decompressedSize;
    }
}
#endif
