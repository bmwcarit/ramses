//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererResourceManagerUtils.h"
#include "RendererAPI/IRendererResourceCache.h"
#include "Utils/BinaryInputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "Resource/IResource.h"
#include "Components/SingleResourceSerialization.h"

namespace ramses_internal
{
    ManagedResource RendererResourceManagerUtils::TryLoadResource(ResourceContentHash resourceId, UInt32 resourceSize, const IRendererResourceCache* cache)
    {
        std::vector<Byte> readBuffer(resourceSize);

        cache->getResourceData(resourceId, readBuffer.data(), resourceSize);

        BinaryInputStream resourceStream(readBuffer.data());
        const IResource* resourceObject = SingleResourceSerialization::DeserializeResource(resourceStream, resourceId);

        static ResourceDeleterCallingCallback deleter = ResourceDeleterCallingCallback(DefaultManagedResourceDeleterCallback::GetInstance());

        if (resourceObject)
        {
            return ManagedResource(*resourceObject, deleter);
        }
        else
        {
            return ManagedResource();
        }
    }

    void RendererResourceManagerUtils::StoreResource(IRendererResourceCache* cache, const IResource* resource, SceneId sceneId)
    {
        assert(cache);
        assert(resource);

        const UInt32 serializedSize = SingleResourceSerialization::SizeOfSerializedResource(*resource);
        const ResourceContentHash resHash = resource->getHash();
        const ResourceCacheFlag cacheFlag = resource->getCacheFlag();

        // First check if the cache wants the resource, before going through the work of preparing it for storage.
        if (cache->shouldResourceBeCached(resHash, serializedSize, cacheFlag, sceneId))
        {
            BinaryOutputStream serializerStream(serializedSize);
            SingleResourceSerialization::SerializeResource(serializerStream, *resource);
            assert(serializerStream.getSize() == serializedSize);

            cache->storeResource(resHash, reinterpret_cast<const uint8_t*>(serializerStream.getData()), serializedSize, cacheFlag, sceneId);
        }
    }
}
