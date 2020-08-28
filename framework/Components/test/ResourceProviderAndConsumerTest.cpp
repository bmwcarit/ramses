//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ResourceMock.h"
#include "ComponentMocks.h"
#include "Components/ManagedResource.h"
#include "Resource/ArrayResource.h"
#include "Components/ResourceComponent.h"
#include "MockConnectionStatusUpdateNotifier.h"
#include "MockTaskQueue.h"
#include "ForwardingCommunicationSystem.h"
#include "ProviderAndConsumerBaseTest.h"


namespace ramses_internal
{
    using namespace testing;

    static float gSingleFloatArray[1] = {123.0f};

    class AResourceProviderAndConsumerTest : public ProviderAndConsumerBaseTest
    {
    public:
        AResourceProviderAndConsumerTest()
            : resourceConsumer(taskQueue, consumerGuid, consumerCommunicationSystem, consumerConnectionStatusUpdateNotifier, statistics, frameworkLock)
            , resourceProvider(taskQueue, providerGuid, providerCommunicationSystem, providerConnectionStatusUpdateNotifier, statistics, frameworkLock)
        {}

        ArrayResource* createArrayResource()
        {
            return new ArrayResource(EResourceType_VertexArray, 1, EDataType::Float, gSingleFloatArray, ResourceCacheFlag(0u), "dummy1");
        }

    protected:
        StatisticCollectionFramework statistics;
        ResourceComponent resourceConsumer;
        ResourceComponent resourceProvider;
    };

    class VertexArrayResourceWithMockDestructor : public ArrayResource
    {
    public:
        VertexArrayResourceWithMockDestructor()
            : ArrayResource(EResourceType_VertexArray, 1, EDataType::Float, gSingleFloatArray, ResourceCacheFlag(0u), "dummy1")
        {}
        ~VertexArrayResourceWithMockDestructor()
        {
            Die();
        }

        MOCK_METHOD(void, Die, ());
    };

    TEST_F(AResourceProviderAndConsumerTest, InformOfAvailableResourceCanDeclareLocationOfResource)
    {
        ArrayResource* arrayResource = createArrayResource();
        const ResourceContentHash resourceHash = arrayResource->getHash();
        ManagedResource providerManagedResource = resourceProvider.manageResource(*arrayResource);
        resourceProvider.newParticipantHasConnected(consumerGuid);
        resourceConsumer.newParticipantHasConnected(providerGuid);

        ResourceRequesterID requesterID(1);
        ResourceContentHashVector consumerRequestedResources;
        consumerRequestedResources.push_back(resourceHash);
        resourceConsumer.requestResourceAsynchronouslyFromFramework(consumerRequestedResources, requesterID, providerGuid);

        ManagedResourceVector popedResources = resourceConsumer.popArrivedResources(requesterID);
        ASSERT_EQ(1u, popedResources.size());
        ManagedResource resource = popedResources.back();
        EXPECT_EQ(resourceHash, resource->getHash());
    }

    TEST_F(AResourceProviderAndConsumerTest, ConsumerResourcesIsValidEvenAfterProviderResourceDeleted)
    {
        VertexArrayResourceWithMockDestructor* providerResourceMock = new VertexArrayResourceWithMockDestructor();
        const ResourceContentHash resourceHash = providerResourceMock->getHash();

        ManagedResource consumerManagedResource;

        {
            ManagedResource providerManagedResource = resourceProvider.manageResource(*providerResourceMock);
            resourceProvider.newParticipantHasConnected(consumerGuid);
            resourceConsumer.newParticipantHasConnected(providerGuid);

            ResourceContentHashVector consumerRequestedResources;
            consumerRequestedResources.push_back(resourceHash);
            ResourceRequesterID requesterID(1);
            resourceConsumer.requestResourceAsynchronouslyFromFramework(consumerRequestedResources, requesterID, providerGuid);

            EXPECT_CALL(*providerResourceMock, Die());
            consumerManagedResource = resourceConsumer.popArrivedResources(requesterID).front();
        }

        //Provider resource must be ALREADY destroyed because it went out of scope
        Mock::VerifyAndClearExpectations(providerResourceMock); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
        ASSERT_EQ(0u, resourceProvider.getResources().size()); //double check

        const IResource* consumerResource = consumerManagedResource.get();
        ASSERT_TRUE(nullptr != consumerResource);
        EXPECT_EQ(EResourceType_VertexArray, consumerResource->getTypeID());
        EXPECT_EQ(resourceHash, consumerResource->getHash());
    }

    TEST_F(AResourceProviderAndConsumerTest, IgnoresResourceRequestIfProviderDeletedResourceBeforeRequestIsMade)
    {
        VertexArrayResourceWithMockDestructor* providerResourceMock = new VertexArrayResourceWithMockDestructor();
        const ResourceContentHash resourceHash = providerResourceMock->getHash();
        resourceConsumer.newParticipantHasConnected(providerGuid);

        {
            ManagedResource providerManagedResource = resourceProvider.manageResource(*providerResourceMock);
            EXPECT_CALL(*providerResourceMock, Die());
        }

        //Provider resource must be ALEADY destroyed because it went out of scope
        Mock::VerifyAndClearExpectations(providerResourceMock); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
        ASSERT_EQ(0u, resourceProvider.getResources().size()); //double check

        ResourceRequesterID requesterID(1);
        ResourceContentHashVector consumerRequestedResources;
        consumerRequestedResources.push_back(resourceHash);
        resourceConsumer.requestResourceAsynchronouslyFromFramework(consumerRequestedResources, requesterID, providerGuid);

        EXPECT_EQ(0u, resourceConsumer.popArrivedResources(requesterID).size());
    }
}
