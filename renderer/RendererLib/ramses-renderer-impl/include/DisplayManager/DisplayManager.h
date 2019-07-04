//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYMANAGER_H
#define RAMSES_DISPLAYMANAGER_H

#include "DisplayManager/IDisplayManager.h"
#include "ramses-renderer-api/IRendererEventHandler.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "DisplayManager/ShowSceneOnDisplay.h"
#include "DisplayManager/HideScene.h"
#include "DisplayManager/UnmapScene.h"
#include "DisplayManager/UnsubscribeScene.h"
#include "DisplayManager/LinkData.h"
#include "DisplayManager/ConfirmationEcho.h"
#include "Ramsh/RamshCommandExit.h"

#include "ramses-capu/container/HashSet.h"
#include "Collections/String.h"
#include "PlatformAbstraction/PlatformLock.h"
#include <unordered_map>
#include <memory>

namespace ramses
{
    class RamsesFramework;
    class RamsesRenderer;
    class DisplayConfig;
}

namespace ramses_display_manager
{
    class DisplayManager final : public IDisplayManager, public ramses::RendererEventHandlerEmpty
    {
    public:
        DisplayManager(ramses::RamsesRenderer& renderer, ramses::RamsesFramework& framework, bool autoShow);
        virtual ~DisplayManager();

        // IDisplayManager
        virtual bool setSceneState(ramses::sceneId_t sceneId, SceneState state, const char* confirmationText = "") override final;
        virtual bool setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder = 0) override final;
        virtual void linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) override final;
        virtual void processConfirmationEchoCommand(const char* text) override final;

        void dispatchAndFlush(IEventHandler* eventHandler = nullptr, ramses::IRendererEventHandler* customRendererEventHandler = nullptr);

        ramses::displayId_t createDisplay(const ramses::DisplayConfig& config);
        void destroyDisplay(ramses::displayId_t displayId);

        bool isRunning() const;
        bool isDisplayCreated(ramses::displayId_t display) const;

        SceneState getLastReportedSceneState(ramses::sceneId_t sceneId) const;
        ramses::displayId_t getDisplaySceneIsMappedTo(ramses::sceneId_t sceneId) const;

    private:
        // IRendererEventHandler methods for scene state transition
        virtual void scenePublished                    (ramses::sceneId_t sceneId) override final;
        virtual void sceneUnpublished                  (ramses::sceneId_t sceneId) override final;
        virtual void sceneSubscribed                   (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void sceneUnsubscribed                 (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void sceneMapped                       (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void sceneUnmapped                     (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void sceneShown                        (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void sceneHidden                       (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override final;
        virtual void displayCreated                    (ramses::displayId_t displayId, ramses::ERendererEventResult result) override final;
        virtual void displayDestroyed                  (ramses::displayId_t displayId, ramses::ERendererEventResult result) override final;
        virtual void keyEvent                          (ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode) override final;
        virtual void windowClosed                      (ramses::displayId_t displayId) override final;

        struct MappingInfo
        {
            ramses::displayId_t display = ramses::InvalidDisplayId;
            int32_t renderOrder = 0;
        };

        enum class ESceneStateCommand
        {
            None,
            Subscribe,
            Unsubscribe,
            Map,
            Unmap,
            Show,
            Hide
        };

        enum class ESceneStateInternal
        {
            Unpublished,
            Published,
            Subscribed,
            MappedAndAssigned,
            Rendered
        };

        struct SceneInfo
        {
            ESceneStateInternal currentState = ESceneStateInternal::Unpublished;
            ESceneStateInternal targetState = ESceneStateInternal::Unpublished;
            ESceneStateCommand lastCommandWaitigForReply = ESceneStateCommand::None;
            MappingInfo mappingInfo;
            std::string targetStateConfirmationText;
        };

        ESceneStateInternal getCurrentSceneState(ramses::sceneId_t sceneId) const;
        ESceneStateInternal getTargetSceneState(ramses::sceneId_t sceneId) const;
        ESceneStateCommand getLastSceneStateCommandWaitingForReply(ramses::sceneId_t sceneId) const;
        void goToTargetState(ramses::sceneId_t sceneId);
        void setCurrentSceneState(ramses::sceneId_t sceneId, ESceneStateInternal state);

        static SceneState GetSceneStateFromInternal(ESceneStateInternal internalState);
        static ESceneStateInternal GetInternalSceneState(SceneState state);

        ramses::RamsesRenderer& m_ramsesRenderer;

        const bool m_autoShow;

        std::unordered_map<ramses::sceneId_t, SceneInfo> m_scenesInfo;
        ramses_capu::HashSet<ramses::displayId_t> m_createdDisplays;

        struct Event
        {
            ramses::sceneId_t sceneId;
            SceneState state;
            ramses::displayId_t displaySceneIsMappedTo;
        };
        std::vector<Event> m_pendingEvents;

        std::unique_ptr<ramses_internal::RamshCommandExit> m_exitCommand;
        std::unique_ptr<ShowSceneOnDisplay>                m_showOnDisplayCommand;
        std::unique_ptr<HideScene>                         m_hideCommand;
        std::unique_ptr<UnmapScene>                        m_unmapCommand;
        std::unique_ptr<UnsubscribeScene>                  m_unsubscribeCommand;
        std::unique_ptr<LinkData>                          m_linkData;
        std::unique_ptr<ConfirmationEcho>                  m_confirmationEchoCommand;
        bool                                               m_isRunning;

        mutable ramses_internal::PlatformLock m_lock;
    };
}

#endif
