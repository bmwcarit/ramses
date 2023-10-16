//  -------------------------------------------------------------------------
//  Copyright (C) 2017 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "gtest/gtest.h"
#include "internal/RendererLib/RendererResourceRegistry.h"
#include "internal/Components/ResourceDeleterCallingCallback.h"
#include "internal/SceneGraph/Resource/ArrayResource.h"
#include "ResourceMock.h"
#include "MockResourceHash.h"
#include "internal/Core/Utils/ThreadLocalLog.h"

using namespace testing;

namespace ramses::internal
{
    class ARendererResourceRegistry : public ::testing::Test
    {
    public:
    protected:
        ARendererResourceRegistry()
        {
            // caller is expected to have a display prefix for logs
            ThreadLocalLog::SetPrefix(1);
        }

        void expectResourcesUsedByScene(SceneId sceneId, const ResourceContentHashVector& res)
        {
            if (res.empty())
            {
                EXPECT_EQ(nullptr, registry.getResourcesInUseByScene(sceneId));
            }
            else
            {
                ASSERT_NE(nullptr, registry.getResourcesInUseByScene(sceneId));
                EXPECT_EQ(res, *registry.getResourcesInUseByScene(sceneId));
            }
        }

        ManagedResource testManagedResource{ MockResourceHash::GetManagedResource(MockResourceHash::IndexArrayHash) };
        RendererResourceRegistry registry;
    };

    TEST_F(ARendererResourceRegistry, doesNotContainAnythingInitially)
    {
        const ResourceContentHash resource(123u, 0u);
        EXPECT_EQ(0u, registry.getAllResourceDescriptors().size());
        EXPECT_FALSE(registry.containsResource(resource));

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
        expectResourcesUsedByScene(SceneId{ 3u }, {});
    }

