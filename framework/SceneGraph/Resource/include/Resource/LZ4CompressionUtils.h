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
#include "Resource/ResourceTypes.h"

namespace ramses_internal
{
    namespace LZ4CompressionUtils
    {
        enum class CompressionLevel : int
        {
            Fast,
            High
        };

        CompressedResourceBlob compress(const ResourceBlob& plainBuffer, CompressionLevel level);
        ResourceBlob decompress(const CompressedResourceBlob& compressedData, uint32_t uncompressedSize);
    }
}
#endif
