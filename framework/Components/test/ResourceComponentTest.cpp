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
#include "gmock/gmock-generated-actions.h"
#include "Resource/ArrayResource.h"
#include "Utils/BinaryFileOutputStream.h"
#include "Utils/BinaryOutputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "PlatformAbstraction/PlatformEvent.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "DummyResource.h"
#include "Collections/String.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "CommunicationSystemMock.h"
#include "Utils/ScopedPointer.h"
#include "TaskFramework/ThreadedTaskExecutor.h"
#include "ResourceSerializationTestHelper.h"
#include "Components/SingleResourceSerialization.h"

using namespace testing;

namespace ramses_internal
{
    ACTION_P(ReleaseSyncCall, syncer)
    {
        UNUSED(arg9);
        UNUSED(arg8);
        UNUSED(arg7);
        UNUSED(arg6);
        UNUSED(arg5);
        UNUSED(arg4);
        UNUSED(arg3);
        UNUSED(arg2);
        UNUSED(arg1);
        UNUSED(arg0);
        UNUSED(args);
        syncer->signal();
    }

    class DelayedSingleTaskExecutor : public ITaskQueue
    {
    public:
        DelayedSingleTaskExecutor()
        {
        }

        virtual Bool enqueue(ITask& Task)
        {
            Task.addRef();
            m_scheduledTasks.push_back(&Task);
            return true;
        }

        virtual void enableAcceptingTasks()
        {
        }

        virtual void disableAcceptingTasksAfterExecutingCurrentQueue()
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
            : m_myID(true)
            , resourceFileName("test.resource")
        {
        }

        virtual ResourceComponent& getResourceComponent() = 0;

        virtual ~ResourceComponentTestBase()
        {
            deleteTestResourceFile();
        }

        static IResource* CreateTestResource(float someValue = 0.0f)
        {
            const Float vertexData[] = {
                0.0f, 0.0f, 0.0f,
                1.0f, 0.0f, 0.0f,
                1.0f, 1.0f, someValue
            };

            ArrayResource* resource = new ArrayResource(EResourceType_VertexArray, 3, EDataType_Vector3F, reinterpret_cast<const Byte*>(vertexData), ResourceCacheFlag(0u), String("resName"));
            return resource;
        }

        static ResourceContentHashVector HashesFromManagedResources(const ManagedResourceVector& vec)
        {
            ResourceContentHashVector hashes;
            for (const auto& res : vec)
            {
                hashes.push_back(res.getResourceObject()->getHash());
            }
            return hashes;
        }

        ResourceContentHashVector writeMultipleTestResourceFile(UInt32 num)
        {
            ResourceComponent& localResourceComponent = getResourceComponent();

            ManagedResourceVector managedResourceVec;
            ResourceContentHashVector hashes;

            for (UInt32 i = 0; i < num; ++i)
            {
                IResource* resource = CreateTestResource(i*1.0f);
                ManagedResource managedResource = localResourceComponent.manageResource(*resource, true);
                managedResourceVec.push_back(managedResource);
                hashes.push_back(managedResource.getResourceObject()->getHash());
            }

            {
                File resourceFile(resourceFileName);
                BinaryFileOutputStream resourceOutputStream(resourceFile);
                ResourcePersistation::WriteNamedResourcesWithTOCToStream(resourceOutputStream, managedResourceVec, false);
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
            const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
            const IResource* res = resVec.front().getResourceObject();
            const ResourceContentHash hash = res->getHash();

            const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, std::numeric_limits<UInt32>::max());
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
        SafeCommunicationSystemMock(ramses_internal::PlatformLock& lock)
            : m_lock(lock)
        {
        }

        MOCK_METHOD2(safe_sendResources, bool(const Guid& to, const ManagedResourceVector& resources));

    private:
        bool sendResources(const Guid& to, const ManagedResourceVector& resources) override
        {
            ramses_internal::PlatformGuard g(m_lock);
            return safe_sendResources(to, resources);
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

        RequesterID requesterID(1);
        Guid providerID(true);
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
        EXPECT_EQ(dummyResourceHash, fetchedResource.getResourceObject()->getHash());
        EXPECT_EQ(dummyResource, fetchedResource.getResourceObject());
    }

    TEST_F(AResourceComponentTest, ResolveResources_RequestResourceFromProviderIfNotAvailableLocaly)
    {
        Guid providerID(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(ResourceContentHash(123, 456));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResourceHashes));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, RequesterID(1), providerID);
    }

