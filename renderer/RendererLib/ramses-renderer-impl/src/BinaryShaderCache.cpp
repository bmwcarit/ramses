//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/BinaryShaderCache.h"
#include "BinaryShaderCacheImpl.h"
#include "SceneAPI/ResourceContentHash.h"

namespace ramses
{
    static ramses_internal::ResourceContentHash EffectIdToResourceContentHash(const effectId_t& effectId)
    {
        return ramses_internal::ResourceContentHash(effectId.lowPart, effectId.highPart);
    }

    BinaryShaderCache::BinaryShaderCache()
    : impl(*new BinaryShaderCacheImpl())
    {

    }

    BinaryShaderCache::~BinaryShaderCache()
    {
        delete &impl;
    }

    bool BinaryShaderCache::hasBinaryShader(effectId_t effectId) const
    {
        return impl.hasBinaryShader(EffectIdToResourceContentHash(effectId));
    }

    uint32_t BinaryShaderCache::getBinaryShaderSize(effectId_t effectId) const
    {
        return impl.getBinaryShaderSize(EffectIdToResourceContentHash(effectId));
    }

    uint32_t BinaryShaderCache::getBinaryShaderFormat(effectId_t effectId) const
    {
        return impl.getBinaryShaderFormat(EffectIdToResourceContentHash(effectId));
    }

    bool BinaryShaderCache::shouldBinaryShaderBeCached(effectId_t effectId) const
    {
        return impl.shouldBinaryShaderBeCached(EffectIdToResourceContentHash(effectId));
    }

    void BinaryShaderCache::getBinaryShaderData(effectId_t effectId, uint8_t* buffer, uint32_t bufferSize) const
    {
        impl.getBinaryShaderData(EffectIdToResourceContentHash(effectId), buffer, bufferSize);
    }

    void BinaryShaderCache::storeBinaryShader(effectId_t effectId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, uint32_t binaryShaderFormat)
    {
        impl.storeBinaryShader(EffectIdToResourceContentHash(effectId), binaryShaderData, binaryShaderDataSize, binaryShaderFormat);
    }

    void BinaryShaderCache::saveToFile(const char* filePath) const
    {
        impl.saveToFile(filePath);
    }

    bool BinaryShaderCache::loadFromFile(const char* filePath)
    {
        return impl.loadFromFile(filePath);
    }

    void BinaryShaderCache::binaryShaderUploaded(effectId_t effectId, bool success) const
    {
        impl.binaryShaderUploaded(ramses_internal::ResourceContentHash(effectId.lowPart, effectId.highPart), success);
    }
}
