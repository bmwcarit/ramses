//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_LZ4COMPRESSIONUTILS_H
#define RAMSES_LZ4COMPRESSIONUTILS_H

#include "Collections/HeapArray.h"

namespace ramses_internal
{
    namespace LZ4CompressionUtils
    {
        enum class CompressionLevel : int
        {
            Fast,
            High
        };

        //! get maximum required buffer size to compress data of size uncompressedSize
        UInt32 compressedSizeBound(UInt32 uncompressedSize);

        //! compress the plainBuffer with size plainSize to compressedBuffer
        //! using LZ4 Block API (No frames, no header, just compressing a memory block)
        //! \param compressedBuffer compressed data will be written here, vector will be resized to
        //!        the size of the compressed data or 0, if function does not succeed
        //! \param plainBuffer the pointer to the memory region with the data to compress
        //! \param plainSize the size of the data to compress
        //! \param quality provide negative value to use standard lz4 compression, 0-16 sets
        //!        compression level for hc compression implementation, values > 16 are as 16
        //! \return true if success.
        bool compress(HeapArray<UInt8>& compressedBuffer,
                      UInt32& compressedBufferSize,
                      UInt8 const* plainBuffer,
                      UInt32 plainSize,
                      CompressionLevel level);

        //! decompress the compressedBuffer with size compressedSize to plainBuffer
        //! using LZ4 Block API (No frames, no header, just decompressing a memory block)
        //! \param plainBuffer uncompressed data will be written here, it will be resized to
        //!        the size of the uncompressed data or 0, if function does not succeed
        //! \param compressedBuffer the pointer to the memory region with the compressed data
        //! \return true if success.
        bool decompress(HeapArray<UInt8>& plainBuffer,
                        UInt8 const* compressedBuffer,
                        UInt32 compressedSize);
    }
}
#endif