    TEST_F(ARendererResourceRegistry, canRegisterResource)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);

        EXPECT_EQ(1u, registry.getAllResourceDescriptors().size());
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
        EXPECT_FALSE(rd.resource);
        EXPECT_TRUE(rd.sceneUsage.empty());
        EXPECT_EQ(EResourceStatus::Registered, rd.status);
        EXPECT_EQ(EResourceType::Invalid, rd.type);

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, unregisteredResourceIsNotContainedAnymore)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.unregisterResource(resource);

        EXPECT_EQ(0u, registry.getAllResourceDescriptors().size());
        EXPECT_FALSE(registry.containsResource(resource));

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canAddSceneUsageRef)
    {
        const SceneId sceneId(11u);
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);

        expectResourcesUsedByScene(sceneId, {});
        registry.addResourceRef(resource, sceneId);
        expectResourcesUsedByScene(sceneId, { resource });

        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        ASSERT_EQ(1u, rd.sceneUsage.size());
        EXPECT_EQ(sceneId, rd.sceneUsage[0]);

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canRemoveSceneUsageRef)
    {
        const SceneId sceneId(11u);
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);

        registry.addResourceRef(resource, sceneId);
        registry.removeResourceRef(resource, sceneId);
        expectResourcesUsedByScene(sceneId, {});

        EXPECT_FALSE(registry.containsResource(resource));
    }

    TEST_F(ARendererResourceRegistry, canSetResourceData)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);

        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        EXPECT_EQ(EResourceStatus::Provided, rd.status);
        EXPECT_EQ(testManagedResource, rd.resource);
        EXPECT_EQ(EResourceType::IndexArray, rd.type);
        EXPECT_EQ(0u, rd.compressedSize);
        EXPECT_EQ(1u, rd.decompressedSize);
        EXPECT_FALSE(rd.deviceHandle.isValid());

        EXPECT_FALSE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canSetResourceFromProvidedToUploaded_willReleaseResourceData)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        EXPECT_EQ(EResourceStatus::Provided, rd.status);

        const DeviceResourceHandle deviceHandle(123456u);
        registry.setResourceUploaded(resource, deviceHandle, 666u);

        EXPECT_EQ(EResourceStatus::Uploaded, rd.status);
        EXPECT_EQ(deviceHandle, rd.deviceHandle);
        EXPECT_FALSE(rd.resource);
        EXPECT_EQ(EResourceType::IndexArray, rd.type);

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canSetResourceProvidedToBroken_willReleaseResourceData)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);

        registry.setResourceBroken(resource);

        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        EXPECT_EQ(EResourceStatus::Broken, rd.status);
        EXPECT_FALSE(rd.resource);
        EXPECT_EQ(EResourceType::IndexArray, rd.type);
        EXPECT_FALSE(rd.deviceHandle.isValid());

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canSetResourceFromScheduledForUploadToUploaded_willReleaseResourceData)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        EXPECT_EQ(EResourceStatus::Provided, rd.status);

        registry.setResourceScheduledForUpload(resource);
        EXPECT_EQ(EResourceStatus::ScheduledForUpload, rd.status);

        const DeviceResourceHandle deviceHandle(123456u);
        registry.setResourceUploaded(resource, deviceHandle, 666u);

        EXPECT_EQ(EResourceStatus::Uploaded, rd.status);
        EXPECT_EQ(deviceHandle, rd.deviceHandle);
        EXPECT_FALSE(rd.resource);
        EXPECT_EQ(EResourceType::IndexArray, rd.type);

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canSetResourceScheduledForUploadToBroken_willReleaseResourceData)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        registry.setResourceScheduledForUpload(resource);

        registry.setResourceBroken(resource);

        const ResourceDescriptor& rd = registry.getResourceDescriptor(resource);
        EXPECT_EQ(EResourceStatus::Broken, rd.status);
        EXPECT_FALSE(rd.resource);
        EXPECT_EQ(EResourceType::IndexArray, rd.type);
        EXPECT_FALSE(rd.deviceHandle.isValid());

        EXPECT_TRUE(registry.getAllProvidedResources().empty());
        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, providedResourceIsInProvidedList)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);

        const ResourceContentHashVector& resources = registry.getAllProvidedResources();
        ASSERT_EQ(1u, resources.size());
        EXPECT_TRUE(contains_c(resources, resource));
    }

    TEST_F(ARendererResourceRegistry, reportsIfHasAnyResourcesScheduledForUpload)
    {
        EXPECT_FALSE(registry.hasAnyResourcesScheduledForUpload());
        const ResourceContentHash resource(123u, 0u);

        registry.registerResource(resource);
        EXPECT_FALSE(registry.hasAnyResourcesScheduledForUpload());

        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.hasAnyResourcesScheduledForUpload());

        registry.setResourceScheduledForUpload(resource);
        EXPECT_EQ(EResourceStatus::ScheduledForUpload, registry.getResourceStatus(resource));
        EXPECT_TRUE(registry.hasAnyResourcesScheduledForUpload());

        const DeviceResourceHandle deviceHandle(123456u);
        registry.setResourceUploaded(resource, deviceHandle, 666u);
        EXPECT_FALSE(registry.hasAnyResourcesScheduledForUpload());
    }

    TEST_F(ARendererResourceRegistry, uploadedResourceIsRemovedFromProvidedList)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.getAllProvidedResources().empty());

        registry.setResourceUploaded(resource, DeviceResourceHandle{ 123u }, 666u);
        EXPECT_TRUE(registry.getAllProvidedResources().empty());
    }

    TEST_F(ARendererResourceRegistry, scheduledForUploadResourceIsRemovedFromProvidedList)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.getAllProvidedResources().empty());

        registry.setResourceScheduledForUpload(resource);
        EXPECT_TRUE(registry.getAllProvidedResources().empty());
    }

    TEST_F(ARendererResourceRegistry, brokenResourceIsRemovedFromProvidedList)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.getAllProvidedResources().empty());

        registry.setResourceBroken(resource);
        EXPECT_TRUE(registry.getAllProvidedResources().empty());
    }

    TEST_F(ARendererResourceRegistry, unregisteredResourceIsRemovedFromProvidedList)
    {
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.getAllProvidedResources().empty());

        registry.unregisterResource(resource);
        EXPECT_TRUE(registry.getAllProvidedResources().empty());
    }

    TEST_F(ARendererResourceRegistry, unusedResourceIsRemovedFromProvidedList)
    {
        const SceneId sceneId(11u);
        const ResourceContentHash resource(123u, 0u);
        registry.registerResource(resource);
        registry.addResourceRef(resource, sceneId);

        registry.setResourceData(resource, testManagedResource);
        EXPECT_FALSE(registry.getAllProvidedResources().empty());

        registry.removeResourceRef(resource, sceneId);
        EXPECT_TRUE(registry.getAllProvidedResources().empty());
    }

    TEST_F(ARendererResourceRegistry, resourceWithoutReferenceIsPutInUnusedListIfUploadedOrUnregisteredIfNotUploaded)
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
        expectResourcesUsedByScene(sceneId, { resource1, resource2, resource3, resource4 });

        registry.setResourceData(resource4, testManagedResource);
        registry.setResourceUploaded(resource4, DeviceResourceHandle{ 123u }, 666u);

        const ResourceContentHashVector& resources = registry.getAllResourcesNotInUseByScenes();
        EXPECT_TRUE(resources.empty());

        registry.removeResourceRef(resource1, sceneId);
        registry.removeResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource2, resource3 });

        ASSERT_EQ(1u, resources.size());
        EXPECT_TRUE(contains_c(resources, resource4));

        EXPECT_FALSE(registry.containsResource(resource1));
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
        expectResourcesUsedByScene(sceneId, { resource1, resource2, resource3, resource4 });

        registry.removeResourceRef(resource1, sceneId);
        registry.removeResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource2, resource3 });

        EXPECT_FALSE(registry.containsResource(resource1));
        EXPECT_FALSE(registry.containsResource(resource4));

        registry.registerResource(resource1);
        registry.registerResource(resource4);

        registry.addResourceRef(resource1, sceneId);
        registry.addResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource2, resource3, resource1, resource4 });

        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }

    TEST_F(ARendererResourceRegistry, canReferenceResourceMultipleTimes)
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
        expectResourcesUsedByScene(sceneId, { resource1, resource2, resource3, resource4 });

        registry.addResourceRef(resource1, sceneId);
        registry.addResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource1, resource2, resource3, resource4 });

        registry.removeResourceRef(resource1, sceneId);
        registry.removeResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource1, resource2, resource3, resource4 });

        registry.removeResourceRef(resource1, sceneId);
        registry.removeResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource2, resource3 });

        EXPECT_FALSE(registry.containsResource(resource1));
        EXPECT_FALSE(registry.containsResource(resource4));

        registry.registerResource(resource1);
        registry.registerResource(resource4);

        registry.addResourceRef(resource1, sceneId);
        registry.addResourceRef(resource4, sceneId);
        expectResourcesUsedByScene(sceneId, { resource2, resource3, resource1, resource4 });

        EXPECT_TRUE(registry.getAllResourcesNotInUseByScenes().empty());
    }
}
