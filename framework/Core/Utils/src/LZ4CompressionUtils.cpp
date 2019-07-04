//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Utils/LZ4CompressionUtils.h"
#include "PlatformAbstraction/PlatformMath.h"

#include "lz4.h"
#include "lz4hc.h"

namespace ramses_internal
{
    namespace LZ4CompressionUtils
    {
        UInt32 compressedSizeBound(UInt32 uncompressedSize)
        {
            return LZ4_compressBound(uncompressedSize);
        }

        bool compress(HeapArray<UInt8>& compressedBuffer, UInt32& compressedBufferSize, UInt8 const* plainBuffer, UInt32 plainSize, CompressionLevel level)
        {
            if (plainSize == 0)
            {
                compressedBufferSize = 0;
                return true;
            }

            int compressedSize = 0;
            if (level == CompressionLevel::Fast)
            {
                compressedSize = LZ4_compress_default(reinterpret_cast<const char*>(plainBuffer),
                    reinterpret_cast<char*>(compressedBuffer.data()),
                    plainSize,
                    static_cast<int>(compressedBuffer.size()));
            }
            else
            {
                compressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(plainBuffer),
                    reinterpret_cast<char*>(compressedBuffer.data()),
                    plainSize,
                    static_cast<int>(compressedBuffer.size()),
                    // using higher compression causes too excessive times to be able to use
                    LZ4HC_CLEVEL_DEFAULT);
            }

            compressedBufferSize = compressedSize;
            return compressedSize > 0;
        }

        bool decompress(HeapArray<UInt8>& plainBuffer, UInt8 const* compressedBuffer,
                        UInt32 compressedSize)
        {
            if (compressedSize == 0)
            {
                return true;
            }

            int bytesDecompressed = LZ4_decompress_safe(reinterpret_cast<const char*>(compressedBuffer),
                                                        reinterpret_cast<char*>(plainBuffer.data()),
                                                        compressedSize,
                                                        static_cast<int>(plainBuffer.size()));

            if (bytesDecompressed != static_cast<int>(plainBuffer.size()))
            {
                return false;
            }
            return true;
        }
    }
}
