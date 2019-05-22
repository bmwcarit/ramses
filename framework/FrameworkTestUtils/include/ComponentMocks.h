//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_COMPONENTMOCKS_H
#define RAMSES_COMPONENTMOCKS_H

#include "framework_common_gmock_header.h"
#include "gmock/gmock.h"

#include "Resource/IResource.h"
#include "Scene/ClientScene.h"
#include "Scene/EScenePublicationMode.h"
#include "Components/ISceneGraphProviderComponent.h"
#include "Components/ManagedResource.h"
#include "Components/ISceneGraphConsumerComponent.h"
#include "Components/IResourceProviderComponent.h"
#include "Components/ResourceTableOfContents.h"
#include "Components/IResourceConsumerComponent.h"

namespace ramses_internal
{
    using namespace testing;

    class ResourceProviderComponentMock : public IResourceProviderComponent
    {
    public:
        ResourceProviderComponentMock();
        ~ResourceProviderComponentMock() override;

        MOCK_METHOD2(manageResource, ManagedResource(const IResource& resource, Bool deletionAllowed));
        MOCK_METHOD0(getResources, ManagedResourceVector());
        MOCK_METHOD1(getResource, ManagedResource(ResourceContentHash hash));
        MOCK_METHOD1(forceLoadResource, ManagedResource(const ResourceContentHash&));
        MOCK_METHOD1(getResourceHashUsage, ResourceHashUsage(const ResourceContentHash&));
        MOCK_METHOD2(addResourceFile, void(ResourceFileInputStreamSPtr resourceFileStream, const ResourceTableOfContents& toc));
        MOCK_CONST_METHOD1(hasResourceFile, bool(const String&));
        MOCK_CONST_METHOD1(getResourceInfo, const ResourceInfo&(const ResourceContentHash& hash));
        MOCK_METHOD2(storeResourceInfo, void(const ResourceContentHash& hash, const ResourceInfo& resourceInfo));
        MOCK_METHOD1(removeResourceFile, void(const String& resourceFileName) );
        virtual void reserveResourceCount(uint32_t) override {};

        };

    class ResourceConsumerComponentMock : public IResourceConsumerComponent
    {
    public:
        ResourceConsumerComponentMock();
        ~ResourceConsumerComponentMock() override;

        MOCK_METHOD1(resolveResources, void(const ResourceContentHashVector& resourceHash));
        MOCK_METHOD1(cancelResourceRequest, void(const ResourceContentHash& resourceHash));

        MOCK_METHOD3(requestResourceAsynchronouslyFromFramework, void(const ResourceContentHashVector& ids, const RequesterID& requesterID, const Guid& providerID));
        MOCK_METHOD2(cancelResourceRequest, void(const ResourceContentHash& resourceHash, const RequesterID& requesterID));
        MOCK_METHOD1(popArrivedResources, ManagedResourceVector(const RequesterID& requesterID));
    };

    class SceneGraphProviderComponentMock : public ISceneGraphProviderComponent
    {
    public:
        SceneGraphProviderComponentMock();
        ~SceneGraphProviderComponentMock() override;

        MOCK_METHOD1(setSceneProviderServiceHandler, void(ISceneProviderServiceHandler* handler));
        MOCK_METHOD2(handleCreateScene, void(ClientScene& scene, bool enableLocalOnlyOptimization));
        MOCK_METHOD2(handlePublishScene, void(SceneId sceneId, EScenePublicationMode publicationMode));
        MOCK_METHOD1(handleUnpublishScene, void(SceneId sceneId));
        MOCK_METHOD4(handleFlush, void(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation&, SceneVersionTag));
        MOCK_METHOD1(handleRemoveScene, void(SceneId sceneId));
        MOCK_METHOD2(handleSceneSubscription, void(SceneId sceneId, const Guid& subscriber));
        MOCK_METHOD2(handleSceneUnsubscription, void(SceneId sceneId, const Guid& subscriber));
    };

    class SceneGraphConsumerComponentMock : public ISceneGraphConsumerComponent
    {
    public:
        SceneGraphConsumerComponentMock();
        ~SceneGraphConsumerComponentMock() override;

        MOCK_METHOD2(subscribeScene, void(const Guid& to, SceneId sceneId));
        MOCK_METHOD2(unsubscribeScene, void(const Guid& to, SceneId sceneId));

        virtual void setSceneRendererServiceHandler(ISceneRendererServiceHandler*) override
        {
        }
    };
}

#endif
