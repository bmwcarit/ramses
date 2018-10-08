//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ForwardingCommunicationSystem.h"
#include "Components/ManagedResource.h"
#include "Utils/BinaryOutputStream.h"
#include "Utils/BinaryInputStream.h"
#include "PlatformAbstraction/PlatformMemory.h"
#include "Components/ResourceStreamSerialization.h"
#include "ResourceSerializationTestHelper.h"
#include "Scene/SceneActionCollection.h"

namespace ramses_internal
{
    ForwardingCommunicationSystem::ForwardingCommunicationSystem(const Guid& id)
        : m_id(id)
        , m_targetCommunicationSystem(0)
        , m_resourceConsumerHandler(0)
        , m_resourceProviderHandler(0)
        , m_sceneProviderHandler(0)
        , m_sceneRendererHandler(0)
    {
    }

    ForwardingCommunicationSystem::~ForwardingCommunicationSystem()
    {
    }

    void ForwardingCommunicationSystem::setForwardingTarget(ForwardingCommunicationSystem* target)
    {
        m_targetCommunicationSystem = target;
    }

    bool ForwardingCommunicationSystem::connectServices()
    {
        return true;
    }

    bool ForwardingCommunicationSystem::disconnectServices()
    {
        return true;
    }

    IConnectionStatusUpdateNotifier& ForwardingCommunicationSystem::getConnectionStatusUpdateNotifier()
    {
        return m_connectionStatusUpdateNotifier;
    }

    bool ForwardingCommunicationSystem::sendSubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneProviderHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneProviderHandler->handleSubscribeScene(sceneId, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendUnsubscribeScene(const Guid& to, const SceneId& sceneId)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneProviderHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneProviderHandler->handleUnsubscribeScene(sceneId, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendSceneNotAvailable(const Guid& to, const SceneId& sceneId)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneRendererHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneRendererHandler->handleSceneNotAvailable(sceneId, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendRequestResources(const Guid& to, const ResourceContentHashVector& resources)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_resourceProviderHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_resourceProviderHandler->handleRequestResources(resources, 0u, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendResourcesNotAvailable(const Guid& to, const ResourceContentHashVector& resources)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_resourceConsumerHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_resourceConsumerHandler->handleResourcesNotAvailable(resources, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendResources(const Guid& to, const ManagedResourceVector& resources)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_resourceConsumerHandler && to == m_targetCommunicationSystem->m_id)
        {
            auto resDataVec = ResourceSerializationTestHelper::ConvertResourcesToResourceDataVector(resources, getSendDataSizes().resourceDataArray);
            for (const auto& resData : resDataVec)
            {
                ByteArrayView view(resData.data(), static_cast<UInt32>(resData.size()));
                m_targetCommunicationSystem->m_resourceConsumerHandler->handleSendResource(view, m_id);
            }
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendScenesAvailable(const Guid& to, const SceneInfoVector& availableScenes)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneRendererHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneRendererHandler->handleNewScenesAvailable(availableScenes, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::broadcastNewScenesAvailable(const SceneInfoVector& newScenes)
    {
        sendScenesAvailable(m_targetCommunicationSystem->m_id, newScenes);
        return true;
    }

    bool ForwardingCommunicationSystem::broadcastScenesBecameUnavailable(const SceneInfoVector& unavailableScenes)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneRendererHandler)
        {
            m_targetCommunicationSystem->m_sceneRendererHandler->handleScenesBecameUnavailable(unavailableScenes, m_id);
        }
        return true;
    }

    bool ForwardingCommunicationSystem::sendInitializeScene(const Guid& to, const SceneInfo& sceneInfo)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneRendererHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneRendererHandler->handleInitializeScene(sceneInfo, m_id);
        }
        return true;
    }

    uint64_t ForwardingCommunicationSystem::sendSceneActionList(const Guid& to, const SceneId& sceneId, const SceneActionCollection& actions, const uint64_t& actionListCounter)
    {
        if (m_targetCommunicationSystem && m_targetCommunicationSystem->m_sceneRendererHandler && to == m_targetCommunicationSystem->m_id)
        {
            m_targetCommunicationSystem->m_sceneRendererHandler->handleSceneActionList(sceneId, actions.copy(), actionListCounter, m_id);
        }
        return 1u;
    }

    CommunicationSendDataSizes ForwardingCommunicationSystem::getSendDataSizes() const
    {
        return CommunicationSendDataSizes{ std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(),
                std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max(),
                std::numeric_limits<UInt32>::max(), std::numeric_limits<UInt32>::max() };
    }

    void ForwardingCommunicationSystem::setSendDataSizes(const CommunicationSendDataSizes& /*sizes*/)
    {
    }

    void ForwardingCommunicationSystem::setResourceProviderServiceHandler(IResourceProviderServiceHandler* handler)
    {
        m_resourceProviderHandler = handler;
    }

    void ForwardingCommunicationSystem::setResourceConsumerServiceHandler(IResourceConsumerServiceHandler* handler)
    {
        m_resourceConsumerHandler = handler;
    }

    void ForwardingCommunicationSystem::setSceneProviderServiceHandler(ISceneProviderServiceHandler* handler)
    {
        m_sceneProviderHandler = handler;
    }

    void ForwardingCommunicationSystem::setSceneRendererServiceHandler(ISceneRendererServiceHandler* handler)
    {
        m_sceneRendererHandler = handler;
    }

    void ForwardingCommunicationSystem::logConnectionInfo() const
    {
    }

    void ForwardingCommunicationSystem::triggerLogMessageForPeriodicLog() const
    {
    }

}
