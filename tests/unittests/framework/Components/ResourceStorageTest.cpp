//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ResourceStorage.h"
#include "ComponentMocks.h"
#include "internal/Components/ManagedResource.h"
#include "ResourceMock.h"
#include "DummyResource.h"
#include "internal/Core/Utils/StatisticCollection.h"

using namespace testing;

namespace ramses::internal
{
    class AResourceStorage : public testing::Test
    {
    public:
        AResourceStorage()
            : storage(lock, stats)
        {
        }

        static void ExpectResourceSizeCalls(const ResourceMock* resource)
        {
            EXPECT_CALL(*resource, getCompressedDataSize()).Times(AnyNumber());
            EXPECT_CALL(*resource, getDecompressedDataSize()).Times(AnyNumber());
        }

    protected:
        PlatformLock lock;
        StatisticCollectionFramework stats;
        ResourceStorage storage;
    };

    TEST_F(AResourceStorage, InitiallyContainsNoResources)
    {
        EXPECT_EQ(0u, storage.getResources().size());
    }

    TEST_F(AResourceStorage, DeletesResourceWhenManagedResourceGoesOutOfScope)
    {
        auto* resource = new ResourceWithDestructorMock(ResourceContentHash(456u, 0), EResourceType::Invalid);
        {
            ExpectResourceSizeCalls(resource);
            EXPECT_CALL(*resource, Die());
            ManagedResource managed = storage.manageResource(*resource);
        }

        Mock::VerifyAndClearExpectations(resource); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
    }

    TEST_F(AResourceStorage, DoesNotDeleteResourceWhenManagedResourceGoesOutOfScopeUntilHashUsageIsAlsoGone)
    {
        const ResourceContentHash hash(456u, 0);
        auto* resource = new ResourceWithDestructorMock(hash, EResourceType::Invalid);
        // take one hash usage
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        // take one resource data usage
        ExpectResourceSizeCalls(resource);
        ManagedResource managed = storage.manageResource(*resource);

        // let go of data
        EXPECT_CALL(*resource, Die()).Times(0);
        managed = ManagedResource();

        Mock::VerifyAndClearExpectations(resource);

        // let go of hash usage
        ExpectResourceSizeCalls(resource);
        EXPECT_CALL(*resource, Die());
        hashUsage = ResourceHashUsage();

        Mock::VerifyAndClearExpectations(resource);
    }

    TEST_F(AResourceStorage, DeletesResourceWhenOnlyHashUsageIsLeftButDeletionOK)
    {
        const ResourceContentHash hash(465, 0);
        auto* resource = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        // take one hash usage
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        // take one resource data usage
        ExpectResourceSizeCalls(resource);
        ManagedResource managed = storage.manageResource(*resource, true);

        // let go of data usage, only hasUsage left, deletion is allowed, so data is deleted
        EXPECT_CALL(*resource, Die());
        managed = ManagedResource();

        Mock::VerifyAndClearExpectations(resource);

        hashUsage = ResourceHashUsage();
    }

    TEST_F(AResourceStorage, StillMaintainsDeletionAllowedWhenSecondResourceWithNotAllowedIsCreated)
    {
        const ResourceContentHash hash(465, 0);
        auto* resource1 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        auto* resource2 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        // take one hash usage
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        // take one resource data usage
        ExpectResourceSizeCalls(resource1);
        ManagedResource managedDel = storage.manageResource(*resource1, true);

        // create second resource with same hash, will be deleted
        EXPECT_CALL(*resource2, Die());
        ManagedResource managedNoDel = storage.manageResource(*resource2, false);
        Mock::VerifyAndClearExpectations(resource2);

        // both point to same resource 1
        EXPECT_TRUE(managedDel.get() == resource1);
        EXPECT_TRUE(managedDel.get() == managedNoDel.get());

        managedDel = ManagedResource();

        // let go of last data usage, only hasUsage left, deletion is still allowed, so data is deleted
        EXPECT_CALL(*resource1, Die());
        managedNoDel = ManagedResource();
        Mock::VerifyAndClearExpectations(resource1);

        hashUsage = ResourceHashUsage();
    }

    TEST_F(AResourceStorage, DeletionAllowedCanButUpgradedToTrueByFollowupResourceWhereItIsAllowed)
    {
        const ResourceContentHash hash(465, 0);
        auto* resource1 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        auto* resource2 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        // take one hash usage
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        // take one resource data usage
        ExpectResourceSizeCalls(resource1);
        ManagedResource managedNoDel = storage.manageResource(*resource1, false);

        // create second resource with same hash, will be deleted but changes deletion status to true
        EXPECT_CALL(*resource2, Die());
        ManagedResource managedDel = storage.manageResource(*resource2, true);
        Mock::VerifyAndClearExpectations(resource2);

        // both point to same resource 1
        EXPECT_TRUE(managedDel.get() == resource1);
        EXPECT_TRUE(managedDel.get() == managedNoDel.get());

        managedDel = ManagedResource();

        // let go of last data usage, only hasUsage left, deletion is still allowed, so data is deleted
        EXPECT_CALL(*resource1, Die());
        managedNoDel = ManagedResource();
        Mock::VerifyAndClearExpectations(resource1);

        hashUsage = ResourceHashUsage();
    }

