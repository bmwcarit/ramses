//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneControlLogic.h"
#include "RamsesFrameworkTypesImpl.h"
#include "RendererSceneControlImpl_legacy.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    RendererSceneControlLogic::RendererSceneControlLogic(IRendererSceneControlImpl_legacy& sceneControl)
        : m_sceneControl(sceneControl)
    {
    }

    void RendererSceneControlLogic::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        m_scenesInfo[sceneId].targetState = GetInternalSceneState(state);
        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        MappingInfo mappingInfo;
        mappingInfo.display = displayId;
        // if newly mapping (to a different display potentially) reset buffer assignment
        mappingInfo.displayBuffer = displayBufferId_t::Invalid();
        mappingInfo.renderOrder = 0;

        m_scenesInfo[sceneId].mappingInfo = mappingInfo;
    }

    void RendererSceneControlLogic::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        // update mapping info for scene
        auto& mapInfo = m_scenesInfo[sceneId].mappingInfo;
        assert(mapInfo.display.isValid());
        mapInfo.displayBuffer = displayBuffer;
        mapInfo.renderOrder = sceneRenderOrder;

        // if scene already mapped/assigned and not requested to unmap, execute new assignment right away
        const auto currState = getCurrentSceneState(sceneId);
        const auto targetState = getTargetSceneState(sceneId);
        if (currState >= ESceneStateInternal::MappedAndAssigned && targetState >= ESceneStateInternal::Mapped)
            m_sceneControl.assignSceneToDisplayBuffer(sceneId, displayBuffer, sceneRenderOrder);
    }

    void RendererSceneControlLogic::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        m_sceneControl.setDisplayBufferClearColor(display, displayBuffer, r, g, b, a);
    }

    void RendererSceneControlLogic::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        m_sceneControl.linkOffscreenBufferToSceneData(offscreenBufferId, consumerSceneId, consumerDataSlotId);
    }

    void RendererSceneControlLogic::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        m_sceneControl.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    }

    void RendererSceneControlLogic::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        m_sceneControl.unlinkData(consumerSceneId, consumerId);
    }

    RendererSceneControlLogic::ESceneStateInternal RendererSceneControlLogic::getCurrentSceneState(sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.currentState : ESceneStateInternal::Unpublished;
    }

    RendererSceneControlLogic::ESceneStateInternal RendererSceneControlLogic::getTargetSceneState(sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.targetState : ESceneStateInternal::Unpublished;
    }

    RendererSceneControlLogic::ESceneStateCommand RendererSceneControlLogic::getLastSceneStateCommandWaitingForReply(sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.lastCommandWaitigForReply : ESceneStateCommand::None;
    }

    void RendererSceneControlLogic::goToTargetState(sceneId_t sceneId)
    {
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        const ESceneStateInternal currentSceneState = sceneInfo.currentState;
        const ESceneStateInternal targetSceneState = sceneInfo.targetState;

        if (currentSceneState == targetSceneState)
            return;

        // wait for last command reply before advancing any state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::None)
            return;

        switch (currentSceneState)
        {
        case ESceneStateInternal::Unpublished:
            // Nothing to do here. When scene is published the appropriate state change will be triggered again
            break;

        case ESceneStateInternal::Published:

            switch (targetSceneState)
            {
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Mapped:
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating subscribe to scene with id: " << sceneId);
                if (m_sceneControl.subscribeScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Subscribe;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::Subscribed:

            switch (targetSceneState)
            {
            case ESceneStateInternal::Mapped:
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
            {
                assert(m_scenesInfo.count(sceneId) > 0);
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating map of scene " << sceneId << " to display " << sceneInfo.mappingInfo.display);
                if (m_sceneControl.mapScene(sceneInfo.mappingInfo.display, sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Map;
                break;
            }
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating unsubscribe of scene with id: " << sceneId);
                if (m_sceneControl.unsubscribeScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unsubscribe;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::Mapped:
            switch (targetSceneState)
            {
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
                goToMappedAndAssignedState(sceneId);
                break;
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating show of scene with id: " << sceneId);
                if (m_sceneControl.unmapScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unmap;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::MappedAndAssigned:

            switch (targetSceneState)
            {
            case ESceneStateInternal::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating show of scene with id: " << sceneId);
                if (m_sceneControl.showScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Show;
                break;
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating show of scene with id: " << sceneId);
                if (m_sceneControl.unmapScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unmap;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::Rendered:

            switch (targetSceneState)
            {
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating hide of scene with id: " << sceneId);
                if (m_sceneControl.hideScene(sceneId) == StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Hide;
                break;
            default:
                break;
            }
            break;

        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::goToMappedAndAssignedState(sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) > 0);
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Mapped);
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        MappingInfo& mapInfo = sceneInfo.mappingInfo;

        if (m_sceneControl.assignSceneToDisplayBuffer(sceneId, mapInfo.displayBuffer, mapInfo.renderOrder) == StatusOK)
        {
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            goToTargetState(sceneId);
        }
    }

    void RendererSceneControlLogic::setCurrentSceneState(sceneId_t sceneId, ESceneStateInternal state)
    {
        auto& sceneInfo = m_scenesInfo[sceneId];
        const RendererSceneState currState = GetSceneStateFromInternal(sceneInfo.currentState);
        const RendererSceneState newState = GetSceneStateFromInternal(state);
        sceneInfo.currentState = state;

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic received scene state change: scene " << sceneId << " is in state " << EnumToString(newState) << " (internal " << int(state) << ")");
        if (currState != newState)
        {
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, ([&](ramses_internal::StringOutputStream& sos)
            {
                sos << "RendererSceneControlLogic generated event: scene state change";
                sos << " (scene " << sceneId;
                sos << ", " << EnumToString(newState) << ")";
            }));
            m_pendingEvents.push_back({ Event::Type::SceneStateChanged, sceneId, newState });
        }
    }

    /* RendererSceneControlEventHandlerEmpty_legacy handlers */
    void RendererSceneControlLogic::scenePublished(sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) == 0 || getCurrentSceneState(sceneId) == ESceneStateInternal::Unpublished);

        setCurrentSceneState(sceneId, ESceneStateInternal::Published);
        m_pendingEvents.push_back({ Event::Type::ScenePublished, sceneId });
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: scene published (scene " << sceneId << ")");

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnpublished(sceneId_t sceneId)
    {
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Published);
        setCurrentSceneState(sceneId, ESceneStateInternal::Unpublished);
    }

    void RendererSceneControlLogic::sceneSubscribed(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Subscribe)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received subscription event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneSubscribed: Could not subscribe scene with id: " << sceneId);
            break;
        case ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Unsubscribe)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received unsubscription event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            goToTargetState(sceneId);
            break;
        case ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneUnsubscribed: Could not unsubscribe scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::sceneMapped(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Map)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received map event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Mapped);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneMapped: Could not map scene with id: " << sceneId);
            break;
        case ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnmapped(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Unmap)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received unmap event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            goToTargetState(sceneId);
            break;
        case ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneUnmapped: Could not unmap scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::sceneShown(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Show)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received show event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Rendered);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneShown: Could not show scene with id: " << sceneId);
            break;
        case ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneHidden(sceneId_t sceneId, ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Hide)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic - received hide event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            goToTargetState(sceneId);
            break;
        case ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            break;
        case ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneHidden: Could not hide scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::offscreenBufferLinkedToSceneData(displayBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result)
    {
        Event evt{ Event::Type::OffscreenBufferLinked };
        evt.displayBufferId = providerOffscreenBuffer;
        evt.consumerSceneId = consumerScene;
        evt.consumerId = consumerId;
        evt.dataLinked = (result == ERendererEventResult_OK);
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: offscreen buffer linked (offscreenBuffer " << providerOffscreenBuffer
            << " consumerScene " << consumerScene << " consumerId " << consumerId << " status=" << evt.dataLinked << ")");
    }

    void RendererSceneControlLogic::dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result)
    {
        Event evt{ Event::Type::DataLinked };
        evt.providerSceneId = providerScene;
        evt.providerId = providerId;
        evt.consumerSceneId = consumerScene;
        evt.consumerId = consumerId;
        evt.dataLinked = (result == ERendererEventResult_OK);
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: data linked (providerScene " << providerScene << " providerId " << providerId
            << " consumerScene " << consumerScene << " consumerId " << consumerId << " status=" << evt.dataLinked << ")");
    }

    void RendererSceneControlLogic::dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result)
    {
        Event evt{ Event::Type::DataUnlinked };
        evt.consumerSceneId = consumerScene;
        evt.consumerId = consumerId;
        evt.dataLinked = (result == ERendererEventResult_OK);
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: data unlinked ("
            << " consumerScene " << consumerScene << " consumerId " << consumerId << " status=" << evt.dataLinked << ")");
    }


    void RendererSceneControlLogic::sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag, ESceneResourceStatus)
    {
        Event evt{ Event::Type::SceneFlushed };
        evt.sceneId = sceneId;
        evt.sceneVersion = sceneVersionTag;
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: scene flushed ("
            << " scene " << sceneId << " versionTag " << sceneVersionTag << ")");
    }

    void RendererSceneControlLogic::sceneExpired(sceneId_t sceneId)
    {
        Event evt{ Event::Type::SceneExpired };
        evt.sceneId = sceneId;
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: scene expired ("
            << " scene " << sceneId << ")");
    }

    void RendererSceneControlLogic::sceneRecoveredFromExpiration(sceneId_t sceneId)
    {
        Event evt{ Event::Type::SceneRecoveredFromExpiration };
        evt.sceneId = sceneId;
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: scene recovered from expiration ("
            << " scene " << sceneId << ")");
    }

    void RendererSceneControlLogic::streamAvailabilityChanged(streamSource_t streamId, bool available)
    {
        Event evt{ Event::Type::StreamAvailable };
        evt.streamSourceId = streamId;
        evt.streamAvailable = available;
        m_pendingEvents.push_back(evt);
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: stream availability changed ("
            << " streamSourceId " << streamId << " available=" << available << ")");
    }

    RendererSceneState RendererSceneControlLogic::GetSceneStateFromInternal(ESceneStateInternal internalState)
    {
        switch (internalState)
        {
        case ESceneStateInternal::Unpublished:
        case ESceneStateInternal::Published:
            return RendererSceneState::Unavailable;
        case ESceneStateInternal::Subscribed:
        case ESceneStateInternal::Mapped:
            return RendererSceneState::Available;
        case ESceneStateInternal::MappedAndAssigned:
            return RendererSceneState::Ready;
        case ESceneStateInternal::Rendered:
            return RendererSceneState::Rendered;
        default:
            assert(false);
            return RendererSceneState::Unavailable;
        }
    }

    RendererSceneControlLogic::ESceneStateInternal RendererSceneControlLogic::GetInternalSceneState(RendererSceneState state)
    {
        switch (state)
        {
        case RendererSceneState::Unavailable:
            return ESceneStateInternal::Published;
        case RendererSceneState::Available:
            return ESceneStateInternal::Subscribed;
        case RendererSceneState::Ready:
            return ESceneStateInternal::MappedAndAssigned;
        case RendererSceneState::Rendered:
            return ESceneStateInternal::Rendered;
        default:
            assert(false);
            return ESceneStateInternal::Unpublished;
        }
    }

    RendererSceneControlLogic::Events RendererSceneControlLogic::consumeEvents()
    {
        const Events ret = m_pendingEvents;
        m_pendingEvents.clear();
        return ret;
    }
}
