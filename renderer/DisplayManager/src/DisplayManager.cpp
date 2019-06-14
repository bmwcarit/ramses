//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "DisplayManager/DisplayManager.h"
#include "ramses-renderer-api/RamsesRenderer.h"
#include "RamsesRendererImpl.h"
#include "RamsesFrameworkImpl.h"
#include "DisplayManager/RendererEventChainer.h"
#include "PlatformAbstraction/PlatformGuard.h"

namespace ramses_display_manager
{
    DisplayManager::DisplayManager(ramses::RamsesRenderer& renderer, ramses::RamsesFramework& framework, bool autoShow)
        : m_ramsesRenderer(renderer)
        , m_autoShow(autoShow)
        , m_isRunning(true)
        , m_lock()
    {
        m_exitCommand.reset(new ramses_internal::RamshCommandExit);
        m_showOnDisplayCommand.reset(new ShowSceneOnDisplay(*this));
        m_hideCommand.reset(new HideScene(*this));
        m_unmapCommand.reset(new UnmapScene(*this));
        m_unsubscribeCommand.reset(new UnsubscribeScene(*this));
        m_linkData.reset(new LinkData(*this));
        m_confirmationEchoCommand.reset(new ConfirmationEcho(*this));

        ramses_internal::Ramsh& ramsh = framework.impl.getRamsh();
        ramsh.add(*m_exitCommand);
        ramsh.add(*m_showOnDisplayCommand);
        ramsh.add(*m_hideCommand);
        ramsh.add(*m_unmapCommand);
        ramsh.add(*m_unsubscribeCommand);
        ramsh.add(*m_linkData);
        ramsh.add(*m_confirmationEchoCommand);
        LOG_INFO(ramses_internal::CONTEXT_SMOKETEST, "Ramsh commands registered from DisplayManager");
    }

    DisplayManager::~DisplayManager()
    {
    }

    bool DisplayManager::setSceneState(ramses::sceneId_t sceneId, SceneState state, const char* confirmationText)
    {
        ramses_internal::PlatformGuard guard(m_lock);

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager::setSceneState: " << SceneStateName(state));

        if (state == SceneState::Ready || state == SceneState::Rendered)
        {
            if (m_scenesInfo[sceneId].mappingInfo.display == ramses::InvalidDisplayId)
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::setSceneState: cannot get scene to ready/rendered without mapping info, set mapping info via DisplayManager::setSceneMapping first!");
                return false;
            }
        }

        assert(confirmationText != nullptr);
        m_scenesInfo[sceneId].targetStateConfirmationText = confirmationText;
        m_scenesInfo[sceneId].targetState = GetInternalSceneState(state);

        goToTargetState(sceneId);

