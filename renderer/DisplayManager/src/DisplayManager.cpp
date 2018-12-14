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
        m_ramsesRenderer.flush();

        return displayId;
    }

    void DisplayManager::destroyDisplay(ramses::displayId_t displayId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        m_ramsesRenderer.destroyDisplay(displayId);
        m_ramsesRenderer.flush();
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

    void DisplayManager::dispatchAndFlush()
    {
        ramses_internal::PlatformGuard guard(m_lock);
        m_ramsesRenderer.dispatchEvents(*this);
        m_ramsesRenderer.flush();
    }

    bool DisplayManager::isRunning() const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return m_isRunning && !m_exitCommand->exitRequested();
    }

    bool DisplayManager::isSceneShown(ramses::sceneId_t sceneId) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        if (m_currentSceneStates.contains(sceneId))
        {
            return m_currentSceneStates.at(sceneId) == ESceneState_Rendered;
        }
        return false;
    }

    bool DisplayManager::isDisplayCreated(ramses::displayId_t display) const
    {
        ramses_internal::PlatformGuard guard(m_lock);
        return m_createdDisplays.hasElement(display);
    }

    DisplayManager::ESceneState DisplayManager::getCurrentSceneState(ramses::sceneId_t sceneId)
    {
        if (m_currentSceneStates.contains(sceneId))
        {
            return m_currentSceneStates[sceneId];
        }
        return ESceneState_Unpublished;
    }

    DisplayManager::ESceneState DisplayManager::getTargetSceneState(ramses::sceneId_t sceneId)
    {
        if (m_targetSceneStates.contains(sceneId))
        {
            return m_targetSceneStates[sceneId];
        }
        return ESceneState_Unpublished;
    }

    bool DisplayManager::isInTargetState(ramses::sceneId_t sceneId)
    {
        return getCurrentSceneState(sceneId) == getTargetSceneState(sceneId);
    }

    void DisplayManager::goToTargetState(ramses::sceneId_t sceneId)
    {
        if (isInTargetState(sceneId))
        {
            return;
        }

        ESceneState currentSceneState = getCurrentSceneState(sceneId);
        ESceneState targetSceneState = getTargetSceneState(sceneId);

        switch (currentSceneState)
        {
        case ESceneState_Unpublished:
            //cannot do anything here. Event handler scenePublished will trigger this again
            break;

        case ESceneState_Published:
            switch (targetSceneState)
            {
            case ESceneState_Subscribed:
            case ESceneState_Mapped:
            case ESceneState_Rendered:
                // Subscribe to scene
                if (m_ramsesRenderer.subscribeScene(sceneId) == ramses::StatusOK)
                {
                    m_currentSceneStates.put(sceneId, ESceneState_GoingToSubscribed);
                }
                break;

            default:
                //no other target state when in published!
                assert(false);
            }
            break;

        case ESceneState_Subscribed:

            switch (targetSceneState)
            {

            case ESceneState_Mapped:
            case ESceneState_Rendered:
            {
                assert(m_scenesMappingInfo.contains(sceneId));
                const MappingInfo& mapInfo = m_scenesMappingInfo[sceneId];
                // only subscribe, if display is created
                if (isDisplayCreated(mapInfo.display))
                {
                    if (m_ramsesRenderer.mapScene(mapInfo.display, sceneId, mapInfo.renderOrder) == ramses::StatusOK)
                    {
                        m_currentSceneStates.put(sceneId, ESceneState_GoingToMapped);
                    }
                }
                break;

            }
            case ESceneState_Published:
                if (m_ramsesRenderer.unsubscribeScene(sceneId) == ramses::StatusOK)
                {
                    m_currentSceneStates.put(sceneId, ESceneState_GoingToPublished);
                }
                break;

            default:
                assert(false);

            }
            break;

        case ESceneState_Mapped:

            switch (targetSceneState)
            {

            case ESceneState_Rendered:
                if (m_ramsesRenderer.showScene(sceneId) == ramses::StatusOK)
                {
                    m_currentSceneStates.put(sceneId, ESceneState_GoingToRendered);
                }
                break;

            case ESceneState_Subscribed:
            case ESceneState_Published:
                if (m_ramsesRenderer.unmapScene(sceneId) == ramses::StatusOK)
                {
                    m_currentSceneStates.put(sceneId, ESceneState_GoingToSubscribed);
                }
                break;

            default:
                assert(false);
            }
            break;

        case ESceneState_Rendered:

            switch (targetSceneState)
            {

            case ESceneState_Mapped:
            case ESceneState_Subscribed:
            case ESceneState_Published:
                if (m_ramsesRenderer.hideScene(sceneId) == ramses::StatusOK)
                {
                    m_currentSceneStates.put(sceneId, ESceneState_GoingToMapped);
                }
                break;

            default:
                assert(false);
            }
            break;

        case ESceneState_GoingToPublished:
        case ESceneState_GoingToSubscribed:
        case ESceneState_GoingToMapped:
        case ESceneState_GoingToRendered:
            //Cannot do anything, waiting for renderer event handlers to continue
            break;

        default:
            assert(false);
        }
    }

    void DisplayManager::handleShowCommand(ramses::sceneId_t sceneId, MappingInfo mappingInfo)
    {
        ESceneState currentSceneState = getCurrentSceneState(sceneId);

        //check whether scene was already mapped
        switch (currentSceneState)
        {
        //check if scene is unpublished
        case ESceneState_Unpublished:
            m_currentSceneStates.put(sceneId, ESceneState_GoingToPublished);
            break;

        //check whether scene was already mapped
        case ESceneState_Mapped:
        case ESceneState_Rendered:
        case ESceneState_GoingToMapped:
        case ESceneState_GoingToRendered:
            assert(m_scenesMappingInfo.contains(sceneId));
            if (!(m_scenesMappingInfo[sceneId] == mappingInfo))
            {
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleShowCommand: cannot execute show command for scene with id :" << sceneId << " because it was mapped with different parameters before!");
                return;
            }
            // trigger confirmation immediatly when alreay shown (avoids race between command and showing)
            if (currentSceneState == ESceneState_Rendered)
            {
                if (mappingInfo.confirmationText != ramses_internal::String())
                {
                    processConfirmationEchoCommand(mappingInfo.confirmationText.c_str());
                    mappingInfo.confirmationText = "";
                }
            }
            break;

        default:
            break;
        }

        m_scenesMappingInfo.put(sceneId, mappingInfo);
        m_targetSceneStates.put(sceneId, ESceneState_Rendered);

        goToTargetState(sceneId);
    }

    void DisplayManager::handleUnsubscribeCommand(ramses::sceneId_t sceneId)
    {
        switch (getCurrentSceneState(sceneId))
        {
        case ESceneState_Rendered:
        case ESceneState_GoingToRendered:
        case ESceneState_Mapped:
        case ESceneState_GoingToMapped:
        case ESceneState_Subscribed:
        case ESceneState_GoingToSubscribed:
            m_scenesMappingInfo.remove(sceneId);
            m_targetSceneStates.put(sceneId, ESceneState_Published);
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
        case ESceneState_Rendered:
        case ESceneState_GoingToRendered:
        case ESceneState_Mapped:
        case ESceneState_GoingToMapped:
            m_scenesMappingInfo.remove(sceneId);
            m_targetSceneStates.put(sceneId, ESceneState_Subscribed);
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
        case ESceneState_Rendered:
        case ESceneState_GoingToRendered:
            m_targetSceneStates.put(sceneId, ESceneState_Mapped);
            goToTargetState(sceneId);
            break;
        default:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::handleHideCommand: cannot execute hide command for scene with id :" << sceneId << " because it was not rendered/shown before!");
        }
    }

    /* IRendererEventHandler handlers */
    void DisplayManager::scenePublished(ramses::sceneId_t sceneId)
    {
        assert(!m_currentSceneStates.contains(sceneId) || m_currentSceneStates[sceneId] == ESceneState_GoingToPublished);

        //update current scene state
        m_currentSceneStates.put(sceneId, ESceneState_Published);

        //update current target scene state
        if (getTargetSceneState(sceneId) == ESceneState_Unpublished)
        {
            if (m_autoShow)
            {
                m_targetSceneStates.put(sceneId, ESceneState_Rendered);
                MappingInfo mapInfo = { 0u, 0, "" };
                m_scenesMappingInfo.put(sceneId, mapInfo);
            }
            else
            {
                m_targetSceneStates.put(sceneId, ESceneState_Published);
            }
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::sceneUnpublished(ramses::sceneId_t sceneId)
    {
        assert(getCurrentSceneState(sceneId) == ESceneState_Published
               || getCurrentSceneState(sceneId) == ESceneState_GoingToSubscribed);

        m_currentSceneStates.remove(sceneId);

        //only remove mapping request if in auto-mode (scene gets mapped again anyway), otherwise keep as scene might be available again after reconnect
        if (getTargetSceneState(sceneId)!= ESceneState_Rendered || m_autoShow)
        {
            m_targetSceneStates.remove(sceneId);
            m_scenesMappingInfo.remove(sceneId);
        }
    }

    void DisplayManager::sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            //another request, other then display manager
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Subscribed);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_FAIL:
            if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to subscribed
                if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
                {
                   LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                   return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Published);
            }
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneSubscribed: Could not subscribe scene with id :" << sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToPublished)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId);
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Published);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_currentSceneStates.put(sceneId, ESceneState_Published);
            break;
        case ramses::ERendererEventResult_FAIL:
            if(ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to published
                if (getCurrentSceneState(sceneId) != ESceneState_GoingToPublished)
                {
                    LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                    return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Subscribed);
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnsubscribed: Could not unsubscribe scene with id :" << sceneId);
            }
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            //another request, other then display manager
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Mapped);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_FAIL:
            if(ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to mapped
                if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
                {
                    LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                    return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Subscribed);
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneMapped: Could not map scene with id :" << sceneId);
            }
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            //another request, other then display manager
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Subscribed);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_currentSceneStates.put(sceneId, ESceneState_Subscribed);
            break;
        case ramses::ERendererEventResult_FAIL:
            if(ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to subscribed
                if (getCurrentSceneState(sceneId) != ESceneState_GoingToSubscribed)
                {
                    LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                    return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Mapped);
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnmapped: Could not unmap scene with id :" << sceneId);
            }
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {

        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            //another request, other then display manager
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToRendered)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Rendered);
            if (m_scenesMappingInfo[sceneId].confirmationText != ramses_internal::String(""))
            {
                processConfirmationEchoCommand(m_scenesMappingInfo[sceneId].confirmationText.c_str());
                m_scenesMappingInfo[sceneId].confirmationText = "";
            }
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_FAIL:
            if (ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to rendered
                if (getCurrentSceneState(sceneId) != ESceneState_GoingToRendered)
                {
                    LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                    return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Mapped);
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneShown: Could not map scene with id :" << sceneId);
            }
            break;
        case ramses::ERendererEventResult_INDIRECT:
        default:
            assert(false);
        }
    }

    void DisplayManager::sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result)
    {
        switch (result)
        {
        case ramses::ERendererEventResult_OK:
            if (getCurrentSceneState(sceneId) != ESceneState_GoingToMapped)
            {
                LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                return;
            }
            m_currentSceneStates.put(sceneId, ESceneState_Mapped);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            m_currentSceneStates.put(sceneId, ESceneState_Mapped);
            break;
        case ramses::ERendererEventResult_FAIL:
            if(ESceneState_Unpublished != getCurrentSceneState(sceneId))
            {
                //if the scene was not unpublished while it was still going to mapped
                if (getCurrentSceneState(sceneId) == ESceneState_GoingToMapped)
                {
                    LOG_WARN(ramses_internal::CONTEXT_RENDERER, "DisplayManager - ignoring unexpected state change event for scene " << sceneId );
                    return;
                }
                m_currentSceneStates.put(sceneId, ESceneState_Rendered);
                LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneHidden: Could not hide scene with id :" << sceneId);
            }
            break;
        default:
            assert(false);
        }
    }

    void DisplayManager::displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_createdDisplays.put(displayId);
            ramses_capu::HashTable<ramses::sceneId_t, MappingInfo>::Iterator mapIt = m_scenesMappingInfo.begin();
            while (mapIt != m_scenesMappingInfo.end())
            {
                if (displayId == mapIt->value.display)
                {
                    goToTargetState(mapIt->key);
                }
                mapIt++;
            }
        }
    }

    void DisplayManager::displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result)
    {
        if (ramses::ERendererEventResult_OK == result)
        {
            m_createdDisplays.remove(displayId);
        }
    }

    void DisplayManager::keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        UNUSED(displayId);
        if (keyEvent != ramses::EKeyEvent_Pressed)
        {
            return;
        }

        ramses_internal::RendererCommandBuffer& commandBuffer = m_ramsesRenderer.impl.getRenderer().getRendererCommandBuffer();
        if (keyCode == ramses::EKeyCode_Escape && keyModifiers == ramses::EKeyModifier_NoModifier)
        {
            m_isRunning = false;
            return;
        }

        // flymode: steer camera with keyboard from rendering window
        if (ramses::EKeyCode_0 == keyCode)
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
