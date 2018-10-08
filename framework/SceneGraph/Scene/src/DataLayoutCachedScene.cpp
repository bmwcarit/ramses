//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Scene/DataLayoutCachedScene.h"

namespace ramses_internal
{
    DataLayoutCachedScene::DataLayoutCachedScene(const SceneInfo& sceneInfo)
        : ActionCollectingScene(sceneInfo)
        , m_dataLayoutCache(32u)
    {
    }

    DataLayoutHandle DataLayoutCachedScene::allocateDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        DataLayoutCacheEntry* cacheEntry = nullptr;
        const DataLayoutHandle cachedHandle = findDataLayoutEntry(dataFields, cacheEntry);
        if (cachedHandle.isValid())
        {
            assert(cacheEntry != nullptr);
            cacheEntry->m_usageCount++;
            return cachedHandle;
        }

        return allocateAndCacheDataLayout(dataFields, handle);
    }

    void DataLayoutCachedScene::releaseDataLayout(DataLayoutHandle handle)
    {
        const UInt fieldCount = ActionCollectingScene::getDataLayout(handle).getFieldCount();
        assert(fieldCount < m_dataLayoutCache.size());

        DataLayoutCacheGroup& dataLayouts = m_dataLayoutCache[fieldCount];
        assert(dataLayouts.contains(handle));

        DataLayoutCacheEntry& entry = *dataLayouts.get(handle);
        assert(entry.m_usageCount > 0u);
        entry.m_usageCount--;
        if (entry.m_usageCount == 0u)
        {
            ActionCollectingScene::releaseDataLayout(handle);
            dataLayouts.remove(handle);
        }
    }

    DataLayoutHandle DataLayoutCachedScene::allocateAndCacheDataLayout(const DataFieldInfoVector& dataFields, DataLayoutHandle handle)
    {
        const DataLayoutHandle actualHandle = ActionCollectingScene::allocateDataLayout(dataFields, handle);

        const UInt fieldCount = dataFields.size();
        if (m_dataLayoutCache.size() <= fieldCount)
        {
            m_dataLayoutCache.resize(fieldCount + 1u);
        }
        DataLayoutCacheGroup& dataLayouts = m_dataLayoutCache[fieldCount];

        DataLayoutCacheEntry newEntry;
        newEntry.m_dataFields = dataFields;
        newEntry.m_usageCount = 1u;
        dataLayouts.put(actualHandle, newEntry);

        return actualHandle;
    }

    DataLayoutHandle DataLayoutCachedScene::findDataLayoutEntry(const DataFieldInfoVector& dataFields, DataLayoutCacheEntry*& entryOut)
    {
        const UInt fieldCount = dataFields.size();
        if (fieldCount >= m_dataLayoutCache.size())
        {
            return DataLayoutHandle::Invalid();
        }

        DataLayoutCacheGroup& dataLayouts = m_dataLayoutCache[fieldCount];
        for (auto& entry : dataLayouts)
        {
            if (dataFields == entry.value.m_dataFields)
            {
                entryOut = &entry.value;
                return entry.key;
            }
        }

        return DataLayoutHandle::Invalid();
    }

    UInt32 DataLayoutCachedScene::getNumDataLayoutReferences(DataLayoutHandle handle) const
    {
        const auto it = std::find_if(m_dataLayoutCache.cbegin(), m_dataLayoutCache.cend(), [handle](const DataLayoutCacheGroup& a) { return a.contains(handle); });
        assert(it != m_dataLayoutCache.cend());
        return it->get(handle)->m_usageCount;
    }
}