        return true;
    }

    bool DisplayManager::setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder)
    {
        ramses_internal::PlatformGuard guard(m_lock);

        const auto currState = getCurrentSceneState(sceneId);
        const auto targetState = getTargetSceneState(sceneId);
        if (currState == ESceneStateInternal::MappedAndAssigned || targetState == ESceneStateInternal::Rendered
            || targetState == ESceneStateInternal::MappedAndAssigned || targetState == ESceneStateInternal::Rendered)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER,
                "DisplayManager::setSceneMapping: cannot change mapping properties, scene's current or desired state already set to READY/RENDERED for scene " << sceneId <<
                ". Set scene state to AVAILABLE first, adjust mapping properties and then it can be made READY/RENDERED with new mapping properties.");
            return false;
        }

        auto& mappingInfo = m_scenesInfo[sceneId].mappingInfo;
        mappingInfo.display = displayId;
        mappingInfo.renderOrder = sceneRenderOrder;

        return true;
    }

    ramses::displayId_t DisplayManager::createDisplay(const ramses::DisplayConfig& config)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return m_ramsesRenderer.createDisplay(config);
    }

    void DisplayManager::destroyDisplay(ramses::displayId_t displayId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        m_ramsesRenderer.destroyDisplay(displayId);
    }

    void DisplayManager::processConfirmationEchoCommand(const char* text)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        m_ramsesRenderer.impl.logConfirmationEcho(text);
    }

    void DisplayManager::linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        m_ramsesRenderer.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    }

    void DisplayManager::dispatchAndFlush(IEventHandler* eventHandler, ramses::IRendererEventHandler* customRendererEventHandler)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        if (customRendererEventHandler)
        {
            RendererEventChainer chainer{ *customRendererEventHandler, *this };
            m_ramsesRenderer.dispatchEvents(chainer);
        }
        else
            m_ramsesRenderer.dispatchEvents(*this);

        if (eventHandler)
        {
            for (const auto& evt : m_pendingEvents)
                eventHandler->sceneStateChanged(evt.sceneId, evt.state, evt.displaySceneIsMappedTo);
            m_pendingEvents.clear();
        }
        else if (m_pendingEvents.size() > 10000)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager collected " << m_pendingEvents.size() << " events that were not dispatched, purging most of them. This is OK if not using event handler at all.");
            m_pendingEvents.erase(m_pendingEvents.begin(), m_pendingEvents.end() - 100); // remove all but last 100 events
        }

        m_ramsesRenderer.flush();
    }

    bool DisplayManager::isRunning() const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return m_isRunning && !m_exitCommand->exitRequested();
    }

    bool DisplayManager::isDisplayCreated(ramses::displayId_t display) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return m_createdDisplays.hasElement(display);
    }

    SceneState DisplayManager::getLastReportedSceneState(ramses::sceneId_t sceneId) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return GetSceneStateFromInternal(getCurrentSceneState(sceneId));
    }

    ramses::displayId_t DisplayManager::getDisplaySceneIsMappedTo(ramses::sceneId_t sceneId) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        const auto sceneState = getCurrentSceneState(sceneId);

        // last reported display scene is mapped to can only be valid if scene is mapped/rendered
        if (sceneState != ESceneStateInternal::MappedAndAssigned && sceneState != ESceneStateInternal::Rendered)
            return ramses::InvalidDisplayId;

        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.mappingInfo.display : ramses::InvalidDisplayId;
    }

    DisplayManager::ESceneStateInternal DisplayManager::getCurrentSceneState(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.currentState : ESceneStateInternal::Unpublished;
    }

    DisplayManager::ESceneStateInternal DisplayManager::getTargetSceneState(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.targetState : ESceneStateInternal::Unpublished;
    }

    DisplayManager::ESceneStateCommand DisplayManager::getLastSceneStateCommandWaitingForReply(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.lastCommandWaitigForReply : ESceneStateCommand::None;
    }

    void DisplayManager::goToTargetState(ramses::sceneId_t sceneId)
    {
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        const ESceneStateInternal currentSceneState = sceneInfo.currentState;
        const ESceneStateInternal targetSceneState = sceneInfo.targetState;

        if (currentSceneState == targetSceneState)
        {
            // consume confirmation message if reached target state
            if (!sceneInfo.targetStateConfirmationText.empty())
            {
                processConfirmationEchoCommand(sceneInfo.targetStateConfirmationText.c_str());
                sceneInfo.targetStateConfirmationText.clear();
            }
            return;
        }

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
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating subscribe to scene with id :" << sceneId);
                if (m_ramsesRenderer.subscribeScene(sceneId) == ramses::StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Subscribe;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::Subscribed:

            switch (targetSceneState)
            {
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
            {
                assert(m_scenesInfo.count(sceneId) > 0);
                MappingInfo& mapInfo = sceneInfo.mappingInfo;
                // only map, if display is created
                if (isDisplayCreated(mapInfo.display))
                {
                    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating map of scene with id :" << sceneId);
                    if (m_ramsesRenderer.mapScene(mapInfo.display, sceneId, mapInfo.renderOrder) == ramses::StatusOK)
                        sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Map;
                }
                else
                    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager::goToTargetState: target display " << mapInfo.display << " does not exist yet to map scene " << sceneId);
                break;
            }
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating unsubscribe of scene with id :" << sceneId);
                if (m_ramsesRenderer.unsubscribeScene(sceneId) == ramses::StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unsubscribe;
                break;
            default:
                break;
            }
            break;

        case ESceneStateInternal::MappedAndAssigned:

            switch (targetSceneState)
            {
            case ESceneStateInternal::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id :" << sceneId);
                if (m_ramsesRenderer.showScene(sceneId) == ramses::StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Show;
                break;
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id :" << sceneId);
                if (m_ramsesRenderer.unmapScene(sceneId) == ramses::StatusOK)
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating hide of scene with id :" << sceneId);
                if (m_ramsesRenderer.hideScene(sceneId) == ramses::StatusOK)
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

    void DisplayManager::setCurrentSceneState(ramses::sceneId_t sceneId, ESceneStateInternal state)
    {
        auto& sceneInfo = m_scenesInfo[sceneId];
        const SceneState currState = GetSceneStateFromInternal(sceneInfo.currentState);
        const SceneState newState = GetSceneStateFromInternal(state);
        sceneInfo.currentState = state;

        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager received scene state change: scene " << sceneId << " is in state " << SceneStateName(newState) << " (internal " << int(state) << ")");
        if (currState != newState)
        {
            const ramses::displayId_t displayMappedTo = getDisplaySceneIsMappedTo(sceneId);
            LOG_INFO_F(ramses_internal::CONTEXT_RENDERER, ([&](ramses_internal::StringOutputStream& sos)
            {
                sos << "DisplayManager generated event: scene state change";
                sos << " (scene " << sceneId;
                sos << ", " << SceneStateName(newState);
                if (displayMappedTo != ramses::InvalidDisplayId)
                    sos << ", mapped to display " << displayMappedTo;
                sos << ")";
            }));
            m_pendingEvents.push_back({ sceneId, newState, displayMappedTo });
        }
    }

    /* IRendererEventHandler handlers */
    void DisplayManager::scenePublished(ramses::sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) == 0 || getCurrentSceneState(sceneId) == ESceneStateInternal::Unpublished);

        auto& sceneInfo = m_scenesInfo[sceneId];
        if (m_autoShow)
        {
            // in autoshow set target state and mapping if not set already
            if (sceneInfo.targetState == ESceneStateInternal::Unpublished)
                sceneInfo.targetState = ESceneStateInternal::Rendered;
            if (sceneInfo.mappingInfo.display == ramses::InvalidDisplayId)
                sceneInfo.mappingInfo.display = 0u; // assuming display 0 to use for auto show
        }

        setCurrentSceneState(sceneId, ESceneStateInternal::Published);

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneUnpublished(ramses::sceneId_t sceneId)
    {
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Published);
        setCurrentSceneState(sceneId, ESceneStateInternal::Unpublished);
    }

    void DisplayManager::sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Subscribe)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received subscription event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneSubscribed: Could not subscribe scene with id :" << sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ramses::ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Unsubscribe)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received unsubscription event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::Published);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnsubscribed: Could not unsubscribe scene with id :" << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Map)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received map event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneMapped: Could not map scene with id :" << sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ramses::ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Unmap)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received unmap event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnmapped: Could not unmap scene with id :" << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Show)
        {
            LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received show event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
            return;
        }
        m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::Rendered);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneShown: Could not show scene with id :" << sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        //another request, other than display manager, or broken state
        if (result != ramses::ERendererEventResult_INDIRECT)
        {
            if (getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::Hide)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - received hide event but did not ask for one, ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_scenesInfo[sceneId].lastCommandWaitigForReply = ESceneStateCommand::None;
        }

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneHidden: Could not hide scene with id :" << sceneId);
            goToTargetState(sceneId);
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (result == ramses::ERendererEventResult_OK)
        {
            m_createdDisplays.put(displayId);
            for (const auto& it : m_scenesInfo)
            {
                if (it.second.mappingInfo.display == displayId)
                    goToTargetState(it.first);
            }
        }
    }

    void DisplayManager::displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (result == ramses::ERendererEventResult_OK)
            m_createdDisplays.remove(displayId);
    }

    void DisplayManager::keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        UNUSED(displayId);
        if (keyEvent != ramses::EKeyEvent_Pressed)
            return;

        ramses_internal::RendererCommandBuffer& commandBuffer = m_ramsesRenderer.impl.getRenderer().getRendererCommandBuffer();
        if (keyCode == ramses::EKeyCode_Escape && keyModifiers == ramses::EKeyModifier_NoModifier)
        {
            m_isRunning = false;
            return;
        }

        // flymode: steer camera with keyboard from rendering window
        if (keyCode == ramses::EKeyCode_0)
        {
            commandBuffer.resetView();
            commandBuffer.setViewPosition(ramses_internal::Vector3());
            commandBuffer.setViewRotation(ramses_internal::Vector3());
            return;
        }
        else
        {
            const bool uppercaseChar = (keyModifiers & ramses::EKeyModifier_Shift) != 0;

            ramses_internal::Float defaultRotateStepSize = 3.0f;
            ramses_internal::Float defaultTranslateStepSize = 0.1f;

            const bool isRotation = (keyCode >= ramses::EKeyCode_X && keyCode <= ramses::EKeyCode_Z);
            ramses_internal::Float step = isRotation ? defaultRotateStepSize : defaultTranslateStepSize;

            ramses_internal::Vector3 movement(0.0f);
            switch (keyCode)
            {
                // translate
            case ramses::EKeyCode_W:
                movement.z = -step;
                break;
            case ramses::EKeyCode_S:
                movement.z = step;
                break;
            case ramses::EKeyCode_A:
                movement.x = -step;
                break;
            case ramses::EKeyCode_D:
                movement.x = step;
                break;
            case ramses::EKeyCode_Q:
                movement.y = -step;
                break;
            case ramses::EKeyCode_E:
                movement.y = step;
                break;
                // rotate
            case ramses::EKeyCode_X:
                movement.x = step;
                break;
            case ramses::EKeyCode_Y:
                movement.y = step;
                break;
            case ramses::EKeyCode_Z:
                movement.z = step;
                break;
            default:
                break;
            }

            if (isRotation)
            {
                commandBuffer.rotateView(uppercaseChar ? -movement : movement);
            }
            else
            {
                commandBuffer.moveView(uppercaseChar ? 100.0f*movement : movement);
            }
        }
    };

    void DisplayManager::windowClosed(ramses::displayId_t displayId)
    {
        UNUSED(displayId);
        m_isRunning = false;
    }

    SceneState DisplayManager::GetSceneStateFromInternal(ESceneStateInternal internalState)
    {
        switch (internalState)
        {
        case ESceneStateInternal::Unpublished:
        case ESceneStateInternal::Published:
            return SceneState::Unavailable;
        case ESceneStateInternal::Subscribed:
            return SceneState::Available;
        case ESceneStateInternal::MappedAndAssigned:
            return SceneState::Ready;
        case ESceneStateInternal::Rendered:
            return SceneState::Rendered;
        default:
            assert(false);
            return SceneState::Unavailable;
        }
    }

    DisplayManager::ESceneStateInternal DisplayManager::GetInternalSceneState(SceneState state)
    {
        switch (state)
        {
        case SceneState::Unavailable:
            return ESceneStateInternal::Published;
        case SceneState::Available:
            return ESceneStateInternal::Subscribed;
        case SceneState::Ready:
            return ESceneStateInternal::MappedAndAssigned;
        case SceneState::Rendered:
            return ESceneStateInternal::Rendered;
        default:
            assert(false);
            return ESceneStateInternal::Unpublished;
        }
    }
}
