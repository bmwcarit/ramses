//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"
#include "Components/ResourceStorage.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/ResourceFilesRegistry.h"

namespace ramses_internal
{
    class AResourceFileRegistry : public ::testing::Test
    {
    public:
        AResourceFileRegistry()
            : lock()
            , stats()
            , storage(lock, stats)
            , registry()
        {}

        PlatformLock lock;
        StatisticCollectionFramework stats;
        ResourceStorage storage;
        ResourceFilesRegistry registry;
    };

    TEST_F(AResourceFileRegistry, CannotFindNotRegisteredResourceFile)
    {
        EXPECT_FALSE(registry.hasResourceFile("testfile"));
    }

    TEST_F(AResourceFileRegistry, FindRegisteredResourceFile)
    {
        ResourceContentHash hash(123, 0);

        ResourceTableOfContents toc;
        String resourceFileName("testfile");
        toc.registerContents(ResourceInfo(EResourceType_VertexArray, hash, 22, 11), 0, 10);

        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream(resourceFileName));
        registry.registerResourceFile(resourceFileStream, toc, storage);

        EXPECT_TRUE(registry.hasResourceFile(resourceFileName));
    }

    TEST_F(AResourceFileRegistry, keepsHashUsageOfRegisteredResources)
    {
        ResourceContentHash hash(123, 0);

        ResourceTableOfContents toc;
        toc.registerContents(ResourceInfo(EResourceType_VertexArray, hash, 22, 11), 0, 10);

        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));
        registry.registerResourceFile(resourceFileStream, toc, storage);

        //get a hash usage and release it, this would trigger deletion of entry
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        hashUsage = ResourceHashUsage();

        EXPECT_EQ(1u, storage.getAllResourceInfo().size());

        registry.unregisterResourceFile(resourceFileStream);

        EXPECT_EQ(0u, storage.getAllResourceInfo().size());
    }

    TEST_F(AResourceFileRegistry, storesResourceFileEntryWithResourceInfo)
    {
        ResourceContentHash hash(123, 0);
        ResourceInfo resInfo(EResourceType_VertexArray, hash, 22, 11);
        UInt32 offset = 11u;
        UInt32 size = 22u;
        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));

        ResourceTableOfContents toc;
        toc.registerContents(resInfo, offset, size);

        registry.registerResourceFile(resourceFileStream, toc, storage);

        ResourceFileEntry storedFileEntry;
        IInputStream* storedResourceFileStream(nullptr);
        EXPECT_EQ(EStatus::Ok, registry.getEntry(hash, storedResourceFileStream, storedFileEntry));
        EXPECT_TRUE(storedResourceFileStream != nullptr);

        EXPECT_EQ(&resourceFileStream->getStream(), storedResourceFileStream);
        EXPECT_EQ(offset, storedFileEntry.offsetInBytes);
        EXPECT_EQ(size, storedFileEntry.sizeInBytes);
        EXPECT_EQ(resInfo, storedFileEntry.resourceInfo);
    }

    TEST_F(AResourceFileRegistry, getsContentsOfResourceFileReturnNullptrIfNoFileKnown)
    {
        EXPECT_FALSE(registry.getContentsOfResourceFile("testfile1"));
    }

    TEST_F(AResourceFileRegistry, getsContentsOfResourceFileReturnNullptrIfFileUnknown)
    {
        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));
        ResourceTableOfContents toc;

        toc.registerContents({ EResourceType_VertexArray, { 2, 2 }, 22, 11 }, 0, 10);
        registry.registerResourceFile(resourceFileStream, toc, storage);

        EXPECT_FALSE(registry.getContentsOfResourceFile("testfile2"));
    }

    TEST_F(AResourceFileRegistry, getsContentOfResourceFile)
    {
        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));
        ResourceTableOfContents toc;
        ResourceInfo info{ EResourceType_VertexArray, { 2, 2 }, 22, 11 };
        toc.registerContents(info, 0, 10);
        registry.registerResourceFile(resourceFileStream, toc, storage);

        ResourceFileInputStreamSPtr resourceFileStream2(new ResourceFileInputStream("testfile2"));
        ResourceTableOfContents toc2;
        toc2.registerContents({ EResourceType_VertexArray, { 3, 3 }, 23, 13 }, 2, 12);
        registry.registerResourceFile(resourceFileStream2, toc2, storage);

        auto content = registry.getContentsOfResourceFile("testfile");
        EXPECT_EQ(content->size(), 1u);
        EXPECT_EQ(content->begin()->key, ResourceContentHash(2, 2));
        EXPECT_EQ(content->begin()->value.fileEntry.resourceInfo, info);
        EXPECT_EQ(content->begin()->value.fileEntry.offsetInBytes, 0u);
        EXPECT_EQ(content->begin()->value.fileEntry.sizeInBytes, 10u);
    }

    TEST_F(AResourceFileRegistry, getsMultipleContentsOfResourceFile)
    {
        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));
        ResourceTableOfContents toc;
        toc.registerContents({ EResourceType_VertexArray, { 2, 2 }, 22, 11 }, 0, 10);
        toc.registerContents({ EResourceType_VertexArray, { 5, 5 }, 22, 11 }, 10, 10);
        toc.registerContents({ EResourceType_VertexArray, { 1, 1 }, 22, 11 }, 20, 10);
        registry.registerResourceFile(resourceFileStream, toc, storage);

        ResourceFileInputStreamSPtr resourceFileStream2(new ResourceFileInputStream("testfile2"));
        ResourceTableOfContents toc2;
        toc2.registerContents({ EResourceType_VertexArray, { 3, 3 }, 23, 13 }, 2, 12);
        toc2.registerContents({ EResourceType_VertexArray, { 5, 5 }, 23, 13 }, 14, 12);
        registry.registerResourceFile(resourceFileStream2, toc2, storage);

        auto content = registry.getContentsOfResourceFile("testfile");
        EXPECT_EQ(content->size(), 3u);
        for (auto const& entry : { ResourceContentHash{ 2, 2 }, ResourceContentHash{ 5, 5 }, ResourceContentHash{ 1, 1 } })
            EXPECT_NE(content->find(entry), content->end());
    }
}
