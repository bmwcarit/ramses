//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ClientSceneLogicBase.h"
#include "internal/Components/ISceneGraphSender.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/SceneDescriber.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/SceneGraph/Scene/SceneActionCollectionCreator.h"
#include "internal/SceneGraph/SceneUtils/ResourceUtils.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Components/IResourceProviderComponent.h"
#include "internal/Components/SceneUpdate.h"

namespace ramses::internal
{
    ClientSceneLogicBase::ClientSceneLogicBase(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress)
        : m_scenegraphSender(sceneGraphSender)
        , m_resourceComponent(res)
        , m_myID(clientAddress)
        , m_sceneId(scene.getSceneId())
        , m_scene(scene)
    {
        std::fill(m_resourceCount.begin(), m_resourceCount.end(), 0);
        std::fill(m_resourceMaxSize.begin(), m_resourceMaxSize.end(), 0);
        std::fill(m_resourceDataSize.begin(), m_resourceDataSize.end(), 0);
    }

    ClientSceneLogicBase::~ClientSceneLogicBase()
    {
        unpublish();
    }

    void ClientSceneLogicBase::publish(EScenePublicationMode publicationMode)
    {
        if (!m_scenePublicationMode.has_value())
        {
            m_scenePublicationMode = publicationMode;
            m_scenegraphSender.sendPublishScene(m_sceneId, publicationMode, m_scene.getName());
        }
    }

    void ClientSceneLogicBase::unpublish()
    {
        if (m_scenePublicationMode.has_value())
        {
            m_scenegraphSender.sendUnpublishScene(m_sceneId, *m_scenePublicationMode);

            m_scenePublicationMode.reset();
        }

        // reset to initial state
        m_subscribersActive.clear();
        m_subscribersWaitingForScene.clear();
    }

    bool ClientSceneLogicBase::isPublished() const
    {
        return m_scenePublicationMode.has_value();
    }

    void ClientSceneLogicBase::addSubscriber(const Guid& newSubscriber)
    {
        if (contains_c(m_subscribersActive, newSubscriber) || contains_c(m_subscribersWaitingForScene, newSubscriber))
        {
            LOG_WARN(CONTEXT_CLIENT, "ClientSceneLogic::addSubscriber: already has " << newSubscriber << " for scene " << m_sceneId);
            return;
        }

        LOG_INFO(CONTEXT_CLIENT, "ClientSceneLogic::addSubscriber: add " << newSubscriber << " for scene " << m_sceneId << ", flushCounter " << m_flushCounter);
        m_subscribersWaitingForScene.push_back(newSubscriber);
        postAddSubscriber();
    }

    void ClientSceneLogicBase::removeSubscriber(const Guid& subscriber)
    {
        auto it = find_c(m_subscribersActive, subscriber);
        if (it != m_subscribersActive.end())
        {
            m_subscribersActive.erase(it);
            LOG_INFO(CONTEXT_CLIENT, "ClientSceneLogic::removeSubscriber: remove active subscriber " << subscriber << " from scene " << m_sceneId << ", numRemaining " << m_subscribersActive.size());
        }
        else
        {
            auto waitingForSceneIter = find_c(m_subscribersWaitingForScene, subscriber);
            if (waitingForSceneIter != m_subscribersWaitingForScene.end())
            {
                m_subscribersWaitingForScene.erase(waitingForSceneIter);
                LOG_INFO(CONTEXT_CLIENT, "ClientSceneLogic::removeSubscriber: remove waiting subscriber " << subscriber << " from scene " << m_sceneId << ", numRemaining " << m_subscribersWaitingForScene.size());
            }
        }
    }

    std::vector<Guid> ClientSceneLogicBase::getWaitingAndActiveSubscribers() const
    {
        std::vector<Guid> result(m_subscribersActive);
        result.insert(result.end(), m_subscribersWaitingForScene.begin(), m_subscribersWaitingForScene.end());
        return result;
    }

