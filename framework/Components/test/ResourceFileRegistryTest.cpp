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
    TEST(AResourceFileRegistry, CannotFindNotRegisteredResourceFile)
    {
        ResourceFilesRegistry registry;

        EXPECT_FALSE(registry.hasResourceFile("testfile"));
    }

    TEST(AResourceFileRegistry, FindRegisteredResourceFile)
    {
        PlatformLock lock;
        ResourceStorage storage(lock);
        ResourceFilesRegistry registry;
        ResourceContentHash hash(123, 0);

        ResourceTableOfContents toc;
        String resourceFileName("testfile");
        toc.registerContents(ResourceInfo(EResourceType_VertexArray, hash, 22, 11), 0, 10);

        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream(resourceFileName));
        registry.registerResourceFile(resourceFileStream, toc, storage);

        EXPECT_TRUE(registry.hasResourceFile(resourceFileName));
    }

    TEST(AResourceFileRegistry, keepsHashUsageOfRegisteredResources)
    {
        PlatformLock lock;
        ResourceStorage storage(lock);
        ResourceFilesRegistry registry;
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

    TEST(AResourceFileRegistry, storesResourceFileEntryWithResourceInfo)
    {
        PlatformLock lock;
        ResourceStorage storage(lock);
        ResourceFilesRegistry registry;
        ResourceContentHash hash(123, 0);
        ResourceInfo resInfo(EResourceType_VertexArray, hash, 22, 11);
        UInt32 offset = 11u;
        UInt32 size = 22u;
        ResourceFileInputStreamSPtr resourceFileStream(new ResourceFileInputStream("testfile"));

        ResourceTableOfContents toc;
        toc.registerContents(resInfo, offset, size);

        registry.registerResourceFile(resourceFileStream, toc, storage);

        ResourceFileEntry storedFileEntry;
        BinaryFileInputStream* storedResourceFileStream(nullptr);
        EXPECT_EQ(EStatus_RAMSES_OK, registry.getEntry(hash, storedResourceFileStream, storedFileEntry));
        EXPECT_TRUE(storedResourceFileStream != nullptr);

        EXPECT_EQ(&resourceFileStream->resourceStream, storedResourceFileStream);
        EXPECT_EQ(offset, storedFileEntry.offsetInBytes);
        EXPECT_EQ(size, storedFileEntry.sizeInBytes);
        EXPECT_EQ(resInfo, storedFileEntry.resourceInfo);
    }
}
