//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTAPPLICATIONLOGIC_H
#define RAMSES_CLIENTAPPLICATIONLOGIC_H

#include "TransportCommon/ServiceHandlerInterfaces.h"
#include "Components/ManagedResource.h"
#include "Components/ResourceHashUsage.h"
#include "Components/ResourceFileInputStream.h"

#include "SceneAPI/SceneVersionTag.h"
#include "Scene/EScenePublicationMode.h"
#include "Scene/ESceneFlushMode.h"
#include "Collections/HashSet.h"

namespace ramses
{
    class RamsesFrameworkImpl;
}

namespace ramses_internal
{
    class ClientScene;
    class ResourceTableOfContents;
    class IResource;
    class IObjectTouchHandler;
    class IResourceProviderComponent;
    class ISceneGraphProviderComponent;
    class PlatformLock;
    struct FlushTimeInformation;

    class ClientApplicationLogic : public ISceneProviderServiceHandler
    {
    public:
        explicit ClientApplicationLogic(const Guid& myId, PlatformLock& frameworkLock);
        virtual ~ClientApplicationLogic();

        void init(IResourceProviderComponent& resources, ISceneGraphProviderComponent& scenegraph);
        void deinit();

        // Scene handling
        void createScene(ClientScene& scene, bool enableLocalOnlyOptimization);
        void publishScene(SceneId sceneId, EScenePublicationMode publicationMode);
        void unpublishScene(SceneId sceneId);
        Bool isScenePublished(SceneId sceneId) const;

        void flush(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation& timeInfo, SceneVersionTag versionTag);
        void removeScene(SceneId sceneId);

        virtual void handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;
        virtual void handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID) override;

        // Resource handling
        ManagedResource         addResource(const IResource* resource);
        ManagedResource         getResource(ResourceContentHash hash) const;
        ManagedResource         forceLoadResource(const ResourceContentHash& hash) const;
        ResourceHashUsage       getHashUsage(const ResourceContentHash& hash) const;
        ManagedResourceVector   getResources() const;
        void                    addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc);
        void                    removeResourceFile(const String& resourceFileName);
        bool                    hasResourceFile(const String& resourceFileName) const;

        void reserveResourceCount(uint32_t totalCount);
    private:
        PlatformLock&                 m_frameworkLock;
        IResourceProviderComponent*   m_resourceComponent;
        ISceneGraphProviderComponent* m_scenegraphProviderComponent;

        const Guid                    m_myId;

        HashSet<IObjectTouchHandler*> m_pTouchHandlers;
        HashSet<SceneId> m_publishedScenes;
    };
}

#endif
