//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/BinaryShaderCache.h"
#include "impl/BinaryShaderCacheImpl.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "ramses/framework/RamsesFrameworkTypes.h"

namespace ramses
{
    static ramses::internal::ResourceContentHash EffectIdToResourceContentHash(const effectId_t& effectId)
    {
        return ramses::internal::ResourceContentHash(effectId.lowPart, effectId.highPart);
    }

    BinaryShaderCache::BinaryShaderCache()
    : m_impl(*new internal::BinaryShaderCacheImpl())
    {

    }

    BinaryShaderCache::~BinaryShaderCache()
    {
        delete &m_impl;
    }

    void BinaryShaderCache::deviceSupportsBinaryShaderFormats(const binaryShaderFormatId_t* supportedFormats, uint32_t numSupportedFormats)
    {
        m_impl.deviceSupportsBinaryShaderFormats(supportedFormats, numSupportedFormats);
    }

    bool BinaryShaderCache::hasBinaryShader(effectId_t effectId) const
    {
        return m_impl.hasBinaryShader(EffectIdToResourceContentHash(effectId));
    }

    uint32_t BinaryShaderCache::getBinaryShaderSize(effectId_t effectId) const
    {
        return m_impl.getBinaryShaderSize(EffectIdToResourceContentHash(effectId));
    }

    binaryShaderFormatId_t BinaryShaderCache::getBinaryShaderFormat(effectId_t effectId) const
    {
        return m_impl.getBinaryShaderFormat(EffectIdToResourceContentHash(effectId));
    }

    bool BinaryShaderCache::shouldBinaryShaderBeCached(effectId_t effectId, sceneId_t sceneId) const
    {
        return m_impl.shouldBinaryShaderBeCached(EffectIdToResourceContentHash(effectId), ramses::internal::SceneId{ sceneId.getValue() });
    }

    void BinaryShaderCache::getBinaryShaderData(effectId_t effectId, std::byte* buffer, uint32_t bufferSize) const
    {
        m_impl.getBinaryShaderData(EffectIdToResourceContentHash(effectId), buffer, bufferSize);
    }

    void BinaryShaderCache::storeBinaryShader(effectId_t effectId, sceneId_t sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, binaryShaderFormatId_t binaryShaderFormat)
    {
        m_impl.storeBinaryShader(EffectIdToResourceContentHash(effectId), ramses::internal::SceneId{ sceneId.getValue() }, binaryShaderData, binaryShaderDataSize, binaryShaderFormat);
    }

    void BinaryShaderCache::saveToFile(std::string_view filePath) const
    {
        m_impl.saveToFile(filePath);
    }

    bool BinaryShaderCache::loadFromFile(std::string_view filePath)
    {
        return m_impl.loadFromFile(filePath);
    }

    void BinaryShaderCache::binaryShaderUploaded(effectId_t effectId, bool success) const
    {
        m_impl.binaryShaderUploaded(ramses::internal::ResourceContentHash(effectId.lowPart, effectId.highPart), success);
    }

    internal::BinaryShaderCacheImpl& BinaryShaderCache::impl()
    {
        return m_impl;
    }

    const internal::BinaryShaderCacheImpl& BinaryShaderCache::impl() const
    {
        return m_impl;
    }
}
