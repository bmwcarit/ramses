//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "Components/ClientSceneLogicDirect.h"
#include "Components/ISceneGraphSender.h"
#include "Scene/ClientScene.h"
#include "Scene/SceneDescriber.h"
#include "Scene/SceneActionApplier.h"
#include "PlatformAbstraction/PlatformTime.h"
#include "Utils/LogMacros.h"
#include "Utils/StatisticCollection.h"
#include "Components/FlushTimeInformation.h"
#include "Components/SceneUpdate.h"
#include "Components/ClientSceneLogicBase.h"
#include "Components/IResourceProviderComponent.h"

namespace ramses_internal
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
        if (!verifyAndGetResourceChanges(sceneUpdate, hasNewActions))
        {
            LOG_ERROR(CONTEXT_CLIENT, "ClientSceneLogicDirect::flushSceneActions: At least one resource can't be loaded, Scene can't be rendered. Consult log and run Scene::validate() for more information");
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
                                << ", subsActive [";
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
            const bool addSizeInfo = sceneSizes > m_previousSceneSizes;
            sceneUpdate.flushInfos = { m_flushCounter, versionTag, addSizeInfo?sceneSizes: SceneSizeInformation(), m_resourceChanges, m_scene.getSceneReferenceActions(), flushTimeInfo, addSizeInfo, true};
            m_previousSceneSizes = sceneSizes;
        }

        // reserve memory in ClientScene after flush because flush might add a lot of data
        m_scene.getSceneActionCollection().reserveAdditionalCapacity(sceneUpdate.actions.collectionData().size(), sceneUpdate.actions.numberOfActions());

        if (hasNewActions)
        {
            m_scene.getStatisticCollection().statSceneActionsGenerated.incCounter(sceneUpdate.actions.numberOfActions());
            m_scene.getStatisticCollection().statSceneActionsGeneratedSize.incCounter(static_cast<UInt32>(sceneUpdate.actions.collectionData().size()));
        }

        LOG_DEBUG_F(CONTEXT_CLIENT, ([&](StringOutputStream& sos) { printFlushInfo(sos, "ClientSceneLogicDirect::flushSceneActions", sceneUpdate.actions); }));

        if (isPublished() && !m_subscribersActive.empty())
        {
            m_scene.getStatisticCollection().statSceneActionsSent.incCounter(sceneUpdate.actions.numberOfActions()*static_cast<UInt32>(m_subscribersActive.size()));
            m_scenegraphSender.sendSceneUpdate(m_subscribersActive, std::move(sceneUpdate), m_sceneId, m_scenePublicationMode, m_scene.getStatisticCollection());
        }

        m_scene.resetResourceChanges();
        m_scene.resetSceneReferenceActions();

        if (isPublished())
            sendSceneToWaitingSubscribers(m_scene, flushTimeInfo, versionTag);

        return true;
    }
}
