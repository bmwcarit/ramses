//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Components/ClientSceneLogicDirect.h"
#include "internal/Components/ISceneGraphSender.h"
#include "internal/SceneGraph/Scene/ClientScene.h"
#include "internal/SceneGraph/Scene/SceneDescriber.h"
#include "internal/SceneGraph/Scene/SceneActionApplier.h"
#include "internal/PlatformAbstraction/PlatformTime.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/StatisticCollection.h"
#include "internal/Components/FlushTimeInformation.h"
#include "internal/Components/SceneUpdate.h"
#include "internal/Components/ClientSceneLogicBase.h"
#include "internal/Components/IResourceProviderComponent.h"

namespace ramses::internal
{
    ClientSceneLogicDirect::ClientSceneLogicDirect(ISceneGraphSender& sceneGraphSender, ClientScene& scene, IResourceProviderComponent& res, const Guid& clientAddress)
        : ClientSceneLogicBase(sceneGraphSender, scene, res, clientAddress)
        , m_previousSceneSizes(m_scene.getSceneSizeInformation())
    {
    }

    bool ClientSceneLogicDirect::flushSceneActions(const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        const bool hasNewActions = !m_scene.getSceneActionCollection().empty();

        SceneUpdate sceneUpdate;
        const auto resourceChangeState = verifyAndGetResourceChanges(sceneUpdate, hasNewActions);
        if (resourceChangeState == ResourceChangeState::MissingResource)
        {
            LOG_ERROR_P(CONTEXT_CLIENT, "ClientSceneLogicDirect::flushSceneActions: At least one resource can't be loaded, "
                        "Scene {} can't be rendered. Consult log and run Scene::validate() for more information",  m_scene.getSceneId());
            return false;
        }

        fillStatisticsCollection();
        const SceneSizeInformation sceneSizes(m_scene.getSceneSizeInformation());

        // swap out of ClientScene and reserve new memory there
        sceneUpdate.actions.swap(m_scene.getSceneActionCollection());

        if (m_flushCounter == 0)
        {
            LOG_INFO_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) {
                            sos << "ClientSceneLogicDirect::flushSceneActions: first flush, sceneId " << m_sceneId
                                << ", numActions " << sceneUpdate.actions.numberOfActions() << ", published " << isPublished()
                                << ", numResources " << sceneUpdate.resources.size()
                                << ", subsActive [";
                            for (const auto& sub : m_subscribersActive)
                                sos << sub << " ";
                            sos << "], subsWaiting [";
                            for (const auto& sub : m_subscribersWaitingForScene)
                                sos << sub << " ";
                            sos << "]";
                        }));
        }

        const bool expirationChanged = updateExpirationAndCheckIfChanged(flushTimeInfo);
        const bool skipSceneActionSend = canSkipSceneActionSend(sceneUpdate.actions.numberOfActions(), versionTag, expirationChanged, flushTimeInfo.isEffectTimeSync);

        ++m_flushCounter;

        if (isPublished())
        {
            const bool addSizeInfo = sceneSizes > m_previousSceneSizes;
            sceneUpdate.flushInfos = { m_flushCounter, versionTag, addSizeInfo?sceneSizes: SceneSizeInformation(), m_resourceChangesSinceLastFlush, m_scene.getSceneReferenceActions(), flushTimeInfo, addSizeInfo, true};
            m_previousSceneSizes = sceneSizes;
        }

        // reserve memory in ClientScene after flush because flush might add a lot of data
        m_scene.getSceneActionCollection().reserveAdditionalCapacity(sceneUpdate.actions.collectionData().size(), sceneUpdate.actions.numberOfActions());

        if (hasNewActions)
        {
            m_scene.getStatisticCollection().statSceneActionsGenerated.incCounter(sceneUpdate.actions.numberOfActions());
            m_scene.getStatisticCollection().statSceneActionsGeneratedSize.incCounter(static_cast<uint32_t>(sceneUpdate.actions.collectionData().size()));
        }

        LOG_DEBUG_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) { printFlushInfo(sos, "ClientSceneLogicDirect::flushSceneActions", sceneUpdate); }));

        if (isPublished() && !m_subscribersActive.empty())
        {
            if (skipSceneActionSend)
            {
                LOG_DEBUG(CONTEXT_CLIENT, "ClientSceneLogicDirect::flushSceneActions: skip flush for sceneId " << m_sceneId << ", cnt " << m_flushCounter << " because empty");
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

        if (flushTimeInfo.isEffectTimeSync)
        {
            m_effectTimeSync = flushTimeInfo.internalTimestamp;
        }

        if (isPublished())
        {
            auto initialFlushTime = flushTimeInfo;
            if (m_effectTimeSync != FlushTime::InvalidTimestamp)
            {
                initialFlushTime.internalTimestamp = m_effectTimeSync;
                initialFlushTime.isEffectTimeSync = true;
            }
            sendSceneToWaitingSubscribers(m_scene, initialFlushTime, versionTag);
        }

        return true;
    }
}
