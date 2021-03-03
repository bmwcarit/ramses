//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ResourceComponent.h"
#include "ResourceMock.h"
#include "gmock/gmock.h"
#include "Resource/ArrayResource.h"
#include "DummyResource.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Components/SceneUpdate.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "TransportCommon/SceneUpdateSerializer.h"
#include "TransportCommon/SceneUpdateStreamDeserializer.h"


namespace ramses_internal
{
    using namespace testing;
    class ResourceComponentTestBase : public testing::Test
    {
    public:
        ResourceComponentTestBase()
            : resourceFileName("test.resource")
        {
        }

        virtual ResourceComponent& getResourceComponent() = 0;

        virtual ~ResourceComponentTestBase()
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

            ArrayResource* resource = new ArrayResource(EResourceType_VertexArray, 3 + static_cast<uint32_t>(extraSize), EDataType::Vector3F, vertexData.data(), ResourceCacheFlag(0u), String("resName"));
            return resource;
        }

        static ResourceContentHashVector HashesFromManagedResources(const ManagedResourceVector& vec)
        {
            ResourceContentHashVector hashes;
            for (const auto& res : vec)
                hashes.push_back(res->getHash());
            return hashes;
        }

        ResourceContentHashVector writeMultipleTestResourceFile(UInt32 num, size_t extraSize = 0, bool compress = false)
        {
            ResourceComponent& localResourceComponent = getResourceComponent();

            ManagedResourceVector managedResourceVec;
            ResourceContentHashVector hashes;

            for (UInt32 i = 0; i < num; ++i)
            {
                IResource* resource = CreateTestResource(i*1.0f, extraSize);
                ManagedResource managedResource = localResourceComponent.manageResource(*resource, true);
                managedResourceVec.push_back(managedResource);
                hashes.push_back(managedResource->getHash());
            }

            {
                File resourceFile(resourceFileName);
                BinaryFileOutputStream resourceOutputStream(resourceFile);
                ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, managedResourceVec, compress);
            }

            ramses_internal::ResourceTableOfContents resourceFileToc;
            ResourceFileInputStreamSPtr resourceInputStream(new ResourceFileInputStream(resourceFileName));
            resourceFileToc.readTOCPosAndTOCFromStream(resourceInputStream->getStream());
            localResourceComponent.addResourceFile(resourceInputStream, resourceFileToc);

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
            {
                resourceFile.remove();
            }
        }

        void expectResourceSizeCalls(const ResourceMock* resource)
        {
            EXPECT_CALL(*resource, getCompressedDataSize()).Times(AnyNumber());
            EXPECT_CALL(*resource, getDecompressedDataSize()).Times(AnyNumber());
        }

        static std::vector<std::vector<Byte>> SerializeResources(const ManagedResourceVector& resVec, uint32_t chunkSize = 100000)
        {
            SceneUpdate update{SceneActionCollection(), resVec, {}};
            return TestSerializeSceneUpdateToVectorChunked(SceneUpdateSerializer(update), chunkSize);
        }

    protected:
        PlatformLock frameworkLock;
        const String resourceFileName;
    };

    class AResourceComponentTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentTest()
            : localResourceComponent(statistics, frameworkLock)
        {}

        virtual ResourceComponent& getResourceComponent() override
        {
            return localResourceComponent;
        }

        struct TestResourceData
        {
            std::vector<Byte> data;
            ResourceContentHash hash;
        };

        TestResourceData CreateTestResourceData()
        {
            const ManagedResourceVector resVec{ ManagedResource{ CreateTestResource(), deleter } };
            const IResource* res = resVec.front().get();
            const ResourceContentHash hash = res->getHash();

            const auto dataVec = SerializeResources(resVec);
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
            : localResourceComponent(statistics, frameworkLock)
        {}

        virtual ResourceComponent& getResourceComponent() override
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
        DummyResource* dummyResource = new DummyResource(dummyResourceHash, EResourceType_VertexArray);
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
        ResourceWithDestructorMock* resource = new ResourceWithDestructorMock(ResourceContentHash(47, 0), EResourceType_VertexArray);
        {
            expectResourceSizeCalls(resource);
            EXPECT_CALL(*resource, Die());
            ManagedResource managedRes = localResourceComponent.manageResource(*resource);
        }// raw pointer was transferred into component, and is deleted when last managed pointer goes away

        Mock::VerifyAndClearExpectations(resource); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
    }

    TEST_F(AResourceComponentTest, canReturnAllResourcesManaged)
    {
        DummyResource* resource = new DummyResource(ResourceContentHash(47, 0), EResourceType_VertexArray);
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);

        DummyResource* resource2 = new DummyResource(ResourceContentHash(49, 0), EResourceType_VertexArray);
        ManagedResource managedRes2 = localResourceComponent.manageResource(*resource2);

        ManagedResourceVector allResources = localResourceComponent.getResources();

        EXPECT_EQ(2u, allResources.size());
        EXPECT_THAT(allResources, testing::UnorderedElementsAre(managedRes, managedRes2));
    }

    TEST_F(AResourceComponentTest, canReturnHashUsageReferringToKnownResource)
    {
        const ResourceContentHash hash(47, 0);
        ResourceMock* resource = new ResourceMock(hash, EResourceType_VertexArray);
        expectResourceSizeCalls(resource);
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);

        ResourceHashUsage hashUsage = localResourceComponent.getResourceHashUsage(hash);

        EXPECT_EQ(hash, hashUsage.getHash());
    }

    ResourceContentHash setupTest(String const& resourceFileName, ResourceComponent& localResourceComponent)
    {
        // setup test
        const Float vertexData[] = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };
        ArrayResource* resource = new ArrayResource(EResourceType_VertexArray, 3, EDataType::Vector3F, vertexData, ResourceCacheFlag(0u), String("resName"));
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

        {
            ramses_internal::ResourceTableOfContents resourceFileToc;
            ResourceFileInputStreamSPtr resourceInputStream(new ResourceFileInputStream(resourceFileName));
            resourceFileToc.readTOCPosAndTOCFromStream(resourceInputStream->getStream());

            localResourceComponent.addResourceFile(resourceInputStream, resourceFileToc);
        }

        return hash;
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFile)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();
        ManagedResource fromfile = localResourceComponent.loadResource(hash);
        EXPECT_TRUE(fromfile);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(fromfile);
        fromfile = ManagedResource();
        EXPECT_FALSE(fromfile);
    }

    TEST_F(AResourceComponentTest, removingFileWillMakeUnloadedResourceUnusable)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_FALSE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canLoadResourceBecauseOfHashUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(resourceFileName);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));
        EXPECT_TRUE(localResourceComponent.getResource(hash));
    }

    TEST_F(AResourceComponentTest, canLoadResourceBecauseOfHashUsageAndRemoveFile)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(resourceFileName);
        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canLoadResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_TRUE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canLoadAndRemoveAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.loadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.loadResourceFromFile(resourceFileName);
        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndDeletesResourceBecauseNoUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_FALSE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.loadResource(hash));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndKeepsResourceBecauseAvailableInOtherFile)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);
        auto hash2 = setupTest("test2.resource", localResourceComponent);
        EXPECT_EQ(hash, hash2);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));
        EXPECT_TRUE(localResourceComponent.hasResourceFile("test2.resource"));

        EXPECT_FALSE(localResourceComponent.getResource(hash));
        auto res = localResourceComponent.loadResource(hash);
        EXPECT_TRUE(res);
        EXPECT_TRUE(localResourceComponent.getResource(hash));
        res = {};
        EXPECT_FALSE(localResourceComponent.getResource(hash));
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
}
