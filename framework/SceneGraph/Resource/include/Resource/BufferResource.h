//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_BUFFERRESOURCE_H
#define RAMSES_BUFFERRESOURCE_H

#include "Resource/ResourceBase.h"

namespace ramses_internal
{
    class BufferResource : public ResourceBase
    {
    public:
        BufferResource(EResourceType typeID, UInt32 dataSize, const Byte* data, ResourceCacheFlag cacheFlag, const String& name)
            : ResourceBase(typeID, cacheFlag, name)
        {
            if (data != 0)
            {
                SceneResourceData emptyResourceData(new MemoryBlob(data, dataSize));
                setResourceData(emptyResourceData);
            }
            else
            {
                setResourceData(SceneResourceData(new MemoryBlob(dataSize)), ResourceContentHash::Invalid());
            }
        };

        virtual ~BufferResource()
        {
        }

        const void* getData() const
        {
            return getResourceData().get()->getRawData();
        }
    };
}

#endif
