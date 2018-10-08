//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gmock/gmock.h"
#include "SceneAPI/ResourceContentHash.h"
#include "Components/ResourceTableOfContents.h"
#include "Utils/File.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryFileInputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Components/ManagedResource.h"
#include "Components/ResourcePersistation.h"

using namespace testing;

namespace ramses_internal
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
        ResourceInfo resourceInfo(EResourceType_IndexArray, hash, 22u, 11u);
        UInt32 offsetWithinFile = 123u;
        UInt32 resourceSizeInBytes = 456u;
        toc.registerContents(resourceInfo, offsetWithinFile, resourceSizeInBytes);
        EXPECT_TRUE(toc.containsResource(hash));

        const TableOfContentsMap& fileContents = toc.getFileContents();
        EXPECT_EQ(1u, fileContents.count());
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
        ResourceInfo resourceInfoA(EResourceType_IndexArray, hashA, 22u, 11u);
        UInt32 offsetInBytesA = 123u;
        UInt32 sizeInBytesA = 456u;
        toc.registerContents(resourceInfoA, offsetInBytesA, sizeInBytesA);
        ResourceContentHash hashB(234678u, 0);
        ResourceInfo resourceInfoB(EResourceType_Texture2D, hashB, 44u, 33u);
        UInt32 offsetInBytesB = 987u;
        UInt32 sizeInBytesB = 654u;
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
        const Bool returnValue = loadedTOC.readTOCPosAndTOCFromStream(instream);
        ASSERT_FALSE(returnValue);
    }

}
