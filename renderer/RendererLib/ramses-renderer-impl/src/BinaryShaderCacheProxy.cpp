//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "BinaryShaderCacheProxy.h"
#include "ramses-renderer-api/IBinaryShaderCache.h"

namespace ramses
{
    BinaryShaderCacheProxy::BinaryShaderCacheProxy(ramses::IBinaryShaderCache& cache)
    : m_cache(cache)
    {

    }

    BinaryShaderCacheProxy::~BinaryShaderCacheProxy()
    {

    }

    ramses_internal::Bool BinaryShaderCacheProxy::hasBinaryShader(ramses_internal::ResourceContentHash effectHash) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        return m_cache.hasBinaryShader(effectId);
    }

    ramses_internal::UInt32 BinaryShaderCacheProxy::getBinaryShaderSize(ramses_internal::ResourceContentHash effectHash) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        return m_cache.getBinaryShaderSize(effectId);
    }

    ramses_internal::UInt32 BinaryShaderCacheProxy::getBinaryShaderFormat(ramses_internal::ResourceContentHash effectHash) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        return m_cache.getBinaryShaderFormat(effectId);
    }

    ramses_internal::Bool BinaryShaderCacheProxy::shouldBinaryShaderBeCached(ramses_internal::ResourceContentHash effectHash) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        return m_cache.shouldBinaryShaderBeCached(effectId);
    }

    void BinaryShaderCacheProxy::getBinaryShaderData(ramses_internal::ResourceContentHash effectHash, ramses_internal::UInt8* buffer, ramses_internal::UInt32 bufferSize) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        m_cache.getBinaryShaderData(effectId, buffer, bufferSize);
    }

    void BinaryShaderCacheProxy::storeBinaryShader(ramses_internal::ResourceContentHash effectHash, const ramses_internal::UInt8* binaryShaderData, ramses_internal::UInt32 binaryShaderDataSize, ramses_internal::UInt32 binaryShaderFormat)
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        m_cache.storeBinaryShader(effectId, binaryShaderData, binaryShaderDataSize, binaryShaderFormat);
    }

    void BinaryShaderCacheProxy::binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const
    {
        const effectId_t effectId = getEffectIdFromEffectHash(effectHash);
        m_cache.binaryShaderUploaded(effectId, success);
    }

    ramses::effectId_t BinaryShaderCacheProxy::getEffectIdFromEffectHash(const ramses_internal::ResourceContentHash& effectHash)
    {
        const effectId_t effectId = { effectHash.lowPart, effectHash.highPart };
        return effectId;
    }
}
