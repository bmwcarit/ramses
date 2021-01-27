//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Resource/LZ4CompressionUtils.h"
#include "lz4.h"
#include "lz4hc.h"

namespace ramses_internal
{
    namespace LZ4CompressionUtils
    {
        CompressedResourceBlob compress(const ResourceBlob& plainBuffer, CompressionLevel level)
        {
            const int plainSize = static_cast<int>(plainBuffer.size());
            if (!plainSize)
                return CompressedResourceBlob();

            CompressedResourceBlob compressedBuffer(LZ4_compressBound(plainSize));
            int realCompressedSize = 0;

            if (level == CompressionLevel::Fast)
            {
                realCompressedSize = LZ4_compress_default(reinterpret_cast<const char*>(plainBuffer.data()),
                    reinterpret_cast<char*>(compressedBuffer.data()),
                    plainSize,
                    static_cast<int>(compressedBuffer.size()));
            }
            else
            {
                realCompressedSize = LZ4_compress_HC(reinterpret_cast<const char*>(plainBuffer.data()),
                    reinterpret_cast<char*>(compressedBuffer.data()),
                    plainSize,
                    static_cast<int>(compressedBuffer.size()),
                    // using higher compression causes too excessive times to be able to use
                    LZ4HC_CLEVEL_DEFAULT);
            }

            if (realCompressedSize <= 0)
                return CompressedResourceBlob();

            // compressedBuffer size might be too large, create properly sized result
            return CompressedResourceBlob(realCompressedSize, std::move(compressedBuffer));
        }

        ResourceBlob decompress(const CompressedResourceBlob& compressedData, uint32_t uncompressedSize)
        {
            if (!compressedData.size() || !uncompressedSize)
                return ResourceBlob();

            ResourceBlob plainBuffer(uncompressedSize);
            int bytesDecompressed = LZ4_decompress_safe(reinterpret_cast<const char*>(compressedData.data()),
                                                        reinterpret_cast<char*>(plainBuffer.data()),
                                                        static_cast<int>(compressedData.size()),
                                                        static_cast<int>(plainBuffer.size()));

            if (bytesDecompressed != static_cast<int>(plainBuffer.size()))
                return ResourceBlob();

            return plainBuffer;
        }
    }
}
