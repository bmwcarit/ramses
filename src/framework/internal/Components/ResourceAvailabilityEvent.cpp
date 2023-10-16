//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>
#include "internal/Core/Utils/BinaryInputStream.h"
#include "internal/Core/Utils/BinaryOutputStream.h"
#include "internal/Components/ResourceAvailabilityEvent.h"
#include "internal/Components/ERendererToClientEventType.h"

namespace ramses::internal
{
    template<class T>
    inline void write(std::vector<std::byte>::iterator& where, T const& value)
    {
        std::memcpy(&(*where), &value, sizeof(T));
        where += sizeof(T);
    }

    template<class T>
    inline void read(std::vector<std::byte>::const_iterator& where, T& value)
    {
        // TODO static_cast is only needed to fix GCC bug until version 11.1
        std::memcpy(&value, static_cast<const void*>(&(*where)), sizeof(T));
        where += sizeof(T);
    }

    void ResourceAvailabilityEvent::readFromBlob(std::vector<std::byte> const& blob)
    {
        auto it = blob.cbegin();
        ERendererToClientEventType eventType;
        read(it, eventType);
        assert (eventType == ERendererToClientEventType::ResourcesAvailableAtRendererEvent);
        uint64_t numberOfResources = 0;
        read(it, sceneid);
        read(it, numberOfResources);
        for (uint64_t i = 0; i < numberOfResources; ++i)
        {
            ResourceContentHash hash;
            read(it, hash.lowPart);
            read(it, hash.highPart);
            availableResources.push_back(hash);
        }

        assert(it == blob.cend());
    }

    void ResourceAvailabilityEvent::writeToBlob(std::vector<std::byte>& blob) const
    {
        assert(blob.empty());

        blob.resize(sizeof(ERendererToClientEventType) + sizeof(SceneId) + sizeof(uint64_t) + sizeof(ResourceContentHash) * availableResources.size());
        auto it = blob.begin();
        write(it, ERendererToClientEventType::ResourcesAvailableAtRendererEvent);
        write(it, sceneid);
        write(it, static_cast<uint64_t>(availableResources.size()));
        for (auto& res: availableResources)
        {
            write(it, res.lowPart);
            write(it, res.highPart);
        }

        assert(it == blob.end());
    }

}
