//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/BinaryShaderCacheProxy.h"
#include "ramses/renderer/IBinaryShaderCache.h"

namespace ramses::internal
{
    BinaryShaderCacheProxy::BinaryShaderCacheProxy(ramses::IBinaryShaderCache& cache)
        : m_cache(cache)
    {
    }

    void BinaryShaderCacheProxy::deviceSupportsBinaryShaderFormats(const std::vector<BinaryShaderFormatID>& supportedFormats)
    {
        // called only once before any other call to binary shader cache, no need to lock
        std::vector<binaryShaderFormatId_t> formats;
        formats.reserve(supportedFormats.size());
        std::transform(supportedFormats.cbegin(), supportedFormats.cend(), std::back_inserter(formats),
            [](BinaryShaderFormatID id) { return binaryShaderFormatId_t{ id.getValue() }; });
        m_cache.deviceSupportsBinaryShaderFormats(formats.data(), uint32_t(formats.size()));
    }

    bool BinaryShaderCacheProxy::hasBinaryShader(ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.hasBinaryShader({ effectHash.lowPart, effectHash.highPart });
    }

    uint32_t BinaryShaderCacheProxy::getBinaryShaderSize(ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.getBinaryShaderSize({ effectHash.lowPart, effectHash.highPart });
    }

    BinaryShaderFormatID BinaryShaderCacheProxy::getBinaryShaderFormat(ResourceContentHash effectHash) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return BinaryShaderFormatID{ m_cache.getBinaryShaderFormat({ effectHash.lowPart, effectHash.highPart }).getValue() };
    }

    bool BinaryShaderCacheProxy::shouldBinaryShaderBeCached(ResourceContentHash effectHash, SceneId sceneId) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_cache.shouldBinaryShaderBeCached({ effectHash.lowPart, effectHash.highPart }, sceneId_t(sceneId.getValue()));
    }

    void BinaryShaderCacheProxy::getBinaryShaderData(ResourceContentHash effectHash, std::byte* buffer, uint32_t bufferSize) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.getBinaryShaderData({ effectHash.lowPart, effectHash.highPart }, buffer, bufferSize);
    }

    void BinaryShaderCacheProxy::storeBinaryShader(ResourceContentHash effectHash, SceneId sceneId, const std::byte* binaryShaderData, uint32_t binaryShaderDataSize, BinaryShaderFormatID binaryShaderFormat)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.storeBinaryShader({ effectHash.lowPart, effectHash.highPart }, sceneId_t{ sceneId.getValue() }, binaryShaderData, binaryShaderDataSize, binaryShaderFormatId_t{ binaryShaderFormat.getValue() });
    }

    void BinaryShaderCacheProxy::binaryShaderUploaded(ResourceContentHash effectHash, bool success) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_cache.binaryShaderUploaded({ effectHash.lowPart, effectHash.highPart }, success);
    }

    std::once_flag& BinaryShaderCacheProxy::binaryShaderFormatsReported()
    {
        return m_supportedFormatsReported;
    }

}
