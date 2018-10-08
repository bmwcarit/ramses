//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BINARYSHADERCACHEPROXY_H
#define RAMSES_BINARYSHADERCACHEPROXY_H

#include "RendererAPI/IBinaryShaderCache.h"
#include "ramses-renderer-api/Types.h"


namespace ramses
{
    class IBinaryShaderCache;
    class BinaryShaderCacheProxy : public ramses_internal::IBinaryShaderCache
    {
    public:
        BinaryShaderCacheProxy(ramses::IBinaryShaderCache& cache);
        virtual ~BinaryShaderCacheProxy();

        virtual ramses_internal::Bool hasBinaryShader(ramses_internal::ResourceContentHash effectHash) const override;
        virtual ramses_internal::UInt32 getBinaryShaderSize(ramses_internal::ResourceContentHash effectHash) const override;
        virtual ramses_internal::UInt32 getBinaryShaderFormat(ramses_internal::ResourceContentHash effectHash) const override;
        virtual void getBinaryShaderData(ramses_internal::ResourceContentHash effectHash, ramses_internal::UInt8* buffer, ramses_internal::UInt32 bufferSize) const override;

        virtual bool shouldBinaryShaderBeCached(ramses_internal::ResourceContentHash effectHash) const override;

        virtual void storeBinaryShader(ramses_internal::ResourceContentHash effectHash, const ramses_internal::UInt8* binaryShaderData, ramses_internal::UInt32 binaryShaderDataSize, ramses_internal::UInt32 binaryShaderFormat) override;

        virtual void binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const override;

    private:
        static effectId_t getEffectIdFromEffectHash(const ramses_internal::ResourceContentHash& effectHash);

        ramses::IBinaryShaderCache& m_cache;
    };
}

#endif
