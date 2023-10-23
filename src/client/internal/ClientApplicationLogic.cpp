//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/ClientApplicationLogic.h"
#include "internal/Components/IResourceProviderComponent.h"
#include "internal/Components/ISceneGraphProviderComponent.h"
#include "internal/Components/ResourceTableOfContents.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "impl/RamsesFrameworkImpl.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/PlatformAbstraction/PlatformLock.h"

namespace ramses::internal
{
    ClientApplicationLogic::ClientApplicationLogic(const Guid& myId, PlatformLock& frameworkLock)
        : m_frameworkLock(frameworkLock)
        , m_resourceComponent(nullptr)
        , m_scenegraphProviderComponent(nullptr)
        , m_myId(myId)
    {
    }

    ClientApplicationLogic::~ClientApplicationLogic() = default;

    void ClientApplicationLogic::init(IResourceProviderComponent& resources, ISceneGraphProviderComponent& scenegraph)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent = &resources;
        m_scenegraphProviderComponent = &scenegraph;
    }

    void ClientApplicationLogic::deinit()
    {
        PlatformGuard guard(m_frameworkLock);
        m_scenegraphProviderComponent = nullptr;
    }

    void ClientApplicationLogic::createScene(ClientScene& scene, bool enableLocalOnlyOptimization)
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_TRACE(CONTEXT_CLIENT, "ClientApplicationLogic::createScene:  '" << scene.getName() << "' with id '" << scene.getSceneId().getValue() << "'");
        m_scenegraphProviderComponent->handleCreateScene(scene, enableLocalOnlyOptimization, *this);
    }

    void ClientApplicationLogic::publishScene(SceneId sceneId, EScenePublicationMode publicationMode)
    {
        PlatformGuard guard(m_frameworkLock);
        m_scenegraphProviderComponent->handlePublishScene(sceneId, publicationMode);
        m_publishedScenes.put(sceneId);
    }

    void ClientApplicationLogic::unpublishScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        m_publishedScenes.remove(sceneId);
        m_scenegraphProviderComponent->handleUnpublishScene(sceneId);
    }

    bool ClientApplicationLogic::isScenePublished(SceneId sceneId) const
    {
        return m_publishedScenes.contains(sceneId);
    }

    bool ClientApplicationLogic::flush(SceneId sceneId, const FlushTimeInformation& timeInfo, SceneVersionTag versionTag)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_scenegraphProviderComponent->handleFlush(sceneId, timeInfo, versionTag);
    }

    void ClientApplicationLogic::removeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        m_publishedScenes.remove(sceneId);
        m_scenegraphProviderComponent->handleUnpublishScene(sceneId);
        m_scenegraphProviderComponent->handleRemoveScene(sceneId);
    }

    void ClientApplicationLogic::handleSceneReferenceEvent(SceneReferenceEvent const& event, const Guid& /*rendererId*/)
    {
        m_sceneReferenceEventVec.push_back(event);
    }

    ManagedResource ClientApplicationLogic::addResource(const IResource* resource)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->manageResource(*resource);
    }

    ManagedResource ClientApplicationLogic::getResource(ResourceContentHash hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->getResource(hash);
    }

    ResourceHashUsage ClientApplicationLogic::getHashUsage(const ResourceContentHash& hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->getResourceHashUsage(hash);
    }

    SceneFileHandle ClientApplicationLogic::addResourceFile(InputStreamContainerSPtr resourceFileInputStream, const ResourceTableOfContents& toc)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->addResourceFile(std::move(resourceFileInputStream), toc);
    }

    void ClientApplicationLogic::removeResourceFile(SceneFileHandle handle)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent->removeResourceFile(handle);
    }

    void ClientApplicationLogic::loadResourceFromFile(SceneFileHandle handle)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent->loadResourceFromFile(handle);
    }

    bool  ClientApplicationLogic::hasResourceFile(SceneFileHandle handle) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->hasResourceFile(handle);
    }

    void ClientApplicationLogic::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceComponent->reserveResourceCount(totalCount);
    }

    std::vector<SceneReferenceEvent> ClientApplicationLogic::popSceneReferenceEvents()
    {
        PlatformGuard guard(m_frameworkLock);
        std::vector<SceneReferenceEvent> ret;
        m_sceneReferenceEventVec.swap(ret);
        return ret;
    }

    void ClientApplicationLogic::handleResourceAvailabilityEvent(ResourceAvailabilityEvent const& /*event*/, const Guid& /*rendererId*/)
    {
        LOG_WARN(CONTEXT_FRAMEWORK, "ClientApplicationLogic::handleResourceAvailabilityEvent: is not implemented yet.");
    }

    ManagedResource ClientApplicationLogic::loadResource(const ResourceContentHash& hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        auto mr = m_resourceComponent->loadResource(hash);
        if (!mr)
            LOG_WARN(CONTEXT_FRAMEWORK, "ResourceComponent::loadResource: Could not find or load requested resource: " << hash);

        return mr;
    }
}
