//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_DEFAULTRENDERERRESOURCECACHEIMPL_H
#define RAMSES_RENDERERAPI_DEFAULTRENDERERRESOURCECACHEIMPL_H

#include "Collections/Vector.h"
#include "PlatformAbstraction/PlatformSharedPointer.h"
#include "RendererAPI/Types.h"
#include "ramses-renderer-api/Types.h"
#include "ramses-renderer-api/IRendererResourceCache.h"
#include <deque>

namespace ramses
{
    class IDataFunctor;

    class DefaultRendererResourceCacheImpl : public IRendererResourceCache
    {
    public:
        DefaultRendererResourceCacheImpl(uint32_t maxCacheSizeInBytes);
        virtual ~DefaultRendererResourceCacheImpl();

        bool virtual hasResource(rendererResourceId_t resourceId, uint32_t& size) const override;
        bool virtual getResourceData(rendererResourceId_t resourceId, uint8_t* buffer, uint32_t bufferSize) const override;
        bool virtual shouldResourceBeCached(rendererResourceId_t resourceId, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) const override;
        void virtual storeResource(rendererResourceId_t resourceId, const uint8_t* resourceData, uint32_t resourceDataSize, resourceCacheFlag_t cacheFlag, sceneId_t sceneId) override;

        void saveToFile(const char* filePath) const;
        bool loadFromFile(const char* filePath);

        struct FileHeader
        {
            uint32_t fileSize;
            uint32_t transportVersion;
            uint32_t checksum;
        };

    private:

        typedef std::vector<uint8_t> ByteVector;

        bool storeResourceInternal(rendererResourceId_t resourceId, ByteVector& data);
        void clear();
        void makeSpaceForNewItem(uint32_t newItemSizeInBytes);
        void removeOldestItem();

        void iterateDataToSave(IDataFunctor& functor) const;

        struct ResourceData
        {
            rendererResourceId_t resourceId;
            ByteVector data;
        };

        typedef std::deque<ResourceData> ResourceDataTable;

        ResourceDataTable m_resourceData;
        uint32_t m_maxCacheSizeInBytes;
        uint32_t m_currentCacheSizeInBytes;
    };
}

#endif
