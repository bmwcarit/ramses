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

    void DisplayManager::showSceneOnDisplay(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder, const char* confirmationText)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        const MappingInfo mappingInfo = { displayId, sceneRenderOrder, confirmationText };
        handleShowCommand(sceneId, mappingInfo);
    }

    void DisplayManager::unmapScene(ramses::sceneId_t sceneId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        handleUnmapCommand(sceneId);
    }

    void DisplayManager::unsubscribeScene(ramses::sceneId_t sceneId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        handleUnsubscribeCommand(sceneId);
    }

    void DisplayManager::hideScene(ramses::sceneId_t sceneId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        handleHideCommand(sceneId);
    }

    ramses::displayId_t DisplayManager::createDisplay(const ramses::DisplayConfig& config)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        const ramses::displayId_t displayId = m_ramsesRenderer.createDisplay(config);

        return displayId;
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

    void DisplayManager::dispatchAndFlush(ramses::IRendererEventHandler* customHandler)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        if (customHandler)
        {
            RendererEventChainer chainer{ *customHandler, *this };
            m_ramsesRenderer.dispatchEvents(chainer);
        }
        else
            m_ramsesRenderer.dispatchEvents(*this);

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

    DisplayManager::ESceneState DisplayManager::getSceneState(ramses::sceneId_t sceneId) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return getCurrentSceneState(sceneId);
    }


    ramses::displayId_t DisplayManager::getDisplaySceneIsMappedTo(ramses::sceneId_t sceneId) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.mappingInfo.displayMappedTo : ramses::InvalidDisplayId;
    }

    DisplayManager::ESceneState DisplayManager::getCurrentSceneState(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.currentState : ESceneState::Unpublished;
    }

    DisplayManager::ESceneState DisplayManager::getTargetSceneState(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.targetState : ESceneState::Unpublished;
    }

    DisplayManager::ESceneStateCommand DisplayManager::getLastSceneStateCommandWaitingForReply(ramses::sceneId_t sceneId) const
    {
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.end() ? it->second.lastCommandWaitigForReply : ESceneStateCommand::None;
    }

    bool DisplayManager::isInTargetState(ramses::sceneId_t sceneId) const
    {
        return getCurrentSceneState(sceneId) == getTargetSceneState(sceneId);
    }

    void DisplayManager::goToTargetState(ramses::sceneId_t sceneId)
    {
        if (isInTargetState(sceneId) || getLastSceneStateCommandWaitingForReply(sceneId) != ESceneStateCommand::None)
            return;

        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        const ESceneState currentSceneState = sceneInfo.currentState;
        const ESceneState targetSceneState = sceneInfo.targetState;

        switch (currentSceneState)
        {
        case ESceneState::Unpublished:
            //cannot do anything here. Event handler scenePublished will trigger this again
            break;

        case ESceneState::Published:

            switch (targetSceneState)
            {
            case ESceneState::Subscribed:
            case ESceneState::Mapped:
            case ESceneState::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating subscribe to scene with id :" << sceneId);
                if (m_ramsesRenderer.subscribeScene(sceneId) == ramses::StatusOK)
                {
                    m_scenesInfo[sceneId].currentState = ESceneState::GoingToSubscribed;
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Subscribe;
                }
                break;

            default:
                //no other target state when in published!
                assert(false);
            }
            break;

        case ESceneState::Subscribed:

            switch (targetSceneState)
            {
            case ESceneState::Mapped:
            case ESceneState::Rendered:
            {
                assert(m_scenesInfo.count(sceneId) > 0);
                MappingInfo& mapInfo = sceneInfo.mappingInfo;
                // only subscribe, if display is created
                if (isDisplayCreated(mapInfo.display))
                {
                    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating map of scene with id :" << sceneId);
                    if (m_ramsesRenderer.mapScene(mapInfo.display, sceneId, mapInfo.renderOrder) == ramses::StatusOK)
                    {
                        sceneInfo.currentState = ESceneState::GoingToMapped;
                        sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Map;
                        mapInfo.displayMappedTo = mapInfo.display;
                    }
                }
                break;
            }
            case ESceneState::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating unsubscribe of scene with id :" << sceneId);
                if (m_ramsesRenderer.unsubscribeScene(sceneId) == ramses::StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unsubscribe;
                break;

            default:
                assert(false);
            }
            break;

        case ESceneState::Mapped:

            switch (targetSceneState)
            {
            case ESceneState::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id :" << sceneId);
                if (m_ramsesRenderer.showScene(sceneId) == ramses::StatusOK)
                {
                    sceneInfo.currentState = ESceneState::GoingToRendered;
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Show;
                }
                break;

            case ESceneState::Subscribed:
            case ESceneState::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id :" << sceneId);
                if (m_ramsesRenderer.unmapScene(sceneId) == ramses::StatusOK)
                {
                    sceneInfo.currentState = ESceneState::GoingToSubscribed;
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Unmap;
                }
                break;

            default:
                assert(false);
            }
            break;

        case ESceneState::Rendered:

            switch (targetSceneState)
            {
            case ESceneState::Mapped:
            case ESceneState::Subscribed:
            case ESceneState::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating hide of scene with id :" << sceneId);
                if (m_ramsesRenderer.hideScene(sceneId) == ramses::StatusOK)
                {
                    sceneInfo.currentState = ESceneState::GoingToMapped;
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Hide;
                }
                break;

            default:
                assert(false);
            }
            break;

        case ESceneState::GoingToSubscribed:
        case ESceneState::GoingToMapped:
        case ESceneState::GoingToRendered:
            //Cannot do anything, waiting for renderer event handlers to continue
            break;

        default:
            assert(false);
        }
    }

    void DisplayManager::handleShowCommand(ramses::sceneId_t sceneId, MappingInfo mappingInfo)
    {
        const ESceneState currentSceneState = getCurrentSceneState(sceneId);

        //check whether scene was already mapped
        switch (currentSceneState)
        {
        //check whether scene was already mapped
        case ESceneState::Mapped:
        case ESceneState::Rendered:
        case ESceneState::GoingToMapped:
        case ESceneState::GoingToRendered:
            assert(m_scenesInfo.count(sceneId));
            if (!(m_scenesInfo[sceneId].mappingInfo == mappingInfo))
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleShowCommand: cannot execute show command for scene with id :" << sceneId << " because it was mapped with different parameters before!");
                return;
            }
            // trigger confirmation immediately when already shown (avoids race between command and showing)
            if (currentSceneState == ESceneState::Rendered)
            {
                if (!mappingInfo.confirmationText.empty())
                {
                    processConfirmationEchoCommand(mappingInfo.confirmationText.c_str());
                    mappingInfo.confirmationText.clear();
                }
            }
            break;

        default:
            break;
        }

        m_scenesInfo[sceneId].mappingInfo = mappingInfo;
        m_scenesInfo[sceneId].targetState = ESceneState::Rendered;

        goToTargetState(sceneId);
    }

    void DisplayManager::handleUnsubscribeCommand(ramses::sceneId_t sceneId)
    {
        switch (getCurrentSceneState(sceneId))
        {
        case ESceneState::Rendered:
        case ESceneState::GoingToRendered:
        case ESceneState::Mapped:
        case ESceneState::GoingToMapped:
        case ESceneState::Subscribed:
        case ESceneState::GoingToSubscribed:
            m_scenesInfo[sceneId].targetState = ESceneState::Published;
            goToTargetState(sceneId);
            break;
        default:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleUnsubscribeCommand: cannot execute unsubscribe command for scene with id :" << sceneId << " because it was not subscribed before!");
        }
    }

    void DisplayManager::handleUnmapCommand(ramses::sceneId_t sceneId)
    {
        switch (getCurrentSceneState(sceneId))
        {
        case ESceneState::Rendered:
        case ESceneState::GoingToRendered:
        case ESceneState::Mapped:
        case ESceneState::GoingToMapped:
            m_scenesInfo[sceneId].targetState = ESceneState::Subscribed;
            goToTargetState(sceneId);
            break;
        default:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleUnmapCommand: cannot execute unmap command for scene with id :" << sceneId << " because it is already mapped with different parameters!");
        }
    }

    void DisplayManager::handleHideCommand(ramses::sceneId_t sceneId)
    {
        switch (getCurrentSceneState(sceneId))
        {
        case ESceneState::Rendered:
        case ESceneState::GoingToRendered:
            m_scenesInfo[sceneId].targetState = ESceneState::Mapped;
            goToTargetState(sceneId);
            break;
        default:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleHideCommand: cannot execute hide command for scene with id :" << sceneId << " because it was not rendered/shown before!");
        }
    }

    /* IRendererEventHandler handlers */
    void DisplayManager::scenePublished(ramses::sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) == 0 || m_scenesInfo[sceneId].currentState == ESceneState::Unpublished);

        //update current scene state
        m_scenesInfo[sceneId].currentState = ESceneState::Published;

        //update current target scene state
        if (getTargetSceneState(sceneId) == ESceneState::Unpublished)
        {
            if (m_autoShow)
            {
                m_scenesInfo[sceneId].targetState = ESceneState::Rendered;
                m_scenesInfo[sceneId].mappingInfo = { 0u, 0, "" };
            }
            else
            {
                m_scenesInfo[sceneId].targetState = ESceneState::Published;
            }
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneUnpublished(ramses::sceneId_t sceneId)
    {
        assert(getCurrentSceneState(sceneId) == ESceneState::Published || getCurrentSceneState(sceneId) == ESceneState::GoingToSubscribed);

        m_scenesInfo[sceneId].currentState = ESceneState::Unpublished;

        // If showOnDisplay used for scene or DM in auto mode, it will get shown again automatically when scene re-published.
        // If any other 'explicit' state requested remove its info, if re-published DM will not do anything.
        // Forget target state if in auto-mode (always implicitly 'rendered' in auto-mode) or there was other explicit state.
        if (getTargetSceneState(sceneId) != ESceneState::Rendered || m_autoShow)
            m_scenesInfo[sceneId].targetState = ESceneState::Unpublished;
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
            m_scenesInfo[sceneId].currentState = ESceneState::Subscribed;
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
            m_scenesInfo[sceneId].currentState = ESceneState::Published;
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_scenesInfo[sceneId].currentState = ESceneState::Published;
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnsubscribed: Could not unsubscribe scene with id :" << sceneId);
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
            m_scenesInfo[sceneId].currentState = ESceneState::Mapped;
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneMapped: Could not map scene with id :" << sceneId);
            m_scenesInfo[sceneId].mappingInfo.displayMappedTo = ramses::InvalidDisplayId;
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
            m_scenesInfo[sceneId].currentState = ESceneState::Subscribed;
            m_scenesInfo[sceneId].mappingInfo.displayMappedTo = ramses::InvalidDisplayId;
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_scenesInfo[sceneId].currentState = ESceneState::Subscribed;
            m_scenesInfo[sceneId].mappingInfo.displayMappedTo = ramses::InvalidDisplayId;
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnmapped: Could not unmap scene with id :" << sceneId);
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
            m_scenesInfo[sceneId].currentState = ESceneState::Rendered;
            if (!m_scenesInfo[sceneId].mappingInfo.confirmationText.empty())
            {
                processConfirmationEchoCommand(m_scenesInfo[sceneId].mappingInfo.confirmationText.c_str());
                m_scenesInfo[sceneId].mappingInfo.confirmationText.clear();
            }
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneShown: Could not map scene with id :" << sceneId);
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
            m_scenesInfo[sceneId].currentState = ESceneState::Mapped;
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_scenesInfo[sceneId].currentState = ESceneState::Mapped;
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneHidden: Could not hide scene with id :" << sceneId);
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
}
