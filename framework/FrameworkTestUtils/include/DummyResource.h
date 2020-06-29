//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DUMMYRESOURCE_H
#define RAMSES_DUMMYRESOURCE_H

#include "Resource/ResourceBase.h"

namespace ramses_internal
{
    class DummyResource : public ResourceBase
    {
    public:
        DummyResource(const ResourceContentHash& hash, EResourceType typeId, ResourceCacheFlag cacheFlag = ResourceCacheFlag(123u), const String& name = String())
            : ResourceBase(typeId, cacheFlag, name)
            , m_hash(hash)
        {
        }

        virtual const ResourceContentHash& getHash() const override
        {
            return m_hash;
        }

        virtual UInt32 getCompressedDataSize() const override
        {
            return 0;
        }

        virtual UInt32 getDecompressedDataSize() const override
        {
            return 42;
        }

        virtual void serializeResourceMetadataToStream(IOutputStream& /*output*/) const override
        {
        }

    private:
        ResourceContentHash m_hash;
    };
}

#endif
