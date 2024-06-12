//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ResourceComponent.h"
#include "ResourceMock.h"
#include "gmock/gmock.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "internal/SceneGraph/Resource/TextureResource.h"
#include "DummyResource.h"
#include "internal/Components/ResourceDeleterCallingCallback.h"
#include "internal/Components/SceneUpdate.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "internal/Communication/TransportCommon/SceneUpdateSerializer.h"
#include "internal/Communication/TransportCommon/SceneUpdateStreamDeserializer.h"
#include "internal/Components/FileInputStreamContainer.h"
#include "InputStreamMock.h"
#include "InputStreamContainerMock.h"

namespace ramses::internal
{
    using namespace testing;
    class ResourceComponentTestBase : public testing::Test
    {
    public:
        ResourceComponentTestBase()
            : resourceFileName("test.resource")
            , anotherResFileName("test2.resource")
            , subDirName("test/")
            , equallyNamedResFileInSubDir(subDirName + resourceFileName)
        {
        }

        virtual ResourceComponent& getResourceComponent() = 0;

        ~ResourceComponentTestBase() override
        {
            deleteTestResourceFile();
        }

        static IResource* CreateTestResource(float someValue = 0.0f, size_t extraSize = 0)
        {
            std::vector<float> vertexData = {
                0.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 1.0f, someValue
            };
            vertexData.resize(vertexData.size() + extraSize*3);

            auto* resource = new ArrayResource(EResourceType::VertexArray, 3 + static_cast<uint32_t>(extraSize), EDataType::Vector3F, vertexData.data(), "resName");
            return resource;
        }

        static ResourceContentHashVector HashesFromManagedResources(const ManagedResourceVector& vec)
        {
            ResourceContentHashVector hashes;
            for (const auto& res : vec)
                hashes.push_back(res->getHash());
            return hashes;
        }

        ResourceContentHashVector writeMultipleTestResourceFile(uint32_t num, size_t extraSize = 0, bool compress = false)
        {
            ResourceComponent& localResourceComponent = getResourceComponent();

            ManagedResourceVector managedResourceVec;
            ResourceContentHashVector hashes;

            for (uint32_t i = 0; i < num; ++i)
            {
                IResource* resource = CreateTestResource(static_cast<float>(i), extraSize);
                ManagedResource managedResource = localResourceComponent.manageResourceDeletionAllowed(*resource);
                managedResourceVec.push_back(managedResource);
                hashes.push_back(managedResource->getHash());
            }

            {
                File resourceFile(resourceFileName);
                BinaryFileOutputStream resourceOutputStream(resourceFile);
                ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, managedResourceVec, compress);
            }

            ResourceTableOfContents resourceFileToc;
            InputStreamContainerSPtr inputStream(std::make_shared<FileInputStreamContainer>(resourceFileName));
            resourceFileToc.readTOCPosAndTOCFromStream(inputStream->getStream());
            localResourceComponent.addResourceFile(inputStream, resourceFileToc);

            return hashes;
        }

        ResourceContentHash writeTestResourceFile()
        {
            return writeMultipleTestResourceFile(1).front();
        }

        void deleteTestResourceFile()
        {
            File resourceFile(resourceFileName);
            if (resourceFile.exists())
                resourceFile.remove();

            File resourceFile2(anotherResFileName);
            if (resourceFile2.exists())
                resourceFile2.remove();

            File resourceFile3(equallyNamedResFileInSubDir);
            if (resourceFile3.exists())
                resourceFile3.remove();

            File subDir(subDirName);
            if (subDir.isDirectory())
                subDir.remove();
        }

        static void ExpectResourceSizeCalls(const ResourceMock* resource)
        {
            EXPECT_CALL(*resource, getCompressedDataSize()).Times(AnyNumber());
            EXPECT_CALL(*resource, getDecompressedDataSize()).Times(AnyNumber());
        }

        std::vector<std::vector<std::byte>> serializeResources(const ManagedResourceVector& resVec, uint32_t chunkSize = 100000)
        {
            SceneUpdate update{SceneActionCollection(), resVec, {}};
            return TestSerializeSceneUpdateToVectorChunked(SceneUpdateSerializer(update, sceneStatistics, EFeatureLevel_Latest), chunkSize);
        }

