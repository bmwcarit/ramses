//  -------------------------------------------------------------------------
//  Copyright (C) 2012 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RESOURCEMOCK_H
#define RAMSES_RESOURCEMOCK_H

#include "Resource/IResource.h"
#include "gmock/gmock.h"
#include "Transfer/ResourceTypes.h"
#include "Components/IManagedResourceDeleterCallback.h"

namespace ramses_internal
{
    using namespace testing;

    class ResourceMock : public IResource
    {
    public:
        ResourceMock(ResourceContentHash hash, EResourceType typeId);
        virtual ~ResourceMock();

        MOCK_CONST_METHOD0(getResourceData, const SceneResourceData&());
        MOCK_CONST_METHOD0(getCompressedResourceData, const CompressedSceneResourceData&());
        MOCK_CONST_METHOD0(getDecompressedDataSize, UInt32());
        MOCK_CONST_METHOD0(getCompressedDataSize, UInt32());
        MOCK_CONST_METHOD0(isCompressedAvailable, ramses_internal::Bool());
        MOCK_CONST_METHOD0(isDeCompressedAvailable, ramses_internal::Bool());
        MOCK_CONST_METHOD1(compress, void(CompressionLevel));
        MOCK_CONST_METHOD0(decompress, void());
        MOCK_METHOD2(setResourceData, void(const SceneResourceData&, const ResourceContentHash&));
        MOCK_METHOD1(setResourceData, void(const SceneResourceData&));
        MOCK_METHOD2(setCompressedResourceData, void(const CompressedSceneResourceData&, const ResourceContentHash&));
        MOCK_CONST_METHOD1(serializeResourceMetadataToStream, void(IOutputStream& output));
        MOCK_CONST_METHOD0(getCacheFlag, ResourceCacheFlag());
        MOCK_CONST_METHOD0(getName, const String&());

        virtual const ResourceContentHash& getHash() const
        {
            return m_hash;
        }

        virtual EResourceType getTypeID() const
        {
            return m_typeId;
        }

        ResourceContentHash m_hash;
        EResourceType m_typeId;
    };

    class ManagedResourceDeleterCallbackMock : public IManagedResourceDeleterCallback
    {
    public:
        MOCK_METHOD1(managedResourceDeleted, void(const IResource&));
    };

    class ResourceWithDestructorMock : public ResourceMock
    {
    public:
    ResourceWithDestructorMock(ResourceContentHash hash, EResourceType typeId)
        : ResourceMock(hash, typeId)
        {
        }

        MOCK_METHOD0(Die, void());

        virtual ~ResourceWithDestructorMock()
        {
            Die();
        }
    };
}
#endif
