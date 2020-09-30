//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "framework_common_gmock_header.h"
#include "ComponentMocks.h"
#include "Components/ResourceComponent.h"
#include "ResourceMock.h"
#include "gmock/gmock.h"
#include "Resource/ArrayResource.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformEvent.h"
#include "DummyResource.h"
#include "Collections/String.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "CommunicationSystemMock.h"
#include "TaskFramework/ThreadedTaskExecutor.h"
#include "Components/SingleResourceSerialization.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Components/SceneUpdate.h"
#include "SceneUpdateSerializerTestHelper.h"
#include "TransportCommon/SceneUpdateSerializer.h"
#include "TransportCommon/SceneUpdateStreamDeserializer.h"


namespace ramses_internal
{
    using namespace testing;

    class DelayedSingleTaskExecutor : public ITaskQueue
    {
    public:
        DelayedSingleTaskExecutor()
        {
        }

        virtual bool enqueue(ITask& Task) override
        {
            Task.addRef();
            m_scheduledTasks.push_back(&Task);
            return true;
        }

        virtual void disableAcceptingTasksAfterExecutingCurrentQueue() override
        {
        }

        void execute()
        {
            assert(!m_scheduledTasks.empty());
            ITask* currentTask = m_scheduledTasks.front();
            m_scheduledTasks.pop_front();
            assert(nullptr != currentTask);
            currentTask->execute();
            currentTask->release();
        }

        bool hasTasks() const
        {
            return !m_scheduledTasks.empty();
        }

        void executeAll()
        {
            while (!m_scheduledTasks.empty())
                execute();
        }

    private:
        std::deque<ITask*> m_scheduledTasks;
    };

    class ResourceComponentTestBase : public testing::Test
    {
    public:
        ResourceComponentTestBase()
            : m_myID(20)
            , resourceFileName("test.resource")
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
            resourceFileToc.readTOCPosAndTOCFromStream(resourceInputStream->resourceStream);
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

        void expectSendResourcesToNetwork(StrictMock<CommunicationSystemMock>& mock, Guid requesterId, ManagedResourceVector& resources)
        {
            EXPECT_CALL(mock, sendResources(requesterId, _)).WillOnce([&](auto, auto& serializer) {
                // grab resources directly out of serializer
                resources = static_cast<const SceneUpdateSerializer&>(serializer).getUpdate().resources;
                return true;
            });
        }

    protected:
        Guid m_myID;
        PlatformLock frameworkLock;
        NiceMock<MockConnectionStatusUpdateNotifier> connectionStatusUpdateNotifier;
        const String resourceFileName;
    };

