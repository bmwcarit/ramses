//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/DefaultRendererResourceCache.h"
#include "DefaultRendererResourceCacheImpl.h"

namespace ramses
{
    DefaultRendererResourceCache::DefaultRendererResourceCache(uint32_t maxCacheSizeInBytes)
        : impl(*new DefaultRendererResourceCacheImpl(maxCacheSizeInBytes))
    { }

    DefaultRendererResourceCache::~DefaultRendererResourceCache()
    {
        delete &impl;
    }

    bool DefaultRendererResourceCache::hasResource(rendererResourceId_t resourceId, uint32_t& size) const
    {
        return impl.hasResource(resourceId, size);
    }

    bool DefaultRendererResourceCache::getResourceData(rendererResourceId_t resourceId, uint8_t* buffer, uint32_t bufferSize) const
    {
        return impl.getResourceData(resourceId, buffer, bufferSize);
    }

    bool DefaultRendererResourceCache::shouldResourceBeCached(rendererResourceId_t resourceId, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) const
    {
        return impl.shouldResourceBeCached(resourceId, resourceDataSize, cacheFlag, sceneId);
    }

    void DefaultRendererResourceCache::storeResource(rendererResourceId_t resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId)
    {
        impl.storeResource(resourceId, resourceData, resourceDataSize, cacheFlag, sceneId);
    }

    void DefaultRendererResourceCache::saveToFile(std::string_view filePath) const
    {
        impl.saveToFile(filePath);
    }

    bool DefaultRendererResourceCache::loadFromFile(std::string_view filePath)
    {
        return impl.loadFromFile(filePath);
    }
}
