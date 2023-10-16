//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "gmock/gmock.h"

#include "internal/SceneGraph/Resource/IResource.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/Components/ISceneGraphProviderComponent.h"
#include "internal/Components/ManagedResource.h"
#include "internal/Components/ISceneGraphConsumerComponent.h"
#include "internal/Components/IResourceProviderComponent.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/Components/ISceneProviderEventConsumer.h"
#include "internal/SceneReferencing/SceneReferenceEvent.h"
#include "internal/Components/ResourceAvailabilityEvent.h"

namespace ramses::internal
{
    using namespace testing;

    class ResourceProviderComponentMock : public IResourceProviderComponent
    {
    public:
        ResourceProviderComponentMock();
        ~ResourceProviderComponentMock() override;

        MOCK_METHOD(ManagedResource, manageResource, (const IResource& resource), (override));
        MOCK_METHOD(ManagedResource, getResource, (ResourceContentHash hash), (override));
        MOCK_METHOD(ManagedResource, loadResource, (const ResourceContentHash&), (override));
        MOCK_METHOD(ResourceHashUsage, getResourceHashUsage, (const ResourceContentHash&), (override));
        MOCK_METHOD(SceneFileHandle, addResourceFile, (InputStreamContainerSPtr inputStream, const ResourceTableOfContents& toc), (override));
        MOCK_METHOD(bool, hasResourceFile, (SceneFileHandle), (const, override));
        MOCK_METHOD(void, removeResourceFile, (SceneFileHandle), (override));
        MOCK_METHOD(void, loadResourceFromFile, (SceneFileHandle), (override));
        void reserveResourceCount(uint32_t /*totalCount*/) override {};
        MOCK_METHOD(ManagedResourceVector, resolveResources, (ResourceContentHashVector& vec), (override));
        MOCK_METHOD(ResourceInfo const&, getResourceInfo, (ResourceContentHash const& hash), (override));
        MOCK_METHOD(bool, knowsResource, (ResourceContentHash const& hash), (const, override));
    };

    class SceneGraphProviderComponentMock : public ISceneGraphProviderComponent
    {
    public:
        SceneGraphProviderComponentMock();
        ~SceneGraphProviderComponentMock() override;

        MOCK_METHOD(void, handleCreateScene, (ClientScene& scene, bool enableLocalOnlyOptimization, ISceneProviderEventConsumer& consumer), (override));
        MOCK_METHOD(void, handlePublishScene, (SceneId sceneId, EScenePublicationMode publicationMode), (override));
        MOCK_METHOD(void, handleUnpublishScene, (SceneId sceneId), (override));
        MOCK_METHOD(bool, handleFlush, (SceneId sceneId, const FlushTimeInformation&, SceneVersionTag), (override));
        MOCK_METHOD(void, handleRemoveScene, (SceneId sceneId), (override));
    };

    class SceneGraphConsumerComponentMock : public ISceneGraphConsumerComponent
    {
    public:
        SceneGraphConsumerComponentMock();
        ~SceneGraphConsumerComponentMock() override;

        MOCK_METHOD(void, subscribeScene, (const Guid& to, SceneId sceneId), (override));
        MOCK_METHOD(void, unsubscribeScene, (const Guid& to, SceneId sceneId), (override));
        MOCK_METHOD(void, sendSceneReferenceEvent, (const Guid& to, SceneReferenceEvent const& event), (override));
        MOCK_METHOD(void, sendResourceAvailabilityEvent,(const Guid& to, ResourceAvailabilityEvent const& event), (override));

        void setSceneRendererHandler(ISceneRendererHandler* /*sceneRendererHandler*/) override
        {
        }
    };

    class SceneProviderEventConsumerMock : public ISceneProviderEventConsumer
    {
    public:
        SceneProviderEventConsumerMock();
        ~SceneProviderEventConsumerMock() override;

        MOCK_METHOD(void, handleSceneReferenceEvent, (SceneReferenceEvent const& event, const Guid& rendererId), (override));
        MOCK_METHOD(void, handleResourceAvailabilityEvent, (ResourceAvailabilityEvent const& event, const Guid& rendererId), (override));
    };
}