    void ClientSceneLogicBase::sendSceneToWaitingSubscribers(const IScene& scene, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        if (m_subscribersWaitingForScene.empty())
        {
            LOG_DEBUG(CONTEXT_CLIENT, "ClientSceneLogicBase::sendSceneToWaitingSubscribers: No subscribers waiting for scene " << m_sceneId);
            return;
        }

        SceneUpdate sceneUpdate;
        SceneActionCollectionCreator creator(sceneUpdate.actions);
        SceneDescriber::describeScene<IScene>(scene, creator);

        m_resourceChangesSinceLastFlush.clear();
        size_t sceneResourcesSize = 0u;
        ResourceUtils::GetAllSceneResourcesFromScene(m_resourceChangesSinceLastFlush.m_sceneResourceActions, scene, sceneResourcesSize);

        m_resourceChangesSinceLastFlush.m_resourcesAdded = m_lastFlushResourcesInUse;

        // flush asynchronously & check if there already was a named flush
        if (!m_resourceChangesSinceLastFlush.m_resourcesAdded.empty())
            sceneUpdate.resources = m_resourceComponent.resolveResources(m_resourceChangesSinceLastFlush.m_resourcesAdded);
        assert(sceneUpdate.resources.size() == m_resourceChangesSinceLastFlush.m_resourcesAdded.size());
        sceneUpdate.flushInfos = { m_flushCounter, versionTag, scene.getSceneSizeInformation(), m_resourceChangesSinceLastFlush, {}, flushTimeInfo, true, true };
        LOG_INFO(CONTEXT_CLIENT, "Sending scene " << scene.getSceneId() << " to " << m_subscribersWaitingForScene.size() << " subscribers, " <<
            sceneUpdate.actions.numberOfActions() << " scene actions (" << sceneUpdate.actions.collectionData().size() << " bytes)" <<
            m_resourceChangesSinceLastFlush.m_resourcesAdded.size() << " client resources, " <<
            m_resourceChangesSinceLastFlush.m_sceneResourceActions.size() << " scene resource actions (" << sceneResourcesSize << " bytes in total used by scene resources)");

        assert(m_scenePublicationMode.has_value());
        for(const auto& subscriber : m_subscribersWaitingForScene)
        {
            m_scenegraphSender.sendCreateScene(subscriber, m_sceneId, *m_scenePublicationMode);
        }
        m_scene.getStatisticCollection().statSceneActionsSent.incCounter(sceneUpdate.actions.numberOfActions()*static_cast<uint32_t>(m_subscribersWaitingForScene.size()));
        m_scenegraphSender.sendSceneUpdate(m_subscribersWaitingForScene, std::move(sceneUpdate), m_sceneId, *m_scenePublicationMode, m_scene.getStatisticCollection());

        m_subscribersActive.insert(m_subscribersActive.end(), m_subscribersWaitingForScene.begin(), m_subscribersWaitingForScene.end());
        m_subscribersWaitingForScene.clear();
    }

    void ClientSceneLogicBase::printFlushInfo(StringOutputStream& sos, const char* name, const SceneUpdate& update) const
    {
        sos << name << ": SceneID " << m_sceneId << ", flushIdx " << m_flushCounter
            << ", numActions " << update.actions.numberOfActions() << ", sizeActions " << update.actions.collectionData().size() <<
            ", numResources " << update.resources.size() <<
            " ; ActionCountPerType:\n";

        struct ActionInfo {
            uint32_t count{0};
            uint32_t size{0};
        };
        std::vector<ActionInfo> sceneActionCountPerType(NumOfSceneActionTypes);
        for (const auto& action : update.actions)
        {
            ActionInfo& info = sceneActionCountPerType[static_cast<uint32_t>(action.type())];
            ++info.count;
            info.size += action.size();
        }

        for (size_t actionType = 0; actionType < NumOfSceneActionTypes; ++actionType)
        {
            const ActionInfo& info = sceneActionCountPerType[actionType];
            if (info.count > 0)
            {
                sos << name << ": " << GetNameForSceneActionId(static_cast<ESceneActionId>(actionType)) << ": num " << info.count << ", size " << info.size << "\n";
            }
        }
    }

    const char* ClientSceneLogicBase::getSceneStateString() const
    {
        if (!m_subscribersActive.empty())
        {
            return "Subscribed";
        }
        if (m_scenePublicationMode.has_value())
        {
            switch (*m_scenePublicationMode)
            {
            case EScenePublicationMode::LocalAndRemote:
                return "Published";
            case EScenePublicationMode::LocalOnly:
                return "Published_LocalOnly";
            }
        }
        return "Unpublished";
    }

