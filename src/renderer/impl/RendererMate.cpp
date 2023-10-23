//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RendererMate.h"
#include "RendererEventChainer.h"
#include "RendererMateRamshCommands.h"
#include "impl/RamsesRendererImpl.h"
#include "impl/RamsesFrameworkImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "internal/Ramsh/Ramsh.h"

namespace ramses::internal
{
    RendererMate::RendererMate(RamsesRendererImpl& renderer, RamsesFrameworkImpl& framework)
        : m_ramsesRenderer(renderer)
        , m_rendererSceneControl(*renderer.getSceneControlAPI())
        , m_exitCommand{ std::make_shared<RamshCommandExit>() }
    {
        framework.getRamsh().add(m_exitCommand);
        m_ramshCommands.push_back(std::make_shared<ShowSceneOnDisplay>(*this));
        m_ramshCommands.push_back(std::make_shared<HideScene>(*this));
        m_ramshCommands.push_back(std::make_shared<ReleaseScene>(*this));
        m_ramshCommands.push_back(std::make_shared<LinkData>(*this));
        m_ramshCommands.push_back(std::make_shared<ConfirmationEcho>(*this));
        for (auto& cmd : m_ramshCommands)
            framework.getRamsh().add(cmd);
        LOG_DEBUG(CONTEXT_SMOKETEST, "Ramsh commands registered from RendererMate");
    }

    bool RendererMate::setSceneState(sceneId_t sceneId, RendererSceneState state, const std::string& confirmationText)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);

        if (getLastReportedSceneState(sceneId) == RendererSceneState::Rendered)
        {
            processConfirmationEchoCommand(m_scenesInfo[sceneId].displayMapped, confirmationText);
            m_scenesInfo[sceneId].renderedStateConfirmationText.clear();
        }
        else
            m_scenesInfo[sceneId].renderedStateConfirmationText = confirmationText;

        return m_rendererSceneControl.setSceneState(sceneId, state);
    }

    bool RendererMate::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_scenesInfo[sceneId].displayMapped = displayId;
        return m_rendererSceneControl.setSceneMapping(sceneId, displayId);
    }

    bool RendererMate::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        return m_rendererSceneControl.setSceneDisplayBufferAssignment(sceneId, displayBuffer, sceneRenderOrder);
    }

    void RendererMate::processConfirmationEchoCommand(displayId_t display, const std::string& confirmationText)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        if (!confirmationText.empty())
            m_ramsesRenderer.logConfirmationEcho(display, confirmationText);
    }

    void RendererMate::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_rendererSceneControl.linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerDataSlotId);
    }

    void RendererMate::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_rendererSceneControl.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
    }

    void RendererMate::dispatchAndFlush(IRendererSceneControlEventHandler& sceneControlHandler, IRendererEventHandler* customRendererEventHandler)
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);

        RendererSceneControlEventChainer chainer{ *this, sceneControlHandler };
        m_rendererSceneControl.dispatchEvents(chainer);

        if (customRendererEventHandler)
        {
            RendererEventChainer chainer2{ *this, *customRendererEventHandler };
            m_ramsesRenderer.dispatchEvents(chainer2);
        }
        else
            m_ramsesRenderer.dispatchEvents(*this);

        m_ramsesRenderer.flush();
        m_rendererSceneControl.flush();
    }

    bool RendererMate::isRunning() const
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        return m_isRunning && !m_exitCommand->exitRequested();
    }

    RendererSceneState RendererMate::getLastReportedSceneState(sceneId_t sceneId) const
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        const auto it = m_scenesInfo.find(sceneId);
        return it != m_scenesInfo.cend() ? it->second.currentState : RendererSceneState::Unavailable;
    }

    /* IRendererSceneControlEventHandler handlers */
    void RendererMate::sceneStateChanged(sceneId_t sceneId, RendererSceneState state)
    {
        auto& sceneInfo = m_scenesInfo[sceneId];
        sceneInfo.currentState = state;

        if (state == RendererSceneState::Rendered)
        {
            processConfirmationEchoCommand(m_scenesInfo[sceneId].displayMapped, sceneInfo.renderedStateConfirmationText);
            sceneInfo.renderedStateConfirmationText.clear();
        }
    }

    void RendererMate::displayCreated([[maybe_unused]] displayId_t displayId, ERendererEventResult result)
    {
        if (result == ERendererEventResult::Failed)
        {
            m_isRunning = false;
        }
    }

    void RendererMate::keyEvent([[maybe_unused]] displayId_t displayId, EKeyEvent keyEvent, KeyModifiers keyModifiers, EKeyCode keyCode)
    {
        if (!m_keysHandling)
            return;

        if (keyEvent != EKeyEvent::Pressed)
            return;

        if (keyCode == EKeyCode_Escape && keyModifiers == EKeyModifier())
        {
            m_isRunning = false;
            return;
        }
    };

    void RendererMate::windowClosed([[maybe_unused]] displayId_t displayId)
    {
        m_isRunning = false;
    }

    void RendererMate::enableKeysHandling()
    {
        std::lock_guard<std::recursive_mutex> guard(m_lock);
        m_keysHandling = true;
    }
}
