//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "renderer_common_gmock_header.h"
#include "gtest/gtest.h"
#include "RendererLib/RendererClientResourceRegistry.h"
#include "Components/ResourceDeleterCallingCallback.h"
#include "Resource/ArrayResource.h"
#include "ResourceMock.h"

using namespace testing;
using namespace ramses_internal;

class ARendererResourceRegistry : public ::testing::Test
{
public:
protected:
    RendererClientResourceRegistry registry;
};

TEST_F(ARendererResourceRegistry, doesNotContainAnythingInitially)
{
    const ResourceContentHash resource(123u, 0u);
    EXPECT_EQ(0u, registry.getAllResourceDescriptors().count());
    EXPECT_FALSE(registry.containsResource(resource));

    EXPECT_TRUE(registry.getAllRegisteredResources().empty());
    EXPECT_TRUE(registry.getAllRequestedResources().empty());
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenesAndNotUploaded().empty());
}

TEST_F(ARendererResourceRegistry, canRegisterResource)
{
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    EXPECT_EQ(1u, registry.getAllResourceDescriptors().count());
    EXPECT_TRUE(registry.containsResource(resource));

    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    EXPECT_EQ(resource, rd.hash);
}

TEST_F(ARendererResourceRegistry, registeredResourceHasCorrectStatus)
{
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    EXPECT_FALSE(rd.deviceHandle.isValid());
    EXPECT_TRUE(NULL == rd.resource.getResourceObject());
    EXPECT_TRUE(rd.sceneUsage.empty());
    EXPECT_EQ(EResourceStatus_Registered, rd.status);
    EXPECT_EQ(EResourceType_Invalid, rd.type);

    ASSERT_EQ(1u, registry.getAllRegisteredResources().size());
    EXPECT_EQ(resource, registry.getAllRegisteredResources()[0]);
    EXPECT_TRUE(registry.getAllRequestedResources().empty());
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenesAndNotUploaded().empty());
}

TEST_F(ARendererResourceRegistry, unregisteredResourceIsNotContainedAnymore)
{
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.unregisterResource(resource);

    EXPECT_EQ(0u, registry.getAllResourceDescriptors().count());
    EXPECT_FALSE(registry.containsResource(resource));

    EXPECT_TRUE(registry.getAllRegisteredResources().empty());
    EXPECT_TRUE(registry.getAllRequestedResources().empty());
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenesAndNotUploaded().empty());
}

TEST_F(ARendererResourceRegistry, canAddSceneUsageRef)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    registry.addResourceRef(resource, sceneId);

    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    ASSERT_EQ(1u, rd.sceneUsage.size());
    EXPECT_EQ(sceneId, rd.sceneUsage[0]);
}

TEST_F(ARendererResourceRegistry, canRemoveSceneUsageRef)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    registry.addResourceRef(resource, sceneId);
    registry.removeResourceRef(resource, sceneId);

    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    EXPECT_TRUE(rd.sceneUsage.empty());
}

TEST_F(ARendererResourceRegistry, canChangeResourceStatus)
{
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    registry.setResourceStatus(resource, EResourceStatus_Requested);

    EXPECT_EQ(EResourceStatus_Requested, registry.getResourceStatus(resource));
    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    EXPECT_EQ(EResourceStatus_Requested, rd.status);
}

TEST_F(ARendererResourceRegistry, canSetResourceData)
{
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    NiceMock<ManagedResourceDeleterCallbackMock> managedResourceDeleter;
    ResourceDeleterCallingCallback dummyManagedResourceCallback(managedResourceDeleter);

    const DeviceResourceHandle deviceHandle(123456u);

    const ArrayResource res(EResourceType_IndexArray, 0, EDataType_UInt16, nullptr, ResourceCacheFlag_DoNotCache, String());
    ManagedResource managedRes(res, dummyManagedResourceCallback);

    registry.setResourceData(resource, managedRes, deviceHandle, EResourceType_IndexArray);

    const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
    EXPECT_EQ(deviceHandle, rd.deviceHandle);
    EXPECT_EQ(managedRes, rd.resource);
    EXPECT_EQ(EResourceType_IndexArray, rd.type);

    // so that resource deleter is called before it is destructed as stack object
    registry.setResourceData(resource, ManagedResource(), deviceHandle, EResourceType_IndexArray);
}

TEST_F(ARendererResourceRegistry, newlyRegisteredResourceIsInRegisteredList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);

    const ResourceContentHashVector& resources = registry.getAllRegisteredResources();
    ASSERT_EQ(1u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource));
}

TEST_F(ARendererResourceRegistry, requestedResourceIsRemovedFromRegisteredList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    EXPECT_FALSE(registry.getAllRegisteredResources().empty());

    registry.setResourceStatus(resource, EResourceStatus_Requested);
    EXPECT_TRUE(registry.getAllRegisteredResources().empty());
}

TEST_F(ARendererResourceRegistry, unregisteredResourceIsRemovedFromRegisteredList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    EXPECT_FALSE(registry.getAllRegisteredResources().empty());

    registry.unregisterResource(resource);
    EXPECT_TRUE(registry.getAllRegisteredResources().empty());
}

TEST_F(ARendererResourceRegistry, requestedResourceIsInRequestedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);

    const ResourceContentHashVector& resources = registry.getAllRequestedResources();
    ASSERT_EQ(1u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource));
}

