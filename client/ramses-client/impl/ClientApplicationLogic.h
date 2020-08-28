//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_CLIENTAPPLICATIONLOGIC_H
#define RAMSES_CLIENTAPPLICATIONLOGIC_H

#include "Components/ManagedResource.h"
#include "Components/ResourceHashUsage.h"
#include "Components/ResourceFileInputStream.h"
#include "Components/ISceneProviderEventConsumer.h"

#include "SceneAPI/SceneVersionTag.h"
#include "Scene/EScenePublicationMode.h"
#include "Collections/HashSet.h"
#include "Collections/Guid.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "SceneReferencing/SceneReferenceEvent.h"

namespace ramses
{
    class RamsesFrameworkImpl;
}

namespace ramses_internal
{
    class ClientScene;
    class ResourceTableOfContents;
    class IResource;
    class IResourceProviderComponent;
    class ISceneGraphProviderComponent;
    struct FlushTimeInformation;

    class ClientApplicationLogic : public ISceneProviderEventConsumer
    {
    public:
        explicit ClientApplicationLogic(const Guid& myId, PlatformLock& frameworkLock);
        virtual ~ClientApplicationLogic() override;

        void init(IResourceProviderComponent& resources, ISceneGraphProviderComponent& scenegraph);
        void deinit();

        // Scene handling
        void createScene(ClientScene& scene, bool enableLocalOnlyOptimization);
        void publishScene(SceneId sceneId, EScenePublicationMode publicationMode);
        void unpublishScene(SceneId sceneId);
        Bool isScenePublished(SceneId sceneId) const;

        void flush(SceneId sceneId, const FlushTimeInformation& timeInfo, SceneVersionTag versionTag);
        void removeScene(SceneId sceneId);

        virtual void handleSceneReferenceEvent(SceneReferenceEvent const& event, const Guid& rendererId) override;
        virtual void handleResourceAvailabilityEvent(ResourceAvailabilityEvent const& event, const Guid& rendererId) override;

        // Resource handling
        ManagedResource         addResource(const IResource* resource);
        ManagedResource         getResource(ResourceContentHash hash) const;
        ManagedResource         forceLoadResource(const ResourceContentHash& hash) const;
        ResourceHashUsage       getHashUsage(const ResourceContentHash& hash) const;
        ManagedResourceVector   getResources() const;
        void                    addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc);
        void                    removeResourceFile(const String& resourceFileName);
        void                    forceLoadFromResourceFile(const String& resourceFileName);
        bool                    hasResourceFile(const String& resourceFileName) const;
        void                    reserveResourceCount(uint32_t totalCount);

        std::vector<ramses_internal::SceneReferenceEvent> popSceneReferenceEvents();

    private:
        PlatformLock&                 m_frameworkLock;
        IResourceProviderComponent*   m_resourceComponent;
        ISceneGraphProviderComponent* m_scenegraphProviderComponent;

        const Guid                    m_myId;

        HashSet<SceneId> m_publishedScenes;

        std::vector<ramses_internal::SceneReferenceEvent> m_sceneReferenceEventVec;
    };
}

#endif
