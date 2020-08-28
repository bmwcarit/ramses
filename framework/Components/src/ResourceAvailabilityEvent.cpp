//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include <cassert>
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "Components/ResourceAvailabilityEvent.h"
#include "Components/ERendererToClientEventType.h"

namespace ramses_internal
{
    template<class T>
    inline void write(std::vector<Byte>::iterator& where, T const& value)
    {
        std::memcpy(&(*where), &value, sizeof(T));
        where += sizeof(T);
    }

    template<class T>
    inline void read(std::vector<Byte>::const_iterator& where, T& value)
    {
        std::memcpy(&value, &(*where), sizeof(T));
        where += sizeof(T);
    }

    void ResourceAvailabilityEvent::readFromBlob(std::vector<Byte> const& blob)
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

    void ResourceAvailabilityEvent::writeToBlob(std::vector<Byte>& blob) const
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