    protected:
        PlatformLock frameworkLock;
        const std::string resourceFileName;
        const std::string anotherResFileName;
        const std::string subDirName;
        const std::string equallyNamedResFileInSubDir;
        StatisticCollectionScene sceneStatistics;
    };

    class AResourceComponentTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentTest()
            : localResourceComponent(statistics, frameworkLock, EFeatureLevel_Latest)
        {}

        ResourceComponent& getResourceComponent() override
        {
            return localResourceComponent;
        }

        struct TestResourceData
        {
            std::vector<std::byte> data;
            ResourceContentHash hash;
        };

        TestResourceData CreateTestResourceData()
        {
            const ManagedResourceVector resVec{ ManagedResource{ CreateTestResource(), deleter } };
            const IResource* res = resVec.front().get();
            const ResourceContentHash hash = res->getHash();

            const auto dataVec = serializeResources(resVec);
            assert(1u == dataVec.size());

            return{ dataVec[0], hash };
        }

    protected:
        StatisticCollectionFramework statistics;
        ResourceComponent localResourceComponent;
        ResourceDeleterCallingCallback deleter;
    };

    class AResourceComponentWithThreadedTaskExecutorTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentWithThreadedTaskExecutorTest()
            : localResourceComponent(statistics, frameworkLock, EFeatureLevel_Latest)
        {}

        ResourceComponent& getResourceComponent() override
        {
            return localResourceComponent;
        }
    protected:
        StatisticCollectionFramework statistics;
        ResourceComponent localResourceComponent;
    };

    TEST_F(AResourceComponentTest, CanGetAlreadyManagedResource)
    {
        ResourceContentHash dummyResourceHash(47, 0);
        auto* dummyResource = new DummyResource(dummyResourceHash, EResourceType::VertexArray);
        ManagedResource dummyManagedResource = localResourceComponent.manageResource(*dummyResource);

        ManagedResource fetchedResource = localResourceComponent.getResource(dummyResourceHash);
        EXPECT_EQ(dummyResourceHash, fetchedResource->getHash());
        EXPECT_EQ(dummyResource, fetchedResource.get());
    }

    TEST_F(AResourceComponentTest, ReloadFromSameResourceFile_doesntLeak)
    {
        // create resource file with restresource and register it
        ResourceContentHash       resourceHash = writeTestResourceFile();

        for (int i = 0; i < 10; i++)
        {
            // make sure managed resource created in writeTestResourceFile() was freed,
            // so resolveResources() has to trigger loading it from file
            EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());
            ManagedResource resource = localResourceComponent.loadResource(resourceHash);
            ASSERT_EQ(resourceHash, resource->getHash());
            resource = ManagedResource();

            ASSERT_EQ(0u, localResourceComponent.getResources().size());
        }
    }

    TEST_F(AResourceComponentTest, ReloadFromSameResourceFileWhileKeepingHashReference_doesntLeak)
    {
        // create resource file with restresource and register it
        ResourceContentHash       resourceHash = writeTestResourceFile();
        ResourceHashUsage hashUsage = localResourceComponent.getResourceHashUsage(resourceHash);

        for (int i = 0; i < 10; i++)
        {
            // make sure managed resource created in writeTestResourceFile() was freed,
            // so resolveResources() has to trigger loading it from file
            EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());
            ManagedResource resource = localResourceComponent.loadResource(resourceHash);
            ASSERT_EQ(resourceHash, resource->getHash());
            resource = ManagedResource();

            ASSERT_EQ(0u, localResourceComponent.getResources().size());
        }
    }

    TEST_F(AResourceComponentTest, CanDeleteResourcesLoadedFromFile)
    {
        ResourceContentHash resourceHash = writeTestResourceFile();
        ManagedResource resource = localResourceComponent.loadResource(resourceHash);

        ResourceHashUsage usage = localResourceComponent.getResourceHashUsage(resourceHash);

        // letting go of the managed resource, but keeping the hash usage allows deletion, when resource came from a file
        resource = ManagedResource();
        EXPECT_EQ(localResourceComponent.getResources().size(), 0u);
    }

    TEST_F(AResourceComponentTest, AfterInitResourcesAreEmpty)
    {
        ManagedResourceVector currentResources = localResourceComponent.getResources();
        ASSERT_EQ(0u, currentResources.size());
    }

    TEST_F(AResourceComponentTest, ManageResourcesTakesOwnership)
    {
        auto* resource = new ResourceWithDestructorMock(ResourceContentHash(47, 0), EResourceType::VertexArray);
        {
            ExpectResourceSizeCalls(resource);
            EXPECT_CALL(*resource, Die());
            ManagedResource managedRes = localResourceComponent.manageResource(*resource);
        }// raw pointer was transferred into component, and is deleted when last managed pointer goes away

        Mock::VerifyAndClearExpectations(resource); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
    }

    TEST_F(AResourceComponentTest, canReturnAllResourcesManaged)
    {
        auto* resource = new DummyResource(ResourceContentHash(47, 0), EResourceType::VertexArray);
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);

        auto* resource2 = new DummyResource(ResourceContentHash(49, 0), EResourceType::VertexArray);
        ManagedResource managedRes2 = localResourceComponent.manageResource(*resource2);

        ManagedResourceVector allResources = localResourceComponent.getResources();

        EXPECT_EQ(2u, allResources.size());
        EXPECT_THAT(allResources, testing::UnorderedElementsAre(managedRes, managedRes2));
    }

    TEST_F(AResourceComponentTest, canReturnHashUsageReferringToKnownResource)
    {
        const ResourceContentHash hash(47, 0);
        auto* resource = new ResourceMock(hash, EResourceType::VertexArray);
        ExpectResourceSizeCalls(resource);
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);

        ResourceHashUsage hashUsage = localResourceComponent.getResourceHashUsage(hash);

        EXPECT_EQ(hash, hashUsage.getHash());
    }

    std::pair<SceneFileHandle, ResourceContentHash> setupTest(std::string const& resourceFileName, ResourceComponent& localResourceComponent)
    {
        // setup test
        const float vertexData[] = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };
        auto* resource = new ArrayResource(EResourceType::VertexArray, 3, EDataType::Vector3F, vertexData, "resName");
        ResourceContentHash hash = resource->getHash();
        {
            File resourceFile(resourceFileName);
            BinaryFileOutputStream resourceOutputStream(resourceFile);

            NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
            ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);
            ManagedResource resource1{ resource, deleterMock };

            ManagedResourceVector resources;
            resources.push_back(resource1);

            ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, resources, false);

            delete resource;
        }

        SceneFileHandle handle;
        {
            ResourceTableOfContents resourceFileToc;
            InputStreamContainerSPtr inputStream(std::make_shared<FileInputStreamContainer>(resourceFileName));
            resourceFileToc.readTOCPosAndTOCFromStream(inputStream->getStream());

            handle = localResourceComponent.addResourceFile(inputStream, resourceFileToc);
        }

        return {handle, hash};
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFile)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();
        ManagedResource fromfile = localResourceComponent.loadResource(handleAndHash.second);
        EXPECT_TRUE(fromfile);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_TRUE(fromfile);
        fromfile = ManagedResource();
        EXPECT_FALSE(fromfile);
    }

    TEST_F(AResourceComponentTest, removingFileWillMakeUnloadedResourceUnusable)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_FALSE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_FALSE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canLoadResourceBecauseOfHashUsage)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(handleAndHash.first);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canLoadResourceBecauseOfHashUsageAndRemoveFile)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(handleAndHash.first);
        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_FALSE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_FALSE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canLoadResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_TRUE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canLoadAndRemoveAndKeepsResourceBecauseOfResourceUsage)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(handleAndHash.second);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(handleAndHash.first);
        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_FALSE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndDeletesResourceBecauseNoUsage)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));

        EXPECT_FALSE(localResourceComponent.getResource(handleAndHash.second));
        EXPECT_FALSE(localResourceComponent.loadResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndKeepsResourceBecauseAvailableInOtherFile)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);
        auto handleAndHash2 = setupTest(anotherResFileName, localResourceComponent);
        EXPECT_EQ(handleAndHash.second, handleAndHash2.second);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHash2.first));

        EXPECT_FALSE(localResourceComponent.getResource(handleAndHash.second));
        auto res = localResourceComponent.loadResource(handleAndHash.second);
        EXPECT_TRUE(res);
        EXPECT_TRUE(localResourceComponent.getResource(handleAndHash.second));
        res = {};
        EXPECT_FALSE(localResourceComponent.getResource(handleAndHash.second));
    }

    TEST_F(AResourceComponentTest, canResolveLocalResource)
    {
        IResource* resource = CreateTestResource();
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);
        ResourceContentHashVector hashes {resource->getHash()};
        ManagedResourceVector resolved = localResourceComponent.resolveResources(hashes);

        ASSERT_EQ(1u, resolved.size());
        EXPECT_TRUE(resolved[0]->getHash() == hashes[0]);
    }

    TEST_F(AResourceComponentTest, canResolveResourcesFromFile)
    {
        ResourceContentHashVector hashes = writeMultipleTestResourceFile(2, 2000, true);

        ManagedResourceVector resolved = localResourceComponent.resolveResources(hashes);

        ASSERT_EQ(2u, resolved.size());
        EXPECT_TRUE(resolved[0]->getHash() == hashes[0]);
        EXPECT_TRUE(resolved[1]->getHash() == hashes[1]);
    }

    TEST_F(AResourceComponentTest, returnsEmptyResourceVectorOnEmptyInput)
    {
        ResourceContentHashVector hashes;
        ManagedResourceVector resolved = localResourceComponent.resolveResources(hashes);

        EXPECT_TRUE(resolved.empty());
    }

    TEST_F(AResourceComponentTest, returnsEmptyResourceVectorOnFullyInvalidInput)
    {
        ResourceContentHashVector hashes;

        hashes.push_back({ 111, 111 });
        hashes.push_back({ std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max() });

        ManagedResourceVector resolved = localResourceComponent.resolveResources(hashes);

        EXPECT_TRUE(resolved.empty());
    }

    TEST_F(AResourceComponentTest, handlesNotExistingResourceProperly)
    {
        ResourceContentHashVector hashes = writeMultipleTestResourceFile(2, 2000, true);

        hashes.push_back({ 111, 111 });
        hashes.push_back({ std::numeric_limits<uint64_t>::max(), std::numeric_limits<uint64_t>::max() });
        ManagedResourceVector resolved = localResourceComponent.resolveResources(hashes);

        EXPECT_NE(hashes.size(), resolved.size());
        ASSERT_EQ(2u, resolved.size());
        EXPECT_TRUE(resolved[0]->getHash() == hashes[0]);
        EXPECT_TRUE(resolved[1]->getHash() == hashes[1]);
    }

    TEST_F(AResourceComponentTest, getsResourceInfoForResourceHashVector)
    {
        IResource* tex = new TextureResource(EResourceType::Texture2D, TextureMetaInfo(1u, 1u, 1u, EPixelStorageFormat::R8, false, {}, { 1u }), {});
        tex->setResourceData(ResourceBlob{ 2 }, { 4u, 4u });
        auto res1 = localResourceComponent.manageResource(*CreateTestResource());
        auto res2 = localResourceComponent.manageResource(*CreateTestResource(1.0f, 2));
        auto res3 = localResourceComponent.manageResource(*tex);

        for (auto& res : { res1, res2, res3 })
        {
            ResourceInfo info = localResourceComponent.getResourceInfo(res->getHash());

            EXPECT_EQ(info.hash, res->getHash());
            EXPECT_EQ(info.compressedSize, res->getCompressedDataSize());
            EXPECT_EQ(info.decompressedSize, res->getDecompressedDataSize());
            EXPECT_EQ(info.type, res->getTypeID());
        }
    }

    TEST_F(AResourceComponentTest, getsResourceInfoForResourcesLoadedFromFile)
    {
        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);

        // resource info available before loading the file
        EXPECT_EQ(localResourceComponent.getResource(handleAndHash.second), ManagedResource{});
        ResourceInfo infoBefore = localResourceComponent.getResourceInfo(handleAndHash.second);
        EXPECT_EQ(infoBefore.hash, handleAndHash.second);
        EXPECT_EQ(infoBefore.compressedSize, 0u);
        EXPECT_EQ(infoBefore.decompressedSize, 36u);
        EXPECT_EQ(infoBefore.type, EResourceType::VertexArray);

        ManagedResource fromfile = localResourceComponent.loadResource(handleAndHash.second);

        ResourceInfo info = localResourceComponent.getResourceInfo(handleAndHash.second);
        EXPECT_EQ(info.hash, fromfile->getHash());
        EXPECT_EQ(info.compressedSize, fromfile->getCompressedDataSize());
        EXPECT_EQ(info.decompressedSize, fromfile->getDecompressedDataSize());
        EXPECT_EQ(info.type, fromfile->getTypeID());
        EXPECT_EQ(info, infoBefore);


        // infos stay available when unloading
        fromfile.reset();
        EXPECT_EQ(localResourceComponent.getResource(handleAndHash.second), ManagedResource{});

        info = localResourceComponent.getResourceInfo(handleAndHash.second);
        EXPECT_EQ(info, infoBefore);
    }

    TEST_F(AResourceComponentTest, properlyClosesMultipleEquallyNamedFiles)
    {
        File subDir(subDirName);
        if (!subDir.isDirectory())
            subDir.createDirectory();

        auto handleAndHash = setupTest(resourceFileName, localResourceComponent);
        auto handleAndHashSubDir = setupTest(equallyNamedResFileInSubDir, localResourceComponent);

        EXPECT_NE(handleAndHash.first, handleAndHashSubDir.first);

        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHashSubDir.first));

        localResourceComponent.removeResourceFile(handleAndHash.first);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_TRUE(localResourceComponent.hasResourceFile(handleAndHashSubDir.first));

        localResourceComponent.removeResourceFile(handleAndHashSubDir.first);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHash.first));
        EXPECT_FALSE(localResourceComponent.hasResourceFile(handleAndHashSubDir.first));
    }

    TEST_F(AResourceComponentTest, loadResourceCanHandlePersistationErrors)
    {
        ResourceContentHash hash(1, 2);

        auto streamContainer = std::make_shared<InputStreamContainerMock>();
        StrictMock<InputStreamMock> stream;
        EXPECT_CALL(*streamContainer, getStream()).WillRepeatedly(ReturnRef(stream));

        ResourceTableOfContents toc;
        toc.registerContents(ResourceInfo(EResourceType::Effect, hash, 10, 0), 0, 5);

        localResourceComponent.addResourceFile(streamContainer, toc);

        EXPECT_CALL(stream, seek(_, _)).WillOnce(Return(EStatus::Error));
        EXPECT_CALL(stream, getState()).WillRepeatedly(Return(EStatus::Ok));
        EXPECT_CALL(stream, getPos(_)).WillOnce(Return(EStatus::Ok));
        EXPECT_EQ(localResourceComponent.loadResource(hash), ManagedResource());
    }

    TEST_F(AResourceComponentTest, catchesExceptionWhenDeserializingResource)
    {
        ResourceContentHash hash(1, 2);

        auto streamContainer = std::make_shared<InputStreamContainerMock>();
        StrictMock<InputStreamMock> stream;
        EXPECT_CALL(*streamContainer, getStream()).WillRepeatedly(ReturnRef(stream));

        ResourceTableOfContents toc;
        toc.registerContents(ResourceInfo(EResourceType::Effect, hash, 10, 0), 0, 5);

        localResourceComponent.addResourceFile(streamContainer, toc);

        EXPECT_CALL(stream, seek(_, _)).WillOnce(Return(EStatus::Ok));
        EXPECT_CALL(stream, getState()).WillRepeatedly(Return(EStatus::Ok));
        EXPECT_CALL(stream, getPos(_)).WillOnce(Return(EStatus::Ok));
        EXPECT_CALL(stream, read(_, _)).WillOnce(Throw(std::bad_alloc()));
        EXPECT_NO_THROW(EXPECT_EQ(localResourceComponent.loadResource(hash), ManagedResource()));
    }
}