    class AResourceComponentTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentTest()
            : executor()
            , communicationSystem()
            , localResourceComponent(executor, m_myID, communicationSystem, connectionStatusUpdateNotifier, statistics, frameworkLock)
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
        DelayedSingleTaskExecutor executor;
        StrictMock<CommunicationSystemMock> communicationSystem;
        StatisticCollectionFramework statistics;
        ResourceComponent localResourceComponent;
        ResourceDeleterCallingCallback deleter;
    };

    class SafeCommunicationSystemMock : public CommunicationSystemMock
    {
    public:
        explicit SafeCommunicationSystemMock(ramses_internal::PlatformLock& lock)
            : m_lock(lock)
        {
        }

        MOCK_METHOD(bool, safe_sendResources, (const Guid& to, const ISceneUpdateSerializer& serializer));

    private:
        bool sendResources(const Guid& to, const ISceneUpdateSerializer& serializer) override
        {
            ramses_internal::PlatformGuard g(m_lock);
            return safe_sendResources(to, serializer);
        }

        ramses_internal::PlatformLock& m_lock;
    };

    class AResourceComponentWithThreadedTaskExecutorTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentWithThreadedTaskExecutorTest()
            : executor(1)
            , communicationSystem(expectLock)
            , localResourceComponent(executor, m_myID, communicationSystem, connectionStatusUpdateNotifier, statistics, frameworkLock)
        {}

        virtual ResourceComponent& getResourceComponent() override
        {
            return localResourceComponent;
        }
    protected:
        ThreadedTaskExecutor executor;
        ramses_internal::PlatformLock expectLock;
        SafeCommunicationSystemMock communicationSystem;
        StatisticCollectionFramework statistics;
        ResourceComponent localResourceComponent;
    };

    TEST_F(AResourceComponentTest, ResolveResources_CallsResourceAvailableForCompletedObjectHandlerForExistingObjects)
    {
        ResourceContentHash dummyResourceHash(47, 0);
        DummyResource* dummyResource = new DummyResource(dummyResourceHash, EResourceType_VertexArray);
        ManagedResource dummyManagedResource = localResourceComponent.manageResource(*dummyResource);

        ResourceRequesterID requesterID(1);
        Guid providerID(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(dummyResourceHash);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        EXPECT_THAT(localResourceComponent.popArrivedResources(requesterID), testing::Contains(dummyManagedResource));
    }

    TEST_F(AResourceComponentTest, CanGetAlreadyManagedResource)
    {
        ResourceContentHash dummyResourceHash(47, 0);
        DummyResource* dummyResource = new DummyResource(dummyResourceHash, EResourceType_VertexArray);
        ManagedResource dummyManagedResource = localResourceComponent.manageResource(*dummyResource);

        ManagedResource fetchedResource = localResourceComponent.getResource(dummyResourceHash);
        EXPECT_EQ(dummyResourceHash, fetchedResource->getHash());
        EXPECT_EQ(dummyResource, fetchedResource.get());
    }

    TEST_F(AResourceComponentTest, ResolveResources_RequestResourceFromProviderIfNotAvailableLocaly)
    {
        Guid providerID(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(ResourceContentHash(123, 456));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResourceHashes));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, ResourceRequesterID(1), providerID);
    }

    TEST_F(AResourceComponentTest, ResolveResources_RequestResourceFromProviderIfNotAvailableLocalyAnymore)
    {
        const ResourceContentHash dummyResourceHash(47, 0);
        {
            DummyResource* dummyResource = new DummyResource(dummyResourceHash, EResourceType_VertexArray);
            ManagedResource dummyManagedResource = localResourceComponent.manageResource(*dummyResource);
        }

        Guid providerID(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(dummyResourceHash);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResourceHashes));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, ResourceRequesterID(1), providerID);
    }

    TEST_F(AResourceComponentTest, ResolveResources_TriggersLoadingOfAvailableResourceFromFile)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

        ResourceRequesterID requesterID(1);
        Guid providerID(222);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        // execute enqueued resource loading task
        executor.execute();

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_TRUE(poppedResources.size() > 0);
        ManagedResource resource = poppedResources.back();
        ASSERT_EQ(resourceHash, resource->getHash());

        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue());
        EXPECT_EQ(SingleResourceSerialization::SizeOfSerializedResource(*resource.get()), statistics.statResourcesLoadedFromFileSize.getCounterValue());
    }

    TEST_F(AResourceComponentTest, RequestHashMultipleTimesAsyncBeforeLoadingThreadExecutes)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

        ResourceRequesterID requesterID(1);
        Guid        providerID(222);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        // execute both enqueued resource loading task
        executor.execute();
        executor.execute();

        // check that resource is only returned once
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_TRUE(poppedResources.size() > 0);
        ManagedResource resource = poppedResources.back();
        ASSERT_EQ(resourceHash, resource->getHash());
    }

    TEST_F(AResourceComponentTest, HandleRequestHashMultipleTimesBeforeLoadingThreadExecutes)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

        Guid        providerID(222);
        localResourceComponent.handleRequestResources(requestedResourceHashes, providerID);
        localResourceComponent.handleRequestResources(requestedResourceHashes, providerID);

        // execute enqueued resource loading task
        // resource will be loaded twice. This is not necessary but should
        // not happen, checking would be overhead and this test ensures that
        // even if it should happen it does not cause memory leak or similar
        EXPECT_CALL(communicationSystem, sendResources(providerID, _)).Times(2);
        executor.execute();
        executor.execute();

        // all resources are unloaded after 'sending' again
        ASSERT_EQ(0u, localResourceComponent.getResources().size());
    }

    TEST_F(AResourceComponentTest, ReloadFromSameResourceFile_doesntLeak)
    {
        // create resource file with restresource and register it
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        for (int i = 0; i < 10; i++)
        {
            // make sure managed resource created in writeTestResourceFile() was freed,
            // so resolveResources() has to trigger loading it from file
            EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

            ResourceRequesterID requesterID(1);
            Guid        providerID(222);
            localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

            // execute enqueued resource loading task
            executor.execute();

            ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
            ASSERT_TRUE(poppedResources.size() > 0);
            ManagedResource resource = poppedResources.back();
            poppedResources.pop_back();
            ASSERT_EQ(resourceHash, resource->getHash());
            resource = ManagedResource();

            ASSERT_EQ(0u, localResourceComponent.getResources().size());
        }
    }

    TEST_F(AResourceComponentTest, ReloadFromSameResourceFileWhileKeepingHashReference_doesntLeak)
    {
        // create resource file with restresource and register it
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);
        ResourceHashUsage hashUsage = localResourceComponent.getResourceHashUsage(resourceHash);

        for (int i = 0; i < 10; i++)
        {
            // make sure managed resource created in writeTestResourceFile() was freed,
            // so resolveResources() has to trigger loading it from file
            EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

            ResourceRequesterID requesterID(1);
            Guid        providerID(222);
            localResourceComponent.requestResourceAsynchronouslyFromFramework(
                requestedResourceHashes, requesterID, providerID);

            // execute enqueued resource loading task
            executor.execute();

            ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
            ASSERT_TRUE(poppedResources.size() > 0);
            ManagedResource resource = poppedResources.back();
            poppedResources.pop_back();
            ASSERT_EQ(resourceHash, resource->getHash());
            resource = ManagedResource();

            ASSERT_EQ(0u, localResourceComponent.getResources().size());
        }
    }

    TEST_F(AResourceComponentTest, RequestResourcesAsync_RequestResourceFromProviderIfNotAvailableLocalyAnymore)
    {
        ResourceContentHashVector requestedResourceHashes;
        {
            ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());
            requestedResourceHashes.push_back(managedResource->getHash());
        }

        // make sure managed resource created before was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(localResourceComponent.getResources().size(), 0u);

        Guid providerID(222);
        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResourceHashes));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        ASSERT_EQ(0u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, CancelResourceRequest_IgnoresResourceReceivedIfRequestCanceled)
    {
        //This case needs to be handled in case the resources could be retrieved
        //from other sources other than memory, e.g., if the resource could be
        //retrieved from the file system
        localResourceComponent.newParticipantHasConnected(m_myID);
        const auto testRes = CreateTestResourceData();
        absl::Span<const Byte> view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        ResourceRequesterID requesterID(1);
        Guid providerID(222);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID));
        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID);
        localResourceComponent.handleSendResource(view, m_myID);
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID).size());
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID));
    }

    TEST_F(AResourceComponentTest, CancelResourceRequest_StillHandleWhenOnlyOneOutOfTwoRequestsGotCancelled)
    {
        Guid providerID(222);
        localResourceComponent.newParticipantHasConnected(providerID);
        const auto testRes = CreateTestResourceData();
        absl::Span<const Byte> view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        ResourceRequesterID requesterID_1(1);
        ResourceRequesterID requesterID_2(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(2);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_1, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_2, providerID);

        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_1));
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_2));

        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID_1);
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_1));
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_2));

        localResourceComponent.handleSendResource(view, providerID);

        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID_1).size());
        EXPECT_EQ(1u, localResourceComponent.popArrivedResources(requesterID_2).size());
    }

    TEST_F(AResourceComponentTest, CancelResourceRequest_IgnoresResourceReceivedIfAllRequestsCanceled)
    {
        Guid providerID(222);
        localResourceComponent.newParticipantHasConnected(providerID);
        const auto testRes = CreateTestResourceData();
        absl::Span<const Byte> view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        ResourceRequesterID requesterID_1(1);
        ResourceRequesterID requesterID_2(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(2);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_1, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_2, providerID);

        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_1));
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_2));

        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID_1);
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_1));
        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID_2);
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(testRes.hash, requesterID_2));

        localResourceComponent.handleSendResource(view, providerID);

        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID_1).size());
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID_2).size());
    }

    TEST_F(AResourceComponentTest, CancelResourceRequest_ResourceWillBeRetrievedAgainAfterCancelledRetrieval)
    {
        const auto testRes = CreateTestResourceData();

        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);
        Guid providerID(222);

        ResourceRequesterID requesterID(1);
        localResourceComponent.newParticipantHasConnected(providerID);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        absl::Span<const Byte> view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        localResourceComponent.handleSendResource(view, providerID);

        EXPECT_EQ(1u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, handleResourcesNotAvailableCancelsAllResourceRequestsForThisResource)
    {
        ResourceContentHash hash(5, 0);
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(hash);
        Guid providerID(222);
        ResourceRequesterID requesterID_1(1);
        ResourceRequesterID requesterID_2(2);
        localResourceComponent.newParticipantHasConnected(providerID);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_1, providerID);
        Mock::VerifyAndClearExpectations(&communicationSystem);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_2, providerID);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        EXPECT_TRUE(localResourceComponent.hasRequestForResource(hash, requesterID_1));
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(hash, requesterID_2));

        localResourceComponent.handleResourcesNotAvailable(requestedResources, providerID);

        EXPECT_FALSE(localResourceComponent.hasRequestForResource(hash, requesterID_1));
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(hash, requesterID_2));
    }

    TEST_F(AResourceComponentTest, CanHandleMultipleResourceHashesInOneRequest)
    {
        Guid providerID(222);
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        ResourceInfo res2(EResourceType_VertexArray, ResourceContentHash(2u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources;
        requestedResources.push_back(res1.hash);
        requestedResources.push_back(res2.hash);

        ResourceInfoVector knownResources;
        knownResources.push_back(res1);
        knownResources.push_back(res2);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, RerequestsAlreadyRequestedResource)
    {
        ResourceInfo res(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        Guid providerID(222);
        ResourceContentHashVector requestedResources(1, res.hash);
        ResourceInfoVector knownResources(1, res);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, RerequestsResourcesFromDifferentProvider)
    {
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        ResourceInfo res2(EResourceType_VertexArray, ResourceContentHash(2u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources_provider1;
        requestedResources_provider1.push_back(res1.hash);

        ResourceContentHashVector requestedResources_provider2;
        requestedResources_provider2.push_back(res1.hash);
        requestedResources_provider2.push_back(res2.hash);

        ResourceRequesterID requesterID(1);
        Guid provider1(222);
        Guid provider2(333);
        EXPECT_CALL(communicationSystem, sendRequestResources(provider1, requestedResources_provider1)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_provider1, requesterID, provider1);

        EXPECT_CALL(communicationSystem, sendRequestResources(provider2, requestedResources_provider2)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_provider2, requesterID, provider2);
    }

    TEST_F(AResourceComponentTest, AllowsRequestingResourceAgainIfClientAnsweredResourceNotAvailable)
    {
        ResourceInfo res(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources(1, res.hash);
        ResourceInfoVector knownResources(1, res);
        Guid clientId(222);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(clientId, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, clientId);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        localResourceComponent.handleResourcesNotAvailable(requestedResources, clientId);

        EXPECT_CALL(communicationSystem, sendRequestResources(clientId, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, clientId);
    }

    TEST_F(AResourceComponentTest, RequestFromSceneProviderIfResourceLocationIsNotGiven)
    {
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        Guid providerID(444);

        ResourceRequesterID requesterID(1);
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(res1.hash);

        ResourceInfoVector knownResources;
        knownResources.push_back(res1);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, CanHandleResourceReceivedFromSource)
    {
        const ManagedResourceVector resVec{ ManagedResource{ CreateTestResource(), deleter } };
        const ResourceContentHash hash = resVec[0]->getHash();
        const auto dataVec = SerializeResources(resVec);
        ASSERT_EQ(1u, dataVec.size());

        const auto resourceData = dataVec[0];
        absl::Span<const Byte> view(resourceData.data(), static_cast<UInt32>(resourceData.size()));

        const Guid dummyGuid(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        localResourceComponent.handleSendResource(view, m_myID);

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ManagedResource managedResource = poppedResources.back();
        ASSERT_EQ(hash, managedResource->getHash());
    }

    TEST_F(AResourceComponentTest, PartiallyReceivedResourcesAreNotPassedToArrivedResources)
    {
        const ManagedResourceVector resVec{ ManagedResource{ CreateTestResource(), deleter } };
        const ResourceContentHash hash = resVec[0]->getHash();
        const auto dataVec = SerializeResources(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const auto resourceData = dataVec[0];
        absl::Span<const Byte> view(resourceData.data(), static_cast<UInt32>(resourceData.size()));

        const Guid dummyGuid(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        localResourceComponent.handleSendResource(view, m_myID);

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());
    }

    TEST_F(AResourceComponentTest, ResourcesArePassedToArrivedResourcesWhenAllFragmentsAreReceived)
    {
        const ManagedResourceVector resVec{ ManagedResource{ CreateTestResource(), deleter } };
        const ResourceContentHash hash = resVec[0]->getHash();

        const IResource* testResource = resVec[0].get();
        const auto dataVec = SerializeResources(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const Guid dummyGuid(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);

        // "receive" all except last
        for (UInt i = 0; i < dataVec.size() - 1; ++i)
        {
            absl::Span<const Byte> view(dataVec[i].data(), static_cast<UInt32>(dataVec[i].size()));
            localResourceComponent.handleSendResource(view, m_myID);
        }
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());

        // "receive" last chunk
        absl::Span<const Byte> view(dataVec.back().data(), static_cast<UInt32>(dataVec.back().size()));
        localResourceComponent.handleSendResource(view, m_myID);

        poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_EQ(1u, poppedResources.size());

        ManagedResource managedResource = poppedResources.back();
        EXPECT_EQ(hash, managedResource->getHash());
        ASSERT_EQ(testResource->getDecompressedDataSize(), managedResource->getDecompressedDataSize());

        const UInt8* resourceDataBlob = managedResource->getResourceData().data();
        EXPECT_TRUE(0 == PlatformMemory::Compare(testResource->getResourceData().data(), resourceDataBlob, testResource->getDecompressedDataSize()));
    }

    TEST_F(AResourceComponentTest, ResourceFragmentsFromOtherParticipantsAreIgnored)
    {
        const ManagedResourceVector resVec = { ManagedResource{ CreateTestResource(), deleter } };
        const IResource* testResource = resVec[0].get();
        const auto dataVec = SerializeResources(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const auto resourceData = dataVec[0];
        ResourceInfo availableResource(EResourceType_VertexArray, testResource->getHash(), testResource->getDecompressedDataSize(), 0u);

        const Guid dummyGuid(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(availableResource.hash);

        Guid otherParticipant(333);
        localResourceComponent.newParticipantHasConnected(m_myID);
        localResourceComponent.newParticipantHasConnected(otherParticipant);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);

        // "receive" all except last from one participant
        for (UInt i = 0; i < dataVec.size() - 1; ++i)
        {
            absl::Span<const Byte> view(dataVec[i].data(), static_cast<UInt32>(dataVec[i].size()));
            localResourceComponent.handleSendResource(view, m_myID);
        }
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());

        // "receive" last chunk from other participant
        absl::Span<const Byte> view(dataVec.back().data(), static_cast<UInt32>(dataVec.back().size()));
        localResourceComponent.handleSendResource(view, otherParticipant);

        poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());

        // "receive" last chunk from correct participant
        localResourceComponent.handleSendResource(view, m_myID);

        poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(1u, poppedResources.size());
    }

    TEST_F(AResourceComponentTest, DropReceivedResourceFragmentsWhenProvidingParticipantDisconnects)
    {
        const ManagedResourceVector resVec{ ManagedResource { CreateTestResource(), deleter } };
        const auto dataVec = SerializeResources(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        ResourceInfo availableResource(EResourceType_VertexArray, resVec[0]->getHash(), resVec[0]->getDecompressedDataSize(), 0u);

        const Guid dummyGuid(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(availableResource.hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        absl::Span<const Byte> view(dataVec[0].data(), static_cast<UInt32>(dataVec[0].size()));
        localResourceComponent.handleSendResource(view, m_myID);
        EXPECT_TRUE(localResourceComponent.hasDeserializerForParticipant(m_myID));

        localResourceComponent.participantHasDisconnected(m_myID);
        EXPECT_FALSE(localResourceComponent.hasDeserializerForParticipant(m_myID));
    }

    TEST_F(AResourceComponentTest, CanDeleteResourcesLoadedFromFile)
    {
        const Guid dummyGuid(222);
        const ResourceContentHash dummyResourceHash(47, 0);
        const ResourceInfo dummyResourceInfo(EResourceType_VertexArray, dummyResourceHash, 2u, 1u);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(dummyResourceHash);

        ResourceWithDestructorMock* resource = new ResourceWithDestructorMock(dummyResourceHash, EResourceType_VertexArray);

        ResourceRequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        expectResourceSizeCalls(resource);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        localResourceComponent.resourceHasBeenLoadedFromFile(resource, 4711);
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);

        ResourceHashUsage usage = localResourceComponent.getResourceHashUsage(dummyResourceHash);
        Mock::VerifyAndClearExpectations(resource);

        // letting go of the managed resource, but keeping the hash usage allows deletion, when resource came from a file
        expectResourceSizeCalls(resource);
        EXPECT_CALL(*resource, Die());
        poppedResources.clear();
        Mock::VerifyAndClearExpectations(resource);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_SendsExistingResource)
    {
        ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());

        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(managedResource->getHash());
        Guid requesterId(222);

        EXPECT_CALL(communicationSystem, sendResources(requesterId, _));
        localResourceComponent.handleRequestResources(requestedResourceHashes, requesterId);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_SendsMultipleExistingResourcesAtOnce)
    {
        ManagedResourceVector managedResources = { localResourceComponent.manageResource(*CreateTestResource(0.f)),
            localResourceComponent.manageResource(*CreateTestResource(1.f)),
            localResourceComponent.manageResource(*CreateTestResource(2.f)) };

        ResourceContentHashVector requestedResourceHashes;
        for (const auto& res : managedResources)
        {
            requestedResourceHashes.push_back(res->getHash());
        }

        Guid requesterId(222);
        ManagedResourceVector sentResources;
        expectSendResourcesToNetwork(communicationSystem, requesterId, sentResources);
        localResourceComponent.handleRequestResources(requestedResourceHashes, requesterId);

        EXPECT_EQ(managedResources, sentResources);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_LoadsResourceFromFileAndSendsIt)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_EQ(nullptr, localResourceComponent.getResource(resourceHash).get());

        Guid requester(222);
        localResourceComponent.handleRequestResources(requestedResourceHashes, requester);

        // execute enqueued resource loading task
        EXPECT_CALL(communicationSystem, sendResources(requester, _));
        executor.execute();
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_LoadsMultipleResourcesFromFileAndSendsThem)
    {
        ResourceContentHashVector requestedResourceHashes = writeMultipleTestResourceFile(4);

        Guid requester(222);
        localResourceComponent.handleRequestResources(requestedResourceHashes, requester);

        // execute enqueued resource loading task
        ManagedResourceVector sentResources;
        expectSendResourcesToNetwork(communicationSystem, requester, sentResources);
        executor.execute();
        EXPECT_EQ(requestedResourceHashes, HashesFromManagedResources(sentResources));
        EXPECT_EQ(4u, statistics.statResourcesLoadedFromFileNumber.getCounterValue());
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_handlesMixedRequestOfExistingAndFromFileResources)
    {
        ResourceContentHashVector fileResHashes = writeMultipleTestResourceFile(2);
        ManagedResourceVector localRes = { localResourceComponent.manageResource(*CreateTestResource(10.f)),
            localResourceComponent.manageResource(*CreateTestResource(11.f)),
            localResourceComponent.manageResource(*CreateTestResource(12.f)) };
        ResourceContentHashVector requestedResourceHashes = { localRes[0]->getHash(),
            fileResHashes[0],
            localRes[1]->getHash(),
            fileResHashes[1],
            localRes[2]->getHash() };

        Guid requester(222);

        ManagedResourceVector sentLocalResources;
        expectSendResourcesToNetwork(communicationSystem, requester, sentLocalResources);
        localResourceComponent.handleRequestResources(requestedResourceHashes, requester);
        Mock::VerifyAndClearExpectations(&communicationSystem);
        EXPECT_EQ(localRes, sentLocalResources);

        ManagedResourceVector sentFileResources;
        expectSendResourcesToNetwork(communicationSystem, requester, sentFileResources);
        executor.execute();
        EXPECT_EQ(fileResHashes, HashesFromManagedResources(sentFileResources));
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_TriesToLoadResourceFromNonExistentFile)
    {
        ResourceContentHashVector requestedResourceHashes;

        {
            ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());
            requestedResourceHashes.push_back(managedResource->getHash());
        }

        // make sure managed resource created before was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_EQ(localResourceComponent.getResources().size(), 0u);

        Guid requesterId(222);
        EXPECT_CALL(communicationSystem, sendResourcesNotAvailable(requesterId, requestedResourceHashes));
        localResourceComponent.handleRequestResources(requestedResourceHashes, requesterId);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_sendNotAvailableForUnknownResources)
    {
        const ResourceContentHash hash1(1u, 0);
        const ResourceContentHash hash2(2u, 0);
        ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());

        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(managedResource->getHash());
        requestedResourceHashes.push_back(hash1);
        requestedResourceHashes.push_back(hash2);

        ResourceContentHashVector unknownResources;
        unknownResources.push_back(hash1);
        unknownResources.push_back(hash2);

        Guid requesterId(222);
        EXPECT_CALL(communicationSystem, sendResources(requesterId, _));
        EXPECT_CALL(communicationSystem, sendResourcesNotAvailable(requesterId, unknownResources));
        localResourceComponent.handleRequestResources(requestedResourceHashes, requesterId);
    }


    TEST_F(AResourceComponentTest, SendsRemoteRequestToProviderIfAvailabilityUnknown)
    {
        Guid providerID(222);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(ResourceContentHash(47, 0));

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, ResourceRequesterID(1), providerID);
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

    TEST_F(AResourceComponentTest, DoesNotSendsInformationAboutAvailableResourcesToNewParticant)
    {
        IResource* resource = CreateTestResource();
        ManagedResource managedRes = localResourceComponent.manageResource(*resource);

        localResourceComponent.newParticipantHasConnected(Guid(222));
    }

    TEST_F(AResourceComponentTest, hasNoArrivedResourcesInitially)
    {
        EXPECT_TRUE(localResourceComponent.popArrivedResources(ResourceRequesterID(1)).empty());
    }

    TEST_F(AResourceComponentTest, keepsTrackOfArrivedResources)
    {
        const NiceMock<ResourceMock>* const lowLevelResourceMock = new NiceMock<ResourceMock>(ResourceContentHash(42, 0), EResourceType_Effect);
        ResourceDeleterCallingCallback deleterMock(DefaultManagedResourceDeleterCallback::GetInstance());
        ManagedResource resource{ lowLevelResourceMock, deleterMock };
        const ResourceRequesterID requesterID(1);
        const Guid providerID(222);

        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(ResourceContentHash(42, 0));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterID, providerID);
        localResourceComponent.handleArrivedResource(resource);
        EXPECT_EQ(1u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, keepsTrackOfArrivedResourcesPerRequesterEach)
    {
        const ResourceRequesterID requesterIDA(1);
        const ResourceRequesterID requesterIDB(2);
        const ResourceContentHash hash1(42, 0);
        const ResourceContentHash hash2(43, 0);
        NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
        ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);

        ResourceMock lowLevelResourceMock(hash1, EResourceType_Effect);
        ManagedResource resource{ &lowLevelResourceMock, deleterMock };
        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(hash1);

        ResourceMock lowLevelResourceMock2(hash2, EResourceType_Effect);
        ManagedResource resource2{ &lowLevelResourceMock2, deleterMock };
        ResourceContentHashVector hashesToRequest2;
        hashesToRequest2.push_back(hash2);

        const Guid providerID(222);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest2));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterIDA, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest2, requesterIDB, providerID);
        localResourceComponent.handleArrivedResource(resource);
        localResourceComponent.handleArrivedResource(resource2);
        ManagedResourceVector arrivedForA = localResourceComponent.popArrivedResources(requesterIDA);
        ManagedResourceVector arrivedForB = localResourceComponent.popArrivedResources(requesterIDB);

        EXPECT_EQ(hash1, arrivedForA[0]->getHash());
        EXPECT_EQ(hash2, arrivedForB[0]->getHash());
    }

    TEST_F(AResourceComponentTest, popsArrivedResources)
    {
        const ResourceContentHash hash1(42, 0);
        const ResourceContentHash hash2(43, 0);
        NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
        ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);
        ResourceMock lowLevelResourceMock(hash1, EResourceType_Effect);
        ManagedResource resource1{ &lowLevelResourceMock, deleterMock };

        ResourceMock lowLevelResourceMock2(hash2, EResourceType_Effect);
        ManagedResource resource2{ &lowLevelResourceMock2, deleterMock };

        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(hash1);
        hashesToRequest.push_back(hash2);

        const Guid providerID(222);
        const ResourceRequesterID requesterID(1);
        const ResourceRequesterID otherResourceRequesterID(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterID, providerID);

        localResourceComponent.handleArrivedResource(resource1);
        localResourceComponent.handleArrivedResource(resource2);
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(otherResourceRequesterID).size());
        EXPECT_EQ(2u, localResourceComponent.popArrivedResources(requesterID).size());
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID).size());
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
            resourceFileToc.readTOCPosAndTOCFromStream(resourceInputStream->resourceStream);

            localResourceComponent.addResourceFile(resourceInputStream, resourceFileToc);
        }

        return hash;
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFile)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();
        ManagedResource fromfile = localResourceComponent.forceLoadResource(hash);
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
        EXPECT_FALSE(localResourceComponent.forceLoadResource(hash));
    }

    TEST_F(AResourceComponentTest, canForceLoadResourceBecauseOfHashUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.forceLoadFromResourceFile(resourceFileName);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));
        EXPECT_TRUE(localResourceComponent.getResource(hash));
    }

    TEST_F(AResourceComponentTest, canForceLoadResourceBecauseOfHashUsageAndRemoveFile)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        auto fakeUsage = localResourceComponent.getResourceHashUsage(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.forceLoadFromResourceFile(resourceFileName);
        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.forceLoadResource(hash));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.forceLoadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.forceLoadResource(hash));
    }

    TEST_F(AResourceComponentTest, canForceLoadResourceFileAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.forceLoadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.forceLoadFromResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_TRUE(localResourceComponent.forceLoadResource(hash));
    }

    TEST_F(AResourceComponentTest, canForceLoadAndRemoveAndKeepsResourceBecauseOfResourceUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        ManagedResource fromfile = localResourceComponent.forceLoadResource(hash);
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.forceLoadFromResourceFile(resourceFileName);
        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.forceLoadResource(hash));
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFileAndDeletesResourceBecauseNoUsage)
    {
        auto hash = setupTest(resourceFileName, localResourceComponent);

        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_EQ(0u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_FALSE(localResourceComponent.getResource(hash));
        EXPECT_FALSE(localResourceComponent.forceLoadResource(hash));
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
        auto res = localResourceComponent.forceLoadResource(hash);
        EXPECT_TRUE(res);
        EXPECT_TRUE(localResourceComponent.getResource(hash));
        res = {};
        EXPECT_FALSE(localResourceComponent.getResource(hash));
    }

    TEST_F(AResourceComponentTest, uncompressesLocallyRequestedResources)
    {
        ResourceContentHashVector hashes = writeMultipleTestResourceFile(2, 2000, true);

        ResourceRequesterID requesterID(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashes, requesterID, Guid(222));

        // execute enqueued resource loading task
        executor.execute();

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_EQ(2u, poppedResources.size());
        EXPECT_TRUE(poppedResources[0]->isCompressedAvailable());
        EXPECT_TRUE(poppedResources[0]->isDeCompressedAvailable());
        EXPECT_TRUE(poppedResources[1]->isCompressedAvailable());
        EXPECT_TRUE(poppedResources[1]->isDeCompressedAvailable());
    }

    TEST_F(AResourceComponentTest, doesNotUncompressRemoteRequestedResources)
    {
        ResourceContentHashVector hashes = writeMultipleTestResourceFile(2, 2000, true);
        Guid requester(123);

        localResourceComponent.handleRequestResources(hashes, requester);

        ManagedResourceVector sentRes;
        expectSendResourcesToNetwork(communicationSystem, requester, sentRes);
        executor.execute();

        ASSERT_EQ(2u, sentRes.size());
        EXPECT_TRUE(sentRes[0]->isCompressedAvailable());
        EXPECT_FALSE(sentRes[0]->isDeCompressedAvailable());
        EXPECT_TRUE(sentRes[1]->isCompressedAvailable());
        EXPECT_FALSE(sentRes[1]->isDeCompressedAvailable());
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

    TEST_F(AResourceComponentWithThreadedTaskExecutorTest, HandleResourceRequest_AsynchronousLoadingResourceFromFileAndSendsIt)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_FALSE(localResourceComponent.getResource(resourceHash));

        Guid requester(222);
        ramses_internal::PlatformEvent syncWaiter;
        {
            ramses_internal::PlatformGuard g(expectLock);
            EXPECT_CALL(communicationSystem, safe_sendResources(requester, _)) .Times(1) .WillRepeatedly(InvokeWithoutArgs([&]()
            {
                syncWaiter.signal();
                return true;
            }));
        }
        localResourceComponent.handleRequestResources(requestedResourceHashes, requester);

        EXPECT_TRUE(syncWaiter.wait(60000));
    }

    class AResourceComponentWithSingleResourceLoadLimitTest : public ResourceComponentTestBase
    {
    public:
        AResourceComponentWithSingleResourceLoadLimitTest()
            : executor()
            , communicationSystem()
            , localResourceComponent(executor, m_myID, communicationSystem, connectionStatusUpdateNotifier, statistics, frameworkLock, byteSizeOfSingleTestResource + 5)
        {}

        ~AResourceComponentWithSingleResourceLoadLimitTest()
        {
            // make sure no tasks pending before ResourceComponent dtor is run to prevent hangs
            executor.executeAll();
        }

        virtual ResourceComponent& getResourceComponent() override
        {
            return localResourceComponent;
        }

        static constexpr uint32_t byteSizeOfSingleTestResource = 75;

    protected:
        DelayedSingleTaskExecutor executor;
        StrictMock<CommunicationSystemMock> communicationSystem;
        StatisticCollectionFramework statistics;
        ResourceComponent localResourceComponent;
        ResourceDeleterCallingCallback deleter;
    };

    TEST_F(AResourceComponentWithSingleResourceLoadLimitTest, LoadsOnlySingleResourceAtATimeWhenTriggeredFromLocal)
    {
        ResourceContentHashVector resourceHashes = writeMultipleTestResourceFile(3);

        ResourceRequesterID requesterID(1);
        Guid        providerID(222);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(resourceHashes, requesterID, providerID);

        // execute enqueued resource loading task
        executor.execute();

        // check that resource was only loaded and returned once
        ManagedResourceVector poppedResources;
        poppedResources  = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_EQ(1u, poppedResources.size());

        // should have no tasks, resource still alive
        EXPECT_FALSE(executor.hasTasks());
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID).size());

        // clear to allow trigger next loading
        poppedResources.clear();

        executor.execute();
        poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_EQ(1u, poppedResources.size());
    }

    TEST_F(AResourceComponentWithSingleResourceLoadLimitTest, LoadsOnlySingleResourceAtATimeWhenTriggeredFromRemote)
    {
        ResourceContentHashVector resourceHashes = writeMultipleTestResourceFile(2);

        Guid requesterID(222);
        localResourceComponent.handleRequestResources(resourceHashes, requesterID);

        // execute enqueued resource loading task
        ManagedResourceVector loadedResources;
        expectSendResourcesToNetwork(communicationSystem, requesterID, loadedResources);
        executor.execute();
        Mock::VerifyAndClearExpectations(&communicationSystem);

        // check that single resource was only loaded and sent
        EXPECT_EQ(1u, loadedResources.size());
        EXPECT_FALSE(executor.hasTasks());

        // clear and allow loading next one
        loadedResources.clear();
        expectSendResourcesToNetwork(communicationSystem, requesterID, loadedResources);
        executor.execute();
        Mock::VerifyAndClearExpectations(&communicationSystem);
        EXPECT_EQ(1u, loadedResources.size());
    }
}
