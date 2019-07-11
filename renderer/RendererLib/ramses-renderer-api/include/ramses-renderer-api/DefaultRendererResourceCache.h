//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_DEFAULTRENDERERRESOURCECACHE_H
#define RAMSES_RENDERERAPI_DEFAULTRENDERERRESOURCECACHE_H

#include "ramses-renderer-api/IRendererResourceCache.h"

namespace ramses
{
    /**
    * @brief The DefaultRendererResourceCache provides a simple example on how the IRendererResourceCache
    *        interface can be implemented. It is only intended as an example, as the optimal implementation
    *        would be very specific to how RAMSES is being utilized.
    */
    class RAMSES_API DefaultRendererResourceCache : public IRendererResourceCache
    {
    public:

        /**
        * @brief Construct a DefaultRendererResourceCache with a given maximum size. Whenever the size
        *        limit is exceeded, items will automatically be unloaded in a FIFO-manner.
        * @param maxCacheSizeInBytes Maximum size of cache content in bytes
        */
        DefaultRendererResourceCache(uint32_t maxCacheSizeInBytes);

        /**
        * @brief Destructor of DefaultRendererResourceCache
        */
        virtual ~DefaultRendererResourceCache() override;

        /**
        * @brief Called by RamsesRenderer to ask for a resource with the given id.
        *
        * @param resourceId Id for the resource.
        * @param[out] size The size of the found resource in bytes. This value is only relevant if
        *                  the resource exists in the cache.
        * @return true if a resource with the given resource id exists in the cache.
        */
        bool hasResource(rendererResourceId_t resourceId, uint32_t& size) const override;

        /**
        * @brief Called by RamsesRenderer to get the resource data associated with a given resource id.
        *        This method will be called immediately after hasResource(...), if the resource exists in the cache.
        *
        * @param resourceId Id for the resource.
        * @param buffer A pre-allocated buffer which the resource data will be copied into.
        * @param bufferSize The size of the pre-allocated buffer in bytes. It should be at least the size of the
        *             requested resource (returned by hasResource(...)).
        * @return true if the resource was copied successfully into the buffer.
        */
        bool getResourceData(rendererResourceId_t resourceId, uint8_t* buffer, uint32_t bufferSize) const override;

        /**
        * @brief Called by RamsesRenderer when a resource was not in the cache and is now available from
        *        other source. The cache is asked if it wants to store a given resource or not. This
        *        avoids the overhead of preparing the resource data in case it is not to be cached.
        *
        * @param resourceId Id for the resource.
        * @param resourceDataSize The size of the resource in bytes.
        * @param cacheFlag The cache flag associated with the resource (set on client side).
        * @param sceneId The id of the first scene which requested the resource. In case of multiple scenes
        *                 using the same resource, only the first scene id is guaranteed to be reported.
        * @return true if the cache wants to store the resource.
        */
        bool shouldResourceBeCached(rendererResourceId_t resourceId, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) const override;

        /**
        * @brief Called by RamsesRenderer with the final resource for storing. This is called
        *        immediately after shouldResourceBeCached(...), if it was requested to be cached.
        *
        * @param resourceId Id for the resource.
        * @param resourceData The resource data which will be copied into the cache.
        * @param resourceDataSize The size of the resource in bytes.
        * @param cacheFlag The cache flag associated with the resource (set on client side).
        * @param sceneId The id of the first scene which requested the resource. In case of multiple scenes
        *                 using the same resource, only the first scene id is guaranteed to be reported.
        */
        void storeResource(rendererResourceId_t resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) override;

        /**
        * @brief Save current content of the cache to a file.
        * @param filePath The file path to save to.
        */
        void saveToFile(const char* filePath) const;

        /**
        * @brief Load all content from a file. It is assumed that the file has been created using saveToFile(...).
        * @param filePath The file path to load from.
        * @return true if the load was successful.
        */
        bool loadFromFile(const char* filePath);

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        DefaultRendererResourceCache(const DefaultRendererResourceCache& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DefaultRendererResourceCache& operator=(const DefaultRendererResourceCache& other) = delete;

        /**
        * Stores internal data for implementation specifics of this class.
        */
        class DefaultRendererResourceCacheImpl& impl;
    };
}

#endif
