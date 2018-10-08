//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererResourceCacheProxy.h"
#include "ramses-renderer-api/IRendererResourceCache.h"

namespace ramses
{
    RendererResourceCacheProxy::RendererResourceCacheProxy(ramses::IRendererResourceCache& cache)
        : m_cache(cache)
    {

    }

    RendererResourceCacheProxy::~RendererResourceCacheProxy()
    {

    }

    bool RendererResourceCacheProxy::hasResource(ramses_internal::ResourceContentHash resourceId, uint32_t& size) const
    {
        return m_cache.hasResource(ConvertInternalTypeToPublic(resourceId), size);
    }

    bool RendererResourceCacheProxy::getResourceData(ramses_internal::ResourceContentHash resourceId, uint8_t* buffer, uint32_t bufferSize) const
    {
        return m_cache.getResourceData(ConvertInternalTypeToPublic(resourceId), buffer, bufferSize);
    }

    bool RendererResourceCacheProxy::shouldResourceBeCached(ramses_internal::ResourceContentHash resourceId, uint32_t resourceDataSize, ramses_internal::ResourceCacheFlag cacheFlag, ramses_internal::SceneId sceneId) const
    {
        return m_cache.shouldResourceBeCached(ConvertInternalTypeToPublic(resourceId), resourceDataSize, ConvertInternalTypeToPublic(cacheFlag), ConvertInternalTypeToPublic(sceneId));
    }

    void RendererResourceCacheProxy::storeResource(ramses_internal::ResourceContentHash resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, ramses_internal::ResourceCacheFlag cacheFlag, ramses_internal::SceneId sceneId)
    {
        m_cache.storeResource(ConvertInternalTypeToPublic(resourceId), resourceData, resourceDataSize, ConvertInternalTypeToPublic(cacheFlag), ConvertInternalTypeToPublic(sceneId));
    }

    ramses::rendererResourceId_t RendererResourceCacheProxy::ConvertInternalTypeToPublic(ramses_internal::ResourceContentHash input)
    {
        ramses::rendererResourceId_t result;
        result.lowPart = input.lowPart;
        result.highPart = input.highPart;

        return result;
    }

    ramses::resourceCacheFlag_t RendererResourceCacheProxy::ConvertInternalTypeToPublic(ramses_internal::ResourceCacheFlag input)
    {
        return ramses::resourceCacheFlag_t(input.getValue());
    }

    ramses::sceneId_t RendererResourceCacheProxy::ConvertInternalTypeToPublic(ramses_internal::SceneId input)
    {
        return ramses::sceneId_t(input.getValue());
    }
}
