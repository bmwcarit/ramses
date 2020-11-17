//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERMATE_H
#define RAMSES_RENDERERMATE_H

#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "Ramsh/RamshCommandExit.h"
#include <unordered_map>
#include <vector>
#include <memory>

namespace ramses
{
    class RamsesRendererImpl;
    class RamsesFrameworkImpl;
    class RendererSceneControl;

    class RendererMate final : public RendererSceneControlEventHandlerEmpty, public RendererEventHandlerEmpty
    {
    public:
        RendererMate(RamsesRendererImpl& renderer, RamsesFrameworkImpl& framework);

        bool setSceneState(sceneId_t sceneId, RendererSceneState state, std::string confirmationText = {});
        bool setSceneMapping(sceneId_t sceneId, displayId_t displayId);
        bool setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder = 0);
        void linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        void linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId);
        void processConfirmationEchoCommand(std::string confirmationText);

        void dispatchAndFlush(IRendererSceneControlEventHandler& sceneControlHandler, IRendererEventHandler* customRendererEventHandler = nullptr);

        bool isRunning() const;
        RendererSceneState getLastReportedSceneState(sceneId_t sceneId) const;

        void enableKeysHandling();

    private:
        // IRendererSceneControlEventHandler
        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override;

        // IRendererEventHandler
        virtual void keyEvent(displayId_t displayId, EKeyEvent keyEvent, uint32_t keyModifiers, EKeyCode keyCode) override;
        virtual void windowClosed(displayId_t displayId) override;

        struct SceneInfo
        {
            RendererSceneState currentState = RendererSceneState::Available;
            std::string renderedStateConfirmationText;
        };

        RamsesRendererImpl& m_ramsesRenderer;
        RendererSceneControl& m_rendererSceneControl;

        std::unordered_map<sceneId_t, SceneInfo> m_scenesInfo;

        std::unique_ptr<ramses_internal::RamshCommandExit> m_exitCommand;
        std::vector<std::unique_ptr<ramses_internal::RamshCommand>> m_ramshCommands;

        bool m_isRunning = true;
        bool m_keysHandling = false;

        // RenderMate registers several Ramsh commands that call RenderMate API from other thread(s)
        mutable std::recursive_mutex m_lock;
    };

    class RendererMateAutoShowHandler : public RendererSceneControlEventHandlerEmpty
    {
    public:
        RendererMateAutoShowHandler(RendererMate& dm, bool autoShow, displayId_t displayToMap = displayId_t{ 0 })
            : m_dm(dm)
            , m_autoShow(autoShow)
            , m_display(displayToMap)
        {
        }

        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
        {
            const auto it = m_oldState.find(sceneId);
            const bool scenePublished = (it == m_oldState.end()) || (it->second == RendererSceneState::Unavailable);
            m_oldState[sceneId] = state;

            if (m_autoShow && scenePublished && state == RendererSceneState::Available)
            {
                m_dm.setSceneMapping(sceneId, m_display);
                m_dm.setSceneState(sceneId, RendererSceneState::Rendered);
            }
        }

    protected:
        RendererMate& m_dm;
        bool m_autoShow;
        displayId_t m_display;
        std::unordered_map<sceneId_t, RendererSceneState> m_oldState;
    };
}

#endif
