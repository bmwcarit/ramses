//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/SceneReferenceLogic.h"
#include "internal/RendererLib/RendererScenes.h"
#include "internal/RendererLib/RendererSceneControlLogic.h"
#include "internal/RendererLib/IRendererSceneUpdater.h"
#include "internal/RendererLib/SceneReferenceOwnership.h"
#include "internal/RendererLib/IRendererSceneEventSender.h"
#include "internal/Core/Utils/LogMacros.h"
#include <algorithm>

namespace ramses::internal
{
    SceneReferenceLogic::SceneReferenceLogic(const RendererScenes& scenes, IRendererSceneControlLogic& sceneLogic, IRendererSceneUpdater& sceneUpdater, IRendererSceneEventSender& sender, SceneReferenceOwnership& sharedOwnership)
        : m_rendererScenes(scenes)
        , m_sceneLogic(sceneLogic)
        , m_sceneUpdater(sceneUpdater)
        , m_eventSender(sender)
        , m_sharedOwnership(sharedOwnership)
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
        cleanupReleasedReferences();
        executePendingActions();
    }

    void SceneReferenceLogic::extractAndSendSceneReferenceEvents(RendererEventVector& events)
    {
        const auto it = std::remove_if(events.begin(), events.end(), [&](const auto& evt)
        {
            switch (evt.eventType)
            {
            case ERendererEventType::SceneStateChanged:
            {
                const auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                if (masterSceneId.isValid())
                {
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: sending event scene reference (master {} / ref {}) state changed to {}", masterSceneId, evt.sceneId, EnumToString(evt.state));
                    m_eventSender.sendSceneStateChanged(masterSceneId, evt.sceneId, evt.state);
                    if (evt.state == RendererSceneState::Unavailable)
                    {
                        m_masterScenes[masterSceneId].expirationStates.erase(evt.sceneId);
                        m_masterScenesWithChangedExpirationState.push_back(masterSceneId);
                    }
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType::SceneFlushed:
            {
                const auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                if (masterSceneId.isValid() && m_masterScenes[masterSceneId].sceneReferencesWithFlushNotification.count(evt.sceneId) != 0)
                {
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: sending event scene reference (master {} / ref {}) flushed with version {}", masterSceneId, evt.sceneId, evt.sceneVersionTag);
                    m_eventSender.sendSceneFlushed(masterSceneId, evt.sceneId, evt.sceneVersionTag);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType::SceneDataLinked:
            case ERendererEventType::SceneDataLinkFailed:
            {
                // check if any ref scene involved either as provider or consumer
                const auto masterSceneId1 = findMasterSceneForReferencedScene(evt.providerSceneId);
                const auto masterSceneId2 = findMasterSceneForReferencedScene(evt.consumerSceneId);
                const auto masterSceneId = (masterSceneId1.isValid() ? masterSceneId1 : masterSceneId2);
                if (masterSceneId.isValid())
                {
                    const bool status = (evt.eventType == ERendererEventType::SceneDataLinked);
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: sending event scene reference data linked to master {} (providerScene:dataId -> consumerSceneId:dataId) {}:{} -> {}:{} STATUS={}",
                        masterSceneId, evt.providerSceneId, evt.providerdataId, evt.consumerSceneId, evt.consumerdataId, status);
                    m_eventSender.sendDataLinked(masterSceneId, evt.providerSceneId, evt.providerdataId, evt.consumerSceneId, evt.consumerdataId, status);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType::SceneDataUnlinked:
            case ERendererEventType::SceneDataUnlinkFailed:
            {
                // check if any ref scene involved either as provider or consumer
                const auto masterSceneId1 = findMasterSceneForReferencedScene(evt.providerSceneId);
                const auto masterSceneId2 = findMasterSceneForReferencedScene(evt.consumerSceneId);
                const auto masterSceneId = (masterSceneId1.isValid() ? masterSceneId1 : masterSceneId2);
                if (masterSceneId.isValid())
                {
                    const bool status = (evt.eventType == ERendererEventType::SceneDataUnlinked);
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: sending event scene reference data unlinked to master {} consumerSceneId:dataId {}:{} STATUS={}",
                        masterSceneId, evt.consumerSceneId, evt.consumerdataId, status);
                    m_eventSender.sendDataUnlinked(masterSceneId, evt.consumerSceneId, evt.consumerdataId, status);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType::SceneExpirationMonitoringEnabled:
            case ERendererEventType::SceneExpirationMonitoringDisabled:
            case ERendererEventType::SceneExpired:
            case ERendererEventType::SceneRecoveredFromExpiration:
            {
                auto logExpirationMsg = [](SceneId master, SceneId evtScene, const char* msg) {
                    if (master != evtScene)
                    {
                        LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: received event scene reference (master {} / ref {}) {}", master, evtScene, msg);
                    }
                    else
                    {
                        LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: received event master scene {} {}", master, msg);
                    }
                };
                auto masterSceneId = findMasterSceneForReferencedScene(evt.sceneId);
                // check if scene from event is master itself
                if (!masterSceneId.isValid() && m_masterScenes.count(evt.sceneId))
                    masterSceneId = evt.sceneId;
                if (masterSceneId.isValid())
                {
                    switch (evt.eventType)
                    {
                    case ERendererEventType::SceneExpirationMonitoringEnabled:
                        m_masterScenes[masterSceneId].expirationStates[evt.sceneId] = ExpirationState::MonitoringEnabled;
                        logExpirationMsg(masterSceneId, evt.sceneId, "enabled for expiration monitoring");
                        break;
                    case ERendererEventType::SceneExpirationMonitoringDisabled:
                        m_masterScenes[masterSceneId].expirationStates[evt.sceneId] = ExpirationState::MonitoringDisabled;
                        logExpirationMsg(masterSceneId, evt.sceneId, "disabled for expiration monitoring");
                        break;
                    case ERendererEventType::SceneExpired:
                        m_masterScenes[masterSceneId].expirationStates[evt.sceneId] = ExpirationState::Expired;
                        logExpirationMsg(masterSceneId, evt.sceneId, "expired");
                        break;
                    case ERendererEventType::SceneRecoveredFromExpiration:
                        m_masterScenes[masterSceneId].expirationStates[evt.sceneId] = ExpirationState::MonitoringEnabled;
                        logExpirationMsg(masterSceneId, evt.sceneId, "recovered from expiration");
                        break;
                    default:
                        assert(false);
                        break;
                    }
                    m_masterScenesWithChangedExpirationState.push_back(masterSceneId);
                }
                return masterSceneId.isValid();
            }
            case ERendererEventType::SceneDataSlotProviderCreated:
            case ERendererEventType::SceneDataSlotProviderDestroyed:
                // implicit or irrelevant - if for referenced scene, these must be removed from event queue but are not sent to master scene client
                return findMasterSceneForReferencedScene(evt.providerSceneId).isValid();
            case ERendererEventType::SceneDataSlotConsumerCreated:
            case ERendererEventType::SceneDataSlotConsumerDestroyed:
                // implicit or irrelevant - if for referenced scene, these must be removed from event queue but are not sent to master scene client
                return findMasterSceneForReferencedScene(evt.consumerSceneId).isValid();
            case ERendererEventType::SceneDataBufferLinked:
            case ERendererEventType::SceneDataBufferLinkFailed:
            case ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange:
                // not supported yet - if for referenced scene, these must be removed from event queue but are not sent to master scene client
                return findMasterSceneForReferencedScene(evt.consumerSceneId).isValid();
            default:
                return false;
            }
        });
        events.erase(it, events.end());

        for (const auto masterSceneId : m_masterScenesWithChangedExpirationState)
            consolidateExpirationState(masterSceneId, events);
        m_masterScenesWithChangedExpirationState.clear();
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
            OffscreenBufferHandle masterOB;
            int32_t masterRenderOrder = 0;
            m_sceneLogic.getSceneInfo(masterSceneId, masterTargetState, masterOB, masterRenderOrder);
            auto& masterSceneInfo = m_masterScenes[masterSceneId];
            masterSceneInfo.destroyed = false;

            for (SceneReferenceHandle handle{ 0u }; handle < masterScene.getSceneReferenceCount(); ++handle)
            {
                if (!masterScene.isSceneReferenceAllocated(handle))
                    continue;

                const auto& refData = masterScene.getSceneReference(handle);
                const auto refSceneId = refData.sceneId;
                if (masterSceneInfo.sceneReferences.count(refSceneId) == 0)
                {
                    // if ref scene was previously owned by another master remove that ownership - this can happen if ref scene changes ownership
                    const auto oldMaster = findMasterSceneForReferencedScene(refSceneId);
                    if (oldMaster.isValid())
                    {
                        LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: discarding old scene reference ownership (master {} / ref {})", oldMaster, refSceneId);
                        m_masterScenes[oldMaster].sceneReferences.erase(refSceneId);
                    }
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: new scene reference ownership (master {} / ref {})", masterSceneId, refSceneId);

                    masterSceneInfo.sceneReferences[refSceneId] = handle;
                    m_sharedOwnership.setOwner(refSceneId, masterSceneId);
                }

                RendererSceneState refTargetState;
                OffscreenBufferHandle refOB;
                int32_t refRenderOrder = 0;
                m_sceneLogic.getSceneInfo(refSceneId, refTargetState, refOB, refRenderOrder);

                // referenced scene inherits OB assignment
                // render order is not inherited but has to be set when setting new mapping or it changed
                // render order to request for referenced scene is relative to master scene
                const int toBeRequestedRefRenderOrder = masterRenderOrder + refData.renderOrder;
                if (refOB != masterOB || refRenderOrder != toBeRequestedRefRenderOrder)
                {
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: setting assignment/renderOrder (master {} / ref {}) to buffer {}, renderOrder {}",
                        masterSceneId, refSceneId, masterOB, toBeRequestedRefRenderOrder);
                    m_sceneLogic.setSceneDisplayBufferAssignment(refSceneId, masterOB, toBeRequestedRefRenderOrder);
                }

                if (refTargetState != refData.requestedState || refTargetState > masterTargetState)
                {
                    // referenced scene can never have 'higher' state than its master scene
                    const auto stateToRequest = std::min(refData.requestedState, masterTargetState);
                    if (refTargetState != stateToRequest)
                    {
                        LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic::updateReferencedScenes: setting state (master {} / ref {}) to {}", masterSceneId, refSceneId, EnumToString(stateToRequest));
                        m_sceneLogic.setSceneState(refSceneId, stateToRequest);
                    }
                }

                // send scene version tag if flush notification just enabled
                // later version tag notifications are sent when processing events
                if (refData.flushNotifications)
                {
                    if (masterSceneInfo.sceneReferencesWithFlushNotification.count(refSceneId) == 0)
                    {
                        masterSceneInfo.sceneReferencesWithFlushNotification.insert(refSceneId);

                        // notifications were just enabled
                        if (m_rendererScenes.hasScene(refSceneId))
                        {
                            const auto& refSceneStagingInfo = m_rendererScenes.getStagingInfo(refSceneId);
                            if (refSceneStagingInfo.lastAppliedVersionTag.isValid())
                            {
                                LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: flush notifications enabled, sending last applied scene reference (master {} / ref {}) version {}", masterSceneId, refSceneId, refSceneStagingInfo.lastAppliedVersionTag);
                                m_eventSender.sendSceneFlushed(masterSceneId, refSceneId, refSceneStagingInfo.lastAppliedVersionTag);
                            }
                        }
                    }
                }
                else
                    masterSceneInfo.sceneReferencesWithFlushNotification.erase(refSceneId);
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
                LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: master scene {} destroyed, cleaning up its referenced scenes", masterScene.first);
                // unsubscribe any scene referenced by destroyed master scene
                for (const auto& sceneRefIt : masterInfo.sceneReferences)
                {
                    const auto sceneRefId = sceneRefIt.first;
                    constexpr auto cleanupState = RendererSceneState::Available;
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: cleaning up (master {} / ref {}) by setting state to {}", masterScene.first, sceneRefId, EnumToString(cleanupState));
                    m_sceneLogic.setSceneState(sceneRefId, cleanupState);
                }
                masterInfo.pendingActions.clear();
                masterInfo.sceneReferencesWithFlushNotification.clear();
                masterInfo.expirationStates.clear();
                masterInfo.consolidatedExpirationState = ExpirationState::MonitoringDisabled;
                masterInfo.destroyed = true;
            }
        }
    }

    void SceneReferenceLogic::cleanupReleasedReferences()
    {
        // check for newly released references in master scenes
        for (auto& masterInfoIt : m_masterScenes)
        {
            auto& masterInfo = masterInfoIt.second;
            if (!masterInfo.destroyed)
            {
                const auto masterSceneId = masterInfoIt.first;
                const auto& masterScene = m_rendererScenes.getScene(masterSceneId);
                std::vector<std::pair<SceneId, SceneReferenceHandle>> releasedRefs;
                for (const auto& sceneRefIt : masterInfo.sceneReferences)
                {
                    if (!masterScene.isSceneReferenceAllocated(sceneRefIt.second))
                        releasedRefs.emplace_back(sceneRefIt.first, sceneRefIt.second);
                }

                if (!releasedRefs.empty())
                {
                    for (const auto& releasedRef : releasedRefs)
                    {
                        const auto sceneRefId = releasedRef.first;
                        LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: reference to scene (master {} / ref {}) released", masterSceneId, sceneRefId);

                        // remove from master info lists
                        masterInfo.sceneReferences.erase(sceneRefId);
                        m_sharedOwnership.setOwner(sceneRefId, SceneId::Invalid());
                        masterInfo.sceneReferencesWithFlushNotification.erase(sceneRefId);
                        masterInfo.expirationStates.erase(sceneRefId);

                        // remove actions for released reference
                        const auto it = std::remove_if(masterInfo.pendingActions.begin(), masterInfo.pendingActions.end(), [&, refHandle = releasedRef.second](const auto& action)
                        {
                            if (action.providerScene == refHandle || action.consumerScene == refHandle)
                            {
                                LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: erasing pending scene reference action for released reference (master {} / ref {}) actionType={}", masterSceneId, sceneRefId, action.type);
                                return true;
                            }
                            return false;
                        });
                        masterInfo.pendingActions.erase(it, masterInfo.pendingActions.end());
                    }

                    // trigger update of expiration state
                    m_masterScenesWithChangedExpirationState.push_back(masterSceneId);
                }
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
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: executing data link for master scene {} (providerScene:dataId -> consumerSceneId:dataId) {}:{} -> {}:{}", masterSceneId, providerSceneId, action.providerId, consumerSceneId, action.consumerId);
                    m_sceneUpdater.handleSceneDataLinkRequest(providerSceneId, action.providerId, consumerSceneId, action.consumerId);
                    break;
                }
                case SceneReferenceActionType::UnlinkData:
                {
                    const auto consumerSceneId = (action.consumerScene.isValid() ? masterScene.getSceneReference(action.consumerScene).sceneId : masterSceneId);
                    LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: executing data unlink for master scene {} (consumerSceneId:dataId) {}:{}", masterSceneId, consumerSceneId, action.consumerId);
                    m_sceneUpdater.handleDataUnlinkRequest(consumerSceneId, action.consumerId);
                    break;
                }
                }
            }
            masterSceneIt.second.pendingActions.clear();
        }
    }

    void SceneReferenceLogic::consolidateExpirationState(SceneId masterSceneId, RendererEventVector& events)
    {
        auto& masterInfo = m_masterScenes[masterSceneId];

        // resolve enable/disable first:
        // - enable must precede any expire/recover event
        // - disable must not be followed by any expire/recover event

        // #1 check disabled -> enabled
        if (masterInfo.consolidatedExpirationState == ExpirationState::MonitoringDisabled
            && !std::all_of(std::cbegin(masterInfo.expirationStates), std::cend(masterInfo.expirationStates), [](const auto& s) { return s.second == ExpirationState::MonitoringDisabled; }))
        {
            events.push_back({ ERendererEventType::SceneExpirationMonitoringEnabled, masterSceneId });
            masterInfo.consolidatedExpirationState = ExpirationState::MonitoringEnabled;
            LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: reporting master scene {} as enabled for expiration monitoring", masterSceneId);
        }

        // #2 check enabled/expired -> disabled
        if (masterInfo.consolidatedExpirationState != ExpirationState::MonitoringDisabled
            && std::all_of(std::cbegin(masterInfo.expirationStates), std::cend(masterInfo.expirationStates), [](const auto& s) { return s.second == ExpirationState::MonitoringDisabled; }))
        {
            events.push_back({ ERendererEventType::SceneExpirationMonitoringDisabled, masterSceneId });
            masterInfo.consolidatedExpirationState = ExpirationState::MonitoringDisabled;
            LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: reporting master scene {} as disabled for expiration monitoring", masterSceneId);
        }

        // #3 check enabled -> expired
        if (masterInfo.consolidatedExpirationState == ExpirationState::MonitoringEnabled
            && std::any_of(std::cbegin(masterInfo.expirationStates), std::cend(masterInfo.expirationStates), [](const auto& s) { return s.second == ExpirationState::Expired; }))
        {
            events.push_back({ ERendererEventType::SceneExpired, masterSceneId });
            masterInfo.consolidatedExpirationState = ExpirationState::Expired;
            LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: reporting master scene {} as expired", masterSceneId);
        }

        // #4 check expired -> recovered
        if (masterInfo.consolidatedExpirationState == ExpirationState::Expired
            && std::none_of(std::cbegin(masterInfo.expirationStates), std::cend(masterInfo.expirationStates), [](const auto& s) { return s.second == ExpirationState::Expired; }))
        {
            events.push_back({ ERendererEventType::SceneRecoveredFromExpiration, masterSceneId });
            masterInfo.consolidatedExpirationState = ExpirationState::MonitoringEnabled;
            LOG_INFO(CONTEXT_RENDERER, "SceneReferenceLogic: reporting master scene {} as recovered from expiration", masterSceneId);
        }
    }

    bool SceneReferenceLogic::hasAnyReferencedScenes() const
    {
        return std::any_of(m_masterScenes.cbegin(), m_masterScenes.cend(), [](const auto& m) { return !m.second.destroyed; });
    }

    SceneId SceneReferenceLogic::findMasterSceneForReferencedScene(SceneId sceneId) const
    {
        assert(std::count_if(m_masterScenes.cbegin(), m_masterScenes.cend(), [sceneId](const auto& s) { return s.second.sceneReferences.count(sceneId) != 0; }) <= 1);
        for (const auto& master : m_masterScenes)
        {
            if (master.second.sceneReferences.count(sceneId) != 0u)
                return master.first;
        }

        return SceneId::Invalid();
    }
}