TEST_F(ARendererResourceRegistry, providedResourceIsRemovedFromRequestedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    EXPECT_FALSE(registry.getAllRequestedResources().empty());

    registry.setResourceStatus(resource, EResourceStatus_Provided);
    EXPECT_TRUE(registry.getAllRequestedResources().empty());
}

TEST_F(ARendererResourceRegistry, unregisteredResourceIsRemovedFromRequestedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    EXPECT_FALSE(registry.getAllRequestedResources().empty());

    registry.unregisterResource(resource);
    EXPECT_TRUE(registry.getAllRequestedResources().empty());
}

TEST_F(ARendererResourceRegistry, providedResourceIsInProvidedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    registry.setResourceStatus(resource, EResourceStatus_Provided);

    const ResourceContentHashVector& resources = registry.getAllProvidedResources();
    ASSERT_EQ(1u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource));
}

TEST_F(ARendererResourceRegistry, uploadedResourceIsRemovedFromProvidedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    registry.setResourceStatus(resource, EResourceStatus_Provided);
    EXPECT_FALSE(registry.getAllProvidedResources().empty());

    registry.setResourceStatus(resource, EResourceStatus_Uploaded);
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
}

TEST_F(ARendererResourceRegistry, brokenResourceIsRemovedFromProvidedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    registry.setResourceStatus(resource, EResourceStatus_Provided);
    EXPECT_FALSE(registry.getAllProvidedResources().empty());

    registry.setResourceStatus(resource, EResourceStatus_Broken);
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
}

TEST_F(ARendererResourceRegistry, unregisteredResourceIsRemovedFromProvidedList)
{
    const SceneId sceneId(11u);
    const ResourceContentHash resource(123u, 0u);
    registry.registerResource(resource);
    registry.setResourceStatus(resource, EResourceStatus_Requested);
    registry.setResourceStatus(resource, EResourceStatus_Provided);
    EXPECT_FALSE(registry.getAllProvidedResources().empty());

    registry.unregisterResource(resource);
    EXPECT_TRUE(registry.getAllProvidedResources().empty());
}

TEST_F(ARendererResourceRegistry, onlyResourcesThatBecameUnreferencedAreInUnusedList)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    const ResourceContentHashVector& resources = registry.getAllResourcesNotInUseByScenes();
    EXPECT_TRUE(resources.empty());

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    ASSERT_EQ(2u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource1));
    EXPECT_TRUE(contains_c(resources, resource4));
}

TEST_F(ARendererResourceRegistry, unregisteredResourcesAreRemovedFromUnusedList)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    registry.unregisterResource(resource1);
    registry.unregisterResource(resource4);

    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
}

TEST_F(ARendererResourceRegistry, previouslyUnusedResourceIsRemovedFromUnusedListWhenUsedAgain)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    const ResourceContentHashVector& resources = registry.getAllResourcesNotInUseByScenes();
    ASSERT_EQ(2u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource1));
    EXPECT_TRUE(contains_c(resources, resource4));

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource4, sceneId);

    EXPECT_TRUE(resources.empty());
}

TEST_F(ARendererResourceRegistry, onlyResourcesThatBecameUnreferencedAndNotUploadedAreInUnusedNotUploadedList)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    const ResourceContentHashVector& resources = registry.getAllResourcesNotInUseByScenesAndNotUploaded();
    EXPECT_TRUE(resources.empty());

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    // resource3 although eventually unused will not be in unused/notuploaded list due to its uploaded state
    registry.setResourceStatus(resource3, EResourceStatus_Requested);
    registry.setResourceStatus(resource3, EResourceStatus_Provided);
    registry.setResourceStatus(resource3, EResourceStatus_Uploaded);
    registry.removeResourceRef(resource3, sceneId);

    ASSERT_EQ(2u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource1));
    EXPECT_TRUE(contains_c(resources, resource4));
}

TEST_F(ARendererResourceRegistry, unregisteredResourcesAreRemovedFromUnusedNotUploadedList)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    registry.unregisterResource(resource1);
    registry.unregisterResource(resource4);

    EXPECT_TRUE(registry.getAllResourcesNotInUseByScenesAndNotUploaded().empty());
}

TEST_F(ARendererResourceRegistry, previouslyUnusedResourceIsRemovedFromUnusedNotUploadedListWhenUsedAgain)
{
    const SceneId sceneId(11u);

    const ResourceContentHash resource1(123u, 0u);
    const ResourceContentHash resource2(124u, 0u);
    const ResourceContentHash resource3(125u, 0u);
    const ResourceContentHash resource4(126u, 0u);

    registry.registerResource(resource1);
    registry.registerResource(resource2);
    registry.registerResource(resource3);
    registry.registerResource(resource4);

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource2, sceneId);
    registry.addResourceRef(resource3, sceneId);
    registry.addResourceRef(resource4, sceneId);

    registry.removeResourceRef(resource1, sceneId);
    registry.removeResourceRef(resource4, sceneId);

    const ResourceContentHashVector& resources = registry.getAllResourcesNotInUseByScenesAndNotUploaded();
    ASSERT_EQ(2u, resources.size());
    EXPECT_TRUE(contains_c(resources, resource1));
    EXPECT_TRUE(contains_c(resources, resource4));

    registry.addResourceRef(resource1, sceneId);
    registry.addResourceRef(resource4, sceneId);

    EXPECT_TRUE(resources.empty());
}
