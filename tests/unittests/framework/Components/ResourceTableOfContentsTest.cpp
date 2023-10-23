//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "internal/SceneGraph/SceneAPI/ResourceContentHash.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/Core/Utils/File.h"
#include "internal/Core/Utils/BinaryFileOutputStream.h"
#include "internal/Core/Utils/BinaryFileInputStream.h"
#include "internal/PlatformAbstraction/PlatformMemory.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/ResourcePersistation.h"

using namespace testing;

namespace ramses::internal
{
    TEST(AResourceTableOfContents, doesNotContainUnregisteredHash)
    {
        ResourceContentHash hash;
        ResourceTableOfContents toc;
        ASSERT_FALSE(toc.containsResource(hash));
    }

    TEST(AResourceTableOfContents, containsRegisteredContents)
    {
        ResourceContentHash hash(4711u, 0);
        ResourceTableOfContents toc;
        ResourceInfo resourceInfo(EResourceType::IndexArray, hash, 22u, 11u);
        uint32_t offsetWithinFile = 123u;
        uint32_t resourceSizeInBytes = 456u;
        toc.registerContents(resourceInfo, offsetWithinFile, resourceSizeInBytes);
        EXPECT_TRUE(toc.containsResource(hash));

        const TableOfContentsMap& fileContents = toc.getFileContents();
        EXPECT_EQ(1u, fileContents.size());
        ResourceFileEntry* savedEntry = fileContents.get(hash);
        EXPECT_EQ(resourceInfo, savedEntry->resourceInfo);
        EXPECT_EQ(offsetWithinFile, savedEntry->offsetInBytes);
        EXPECT_EQ(resourceSizeInBytes, savedEntry->sizeInBytes);
    }

    TEST(AResourceTableOfContents, canWriteAndReadTableOfContentsWithStreams)
    {
        File tempFile("onDemandResourceFile");

        ResourceTableOfContents toc;
        ResourceContentHash hashA(4711u, 0);
        ResourceInfo resourceInfoA(EResourceType::IndexArray, hashA, 22u, 11u);
        uint32_t offsetInBytesA = 123u;
        uint32_t sizeInBytesA = 456u;
        toc.registerContents(resourceInfoA, offsetInBytesA, sizeInBytesA);
        ResourceContentHash hashB(234678u, 0);
        ResourceInfo resourceInfoB(EResourceType::Texture2D, hashB, 44u, 33u);
        uint32_t offsetInBytesB = 987u;
        uint32_t sizeInBytesB = 654u;
        toc.registerContents(resourceInfoB, offsetInBytesB, sizeInBytesB);

        {
            BinaryFileOutputStream outstream(tempFile);
            toc.writeTOCToStream(outstream);
        }

        BinaryFileInputStream instream(tempFile);

        ResourceTableOfContents loadedTOC;
        loadedTOC.readTOCPosAndTOCFromStream(instream);
        ASSERT_TRUE(loadedTOC.containsResource(hashA));
        ASSERT_TRUE(loadedTOC.containsResource(hashB));

        const ResourceFileEntry loadedEntryA = loadedTOC.getEntryForHash(hashA);
        ASSERT_EQ(resourceInfoA, loadedEntryA.resourceInfo);
        ASSERT_EQ(offsetInBytesA, loadedEntryA.offsetInBytes);
        ASSERT_EQ(sizeInBytesA, loadedEntryA.sizeInBytes);

        const ResourceFileEntry loadedEntryB = loadedTOC.getEntryForHash(hashB);
        ASSERT_EQ(offsetInBytesB, loadedEntryB.offsetInBytes);
        ASSERT_EQ(sizeInBytesB, loadedEntryB.sizeInBytes);
    }

    TEST(AResourceTableOfContents, returnFalseIfReadingTableOfContentsWasNotSuccessfull)
    {
        File tempFile("EmptyOrWrongResourceFile");
        tempFile.createFile();

        BinaryFileInputStream instream(tempFile);

        ResourceTableOfContents loadedTOC;
        const bool returnValue = loadedTOC.readTOCPosAndTOCFromStream(instream);
        ASSERT_FALSE(returnValue);
    }

}
