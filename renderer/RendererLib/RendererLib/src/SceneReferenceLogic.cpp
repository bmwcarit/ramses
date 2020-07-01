//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/SceneReferenceLogic.h"
#include "RendererLib/RendererScenes.h"
#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/IRendererSceneControl.h"
#include "RendererFramework/IRendererSceneEventSender.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    SceneReferenceLogic::SceneReferenceLogic(const RendererScenes& scenes, IRendererSceneControlLogic& sceneLogic, IRendererSceneControl& sceneControl, IRendererSceneEventSender& sender)
        : m_rendererScenes(scenes)
        , m_sceneLogic(sceneLogic)
        , m_sceneControl(sceneControl)
        , m_eventSender(sender)
    {
    }

    void SceneReferenceLogic::addActions(SceneId masterScene, const SceneReferenceActionVector& actions)
    {
        auto& data = m_masterScenes[masterScene].pendingActions;
        data.insert(data.end(), actions.cbegin(), actions.cend());
    }

    void SceneReferenceLogic::update()
    {
        updateReferencedScenes();
        cleanupDestroyedMasterScenes();
        executePendingActions();
    }

    void SceneReferenceLogic::extractAndSendSceneReferenceEvents(RendererEventVector& events)
    {
        std::vector<SceneId> mastersWithChangedExpiration;
        const auto it = std::remove_if(events.begin(), events.end(), [&](const auto& evt)
        {
            switch (evt.eventType)
            {
            case ERendererEventType_SceneStateChanged:
            {
                const auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                if (masterSceneId.isValid())
                {
                    m_eventSender.sendSceneStateChanged(masterSceneId, evt.sceneId, evt.state);
                    if (evt.state == RendererSceneState::Unavailable)
                    {
                        m_masterScenes[masterSceneId].expiredSceneReferences.erase(evt.sceneId);
                        mastersWithChangedExpiration.push_back(masterSceneId);
                    }
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType_SceneFlushed:
            {
                const auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                if (m_rendererScenes.hasScene(masterSceneId))
                {
                    const auto& masterScene = m_rendererScenes.getScene(masterSceneId);
                    const auto& sceneRefs = masterScene.getSceneReferences();
                    const auto sceneRefIt = std::find_if(sceneRefs.cbegin(), sceneRefs.cend(), [&evt](const auto& sr) { return sr.second->sceneId == evt.sceneId; });
                    if (sceneRefIt != sceneRefs.cend() && sceneRefIt->second->flushNotifications)
                        m_eventSender.sendSceneFlushed(masterSceneId, evt.sceneId, evt.sceneVersionTag);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType_SceneDataLinked:
            case ERendererEventType_SceneDataLinkFailed:
            {
                // check if any ref scene involved either as provider or consumer
                const auto masterSceneId1 = findMasterSceneForReferencedScene(evt.providerSceneId);
                const auto masterSceneId2 = findMasterSceneForReferencedScene(evt.consumerSceneId);
                const auto masterSceneId = (masterSceneId1.isValid() ? masterSceneId1 : masterSceneId2);
                if (masterSceneId.isValid())
                    m_eventSender.sendDataLinked(masterSceneId, evt.providerSceneId, evt.providerdataId, evt.consumerSceneId, evt.consumerdataId, (evt.eventType == ERendererEventType_SceneDataLinked));
                return masterSceneId.isValid();
            }
            case ERendererEventType_SceneDataUnlinked:
            case ERendererEventType_SceneDataUnlinkFailed:
            {
                // check if any ref scene involved either as provider or consumer
                const auto masterSceneId1 = findMasterSceneForReferencedScene(evt.providerSceneId);
                const auto masterSceneId2 = findMasterSceneForReferencedScene(evt.consumerSceneId);
                const auto masterSceneId = (masterSceneId1.isValid() ? masterSceneId1 : masterSceneId2);
                if (masterSceneId.isValid())
                    m_eventSender.sendDataUnlinked(masterSceneId, evt.consumerSceneId, evt.consumerdataId, (evt.eventType == ERendererEventType_SceneDataUnlinked));
                return masterSceneId.isValid();
            }
            case ERendererEventType_SceneExpired:
            case ERendererEventType_SceneRecoveredFromExpiration:
            {
                const auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                if (masterSceneId.isValid())
                {
                    if (evt.eventType == ERendererEventType_SceneExpired)
                        m_masterScenes[masterSceneId].expiredSceneReferences.insert(evt.sceneId);
                    else
                        m_masterScenes[masterSceneId].expiredSceneReferences.erase(evt.sceneId);
                    mastersWithChangedExpiration.push_back(masterSceneId);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType_SceneAssignedToDisplayBuffer:
            case ERendererEventType_SceneAssignedToDisplayBufferFailed:
            case ERendererEventType_SceneDataSlotProviderCreated:
            case ERendererEventType_SceneDataSlotProviderDestroyed:
            case ERendererEventType_SceneDataSlotConsumerCreated:
            case ERendererEventType_SceneDataSlotConsumerDestroyed:
                // implicit or irrelevant - if for referenced scene, these must be removed from event queue but are not sent to master scene client
                return findMasterSceneForReferencedScene(evt.sceneId).isValid();
            case ERendererEventType_SceneDataBufferLinked:
            case ERendererEventType_SceneDataBufferLinkFailed:
            case ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange:
                // not supported yet - if for referenced scene, these must be removed from event queue but are not sent to master scene client
                return findMasterSceneForReferencedScene(evt.consumerSceneId).isValid();
            default:
                return false;
            }
        });
        events.erase(it, events.end());

        for (const auto masterSceneId : mastersWithChangedExpiration)
        {
            auto& masterInfo = m_masterScenes[masterSceneId];
            if (!masterInfo.reportedAsExpired && !masterInfo.expiredSceneReferences.empty())
            {
                events.push_back({ ERendererEventType_SceneExpired, masterSceneId });
                masterInfo.reportedAsExpired = true;
            }
            else if (masterInfo.reportedAsExpired && masterInfo.expiredSceneReferences.empty())
            {
                events.push_back({ ERendererEventType_SceneRecoveredFromExpiration, masterSceneId });
                masterInfo.reportedAsExpired = false;
            }
        }
    }

    void SceneReferenceLogic::updateReferencedScenes()
    {
        // check for any scene references, compare values stored in their master scenes and update states accordingly
        for (const auto& sceneIt : m_rendererScenes)
        {
            const auto& masterScene = *(sceneIt.value.scene);
            if (masterScene.getSceneReferenceCount() == 0)
                continue;

            const auto masterSceneId = sceneIt.key;
            RendererSceneState masterTargetState;
            DisplayHandle masterDisplay;
            OffscreenBufferHandle masterOB;
            int32_t masterRenderOrder = 0;
            m_sceneLogic.getSceneInfo(masterSceneId, masterTargetState, masterDisplay, masterOB, masterRenderOrder);
            auto& masterSceneInfo = m_masterScenes[masterSceneId];
            masterSceneInfo.destroyed = false;

            for (SceneReferenceHandle handle{ 0u }; handle < masterScene.getSceneReferenceCount(); ++handle)
            {
                if (!masterScene.isSceneReferenceAllocated(handle))
                    continue;

                const auto& refData = masterScene.getSceneReference(handle);
                const auto refSceneId = refData.sceneId;
                masterSceneInfo.sceneReferences.insert(refSceneId);
                RendererSceneState refTargetState;
                DisplayHandle refDisplay;
                OffscreenBufferHandle refOB;
                int32_t refRenderOrder = 0;
                m_sceneLogic.getSceneInfo(refSceneId, refTargetState, refDisplay, refOB, refRenderOrder);

                // referenced scene inherits display mapping and OB assignment
                // render order is not inherited but has to be set when setting new mapping or it changed
                if (masterDisplay.isValid())
                {
                    // render order to request for referenced scene is relative to master scene
                    const int toBeRequestedRefRenderOrder = masterRenderOrder + refData.renderOrder;

                    if (refDisplay != masterDisplay)
                    {
                        LOG_INFO_P(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: setting mapping and assignment/renderOrder (master {} / reffed {}) to display {}, buffer {}, renderOrder {}",
                            masterSceneId, refSceneId, masterDisplay, masterOB, toBeRequestedRefRenderOrder);
                        m_sceneLogic.setSceneMapping(refSceneId, masterDisplay);
                        m_sceneLogic.setSceneDisplayBufferAssignment(refSceneId, masterOB, toBeRequestedRefRenderOrder);
                    }
                    else if (refOB != masterOB || refRenderOrder != toBeRequestedRefRenderOrder)
                    {
                        LOG_INFO_P(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: setting assignment/renderOrder (master {} / reffed {}) to buffer {}, renderOrder {}",
                            masterSceneId, refSceneId, masterOB, toBeRequestedRefRenderOrder);
                        m_sceneLogic.setSceneDisplayBufferAssignment(refSceneId, masterOB, toBeRequestedRefRenderOrder);
                    }
                }

                if (refTargetState != refData.requestedState || refTargetState > masterTargetState)
                {
                    // referenced scene can never have 'higher' state than its master scene
                    const auto stateToRequest = std::min(refData.requestedState, masterTargetState);
                    if (refTargetState != stateToRequest)
                    {
                        LOG_INFO_P(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: setting state (master {} / reffed {}) to {}", masterSceneId, refSceneId, EnumToString(stateToRequest));
                        m_sceneLogic.setSceneState(refSceneId, stateToRequest);
                    }
                }
            }
        }
    }

    void SceneReferenceLogic::cleanupDestroyedMasterScenes()
    {
        // check for newly destroyed master scenes
        for (auto& masterScene : m_masterScenes)
        {
            auto& masterInfo = masterScene.second;
            if (!m_rendererScenes.hasScene(masterScene.first) && !masterInfo.destroyed)
            {
                // unsubscribe any scene referenced by destroyed master scene
                for (const auto sceneRefId : masterInfo.sceneReferences)
                {
                    if (m_rendererScenes.hasScene(sceneRefId))
                        m_sceneLogic.setSceneState(sceneRefId, RendererSceneState::Unavailable);
                }
                masterInfo.pendingActions.clear();
                masterInfo.expiredSceneReferences.clear();
                masterInfo.reportedAsExpired = false;
                masterInfo.destroyed = true;
            }
        }
    }

    void SceneReferenceLogic::executePendingActions()
    {
        // apply scene reference actions
        for (auto& masterSceneIt : m_masterScenes)
        {
            if (masterSceneIt.second.destroyed)
                continue;

            const auto masterSceneId = masterSceneIt.first;
            const auto& masterScene = m_rendererScenes.getScene(masterSceneId);
            for (const auto& action : masterSceneIt.second.pendingActions)
            {
                switch (action.type)
                {
                case SceneReferenceActionType::LinkData:
                {
                    const auto providerSceneId = (action.providerScene.isValid() ? masterScene.getSceneReference(action.providerScene).sceneId : masterSceneId);
                    const auto consumerSceneId = (action.consumerScene.isValid() ? masterScene.getSceneReference(action.consumerScene).sceneId : masterSceneId);
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: executing data link (providerScene:dataId -> consumerSceneId:dataId) " << providerSceneId << ":" << action.providerId << " -> " << consumerSceneId << ":" << action.consumerId);
                    m_sceneControl.handleSceneDataLinkRequest(providerSceneId, action.providerId, consumerSceneId, action.consumerId);
                    break;
                }
                case SceneReferenceActionType::UnlinkData:
                {
                    const auto consumerSceneId = (action.consumerScene.isValid() ? masterScene.getSceneReference(action.consumerScene).sceneId : masterSceneId);
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: executing data unlink (consumerSceneId:dataId) " << consumerSceneId << ":" << action.consumerId);
                    m_sceneControl.handleDataUnlinkRequest(consumerSceneId, action.consumerId);
                    break;
                }
                }
            }
            masterSceneIt.second.pendingActions.clear();
        }
    }

    bool SceneReferenceLogic::hasAnyReferencedScenes() const
    {
        return std::any_of(m_masterScenes.cbegin(), m_masterScenes.cend(), [](const auto& m) { return !m.second.destroyed; });
    }

    SceneId SceneReferenceLogic::findMasterSceneForReferencedScene(SceneId sceneId) const
    {
        for (const auto& master : m_masterScenes)
            if (master.second.sceneReferences.count(sceneId))
                return master.first;

        return SceneId::Invalid();
    }
}