    TEST_F(AResourceStorage, DeletionAllowedStateIsMaintainedEvenWhenResourceWasAlreadyDeletedOnce)
    {
        const ResourceContentHash hash(465, 0);
        auto* resource1 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        // take one hash usage
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);

        {
            // create and delete resource with deletion allowed
            ExpectResourceSizeCalls(resource1);
            ManagedResource managed = storage.manageResource(*resource1, true);

            EXPECT_CALL(*resource1, Die());
        }
        Mock::VerifyAndClearExpectations(resource1);

        auto* resource2 = new NiceMock<ResourceWithDestructorMock>(hash, EResourceType::Invalid);
        {
            ExpectResourceSizeCalls(resource2);
            ManagedResource managed = storage.manageResource(*resource2, false);
            EXPECT_CALL(*resource2, Die());
        }
        Mock::VerifyAndClearExpectations(resource2);
    }

    TEST_F(AResourceStorage, DeletesResourceAfterAllManagedResourcesGoOutOfScope)
    {
        const ResourceContentHash hash(456, 0);
        auto* resource = new ResourceWithDestructorMock(hash, EResourceType::Invalid);
        ExpectResourceSizeCalls(resource);
        ManagedResource managed = storage.manageResource(*resource);

        ManagedResource usage1 = storage.getResource(hash);

        // let go of initial managed pointer
        EXPECT_CALL(*resource, Die()).Times(0);
        managed = ManagedResource();
        Mock::VerifyAndClearExpectations(resource);

        // take another usage
        ManagedResource usage2 = storage.getResource(hash);

        EXPECT_CALL(*resource, Die()).Times(0);
        usage1 = ManagedResource();
        Mock::VerifyAndClearExpectations(resource);

        // last usage goes away, not its really deleted
        ExpectResourceSizeCalls(resource);
        EXPECT_CALL(*resource, Die()).Times(1);
        usage2 = ManagedResource();
        Mock::VerifyAndClearExpectations(resource); //to make sure the dynamically allocated mock resource is destroyed (Die() is called)
    }

    TEST_F(AResourceStorage, ReturnsAManagedResource)
    {
        const ResourceContentHash hash(456, 0);
        auto* resource = new ResourceMock(hash, EResourceType::Invalid);

        ExpectResourceSizeCalls(resource);
        ManagedResource managedRes = storage.manageResource(*resource);
        ManagedResource obtainedRes = storage.getResource(hash);
        EXPECT_EQ(resource, obtainedRes.get());
    }

    TEST_F(AResourceStorage, CanReturnAHashUsageForAPreviouslyUnknownHash)
    {
        const ResourceContentHash hash(1234u, 0);
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        EXPECT_EQ(hash, hashUsage.getHash());
    }

    TEST_F(AResourceStorage, ReturnEmptyManagedResourceAlsoWhenAlreadyExistingAsHashUsage)
    {
        const ResourceContentHash hash(123456u, 0);
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash);
        EXPECT_EQ(hash, hashUsage.getHash());

        ManagedResource obtainedRes = storage.getResource(hash);
        EXPECT_EQ(nullptr, obtainedRes.get());
    }

    TEST_F(AResourceStorage, ReturnsEmptyWhenAskesForUnavailableResourceHash)
    {

        ManagedResource obtainedRes = storage.getResource(ResourceContentHash(123456, 0));
        EXPECT_EQ(nullptr, obtainedRes.get());
    }

    TEST_F(AResourceStorage, ReturnsAllManagedResources)
    {
        auto* resource = new ResourceMock(ResourceContentHash(456, 0), EResourceType::Invalid);
        ExpectResourceSizeCalls(resource);
        ManagedResource managedRes = storage.manageResource(*resource);

        auto* resource2 = new ResourceMock(ResourceContentHash(123, 0), EResourceType::Invalid);
        ExpectResourceSizeCalls(resource2);
        ManagedResource managedRes2 = storage.manageResource(*resource2);

        ManagedResourceVector allResources = storage.getResources();

        EXPECT_EQ(2u, allResources.size());
        EXPECT_THAT(allResources, testing::UnorderedElementsAre(managedRes, managedRes2));
    }

    TEST_F(AResourceStorage, StoresAndReturnsResourceInfo)
    {
        ResourceContentHash hash(123, 0);
        ResourceInfo testResourceInfo(EResourceType::VertexArray, hash, 22, 11);

        storage.storeResourceInfo(hash, testResourceInfo);
        ResourceHashUsage hashUsage = storage.getResourceHashUsage(hash); //ensure that resource entry is destructed

        EXPECT_EQ(testResourceInfo, storage.getResourceInfo(hash));
    }

    TEST_F(AResourceStorage, StoresResourceInfoFromProvidedResource)
    {
        ResourceContentHash hash(123, 0);
        auto* res = new DummyResource(hash, EResourceType::VertexArray);

        ManagedResource managedRes = storage.manageResource(*res);
        ResourceInfo returnedResourceInfo = storage.getResourceInfo(hash);

        EXPECT_EQ(EResourceType::VertexArray, returnedResourceInfo.type);
        EXPECT_EQ(hash, returnedResourceInfo.hash);
        EXPECT_EQ(res->getDecompressedDataSize(), returnedResourceInfo.decompressedSize);
        EXPECT_EQ(res->getCompressedDataSize(), returnedResourceInfo.compressedSize);
    }
}
