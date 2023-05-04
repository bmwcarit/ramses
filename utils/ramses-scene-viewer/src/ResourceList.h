//  -------------------------------------------------------------------------
//  Copyright (C) 2021 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "RamsesObjectVector.h"
#include "SceneAPI/ResourceContentHash.h"
#include "ramses-client.h"
#include "RamsesClientImpl.h"
#include "ResourceImpl.h"
#include "SceneDumper.h"
#include <unordered_map>
#include <array>

namespace ramses_internal
{
    class ResourceList
    {
    public:
        ResourceList(ramses::Scene& scene, const ramses::SceneDumper::RamsesObjectImplSet& usedObjects)
            : m_scene(scene)
            , m_usedObjects(usedObjects)
        {
        }

        void reloadIfEmpty();

        void clear();

        auto totalResources() const
        {
            return m_objects.size();
        }

        [[nodiscard]] uint32_t unavailable() const
        {
            return m_unavailable;
        }

        [[nodiscard]] uint32_t compressedSize() const
        {
            return m_compressedSize;
        }

        [[nodiscard]] uint32_t decompressedSize() const
        {
            return m_decompressedSize;
        }

        [[nodiscard]] int getDisplayLimit() const
        {
            return m_displayLimit;
        }

        void setDisplayLimit(int displayLimit)
        {
            assert(displayLimit >= 0);
            m_displayLimit = std::min(displayLimit, static_cast<int>(m_objects.size()));
            updateDisplayedSize();
        }

        /**
         * Returns the index of the current order criteria.
         * The returned index can be used to access an item in #ramses_internal::ResourceList::orderCriteriaItems
         */
        [[nodiscard]] int getOrderCriteriaIndex() const
        {
            return static_cast<int>(m_orderCriteria);
        }

        /**
         * Sets the current order criteria.
         * The provided index must be in the range 0 .. orderCriteriaItems.size() - 1
         */
        void setOrderCriteriaIndex(int orderCriteria)
        {
            const auto criteria = static_cast<OrderCriteria>(orderCriteria);
            assert(criteria >= OrderCriteria::Uncompressed && criteria <= OrderCriteria::Compressed);
            m_orderCriteria = criteria;
            sort();
            updateDisplayedSize();
        }

        auto begin() const
        {
            return m_objects.begin();
        }

        auto end() const
        {
            return m_objects.begin() + m_displayLimit;
        }

        auto equal_range(const ResourceContentHash& hash) const
        {
            return m_hashLookup.equal_range(hash);
        }

        [[nodiscard]] uint32_t getDisplayedSize() const
        {
            return m_displayedSize;
        }

        const std::array<const char*, 2> orderCriteriaItems = {"uncompressed size", "compressed size"};

    private:
        // needs to match with orderCriteriaItems
        enum class OrderCriteria
        {
            Uncompressed = 0,
            Compressed = 1,
        };

        static UInt32 GetCompressedSize(const ramses_internal::ManagedResource& resource)
        {
            const auto compressed = resource->getCompressedDataSize();
            return (compressed == 0) ? resource->getDecompressedDataSize() : compressed;
        }

        void updateDisplayedSize();

        void sort();

        ramses::Scene& m_scene;
        const ramses::SceneDumper::RamsesObjectImplSet& m_usedObjects;

        ramses::RamsesObjectVector m_objects;

        std::unordered_multimap<ramses_internal::ResourceContentHash, ramses::Resource*> m_hashLookup;

        int m_displayLimit  = 1000;
        OrderCriteria m_orderCriteria = OrderCriteria::Uncompressed;

        uint32_t m_compressedSize = 0U;
        uint32_t m_decompressedSize = 0U;
        uint32_t m_unavailable      = 0U;

        uint32_t m_displayedSize = 0u;
    };

    inline void ResourceList::sort()
    {
        auto cmp = [&](ramses::RamsesObject* a, ramses::RamsesObject* b) {
            ManagedResource resourceA = m_scene.getRamsesClient().m_impl.getResource(static_cast<ramses::Resource*>(a)->m_impl.getLowlevelResourceHash());
            ManagedResource resourceB = m_scene.getRamsesClient().m_impl.getResource(static_cast<ramses::Resource*>(b)->m_impl.getLowlevelResourceHash());
            if (m_orderCriteria == OrderCriteria::Compressed)
            {
                const auto sizeA = resourceA ? GetCompressedSize(resourceA) : 0u;
                const auto sizeB = resourceB ? GetCompressedSize(resourceB) : 0u;
                return sizeA > sizeB;
            }
            else
            {
                const auto sizeA = resourceA ? resourceA->getDecompressedDataSize() : 0u;
                const auto sizeB = resourceB ? resourceB->getDecompressedDataSize() : 0u;
                return sizeA > sizeB;
            }
        };
        std::sort(m_objects.begin(), m_objects.end(), cmp);
    }

    inline void ResourceList::reloadIfEmpty()
    {
        if (m_objects.empty())
        {
            const auto& reg = m_scene.m_impl.getObjectRegistry();
            reg.getObjectsOfType(m_objects, ramses::ERamsesObjectType_Resource);
            m_displayLimit = std::min(m_displayLimit, static_cast<int>(m_objects.size()));

            sort();

            for (auto it : m_objects)
            {
                auto hlResource = static_cast<ramses::Resource*>(it);
                auto resource   = m_scene.getRamsesClient().m_impl.getResource(hlResource->m_impl.getLowlevelResourceHash());
                m_hashLookup.insert({hlResource->m_impl.getLowlevelResourceHash(), hlResource});
                if (resource)
                {
                    if (m_usedObjects.contains(&hlResource->m_impl))
                    {
                        // don't count duplicates
                        m_compressedSize += GetCompressedSize(resource);
                        m_decompressedSize += resource->getDecompressedDataSize();
                    }
                }
                else
                    ++m_unavailable;
            }
            updateDisplayedSize();
        }
    }

    inline void ResourceList::updateDisplayedSize()
    {
        m_displayedSize = 0u;
        std::for_each(begin(), end(), [&](ramses::RamsesObject* obj) {
            auto hlResource = static_cast<ramses::Resource*>(obj);
            auto resource   = m_scene.getRamsesClient().m_impl.getResource(hlResource->m_impl.getLowlevelResourceHash());
            if (resource && m_usedObjects.contains(&hlResource->m_impl))
            {
                // don't count duplicates
                m_displayedSize += (m_orderCriteria == OrderCriteria::Compressed) ? GetCompressedSize(resource) : resource->getDecompressedDataSize();
            }
        });
    }

    inline void ResourceList::clear()
    {
        m_objects.clear();
        m_hashLookup.clear();
        m_compressedSize = 0U;
        m_decompressedSize = 0U;
        m_unavailable      = 0U;
        m_displayedSize = 0U;
    }
}
