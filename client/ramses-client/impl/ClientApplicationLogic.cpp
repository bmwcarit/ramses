//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ClientApplicationLogic.h"
#include "Components/IResourceProviderComponent.h"
#include "Components/ISceneGraphProviderComponent.h"
#include "Components/ResourceTableOfContents.h"
#include "Scene/ClientScene.h"
#include "RamsesFrameworkImpl.h"
#include "Utils/LogMacros.h"
#include "PlatformAbstraction/PlatformLock.h"
#include "PlatformAbstraction/PlatformGuard.h"

namespace ramses_internal
{
    ClientApplicationLogic::ClientApplicationLogic(const Guid& myId, PlatformLock& frameworkLock)
        : m_frameworkLock(frameworkLock)
        , m_resourceComponent(0)
        , m_scenegraphProviderComponent(0)
        , m_myId(myId)
    {
    }

    ClientApplicationLogic::~ClientApplicationLogic()
    {
    }

    void ClientApplicationLogic::init(IResourceProviderComponent& resources, ISceneGraphProviderComponent& scenegraph)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent = &resources;
        m_scenegraphProviderComponent = &scenegraph;
        m_scenegraphProviderComponent->setSceneProviderServiceHandler(this);
    }

    void ClientApplicationLogic::deinit()
    {
        PlatformGuard guard(m_frameworkLock);
        if (m_scenegraphProviderComponent)
        {
            m_scenegraphProviderComponent->setSceneProviderServiceHandler(NULL);
        }
        m_scenegraphProviderComponent = 0;
    }

    void ClientApplicationLogic::createScene(ClientScene& scene, bool enableLocalOnlyOptimization)
    {
        PlatformGuard guard(m_frameworkLock);
        LOG_TRACE(CONTEXT_CLIENT, "ClientApplicationLogic::createScene:  '" << scene.getName() << "' with id '" << scene.getSceneId().getValue() << "'");
        m_scenegraphProviderComponent->handleCreateScene(scene, enableLocalOnlyOptimization);
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

    Bool ClientApplicationLogic::isScenePublished(SceneId sceneId) const
    {
        return m_publishedScenes.hasElement(sceneId);
    }

    void ClientApplicationLogic::flush(SceneId sceneId, ESceneFlushMode flushMode, const FlushTimeInformation& timeInfo, SceneVersionTag versionTag)
    {
        PlatformGuard guard(m_frameworkLock);
        m_scenegraphProviderComponent->handleFlush(sceneId, flushMode, timeInfo, versionTag);
    }

    void ClientApplicationLogic::removeScene(SceneId sceneId)
    {
        PlatformGuard guard(m_frameworkLock);
        m_publishedScenes.remove(sceneId);
        m_scenegraphProviderComponent->handleUnpublishScene(sceneId);
        m_scenegraphProviderComponent->handleRemoveScene(sceneId);
    }

    void ClientApplicationLogic::handleSubscribeScene(const SceneId& sceneId, const Guid& consumerID)
    {
        m_scenegraphProviderComponent->handleSceneSubscription(sceneId, consumerID);
    }

    void ClientApplicationLogic::handleUnsubscribeScene(const SceneId& sceneId, const Guid& consumerID)
    {
        m_scenegraphProviderComponent->handleSceneUnsubscription(sceneId, consumerID);
    }

    ramses_internal::ManagedResource ClientApplicationLogic::addResource(const IResource* resource)
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->manageResource(*resource);
    }

    ramses_internal::ManagedResource ClientApplicationLogic::getResource(ResourceContentHash hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->getResource(hash);
    }

    ramses_internal::ResourceHashUsage ClientApplicationLogic::getHashUsage(const ResourceContentHash& hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->getResourceHashUsage(hash);
    }

    ManagedResourceVector ClientApplicationLogic::getResources() const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->getResources();
    }

    void ClientApplicationLogic::addResourceFile(ResourceFileInputStreamSPtr resourceFileInputStream, const ResourceTableOfContents& toc)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent->addResourceFile(resourceFileInputStream, toc);
    }

    void ClientApplicationLogic::removeResourceFile(const String& resourceFileName)
    {
        PlatformGuard guard(m_frameworkLock);
        m_resourceComponent->removeResourceFile(resourceFileName);
    }

    bool ClientApplicationLogic::hasResourceFile(const String& resourceFileName) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->hasResourceFile(resourceFileName);
    }

    void ClientApplicationLogic::reserveResourceCount(uint32_t totalCount)
    {
        m_resourceComponent->reserveResourceCount(totalCount);
    }

    ManagedResource ClientApplicationLogic::forceLoadResource(const ResourceContentHash& hash) const
    {
        PlatformGuard guard(m_frameworkLock);
        return m_resourceComponent->forceLoadResource(hash);
    }
}