    TEST_F(AResourceComponentTest, ResolveResources_RequestResourceFromProviderIfNotAvailableLocalyAnymore)
    {
        const ResourceContentHash dummyResourceHash(47, 0);
        {
            DummyResource* dummyResource = new DummyResource(dummyResourceHash, EResourceType_VertexArray);
            ManagedResource dummyManagedResource = localResourceComponent.manageResource(*dummyResource);
        }

        Guid providerID(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(dummyResourceHash);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResourceHashes));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, RequesterID(1), providerID);
    }

    TEST_F(AResourceComponentTest, ResolveResources_TriggersLoadingOfAvailableResourceFromFile)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(NULL, localResourceComponent.getResource(resourceHash).getResourceObject());

        RequesterID requesterID(1);
        Guid providerID(true);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        // execute enqueued resource loading task
        executor.execute();

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_TRUE(poppedResources.size() > 0);
        ManagedResource resource = poppedResources.back();
        ASSERT_EQ(resourceHash, resource.getResourceObject()->getHash());

        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue());
        EXPECT_EQ(SingleResourceSerialization::SizeOfSerializedResource(*resource.getResourceObject()), statistics.statResourcesLoadedFromFileSize.getCounterValue());
    }

    TEST_F(AResourceComponentTest, RequestHashMultipleTimesAsyncBeforeLoadingThreadExecutes)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(NULL, localResourceComponent.getResource(resourceHash).getResourceObject());

        RequesterID requesterID(1);
        Guid        providerID(true);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

        // execute enqueued resource loading task
        executor.execute();

        // check that resource was only loaded and returned once
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_TRUE(poppedResources.size() > 0);
        ManagedResource resource = poppedResources.back();
        ASSERT_EQ(resourceHash, resource.getResourceObject()->getHash());
    }

    TEST_F(AResourceComponentTest, HandleRequestHashMultipleTimesBeforeLoadingThreadExecutes)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash       resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(NULL, localResourceComponent.getResource(resourceHash).getResourceObject());

        RequesterID requesterID(1);
        Guid        providerID(true);
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0, providerID);
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0, providerID);

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
            EXPECT_EQ(NULL, localResourceComponent.getResource(resourceHash).getResourceObject());

            RequesterID requesterID(1);
            Guid        providerID(true);
            localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, providerID);

            // execute enqueued resource loading task
            executor.execute();

            ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
            ASSERT_TRUE(poppedResources.size() > 0);
            ManagedResource resource = poppedResources.back();
            poppedResources.pop_back();
            ASSERT_EQ(resourceHash, resource.getResourceObject()->getHash());
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
            EXPECT_EQ(NULL, localResourceComponent.getResource(resourceHash).getResourceObject());

            RequesterID requesterID(1);
            Guid        providerID(true);
            localResourceComponent.requestResourceAsynchronouslyFromFramework(
                requestedResourceHashes, requesterID, providerID);

            // execute enqueued resource loading task
            executor.execute();

            ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
            ASSERT_TRUE(poppedResources.size() > 0);
            ManagedResource resource = poppedResources.back();
            poppedResources.pop_back();
            ASSERT_EQ(resourceHash, resource.getResourceObject()->getHash());
            resource = ManagedResource();

            ASSERT_EQ(0u, localResourceComponent.getResources().size());
        }
    }

    TEST_F(AResourceComponentTest, RequestResourcesAsync_RequestResourceFromProviderIfNotAvailableLocalyAnymore)
    {
        ResourceContentHashVector requestedResourceHashes;
        {
            ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());
            requestedResourceHashes.push_back(managedResource.getResourceObject()->getHash());
        }

        // make sure managed resource created before was freed,
        // so resolveResources() has to trigger loading it from file
        EXPECT_EQ(localResourceComponent.getResources().size(), 0u);

        Guid providerID(true);
        RequesterID requesterID(1);
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
        ByteArrayView view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        RequesterID requesterID(1);
        Guid providerID(true);
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
        Guid providerID(true);
        localResourceComponent.newParticipantHasConnected(providerID);
        const auto testRes = CreateTestResourceData();
        ByteArrayView view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        RequesterID requesterID_1(1);
        RequesterID requesterID_2(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
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
        Guid providerID(true);
        localResourceComponent.newParticipantHasConnected(providerID);
        const auto testRes = CreateTestResourceData();
        ByteArrayView view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(testRes.hash);

        RequesterID requesterID_1(1);
        RequesterID requesterID_2(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
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
        Guid providerID(true);

        RequesterID requesterID(1);
        localResourceComponent.newParticipantHasConnected(providerID);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        localResourceComponent.cancelResourceRequest(testRes.hash, requesterID);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        ByteArrayView view(testRes.data.data(), static_cast<UInt32>(testRes.data.size()));
        localResourceComponent.handleSendResource(view, providerID);

        EXPECT_EQ(1u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, handleResourcesNotAvailableCancelsAllResourceRequestsForThisResource)
    {
        ResourceContentHash hash(5, 0);
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(hash);
        Guid providerID(true);
        RequesterID requesterID_1(1);
        RequesterID requesterID_2(2);
        localResourceComponent.newParticipantHasConnected(providerID);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_1, providerID);
        Mock::VerifyAndClearExpectations(&communicationSystem);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID_2, providerID);

        EXPECT_TRUE(localResourceComponent.hasRequestForResource(hash, requesterID_1));
        EXPECT_TRUE(localResourceComponent.hasRequestForResource(hash, requesterID_2));

        localResourceComponent.handleResourcesNotAvailable(requestedResources, providerID);

        EXPECT_FALSE(localResourceComponent.hasRequestForResource(hash, requesterID_1));
        EXPECT_FALSE(localResourceComponent.hasRequestForResource(hash, requesterID_2));
    }

    TEST_F(AResourceComponentTest, CanHandleMultipleResourceHashesInOneRequest)
    {
        Guid providerID(true);
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        ResourceInfo res2(EResourceType_VertexArray, ResourceContentHash(2u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources;
        requestedResources.push_back(res1.hash);
        requestedResources.push_back(res2.hash);

        ResourceInfoVector knownResources;
        knownResources.push_back(res1);
        knownResources.push_back(res2);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, DoesNotRequestAlreadyRequestedResource)
    {
        ResourceInfo res(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        Guid providerID(true);
        ResourceContentHashVector requestedResources(1, res.hash);
        ResourceInfoVector knownResources(1, res);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
        Mock::VerifyAndClearExpectations(&communicationSystem);

        EXPECT_CALL(communicationSystem, sendRequestResources(_, _)).Times(0);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, FiltersOutAlreadyRequestedResourcesFromProvider)
    {
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        ResourceInfo res2(EResourceType_VertexArray, ResourceContentHash(2u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources_single;
        requestedResources_single.push_back(res1.hash);

        ResourceContentHashVector requestedResources_other;
        requestedResources_other.push_back(res2.hash);

        ResourceContentHashVector requestedResources_all;
        requestedResources_all.push_back(res1.hash);
        requestedResources_all.push_back(res2.hash);

        RequesterID requesterID(1);
        Guid providerID(true);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources_single)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_single, requesterID, providerID);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources_other)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_all, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, FiltersOutAlreadyRequestedResourcesFromDifferentProvider)
    {
        ResourceInfo res1(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);
        ResourceInfo res2(EResourceType_VertexArray, ResourceContentHash(2u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources_single;
        requestedResources_single.push_back(res1.hash);

        ResourceContentHashVector requestedResources_other;
        requestedResources_other.push_back(res2.hash);

        ResourceContentHashVector requestedResources_all;
        requestedResources_all.push_back(res1.hash);
        requestedResources_all.push_back(res2.hash);

        RequesterID requesterID(1);
        Guid provider1(true);
        Guid provider2(true);
        EXPECT_CALL(communicationSystem, sendRequestResources(provider1, requestedResources_single)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_single, requesterID, provider1);

        EXPECT_CALL(communicationSystem, sendRequestResources(provider2, requestedResources_other)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources_all, requesterID, provider2);
    }

    TEST_F(AResourceComponentTest, AllowsRequestingResourceAgainIfClientAnsweredResourceNotAvailable)
    {
        ResourceInfo res(EResourceType_VertexArray, ResourceContentHash(1u, 0), 0u, 0u);

        ResourceContentHashVector requestedResources(1, res.hash);
        ResourceInfoVector knownResources(1, res);
        Guid clientId(true);

        RequesterID requesterID(1);
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
        Guid client1(true);
        Guid client2(true);
        Guid providerID(true);

        RequesterID requesterID(1);
        ResourceContentHashVector requestedResources;
        requestedResources.push_back(res1.hash);

        ResourceInfoVector knownResources;
        knownResources.push_back(res1);

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, requestedResources)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResources, requesterID, providerID);
    }

    TEST_F(AResourceComponentTest, CanHandleResourceReceivedFromSource)
    {
        const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
        const ResourceContentHash hash = resVec[0].getResourceObject()->getHash();
        const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, std::numeric_limits<UInt32>::max());
        ASSERT_EQ(1u, dataVec.size());

        const auto resourceData = dataVec[0];
        ByteArrayView view(resourceData.data(), static_cast<UInt32>(resourceData.size()));

        const Guid dummyGuid(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        localResourceComponent.handleSendResource(view, m_myID);

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ManagedResource managedResource = poppedResources.back();
        ASSERT_EQ(hash, managedResource.getResourceObject()->getHash());
    }

    TEST_F(AResourceComponentTest, PartiallyReceivedResourcesAreNotPassedToArrivedResources)
    {
        const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
        const ResourceContentHash hash = resVec[0].getResourceObject()->getHash();
        const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const auto resourceData = dataVec[0];
        ByteArrayView view(resourceData.data(), static_cast<UInt32>(resourceData.size()));

        const Guid dummyGuid(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        localResourceComponent.handleSendResource(view, m_myID);

        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());
    }

    TEST_F(AResourceComponentTest, ResourcesArePassedToArrivedResourcesWhenAllFragmentsAreReceived)
    {
        const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
        const ResourceContentHash hash = resVec[0].getResourceObject()->getHash();

        const IResource* testResource = resVec[0].getResourceObject();
        const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const Guid dummyGuid(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);

        // "receive" all except last
        for (UInt i = 0; i < dataVec.size() - 1; ++i)
        {
            ByteArrayView view(dataVec[i].data(), static_cast<UInt32>(dataVec[i].size()));
            localResourceComponent.handleSendResource(view, m_myID);
        }
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());

        // "receive" last chunk
        ByteArrayView view(dataVec.back().data(), static_cast<UInt32>(dataVec.back().size()));
        localResourceComponent.handleSendResource(view, m_myID);

        poppedResources = localResourceComponent.popArrivedResources(requesterID);
        ASSERT_EQ(1u, poppedResources.size());

        ManagedResource managedResource = poppedResources.back();
        EXPECT_EQ(hash, managedResource.getResourceObject()->getHash());
        ASSERT_EQ(testResource->getDecompressedDataSize(), managedResource.getResourceObject()->getDecompressedDataSize());

        const UInt8* resourceDataBlob = managedResource.getResourceObject()->getResourceData()->getRawData();
        EXPECT_TRUE(0 == PlatformMemory::Compare(testResource->getResourceData()->getRawData(), resourceDataBlob, testResource->getDecompressedDataSize()));
    }

    TEST_F(AResourceComponentTest, ResourceFragmentsFromOtherParticipantsAreIgnored)
    {
        const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
        const IResource* testResource = resVec[0].getResourceObject();
        const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        const auto resourceData = dataVec[0];
        ResourceInfo availableResource(EResourceType_VertexArray, testResource->getHash(), testResource->getDecompressedDataSize(), 0u);

        const Guid dummyGuid(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(availableResource.hash);

        Guid otherParticipant(true);
        localResourceComponent.newParticipantHasConnected(m_myID);
        localResourceComponent.newParticipantHasConnected(otherParticipant);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);

        // "receive" all except last from one participant
        for (UInt i = 0; i < dataVec.size() - 1; ++i)
        {
            ByteArrayView view(dataVec[i].data(), static_cast<UInt32>(dataVec[i].size()));
            localResourceComponent.handleSendResource(view, m_myID);
        }
        ManagedResourceVector poppedResources = localResourceComponent.popArrivedResources(requesterID);
        EXPECT_EQ(0u, poppedResources.size());

        // "receive" last chunk from other participant
        ByteArrayView view(dataVec.back().data(), static_cast<UInt32>(dataVec.back().size()));
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
        const ManagedResourceVector resVec = { ManagedResource(*CreateTestResource(), deleter) };
        const auto dataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resVec, 50);
        ASSERT_GT(dataVec.size(), 1u);

        ResourceInfo availableResource(EResourceType_VertexArray, resVec[0].getResourceObject()->getHash(), resVec[0].getResourceObject()->getDecompressedDataSize(), 0u);

        const Guid dummyGuid(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(availableResource.hash);

        localResourceComponent.newParticipantHasConnected(m_myID);

        RequesterID requesterID(1);
        EXPECT_CALL(communicationSystem, sendRequestResources(dummyGuid, _)).Times(1);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, requesterID, dummyGuid);
        ByteArrayView view(dataVec[0].data(), static_cast<UInt32>(dataVec[0].size()));
        localResourceComponent.handleSendResource(view, m_myID);
        EXPECT_TRUE(localResourceComponent.isReceivingFromParticipant(m_myID));

        localResourceComponent.participantHasDisconnected(m_myID);
        EXPECT_FALSE(localResourceComponent.isReceivingFromParticipant(m_myID));
    }

    TEST_F(AResourceComponentTest, CanDeleteResourcesLoadedFromFile)
    {
        const Guid dummyGuid(true);
        const ResourceContentHash dummyResourceHash(47, 0);
        const ResourceInfo dummyResourceInfo(EResourceType_VertexArray, dummyResourceHash, 2u, 1u);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(dummyResourceHash);

        ResourceWithDestructorMock* resource = new ResourceWithDestructorMock(dummyResourceHash, EResourceType_VertexArray);

        RequesterID requesterID(1);
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
        requestedResourceHashes.push_back(managedResource.getResourceObject()->getHash());
        Guid requesterId(true);

        EXPECT_CALL(communicationSystem, sendResources(requesterId, _));
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requesterId);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_SendsMultipleExistingResourcesAtOnce)
    {
        ManagedResourceVector managedResources = { localResourceComponent.manageResource(*CreateTestResource(0.f)),
            localResourceComponent.manageResource(*CreateTestResource(1.f)),
            localResourceComponent.manageResource(*CreateTestResource(2.f)) };

        ResourceContentHashVector requestedResourceHashes;
        for (const auto& res : managedResources)
        {
            requestedResourceHashes.push_back(res.getResourceObject()->getHash());
        }

        Guid requesterId(true);
        ManagedResourceVector sentResources;
        EXPECT_CALL(communicationSystem, sendResources(requesterId, _)).WillOnce(DoAll(SaveArg<1>(&sentResources), Return(true)));
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requesterId);
        EXPECT_EQ(managedResources, sentResources);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_LoadsResourceFromFileAndSendsIt)
    {
        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_EQ(0, localResourceComponent.getResource(resourceHash).getResourceObject());

        Guid requester(true);
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requester);

        // execute enqueued resource loading task
        EXPECT_CALL(communicationSystem, sendResources(requester, _));
        executor.execute();
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_LoadsMultipleResourcesFromFileAndSendsThem)
    {
        ResourceContentHashVector requestedResourceHashes = writeMultipleTestResourceFile(4);

        Guid requester(true);
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requester);

        // execute enqueued resource loading task
        ManagedResourceVector sentResources;
        EXPECT_CALL(communicationSystem, sendResources(requester, _)).WillOnce(DoAll(SaveArg<1>(&sentResources), Return(true)));
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
        ResourceContentHashVector requestedResourceHashes = { localRes[0].getResourceObject()->getHash(),
            fileResHashes[0],
            localRes[1].getResourceObject()->getHash(),
            fileResHashes[1],
            localRes[2].getResourceObject()->getHash() };

        Guid requester(true);

        ManagedResourceVector sentLocalResources;
        EXPECT_CALL(communicationSystem, sendResources(requester, _)).WillOnce(DoAll(SaveArg<1>(&sentLocalResources), Return(true)));
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requester);
        Mock::VerifyAndClearExpectations(&communicationSystem);
        EXPECT_EQ(localRes, sentLocalResources);

        ManagedResourceVector sentFileResources;
        EXPECT_CALL(communicationSystem, sendResources(requester, _)).WillOnce(DoAll(SaveArg<1>(&sentFileResources), Return(true)));
        executor.execute();
        EXPECT_EQ(fileResHashes, HashesFromManagedResources(sentFileResources));
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_TriesToLoadResourceFromNonExistentFile)
    {
        ResourceContentHashVector requestedResourceHashes;

        {
            ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());
            requestedResourceHashes.push_back(managedResource.getResourceObject()->getHash());
        }

        // make sure managed resource created before was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_EQ(localResourceComponent.getResources().size(), 0u);

        Guid requesterId(true);
        EXPECT_CALL(communicationSystem, sendResourcesNotAvailable(requesterId, requestedResourceHashes));
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requesterId);
    }

    TEST_F(AResourceComponentTest, HandleResourceRequest_sendNotAvailableForUnknownResources)
    {
        const ResourceContentHash hash1(1u, 0);
        const ResourceContentHash hash2(2u, 0);
        ManagedResource managedResource = localResourceComponent.manageResource(*CreateTestResource());

        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(managedResource.getResourceObject()->getHash());
        requestedResourceHashes.push_back(hash1);
        requestedResourceHashes.push_back(hash2);

        ResourceContentHashVector unknownResources;
        unknownResources.push_back(hash1);
        unknownResources.push_back(hash2);

        Guid requesterId(true);
        EXPECT_CALL(communicationSystem, sendResources(requesterId, _));
        EXPECT_CALL(communicationSystem, sendResourcesNotAvailable(requesterId, unknownResources));
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requesterId);
    }


    TEST_F(AResourceComponentTest, SendsRemoteRequestToProviderIfAvailabilityUnknown)
    {
        Guid providerID(true);
        ResourceContentHashVector requestedResourceHashes;
        requestedResourceHashes.push_back(ResourceContentHash(47, 0));

        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(requestedResourceHashes, RequesterID(1), providerID);
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

        localResourceComponent.newParticipantHasConnected(Guid(true));
    }

    TEST_F(AResourceComponentTest, hasNoArrivedResourcesInitially)
    {
        EXPECT_TRUE(localResourceComponent.popArrivedResources(RequesterID(1)).empty());
    }

    TEST_F(AResourceComponentTest, keepsTrackOfArrivedResources)
    {
        const NiceMock<ResourceMock>* const lowLevelResourceMock = new NiceMock<ResourceMock>(ResourceContentHash(42, 0), EResourceType_Effect);
        ResourceDeleterCallingCallback deleterMock(DefaultManagedResourceDeleterCallback::GetInstance());
        ManagedResource resource(*lowLevelResourceMock, deleterMock);
        const RequesterID requesterID(1);
        const Guid providerID(true);

        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(ResourceContentHash(42, 0));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, _));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterID, providerID);
        localResourceComponent.handleArrivedResource(resource);
        EXPECT_EQ(1u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, keepsTrackOfArrivedResourcesPerRequesterEach)
    {
        const RequesterID requesterIDA(1);
        const RequesterID requesterIDB(2);
        const ResourceContentHash hash1(42, 0);
        const ResourceContentHash hash2(43, 0);
        NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
        ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);

        ResourceMock lowLevelResourceMock(hash1, EResourceType_Effect);
        ManagedResource resource(lowLevelResourceMock, deleterMock);
        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(hash1);

        ResourceMock lowLevelResourceMock2(hash2, EResourceType_Effect);
        ManagedResource resource2(lowLevelResourceMock2, deleterMock);
        ResourceContentHashVector hashesToRequest2;
        hashesToRequest2.push_back(hash2);

        const Guid providerID(true);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest));
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest2));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterIDA, providerID);
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest2, requesterIDB, providerID);
        localResourceComponent.handleArrivedResource(resource);
        localResourceComponent.handleArrivedResource(resource2);
        ManagedResourceVector arrivedForA = localResourceComponent.popArrivedResources(requesterIDA);
        ManagedResourceVector arrivedForB = localResourceComponent.popArrivedResources(requesterIDB);

        EXPECT_EQ(hash1, arrivedForA[0].getResourceObject()->getHash());
        EXPECT_EQ(hash2, arrivedForB[0].getResourceObject()->getHash());
    }

    TEST_F(AResourceComponentTest, popsArrivedResources)
    {
        const ResourceContentHash hash1(42, 0);
        const ResourceContentHash hash2(43, 0);
        NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
        ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);
        ResourceMock lowLevelResourceMock(hash1, EResourceType_Effect);
        ManagedResource resource1(lowLevelResourceMock, deleterMock);

        ResourceMock lowLevelResourceMock2(hash2, EResourceType_Effect);
        ManagedResource resource2(lowLevelResourceMock2, deleterMock);

        ResourceContentHashVector hashesToRequest;
        hashesToRequest.push_back(hash1);
        hashesToRequest.push_back(hash2);

        const Guid providerID(true);
        const RequesterID requesterID(1);
        const RequesterID otherRequesterID(2);
        EXPECT_CALL(communicationSystem, sendRequestResources(providerID, hashesToRequest));
        localResourceComponent.requestResourceAsynchronouslyFromFramework(hashesToRequest, requesterID, providerID);

        localResourceComponent.handleArrivedResource(resource1);
        localResourceComponent.handleArrivedResource(resource2);
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(otherRequesterID).size());
        EXPECT_EQ(2u, localResourceComponent.popArrivedResources(requesterID).size());
        EXPECT_EQ(0u, localResourceComponent.popArrivedResources(requesterID).size());
    }

    TEST_F(AResourceComponentTest, canRemoveResourceFile)
    {
        // setup test
        const Float vertexData[] = {
            0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f
        };
        ArrayResource* resource = new ArrayResource(EResourceType_VertexArray, 3, EDataType_Vector3F, reinterpret_cast<const Byte*>(vertexData), ResourceCacheFlag(0u), String("resName"));
        ResourceContentHash hash = resource->getHash();
        {
            File resourceFile(resourceFileName);
            BinaryFileOutputStream resourceOutputStream(resourceFile);

            NiceMock<ManagedResourceDeleterCallbackMock> deleterCallbackMock;
            ResourceDeleterCallingCallback deleterMock(deleterCallbackMock);
            ManagedResource resource1(*resource, deleterMock);

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


        // test
        const auto loadedResourcesBefore = statistics.statResourcesLoadedFromFileNumber.getCounterValue();
        ManagedResource fromfile = localResourceComponent.forceLoadResource(hash);
        EXPECT_TRUE(fromfile.getResourceObject() != 0);
        EXPECT_TRUE(localResourceComponent.hasResourceFile(resourceFileName));
        EXPECT_EQ(1u, statistics.statResourcesLoadedFromFileNumber.getCounterValue() - loadedResourcesBefore);

        localResourceComponent.removeResourceFile(resourceFileName);
        EXPECT_FALSE(localResourceComponent.hasResourceFile(resourceFileName));

        EXPECT_TRUE(fromfile.getResourceObject() != 0);
        fromfile = ManagedResource();
        EXPECT_TRUE(fromfile.getResourceObject() == 0);
    }

    TEST_F(AResourceComponentWithThreadedTaskExecutorTest, HandleResourceRequest_AsynchronousLoadingResourceFromFileAndSendsIt)
    {
        executor.start();

        ResourceContentHashVector requestedResourceHashes;
        ResourceContentHash resourceHash = writeTestResourceFile();
        requestedResourceHashes.push_back(resourceHash);

        // make sure managed resource created in writeTestResourceFile() was freed,
        // so handleResourceRequest has to load it from file before sending
        EXPECT_EQ(0, localResourceComponent.getResource(resourceHash).getResourceObject());

        Guid requester(true);
        ramses_internal::PlatformEvent syncWaiter;
        {
            ramses_internal::PlatformGuard g(expectLock);
            EXPECT_CALL(communicationSystem, safe_sendResources(requester, _)).Times(1).WillRepeatedly(DoAll(ReleaseSyncCall(&syncWaiter), Return(true)));
        }
        localResourceComponent.handleRequestResources(requestedResourceHashes, 0u, requester);

        const EStatus status = syncWaiter.wait(60000);
        EXPECT_EQ(EStatus_RAMSES_OK, status);
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

        RequesterID requesterID(1);
        Guid        providerID(true);
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

        Guid requesterID(true);
        localResourceComponent.handleRequestResources(resourceHashes, 0, requesterID);

        // execute enqueued resource loading task
        ManagedResourceVector loadedResources;
        EXPECT_CALL(communicationSystem, sendResources(requesterID, _)).WillOnce(DoAll(SaveArg<1>(&loadedResources), Return(true)));
        executor.execute();
        Mock::VerifyAndClearExpectations(&communicationSystem);

        // check that single resource was only loaded and sent
        EXPECT_EQ(1u, loadedResources.size());
        EXPECT_FALSE(executor.hasTasks());

        // clear and allow loading next one
        loadedResources.clear();
        EXPECT_CALL(communicationSystem, sendResources(requesterID, _)).WillOnce(DoAll(SaveArg<1>(&loadedResources), Return(true)));
        executor.execute();
        Mock::VerifyAndClearExpectations(&communicationSystem);
        EXPECT_EQ(1u, loadedResources.size());
    }
}
