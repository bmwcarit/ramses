//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Components/ManagedResource.h"
#include "internal/Components/ResourceHashUsage.h"
#include "internal/Components/InputStreamContainer.h"
#include "internal/Components/ISceneProviderEventConsumer.h"
#include "internal/Components/SceneFileHandle.h"

#include "internal/SceneGraph/SceneAPI/SceneVersionTag.h"
#include "internal/SceneGraph/Scene/EScenePublicationMode.h"
#include "internal/PlatformAbstraction/Collections/HashSet.h"
#include "internal/PlatformAbstraction/Collections/Guid.h"
#include "internal/PlatformAbstraction/PlatformLock.h"
#include "internal/SceneReferencing/SceneReferenceEvent.h"

namespace ramses::internal
{
    class ClientScene;
    class ResourceTableOfContents;
    class IResource;
    class IResourceProviderComponent;
    class ISceneGraphProviderComponent;
    struct FlushTimeInformation;
    class RamsesFrameworkImpl;

    class ClientApplicationLogic : public ISceneProviderEventConsumer
    {
    public:
        explicit ClientApplicationLogic(const Guid& myId, PlatformLock& frameworkLock);
        ~ClientApplicationLogic() override;

        void init(IResourceProviderComponent& resources, ISceneGraphProviderComponent& scenegraph);
        void deinit();

        // Scene handling
        void createScene(ClientScene& scene, bool enableLocalOnlyOptimization);
        void publishScene(SceneId sceneId, EScenePublicationMode publicationMode);
        void unpublishScene(SceneId sceneId);
        [[nodiscard]] bool isScenePublished(SceneId sceneId) const;

        bool flush(SceneId sceneId, const FlushTimeInformation& timeInfo, SceneVersionTag versionTag);
        void removeScene(SceneId sceneId);

        void handleSceneReferenceEvent(SceneReferenceEvent const& event, const Guid& rendererId) override;
        void handleResourceAvailabilityEvent(ResourceAvailabilityEvent const& event, const Guid& rendererId) override;

        // Resource handling
        ManagedResource         addResource(const IResource* resource);
        [[nodiscard]] ManagedResource         getResource(ResourceContentHash hash) const;
        [[nodiscard]] ManagedResource         loadResource(const ResourceContentHash& hash) const;
        [[nodiscard]] ResourceHashUsage       getHashUsage(const ResourceContentHash& hash) const;
        SceneFileHandle         addResourceFile(InputStreamContainerSPtr resourceFileInputStream, const ResourceTableOfContents& toc);
        void                    removeResourceFile(SceneFileHandle handle);
        void                    loadResourceFromFile(SceneFileHandle handle);
        void                    reserveResourceCount(uint32_t totalCount);
        [[nodiscard]] bool                    hasResourceFile(SceneFileHandle handle) const;

        std::vector<SceneReferenceEvent> popSceneReferenceEvents();

    private:
        PlatformLock&                 m_frameworkLock;
        IResourceProviderComponent*   m_resourceComponent;
        ISceneGraphProviderComponent* m_scenegraphProviderComponent;

        const Guid                    m_myId;

        HashSet<SceneId> m_publishedScenes;

        std::vector<SceneReferenceEvent> m_sceneReferenceEventVec;
    };
}
