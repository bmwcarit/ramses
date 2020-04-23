//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/RendererSceneControlLogic.h"
#include "RendererLib/IRendererSceneControl.h"
#include "RendererLib/RendererEvent.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    RendererSceneControlLogic::RendererSceneControlLogic(IRendererSceneControl& sceneStateControl)
        : m_sceneStateControl(sceneStateControl)
    {
    }

    void RendererSceneControlLogic::setSceneState(SceneId sceneId, RendererSceneState state)
    {
        m_scenesInfo[sceneId].targetState = GetInternalSceneState(state);
        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::setSceneMapping(SceneId sceneId, DisplayHandle displayId)
    {
        MappingInfo mappingInfo;
        mappingInfo.display = displayId;
        // if newly mapping (to a different display potentially) reset buffer assignment
        mappingInfo.displayBuffer = OffscreenBufferHandle::Invalid();
        mappingInfo.renderOrder = 0;

        m_scenesInfo[sceneId].mappingInfo = mappingInfo;
    }

    void RendererSceneControlLogic::setSceneDisplayBufferAssignment(SceneId sceneId, OffscreenBufferHandle displayBuffer, int32_t sceneRenderOrder)
    {
        auto& mapInfo = m_scenesInfo[sceneId].mappingInfo;
        assert(mapInfo.display.isValid());
        // update mapping info for scene
        mapInfo.displayBuffer = displayBuffer;
        mapInfo.renderOrder = sceneRenderOrder;

        // if scene already mapped/assigned and not requested to unmap, execute new assignment right away
        const auto currState = getCurrentSceneState(sceneId);
        const auto targetState = getTargetSceneState(sceneId);
        if (currState >= ESceneStateInternal::MappedAndAssigned && targetState >= ESceneStateInternal::Mapped)
            m_sceneStateControl.handleSceneDisplayBufferAssignmentRequest(sceneId, displayBuffer, sceneRenderOrder);
    }

    void RendererSceneControlLogic::getSceneInfo(SceneId sceneId, RendererSceneState& targetState, DisplayHandle& displayToMap, OffscreenBufferHandle& bufferToAssign, int32_t& renderOrder) const
    {
        targetState = RendererSceneState::Unavailable;
        displayToMap = {};
        bufferToAssign = {};
        renderOrder = 0;
        const auto it = m_scenesInfo.find(sceneId);
        if (it != m_scenesInfo.cend())
        {
            targetState = GetSceneStateFromInternal(it->second.targetState);
            const auto& mappingInfo = it->second.mappingInfo;
            displayToMap = mappingInfo.display;
            bufferToAssign = mappingInfo.displayBuffer;
            renderOrder = mappingInfo.renderOrder;
        }
    }

    RendererSceneControlLogic::ESceneStateInternal RendererSceneControlLogic::getCurrentSceneState(SceneId sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.currentState : ESceneStateInternal::Unpublished;
    }

    RendererSceneControlLogic::ESceneStateInternal RendererSceneControlLogic::getTargetSceneState(SceneId sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.targetState : ESceneStateInternal::Unpublished;
    }

    RendererSceneControlLogic::ESceneStateCommand RendererSceneControlLogic::getLastSceneStateCommandWaitingForReply(SceneId sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.lastCommandWaitigForReply : ESceneStateCommand::None;
    }

    void RendererSceneControlLogic::goToTargetState(SceneId sceneId)
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
                m_sceneStateControl.handleSceneSubscriptionRequest(sceneId);
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
                m_sceneStateControl.handleSceneMappingRequest(sceneId, sceneInfo.mappingInfo.display);
                sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Map;
                break;
            }
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating unsubscribe of scene with id: " << sceneId);
                m_sceneStateControl.handleSceneUnsubscriptionRequest(sceneId, false);
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating unmap of scene with id: " << sceneId);
                m_sceneStateControl.handleSceneUnmappingRequest(sceneId);
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
                m_sceneStateControl.handleSceneShowRequest(sceneId);
                sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Show;
                break;
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic initiating unmap of scene with id: " << sceneId);
                m_sceneStateControl.handleSceneUnmappingRequest(sceneId);
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
                m_sceneStateControl.handleSceneHideRequest(sceneId);
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

    void RendererSceneControlLogic::goToMappedAndAssignedState(SceneId sceneId)
    {
        assert(m_scenesInfo.count(sceneId) > 0);
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Mapped);
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        MappingInfo& mapInfo = sceneInfo.mappingInfo;

        if (m_sceneStateControl.handleSceneDisplayBufferAssignmentRequest(sceneId, mapInfo.displayBuffer, mapInfo.renderOrder))
        {
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            goToTargetState(sceneId);
        }
    }

    void RendererSceneControlLogic::setCurrentSceneState(SceneId sceneId, ESceneStateInternal state)
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

    void RendererSceneControlLogic::scenePublished(SceneId sceneId)
    {
        assert(m_scenesInfo.count(sceneId) == 0 || getCurrentSceneState(sceneId) == ESceneStateInternal::Unpublished);

        setCurrentSceneState(sceneId, ESceneStateInternal::Published);
        m_pendingEvents.push_back({ Event::Type::ScenePublished, sceneId, RendererSceneState::Unavailable });
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic generated event: scene published (scene " << sceneId << ")");

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnpublished(SceneId sceneId)
    {
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Published);
        setCurrentSceneState(sceneId, ESceneStateInternal::Unpublished);
    }

    void RendererSceneControlLogic::sceneSubscribed(SceneId sceneId, EventResult result)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneSubscribed: Could not subscribe scene with id: " << sceneId);
            break;
        case EventResult::Indirect:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnsubscribed(SceneId sceneId, EventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != EventResult::Indirect)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            goToTargetState(sceneId);
            break;
        case EventResult::Indirect:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneUnsubscribed: Could not unsubscribe scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::sceneMapped(SceneId sceneId, EventResult result)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Mapped);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneMapped: Could not map scene with id: " << sceneId);
            break;
        case EventResult::Indirect:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneUnmapped(SceneId sceneId, EventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != EventResult::Indirect)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            goToTargetState(sceneId);
            break;
        case EventResult::Indirect:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneUnmapped: Could not unmap scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::sceneShown(SceneId sceneId, EventResult result)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Rendered);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneShown: Could not show scene with id: " << sceneId);
            break;
        case EventResult::Indirect:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void RendererSceneControlLogic::sceneHidden(SceneId sceneId, EventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != EventResult::Indirect)
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
        case EventResult::OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            goToTargetState(sceneId);
            break;
        case EventResult::Indirect:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            break;
        case EventResult::Failed:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "RendererSceneControlLogic::sceneHidden: Could not hide scene with id: " << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void RendererSceneControlLogic::processInternalEvent(const InternalSceneStateEvent& evt)
    {
        switch (evt.type)
        {
        case ERendererEventType_ScenePublished:
            scenePublished(evt.sceneId);
            break;
        case ERendererEventType_SceneUnpublished:
            sceneUnpublished(evt.sceneId);
            break;
        case ERendererEventType_SceneSubscribed:
            sceneSubscribed(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneSubscribeFailed:
            sceneSubscribed(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        case ERendererEventType_SceneUnsubscribed:
            sceneUnsubscribed(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneUnsubscribedIndirect:
            sceneUnsubscribed(evt.sceneId, RendererSceneControlLogic::EventResult::Indirect);
            break;
        case ERendererEventType_SceneUnsubscribeFailed:
            sceneUnsubscribed(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        case ERendererEventType_SceneMapped:
            sceneMapped(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneMapFailed:
            sceneMapped(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        case ERendererEventType_SceneUnmapped:
            sceneUnmapped(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneUnmappedIndirect:
            sceneUnmapped(evt.sceneId, RendererSceneControlLogic::EventResult::Indirect);
            break;
        case ERendererEventType_SceneUnmapFailed:
            sceneUnmapped(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        case ERendererEventType_SceneShown:
            sceneShown(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneShowFailed:
            sceneShown(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        case ERendererEventType_SceneHidden:
            sceneHidden(evt.sceneId, RendererSceneControlLogic::EventResult::OK);
            break;
        case ERendererEventType_SceneHiddenIndirect:
            sceneHidden(evt.sceneId, RendererSceneControlLogic::EventResult::Indirect);
            break;
        case ERendererEventType_SceneHideFailed:
            sceneHidden(evt.sceneId, RendererSceneControlLogic::EventResult::Failed);
            break;
        default:
            assert(false);
            break;
        }
    }

    void RendererSceneControlLogic::consumeEvents(Events& eventsOut)
    {
        m_pendingEvents.swap(eventsOut);
        m_pendingEvents.clear();
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
}
