//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ClientSceneLogicShadowCopy.h"
#include "Components/ISceneGraphSender.h"
#include "Scene/ClientScene.h"
#include "Scene/SceneDescriber.h"
#include "Scene/SceneActionApplier.h"
#include "Scene/SceneActionCollectionCreator.h"
#include "Scene/SceneResourceUtils.h"
#include "PlatformAbstraction/PlatformGuard.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "Utils/LogMacros.h"
#include "Utils/StatisticCollection.h"
#include "Components/FlushTimeInformation.h"

namespace ramses_internal
{
    ClientSceneLogicShadowCopy::ClientSceneLogicShadowCopy(ISceneGraphSender& sceneGraphSender, ClientScene& scene, const Guid& clientAddress)
        : ClientSceneLogicBase(sceneGraphSender, scene, clientAddress)
        , m_sceneShadowCopy(SceneInfo(scene.getSceneId(), scene.getName()))
    {
        m_sceneShadowCopy.preallocateSceneSize(m_scene.getSceneSizeInformation());
    }

    void ClientSceneLogicShadowCopy::postAddSubscriber()
    {
        sendShadowCopySceneToWaitingSubscribers();
    }

    void ClientSceneLogicShadowCopy::flushSceneActions(ESceneFlushMode flushMode, const FlushTimeInformation& flushTimeInfo, SceneVersionTag versionTag)
    {
        const SceneSizeInformation sceneSizes(m_scene.getSceneSizeInformation());

        // swap out of ClientScene and reserve new memory there
        SceneActionCollection collection;
        collection.swap(m_scene.getSceneActionCollection());

        const bool hasNewActions = !collection.empty();

        if (m_flushCounter == 0)
        {
            LOG_INFO_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) {
                        sos << "ClientSceneLogicShadowCopy::flushSceneActions: first flush, sceneId " << m_sceneId << ", numActions " << collection.numberOfActions() << ", published " << isPublished() << ", subsActive [";
                        for (const auto& sub : m_subscribersActive)
                            sos << sub << " ";
                        sos << "], subsWaiting [";
                        for (const auto& sub : m_subscribersWaitingForScene)
                            sos << sub << " ";
                        sos << "]";
                    }));
        }

        ++m_flushCounter;

        if (isPublished())
        {
            SceneActionCollectionCreator creator(collection);
            creator.flush(
                m_flushCounter,
                flushMode == ESceneFlushMode_Synchronous,
                sceneSizes > m_sceneShadowCopy.getSceneSizeInformation(),
                sceneSizes,
                m_scene.getResourceChanges(),
                flushTimeInfo,
                versionTag);
        }

        // reserve memory in ClientScene after flush because flush might add a lot of data
        m_scene.getSceneActionCollection().reserveAdditionalCapacity(collection.collectionData().size(), collection.numberOfActions());

        if (hasNewActions)
        {
            m_sceneShadowCopy.preallocateSceneSize(sceneSizes);
            SceneActionApplier::ApplyActionsOnScene(m_sceneShadowCopy, collection, &m_animationSystemFactory);
            m_scene.getStatisticCollection().statSceneActionsGenerated.incCounter(collection.numberOfActions());
            m_scene.getStatisticCollection().statSceneActionsGeneratedSize.incCounter(static_cast<UInt32>(collection.collectionData().size()));
        }

        LOG_DEBUG_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) { printFlushInfo(sos, "ClientSceneLogicShadowCopy::flushSceneActions", collection, flushMode); }));

        if (isPublished() && !m_subscribersActive.empty())
        {
            m_scene.getStatisticCollection().statSceneActionsSent.incCounter(collection.numberOfActions()*static_cast<UInt32>(m_subscribersActive.size()));
            m_scenegraphSender.sendSceneActionList(m_subscribersActive, std::move(collection), m_sceneId, m_scenePublicationMode);
        }

        m_scene.clearResourceChanges();

        // store flush time info and version for async new subscribers, scene validity must also be guaranteed for them
        m_flushTimeInfoOfLastFlush = flushTimeInfo;
        if (versionTag != InvalidSceneVersionTag)
            m_lastVersionTag = versionTag;

        // send to subscribers if flushed for first time
        if (m_flushCounter == 1u)
        {
            sendShadowCopySceneToWaitingSubscribers();
        }
    }

    void ClientSceneLogicShadowCopy::sendShadowCopySceneToWaitingSubscribers()
    {
        if (m_flushCounter == 0u || !isPublished())
        {
            LOG_DEBUG(CONTEXT_CLIENT, "ClientSceneLogic::sendShadowCopySceneToWaitingSubscribers: delay sending of scene " << m_sceneId.getValue() << " (numWaiting " <<
                m_subscribersWaitingForScene.size() << ", flushCnt " << m_flushCounter << ", published " << isPublished() << ")");
            return;
        }

        sendSceneToWaitingSubscribers(m_sceneShadowCopy, m_flushTimeInfoOfLastFlush, m_lastVersionTag);
    }
}
