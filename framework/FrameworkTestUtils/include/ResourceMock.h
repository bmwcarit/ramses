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
#include "Components/IManagedResourceDeleterCallback.h"

namespace ramses_internal
{
    using namespace testing;

    class ResourceMock : public IResource
    {
    public:
        ResourceMock(ResourceContentHash hash, EResourceType typeId);
        ~ResourceMock() override;

        MOCK_METHOD(const ResourceBlob&, getResourceData, (), (const, override));
        MOCK_METHOD(const CompressedResourceBlob&, getCompressedResourceData, (), (const, override));
        MOCK_METHOD(UInt32, getDecompressedDataSize, (), (const, override));
        MOCK_METHOD(UInt32, getCompressedDataSize, (), (const, override));
        MOCK_METHOD(bool, isCompressedAvailable, (), (const, override));
        MOCK_METHOD(bool, isDeCompressedAvailable, (), (const, override));
        MOCK_METHOD(void, compress, (CompressionLevel), (const, override));
        MOCK_METHOD(void, decompress, (), (const, override));
        MOCK_METHOD(void, setResourceData, (ResourceBlob, const ResourceContentHash&), (override));
        MOCK_METHOD(void, setResourceData, (ResourceBlob), (override));
        MOCK_METHOD(void, setCompressedResourceData, (CompressedResourceBlob, CompressionLevel, uint32_t uncompressedSize, const ResourceContentHash&), (override));
        MOCK_METHOD(void, serializeResourceMetadataToStream, (IOutputStream& output), (const, override));
        MOCK_METHOD(ResourceCacheFlag, getCacheFlag, (), (const, override));
        MOCK_METHOD(const std::string&, getName, (), (const, override));

        [[nodiscard]] const ResourceContentHash& getHash() const override
        {
            return m_hash;
        }

        [[nodiscard]] EResourceType getTypeID() const override
        {
            return m_typeId;
        }

        ResourceContentHash m_hash;
        EResourceType m_typeId;
    };

    class ManagedResourceDeleterCallbackMock : public IManagedResourceDeleterCallback
    {
    public:
        MOCK_METHOD(void, managedResourceDeleted, (const IResource&), (override));
    };

    class ResourceWithDestructorMock : public ResourceMock
    {
    public:
    ResourceWithDestructorMock(ResourceContentHash hash, EResourceType typeId)
        : ResourceMock(hash, typeId)
        {
        }

        MOCK_METHOD(void, Die, ());

        ~ResourceWithDestructorMock() override
        {
            Die();
        }
    };
}
#endif
