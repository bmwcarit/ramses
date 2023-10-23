//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/Components/ResourceStorage.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/Components/ResourceFilesRegistry.h"
#include "internal/Components/FileInputStreamContainer.h"

namespace ramses::internal
{
    class AResourceFileRegistry : public ::testing::Test
    {
    public:
        AResourceFileRegistry()
            : storage(lock, stats)
        {
        }

        PlatformLock lock;
        StatisticCollectionFramework stats;
        ResourceStorage storage;
        ResourceFilesRegistry registry;
    };

    TEST_F(AResourceFileRegistry, CannotFindNotRegisteredResourceFile)
    {
        EXPECT_EQ(nullptr, registry.getContentsOfResourceFile(SceneFileHandle(123)));
        EXPECT_EQ(nullptr, registry.getContentsOfResourceFile(SceneFileHandle()));
    }

    TEST_F(AResourceFileRegistry, FindRegisteredResourceFile)
    {
        ResourceContentHash hash(123, 0);

        ResourceTableOfContents toc;
        std::string resourceFileName("testfile");
        toc.registerContents(ResourceInfo(EResourceType::VertexArray, hash, 22, 11), 0, 10);

        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>(resourceFileName));
        const auto handle = registry.registerResourceFile(resourceFileStream, toc, storage);

        EXPECT_NE(nullptr, registry.getContentsOfResourceFile(handle));
    }

    TEST_F(AResourceFileRegistry, keepsHashUsageOfRegisteredResources)
    {
        ResourceContentHash hash(123, 0);

        ResourceTableOfContents toc;
        toc.registerContents(ResourceInfo(EResourceType::VertexArray, hash, 22, 11), 0, 10);

        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>("testfile"));
        const auto handle = registry.registerResourceFile(resourceFileStream, toc, storage);

        //get a hash usage and release it, this would trigger deletion of entry
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        hashUsage = ResourceHashUsage();

        EXPECT_EQ(1u, storage.getAllResourceInfo().size());

        registry.unregisterResourceFile(handle);

        EXPECT_EQ(0u, storage.getAllResourceInfo().size());
    }

    TEST_F(AResourceFileRegistry, storesResourceFileEntryWithResourceInfo)
    {
        ResourceContentHash hash(123, 0);
        ResourceInfo resInfo(EResourceType::VertexArray, hash, 22, 11);
        uint32_t offset = 11u;
        uint32_t size = 22u;
        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>("testfile"));

        ResourceTableOfContents toc;
        toc.registerContents(resInfo, offset, size);

        const SceneFileHandle handle = registry.registerResourceFile(resourceFileStream, toc, storage);

        ResourceFileEntry storedFileEntry;
        IInputStream* storedResourceFileStream(nullptr);
        SceneFileHandle outHandle;
        EXPECT_EQ(EStatus::Ok, registry.getEntry(hash, storedResourceFileStream, storedFileEntry, outHandle));
        EXPECT_TRUE(storedResourceFileStream != nullptr);

        EXPECT_EQ(&resourceFileStream->getStream(), storedResourceFileStream);
        EXPECT_EQ(offset, storedFileEntry.offsetInBytes);
        EXPECT_EQ(size, storedFileEntry.sizeInBytes);
        EXPECT_EQ(resInfo, storedFileEntry.resourceInfo);
        EXPECT_EQ(handle, outHandle);
    }

    TEST_F(AResourceFileRegistry, getsContentsOfResourceFileReturnNullptrIfFileUnknown)
    {
        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>("testfile"));
        ResourceTableOfContents toc;

        toc.registerContents({ EResourceType::VertexArray, { 2, 2 }, 22, 11 }, 0, 10);
        const SceneFileHandle handle = registry.registerResourceFile(resourceFileStream, toc, storage);

        EXPECT_EQ(nullptr, registry.getContentsOfResourceFile(SceneFileHandle(handle.getValue() + 1)));
    }

    TEST_F(AResourceFileRegistry, getsContentOfResourceFile)
    {
        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>("testfile"));
        ResourceTableOfContents toc;
        ResourceInfo info{ EResourceType::VertexArray, { 2, 2 }, 22, 11 };
        toc.registerContents(info, 0, 10);
        const auto handle_1 = registry.registerResourceFile(resourceFileStream, toc, storage);

        InputStreamContainerSPtr resourceFileStream2(std::make_shared<FileInputStreamContainer>("testfile2"));
        ResourceTableOfContents toc2;
        toc2.registerContents({ EResourceType::VertexArray, { 3, 3 }, 23, 13 }, 2, 12);
        const auto handle_2 = registry.registerResourceFile(resourceFileStream2, toc2, storage);
        (void)handle_2;

        auto content = registry.getContentsOfResourceFile(handle_1);
        EXPECT_EQ(content->size(), 1u);
        EXPECT_EQ(content->begin()->key, ResourceContentHash(2, 2));
        EXPECT_EQ(content->begin()->value.fileEntry.resourceInfo, info);
        EXPECT_EQ(content->begin()->value.fileEntry.offsetInBytes, 0u);
        EXPECT_EQ(content->begin()->value.fileEntry.sizeInBytes, 10u);
    }

    TEST_F(AResourceFileRegistry, getsMultipleContentsOfResourceFile)
    {
        InputStreamContainerSPtr resourceFileStream(std::make_shared<FileInputStreamContainer>("testfile"));
        ResourceTableOfContents toc;
        toc.registerContents({ EResourceType::VertexArray, { 2, 2 }, 22, 11 }, 0, 10);
        toc.registerContents({ EResourceType::VertexArray, { 5, 5 }, 22, 11 }, 10, 10);
        toc.registerContents({ EResourceType::VertexArray, { 1, 1 }, 22, 11 }, 20, 10);
        const auto handle_1 = registry.registerResourceFile(resourceFileStream, toc, storage);

        InputStreamContainerSPtr resourceFileStream2(std::make_shared<FileInputStreamContainer>("testfile2"));
        ResourceTableOfContents toc2;
        toc2.registerContents({ EResourceType::VertexArray, { 3, 3 }, 23, 13 }, 2, 12);
        toc2.registerContents({ EResourceType::VertexArray, { 5, 5 }, 23, 13 }, 14, 12);
        const auto handle_2 = registry.registerResourceFile(resourceFileStream2, toc2, storage);
        (void)handle_2;

        auto content = registry.getContentsOfResourceFile(handle_1);
        EXPECT_EQ(content->size(), 3u);
        for (auto const& entry : { ResourceContentHash{ 2, 2 }, ResourceContentHash{ 5, 5 }, ResourceContentHash{ 1, 1 } })
            EXPECT_NE(content->find(entry), content->end());
    }
}
