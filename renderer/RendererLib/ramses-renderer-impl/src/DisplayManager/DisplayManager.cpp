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

        MappingInfo mappingInfo;
        mappingInfo.display = displayId;
        mappingInfo.renderOrder = sceneRenderOrder;

        return setMappingInternal(sceneId, mappingInfo);
    }

    bool DisplayManager::setSceneOffscreenBufferMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, uint32_t width, uint32_t height, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTextureSamplerId)
    {
        ramses_internal::PlatformGuard guard(m_lock);
        if (width == 0 || height == 0)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::setSceneOffscreenBufferMapping: invalid offscreen buffer dimensions " << width << "x" << height);
            return false;
        }

        MappingInfo mappingInfo;
        mappingInfo.display = displayId;
        mappingInfo.obWidth = width;
        mappingInfo.obHeight = height;
        mappingInfo.consumerScene = consumerScene;
        mappingInfo.consumerSamplerId = consumerTextureSamplerId;

        return setMappingInternal(sceneId, mappingInfo);
    }

    bool DisplayManager::setMappingInternal(ramses::sceneId_t sceneId, const MappingInfo& mappingInfo)
    {
        const auto currState = getCurrentSceneState(sceneId);
        const auto targetState = getTargetSceneState(sceneId);
        if (currState == ESceneStateInternal::Mapped || currState == ESceneStateInternal::MappedAndAssigned || targetState == ESceneStateInternal::Rendered
            || targetState == ESceneStateInternal::MappedAndAssigned || targetState == ESceneStateInternal::Rendered)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER,
                "DisplayManager::setSceneMapping: cannot change mapping properties, scene's current or desired state already set to READY/RENDERED for scene " << sceneId <<
                ". Set scene state to AVAILABLE first, adjust mapping properties and then it can be made READY/RENDERED with new mapping properties.");
            return false;
        }

        m_scenesInfo[sceneId].mappingInfo = mappingInfo;

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
        return m_createdDisplays.count(display) != 0;
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
            case ESceneStateInternal::Mapped:
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating subscribe to scene with id: " << sceneId);
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
            case ESceneStateInternal::Mapped:
            case ESceneStateInternal::MappedAndAssigned:
            case ESceneStateInternal::Rendered:
            {
                assert(m_scenesInfo.count(sceneId) > 0);
                MappingInfo& mapInfo = sceneInfo.mappingInfo;
                // only map, if display is created
                if (isDisplayCreated(mapInfo.display))
                {
                    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating map of scene with id: " << sceneId);
                    if (m_ramsesRenderer.mapScene(mapInfo.display, sceneId, mapInfo.renderOrder) == ramses::StatusOK)
                        sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Map;
                }
                else
                    LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager::goToTargetState: target display " << mapInfo.display << " does not exist yet to map scene " << sceneId);
                break;
            }
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating unsubscribe of scene with id: " << sceneId);
                if (m_ramsesRenderer.unsubscribeScene(sceneId) == ramses::StatusOK)
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id: " << sceneId);
                if (m_ramsesRenderer.unmapScene(sceneId) == ramses::StatusOK)
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id: " << sceneId);
                if (m_ramsesRenderer.showScene(sceneId) == ramses::StatusOK)
                    sceneInfo.lastCommandWaitigForReply = ESceneStateCommand::Show;
                break;
            case ESceneStateInternal::Subscribed:
            case ESceneStateInternal::Published:
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating show of scene with id: " << sceneId);
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
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating hide of scene with id: " << sceneId);
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

    void DisplayManager::goToMappedAndAssignedState(ramses::sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) > 0);
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Mapped);
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        MappingInfo& mapInfo = sceneInfo.mappingInfo;

        // advance OB mapping state if OB mapping needed
        const bool mapsToOB = sceneInfo.mappingInfo.obWidth > 0;
        if (mapsToOB)
            goToOBLinkedState(sceneId);

        // state can advance if either no OB mapping needed or it is fully OB mapped/assigned/linked
        const bool sceneAssignedAndLinked = (!mapsToOB || (mapInfo.obMappingState == MappingInfo::OBMappingState::Linked));
        if (sceneAssignedAndLinked)
        {
            setCurrentSceneState(sceneId, ESceneStateInternal::MappedAndAssigned);

            goToTargetState(sceneId);

            // additionally trigger scenes consumed by this scene to continue with mapping
            for (const auto& it : m_scenesInfo)
            {
                if (it.second.mappingInfo.consumerScene == sceneId)
                    goToTargetState(it.first);
            }
        }
    }

    void DisplayManager::goToOBLinkedState(ramses::sceneId_t sceneId)
    {
        assert(m_scenesInfo.count(sceneId) > 0);
        assert(getCurrentSceneState(sceneId) == ESceneStateInternal::Mapped);
        SceneInfo& sceneInfo = m_scenesInfo[sceneId];
        MappingInfo& mapInfo = sceneInfo.mappingInfo;

        switch (mapInfo.obMappingState)
        {
        case MappingInfo::OBMappingState::None:
            // Create OB
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating creation of offscreen buffer " << mapInfo.obWidth << "x" << mapInfo.obHeight << " to map scene " << sceneId);
            mapInfo.offscreenBuffer = m_ramsesRenderer.createOffscreenBuffer(mapInfo.display, mapInfo.obWidth, mapInfo.obHeight);
            if (mapInfo.offscreenBuffer != ramses::InvalidOffscreenBufferId)
                mapInfo.obMappingState = MappingInfo::OBMappingState::OBCreationRequested;
            break;
        case MappingInfo::OBMappingState::OBCreationRequested:
            break;
        case MappingInfo::OBMappingState::OBCreated:
            LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating assignment of scene " << sceneId << " to offscreen buffer " << mapInfo.offscreenBuffer);
            if (m_ramsesRenderer.assignSceneToOffscreenBuffer(sceneId, mapInfo.offscreenBuffer) == ramses::StatusOK)
                mapInfo.obMappingState = MappingInfo::OBMappingState::AssignmentRequested;
            break;
        case MappingInfo::OBMappingState::AssignmentRequested:
            break;
        case MappingInfo::OBMappingState::Assigned:
            // Check if consumer mapped
            if (getDisplaySceneIsMappedTo(mapInfo.consumerScene) != mapInfo.display)
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager cannot advance with linking scene " << sceneId << " via offscreen buffer " << mapInfo.offscreenBuffer
                    << " to consumer, because consumer scene " << mapInfo.consumerScene << " is not mapped yet or mapped to another display");
            }
            else
            {
                LOG_INFO(ramses_internal::CONTEXT_RENDERER, "DisplayManager initiating linking of scene " << sceneId << " via offscreen buffer " << mapInfo.offscreenBuffer
                    << " to consumer scene " << mapInfo.consumerScene << " sampler consumer " << mapInfo.consumerSamplerId);
                if (m_ramsesRenderer.linkOffscreenBufferToSceneData(mapInfo.offscreenBuffer, mapInfo.consumerScene, mapInfo.consumerSamplerId) == ramses::StatusOK)
                    mapInfo.obMappingState = MappingInfo::OBMappingState::LinkRequested;
            }
            break;
        case MappingInfo::OBMappingState::LinkRequested:
            break;
        case MappingInfo::OBMappingState::Linked:
            // Done
            break;
        default:
            assert(false);
            break;
        }
    }

    void DisplayManager::resetMappingState(ramses::sceneId_t sceneId)
    {
        auto& mapInfo = m_scenesInfo[sceneId].mappingInfo;

        // destroy no longer used OB
        if (mapInfo.offscreenBuffer != ramses::InvalidOffscreenBufferId)
            m_ramsesRenderer.destroyOffscreenBuffer(mapInfo.display, mapInfo.offscreenBuffer);

        mapInfo.obMappingState = MappingInfo::OBMappingState::None;
        mapInfo.offscreenBuffer = ramses::InvalidOffscreenBufferId;
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
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneSubscribed: Could not subscribe scene with id: " << sceneId);
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
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnsubscribed: Could not unsubscribe scene with id: " << sceneId);
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
            setCurrentSceneState(sceneId, ESceneStateInternal::Mapped);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneMapped: Could not map scene with id: " << sceneId);
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
            resetMappingState(sceneId);
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            goToTargetState(sceneId);
            break;
        case ramses::ERendererEventResult_INDIRECT:
            resetMappingState(sceneId);
            setCurrentSceneState(sceneId, ESceneStateInternal::Subscribed);
            break;
        case ramses::ERendererEventResult_FAIL:
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneUnmapped: Could not unmap scene with id: " << sceneId);
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
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneShown: Could not show scene with id: " << sceneId);
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
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneHidden: Could not hide scene with id: " << sceneId);
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
            m_createdDisplays.insert({ displayId, {} });
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
            m_createdDisplays.erase(displayId);
    }

    void DisplayManager::offscreenBufferCreated(ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        if (result == ramses::ERendererEventResult_OK)
            m_createdDisplays[displayId].offscreenBuffers.insert(offscreenBufferId);

        for (auto& it : m_scenesInfo)
        {
            auto& mapInfo = m_scenesInfo[it.first].mappingInfo;
            if (mapInfo.offscreenBuffer == offscreenBufferId)
            {
                if (mapInfo.obMappingState != MappingInfo::OBMappingState::OBCreationRequested)
                {
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::offscreenBufferCreated: scene " << it.first << " mapping to OB " << offscreenBufferId
                        << " did not request OB creation, but OB creation event received, ignoring event. This is OK if scene was unexpectedly unpublished.");
                    continue;
                }

                if (result == ramses::ERendererEventResult_OK)
                    mapInfo.obMappingState = MappingInfo::OBMappingState::OBCreated;
                else
                {
                    // reset any mapping that points to the OB that failed to create
                    mapInfo.offscreenBuffer = ramses::InvalidOffscreenBufferId;
                    mapInfo.obMappingState = MappingInfo::OBMappingState::None; // re-trigger on fail
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::offscreenBufferCreated: OB " << offscreenBufferId
                        << " to be used by scene " << it.first << " failed to create, will retry creation in next step");
                }

                goToTargetState(it.first);
            }
        }
    }

    void DisplayManager::offscreenBufferDestroyed(ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        if (result == ramses::ERendererEventResult_OK)
            m_createdDisplays[displayId].offscreenBuffers.erase(offscreenBufferId);
    }

    void DisplayManager::sceneAssignedToOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result)
    {
        auto& mapInfo = m_scenesInfo[sceneId].mappingInfo;
        if (mapInfo.obMappingState != MappingInfo::OBMappingState::AssignmentRequested)
        {
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneAssignedToOffscreenBuffer: scene " << sceneId << " was not requested to be assigned to OB " << offscreenBufferId
                << ", ignoring event. This is OK if scene was unexpectedly unpublished.");
            return;
        }

        if (result == ramses::ERendererEventResult_OK)
            mapInfo.obMappingState = MappingInfo::OBMappingState::Assigned;
        else
        {
            mapInfo.obMappingState = MappingInfo::OBMappingState::OBCreated; // re-trigger on fail
            LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::sceneAssignedToOffscreenBuffer: scene " << sceneId << " failed to be assigned to OB " << offscreenBufferId
                << ", will retry again.");
        }

        goToTargetState(sceneId);
    }

    void DisplayManager::offscreenBufferLinkedToSceneData(ramses::offscreenBufferId_t providerOffscreenBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, ramses::ERendererEventResult result)
    {
        for (auto& it : m_scenesInfo)
        {
            auto& mapInfo = it.second.mappingInfo;
            if (mapInfo.offscreenBuffer == providerOffscreenBuffer)
            {
                if (mapInfo.obMappingState != MappingInfo::OBMappingState::LinkRequested)
                {
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::offscreenBufferLinkedToSceneData: scene " << it.first << " mapping to OB " << providerOffscreenBuffer
                        << " did not request link to consumer scene " << mapInfo.consumerScene << " sampler consumer " << mapInfo.consumerSamplerId
                        << ", but link event received, ignoring event. This is OK if scene was unexpectedly unpublished.");
                    continue;
                }

                if (mapInfo.consumerScene != consumerScene || mapInfo.consumerSamplerId != consumerId)
                {
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::offscreenBufferLinkedToSceneData: scene " << it.first << " mapping to OB " << providerOffscreenBuffer
                        << " linking to consumer scene " << mapInfo.consumerScene << " sampler consumer " << mapInfo.consumerSamplerId
                        << " do not match event's consumer scene " << consumerScene << " and/or consumer " << consumerId << ", ignoring event.");
                    continue;
                }

                if (result == ramses::ERendererEventResult_OK)
                    mapInfo.obMappingState = MappingInfo::OBMappingState::Linked;
                else
                {
                    mapInfo.obMappingState = MappingInfo::OBMappingState::Assigned; // re-trigger on fail
                    LOG_ERROR(ramses_internal::CONTEXT_RENDERER, "DisplayManager::offscreenBufferLinkedToSceneData: OB " << mapInfo.offscreenBuffer << " used by scene " << it.first
                        << " failed to be linked to consumer scene " << mapInfo.consumerScene << " sampler consumer " << mapInfo.consumerSamplerId);
                }

                goToTargetState(it.first);
            }
        }
    }

    void DisplayManager::keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode)
    {
        if (!m_keysHandling)
            return;

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
        case ESceneStateInternal::Mapped:
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

    void DisplayManager::enableKeysHandling()
    {
        m_keysHandling = true;
    }
}
