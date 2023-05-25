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

    void BinaryShaderCacheProxy::deviceSupportsBinaryShaderFormats(const std::vector<ramses_internal::BinaryShaderFormatID>& supportedFormats)
    {
        // called only once before any other call to binary shader cache, no need to lock
        std::vector<binaryShaderFormatId_t> formats;
        formats.reserve(supportedFormats.size());
        std::transform(supportedFormats.cbegin(), supportedFormats.cend(), std::back_inserter(formats),
            [](ramses_internal::BinaryShaderFormatID id) { return binaryShaderFormatId_t{ id.getValue() }; });
        m_cache.deviceSupportsBinaryShaderFormats(formats.data(), uint32_t(formats.size()));
    }

    bool BinaryShaderCacheProxy::hasBinaryShader(ramses_internal::ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.hasBinaryShader({ effectHash.lowPart, effectHash.highPart });
    }

    uint32_t BinaryShaderCacheProxy::getBinaryShaderSize(ramses_internal::ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.getBinaryShaderSize({ effectHash.lowPart, effectHash.highPart });
    }

    ramses_internal::BinaryShaderFormatID BinaryShaderCacheProxy::getBinaryShaderFormat(ramses_internal::ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return ramses_internal::BinaryShaderFormatID{ m_cache.getBinaryShaderFormat({ effectHash.lowPart, effectHash.highPart }).getValue() };
    }

    bool BinaryShaderCacheProxy::shouldBinaryShaderBeCached(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.shouldBinaryShaderBeCached({ effectHash.lowPart, effectHash.highPart }, sceneId_t(sceneId.getValue()));
    }

    void BinaryShaderCacheProxy::getBinaryShaderData(ramses_internal::ResourceContentHash effectHash, ramses_internal::UInt8* buffer, ramses_internal::UInt32 bufferSize) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.getBinaryShaderData({ effectHash.lowPart, effectHash.highPart }, buffer, bufferSize);
    }

    void BinaryShaderCacheProxy::storeBinaryShader(ramses_internal::ResourceContentHash effectHash, ramses_internal::SceneId sceneId, const uint8_t* binaryShaderData, uint32_t binaryShaderDataSize, ramses_internal::BinaryShaderFormatID binaryShaderFormat)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.storeBinaryShader({ effectHash.lowPart, effectHash.highPart }, sceneId_t{ sceneId.getValue() }, binaryShaderData, binaryShaderDataSize, binaryShaderFormatId_t{ binaryShaderFormat.getValue() });
    }

    void BinaryShaderCacheProxy::binaryShaderUploaded(ramses_internal::ResourceContentHash effectHash, bool success) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.binaryShaderUploaded({ effectHash.lowPart, effectHash.highPart }, success);
    }

    std::once_flag& BinaryShaderCacheProxy::binaryShaderFormatsReported()
    {
        return m_supportedFormatsReported;
    }

}