    void ClientSceneLogicBase::updateResourceStatistics()
    {
        // reset locally gathered resource statistics
        std::fill(m_resourceCount.begin(), m_resourceCount.end(), 0);
        std::fill(m_resourceMaxSize.begin(), m_resourceMaxSize.end(), 0);
        std::fill(m_resourceDataSize.begin(), m_resourceDataSize.end(), 0);

        for (auto const& hash : m_currentFlushResourcesInUse)
        {
            if (!m_resourceComponent.knowsResource(hash))
                continue; // no log, this will be logged when trying to load it

            ResourceInfo const& info = m_resourceComponent.getResourceInfo(hash);
            EResourceStatisticIndex index = EResourceStatisticIndex_ArrayResource;
            switch (info.type)
            {
            case EResourceType::VertexArray:
            case EResourceType::IndexArray:
                index = EResourceStatisticIndex_ArrayResource;
                break;
            case EResourceType::Effect:
                index = EResourceStatisticIndex_Effect;
                break;
            case EResourceType::Texture2D:
            case EResourceType::Texture3D:
            case EResourceType::TextureCube:
                index = EResourceStatisticIndex_Texture;
                break;
            case EResourceType::Invalid:
                assert(0);
            }
            const auto size = info.decompressedSize;

            m_resourceCount[index]++;
            m_resourceDataSize[index] += size;
            if (m_resourceMaxSize[index] < size)
                m_resourceMaxSize[index] = size;
        }
    }

    void ClientSceneLogicBase::fillStatisticsCollection()
    {
        auto& stats = m_scene.getStatisticCollection();
        for (size_t i = 0; i < EResourceStatisticIndex_NumIndices; ++i)
        {
            stats.statResourceCount[i].setCounterValue(m_resourceCount[i]);
            stats.statResourceAvgSize[i].setCounterValue(m_resourceCount[i] == 0 ? 0 : (m_resourceDataSize[i] / m_resourceCount[i]));
            stats.statResourceMaxSize[i].setCounterValue(m_resourceMaxSize[i]);
        }
    }

    bool ClientSceneLogicBase::updateExpirationAndCheckIfChanged(const FlushTimeInformation& flushTimeInfo)
    {
        const bool hasExpirationTSChange = (flushTimeInfo.expirationTimestamp != m_lastFlushedExpirationTimestamp);
        m_lastFlushedExpirationTimestamp = flushTimeInfo.expirationTimestamp;

        return hasExpirationTSChange;
    }

    bool ClientSceneLogicBase::canSkipSceneActionSend(uint32_t numSceneActions, SceneVersionTag versionTag, bool expirationChanged, bool isEffectTimeSync) const
    {
        return
            m_flushCounter != 0 &&      // never skip first flush (might block renderer side transition subscription pending -> subscibed)
            !isEffectTimeSync &&      // no effect time synchronization
            m_resourceChangesSinceLastFlush.empty() &&   // no resource changes (client+scene)
            numSceneActions == 0u &&  // no other sceneactions yet
            m_scene.getSceneReferenceActions().empty() &&  // no scenereference updates
            !expirationChanged && // no expiration monitoring change
            versionTag == SceneVersionTag::Invalid();  // no scene version
    }

    ClientSceneLogicBase::ResourceChangeState ClientSceneLogicBase::verifyAndGetResourceChanges(SceneUpdate& sceneUpdate, bool hasNewActions)
    {
        m_resourceChangesSinceLastFlush.clear();

        // optimization: early out if nothing changed in the scene
        if (!hasNewActions)
            return ResourceChangeState::NoChange;

        ResourceChangeState result = ResourceChangeState::NoChange;
        if (m_scene.haveResourcesChanged())
        {
            m_currentFlushResourcesInUse.clear();
            ResourceUtils::GetAllResourcesFromScene(m_currentFlushResourcesInUse, m_scene);
            ResourceUtils::DiffResources(m_lastFlushResourcesInUse, m_currentFlushResourcesInUse, m_resourceChangesSinceLastFlush);

            if (!m_resourceChangesSinceLastFlush.m_resourcesAdded.empty())
            {
                sceneUpdate.resources = m_resourceComponent.resolveResources(m_resourceChangesSinceLastFlush.m_resourcesAdded);
                if (sceneUpdate.resources.size() != m_resourceChangesSinceLastFlush.m_resourcesAdded.size())
                    return ResourceChangeState::MissingResource;
            }

            updateResourceStatistics();

            m_lastFlushResourcesInUse.swap(m_currentFlushResourcesInUse);
            result = ResourceChangeState::HasChanges;
        }

        m_resourceChangesSinceLastFlush.m_sceneResourceActions = m_scene.getSceneResourceActions();

        return result;
    }
}
