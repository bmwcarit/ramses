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
#include "ramses-framework-api/RamsesFrameworkTypes.h"

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

    void BinaryShaderCache::deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats)
    {
        impl.deviceSupportsBinaryShaderFormats(supportedFormats, numSupportedFormats);
    }

    bool BinaryShaderCache::hasBinaryShader(effectId_t effectId) const
    {
        return impl.hasBinaryShader(EffectIdToResourceContentHash(effectId));
    }

    uint32_t BinaryShaderCache::getBinaryShaderSize(effectId_t effectId) const
    {
        return impl.getBinaryShaderSize(EffectIdToResourceContentHash(effectId));
    }

    binaryShaderFormatId_t BinaryShaderCache::getBinaryShaderFormat(effectId_t effectId) const
    {
        return impl.getBinaryShaderFormat(EffectIdToResourceContentHash(effectId));
    }

    bool BinaryShaderCache::shouldBinaryShaderBeCached(effectId_t effectId, sceneId_t sceneId) const
    {
        return impl.shouldBinaryShaderBeCached(EffectIdToResourceContentHash(effectId), ramses_internal::SceneId{ sceneId.getValue() });
    }

    void BinaryShaderCache::getBinaryShaderData(effectId_t effectId, uint8_t* buffer, uint32_t bufferSize) const
    {
        impl.getBinaryShaderData(EffectIdToResourceContentHash(effectId), buffer, bufferSize);
    }

    void BinaryShaderCache::storeBinaryShader(effectId_t effectId, sceneId_t sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat)
    {
        impl.storeBinaryShader(EffectIdToResourceContentHash(effectId), ramses_internal::SceneId{ sceneId.getValue() }, binaryShaderData, binaryShaderDataSize, binaryShaderFormat);
    }

    void BinaryShaderCache::saveToFile(std::string_view filePath) const
    {
        impl.saveToFile(filePath);
    }

    bool BinaryShaderCache::loadFromFile(std::string_view filePath)
    {
        return impl.loadFromFile(filePath);
    }

    void BinaryShaderCache::binaryShaderUploaded(effectId_t effectId, bool success) const
    {
        impl.binaryShaderUploaded(ramses_internal::ResourceContentHash(effectId.lowPart, effectId.highPart), success);
    }
}
