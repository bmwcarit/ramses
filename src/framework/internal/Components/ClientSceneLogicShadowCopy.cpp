//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ClientSceneLogicShadowCopy.h"
#include "internal/Components/ISceneGraphSender.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/SceneDescriber.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/IResourceProviderComponent.h"
#include "internal/Components/SceneUpdate.h"

namespace ramses::internal
{
    ClientSceneLogicShadowCopy::ClientSceneLogicShadowCopy(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress, EFeatureLevel featureLevel)
        : ClientSceneLogicBase(sceneGraphSender, scene, res, clientAddress, featureLevel)
        , m_sceneShadowCopy(SceneInfo{ scene.getSceneId(), scene.getName(), EScenePublicationMode::LocalOnly, scene.getRenderBackendCompatibility(), scene.getVulkanAPIVersion(), scene.getSPIRVVersion() })
    {
        m_sceneShadowCopy.preallocateSceneSize(m_scene.getSceneSizeInformation());
    }

    void ClientSceneLogicShadowCopy::postAddSubscriber()
    {
        sendShadowCopySceneToWaitingSubscribers();
    }

    bool ClientSceneLogicShadowCopy::flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        const bool hasNewActions = !m_scene.getSceneActionCollection().empty();

        SceneUpdate sceneUpdate;
        const auto resourceChangeState = verifyAndGetResourceChanges(sceneUpdate, hasNewActions);
        if (resourceChangeState == ResourceChangeState::MissingResource)
        {
            LOG_ERROR(CONTEXT_CLIENT, "ClientSceneLogicShadowCopy::flushSceneActions: At least one resource can't be loaded, "
                        "Scene {} can't be rendered. Consult log and run Scene::validate() for more information", m_scene.getSceneId());
            return false;
        }

        fillStatisticsCollection();
        const SceneSizeInformation sceneSizes(m_scene.getSceneSizeInformation());

        // swap out of ClientScene and reserve new memory there
        sceneUpdate.actions.swap(m_scene.getSceneActionCollection());
        if (resourceChangeState == ResourceChangeState::HasChanges)
            m_lastFlushUsedResources = m_resourceComponent.resolveResources(m_lastFlushResourcesInUse); // keep ll resources alive, in case we need to send a scene update to a new subscriber

        if (m_flushCounter == 0)
        {
            LOG_INFO(CONTEXT_CLIENT,
                "ClientSceneLogicShadowCopy::flushSceneActions: first flush, sceneId {}, numActions {}, published {}, numResources {}, subsActive [{}], subsWaiting [{}]",
                m_sceneId,
                sceneUpdate.actions.numberOfActions(),
                isPublished(),
                sceneUpdate.resources.size(),
                fmt::join(m_subscribersActive, " "),
                fmt::join(m_subscribersWaitingForScene, " "));
        }

        const bool expirationChanged = updateExpirationAndCheckIfChanged(flushTimeInfo);
        const bool skipSceneActionSend = canSkipSceneActionSend(sceneUpdate.actions.numberOfActions(), versionTag, expirationChanged, flushTimeInfo.isEffectTimeSync);

        ++m_flushCounter;

        if (isPublished())
            sceneUpdate.flushInfos = { m_flushCounter, versionTag, sceneSizes, m_resourceChangesSinceLastFlush, m_scene.getSceneReferenceActions(), flushTimeInfo,sceneSizes > m_sceneShadowCopy.getSceneSizeInformation(), true };

        // reserve memory in ClientScene after flush because flush might add a lot of data
        m_scene.getSceneActionCollection().reserveAdditionalCapacity(sceneUpdate.actions.collectionData().size(), sceneUpdate.actions.numberOfActions());

        if (hasNewActions)
        {
            m_sceneShadowCopy.preallocateSceneSize(sceneSizes);
            SceneActionApplier::ApplyActionsOnScene(m_sceneShadowCopy, sceneUpdate.actions, m_featureLevel);
            m_scene.getStatisticCollection().statSceneActionsGenerated.incCounter(sceneUpdate.actions.numberOfActions());
            m_scene.getStatisticCollection().statSceneActionsGeneratedSize.incCounter(static_cast<uint32_t>(sceneUpdate.actions.collectionData().size()));
        }

        LOG_DEBUG_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) { printFlushInfo(sos, "ClientSceneLogicShadowCopy::flushSceneActions", sceneUpdate); }));

        if (isPublished() && !m_subscribersActive.empty())
        {
            if (skipSceneActionSend)
            {
                LOG_DEBUG(CONTEXT_CLIENT, "ClientSceneLogicShadowCopy::flushSceneActions: skip flush for sceneId {}, cnt {} because empty", m_sceneId, m_flushCounter);
                m_scene.getStatisticCollection().statSceneActionsSentSkipped.incCounter(1);
            }
            else
            {
                assert(m_scenePublicationMode.has_value());
                m_scene.getStatisticCollection().statSceneActionsSent.incCounter(sceneUpdate.actions.numberOfActions() * static_cast<uint32_t>(m_subscribersActive.size()));
                m_scenegraphSender.sendSceneUpdate(m_subscribersActive, std::move(sceneUpdate), m_sceneId, *m_scenePublicationMode, m_scene.getStatisticCollection());
            }
        }

        m_scene.resetResourceChanges();
        m_scene.resetSceneReferenceActions();

        // store flush time info and version for async new subscribers, scene validity must also be guaranteed for them
        m_flushTimeInfoOfLastFlush = flushTimeInfo;
        if (flushTimeInfo.isEffectTimeSync)
        {
            m_effectTimeSync = flushTimeInfo.internalTimestamp;
        }
        if (versionTag.isValid())
            m_lastVersionTag = versionTag;

        // send to subscribers if flushed for first time
        if (m_flushCounter == 1u)
            sendShadowCopySceneToWaitingSubscribers();

        return true;
    }

    void ClientSceneLogicShadowCopy::sendShadowCopySceneToWaitingSubscribers()
    {
        if (m_flushCounter == 0u || !isPublished())
        {
            LOG_DEBUG(CONTEXT_CLIENT, "ClientSceneLogic::sendShadowCopySceneToWaitingSubscribers: delay sending of scene {} (numWaiting {}, flushCnt {}, published {})",
                m_sceneId, m_subscribersWaitingForScene.size(), m_flushCounter, isPublished());
            return;
        }

        if (m_effectTimeSync != FlushTime::InvalidTimestamp)
        {
            m_flushTimeInfoOfLastFlush.internalTimestamp = m_effectTimeSync;
            m_flushTimeInfoOfLastFlush.isEffectTimeSync = true;
        }

        sendSceneToWaitingSubscribers(m_sceneShadowCopy, m_flushTimeInfoOfLastFlush, m_lastVersionTag);
    }
}
