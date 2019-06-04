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
        virtual void showSceneOnDisplay            (ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder = 0, const char* confirmationText = nullptr) override final;
        virtual void unsubscribeScene              (ramses::sceneId_t sceneId) override final;
        virtual void unmapScene                    (ramses::sceneId_t sceneId) override final;
        virtual void hideScene                     (ramses::sceneId_t sceneId) override final;
        virtual void linkData                      (ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) override final;
        virtual void processConfirmationEchoCommand(const char* text) override final;

        void dispatchAndFlush(ramses::IRendererEventHandler* customHandler = nullptr);

        ramses::displayId_t createDisplay(const ramses::DisplayConfig& config);
        void destroyDisplay(ramses::displayId_t displayId);

        bool isRunning() const;
        bool isDisplayCreated(ramses::displayId_t display) const;

        enum class ESceneState
        {
            Unpublished,
            Published,
            Subscribed,
            Mapped,
            Rendered,

            GoingToSubscribed,
            GoingToMapped,
            GoingToRendered
        };

        ESceneState getSceneState(ramses::sceneId_t sceneId) const;
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
            ramses_internal::String confirmationText;
            ramses::displayId_t displayMappedTo = ramses::InvalidDisplayId;

            bool operator==(const MappingInfo& other)
            {
                return display == other.display && renderOrder == other.renderOrder;
            }
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

        struct SceneInfo
        {
            ESceneState currentState = ESceneState::Unpublished;
            ESceneState targetState = ESceneState::Unpublished;
            ESceneStateCommand lastCommandWaitigForReply = ESceneStateCommand::None;
            MappingInfo mappingInfo;
        };

        ESceneState getCurrentSceneState(ramses::sceneId_t sceneId) const;
        ESceneState getTargetSceneState(ramses::sceneId_t sceneId) const;
        ESceneStateCommand getLastSceneStateCommandWaitingForReply(ramses::sceneId_t sceneId) const;
        bool isInTargetState(ramses::sceneId_t sceneId) const;
        void goToTargetState(ramses::sceneId_t sceneId);

        void handleShowCommand(ramses::sceneId_t sceneId, MappingInfo mappingInfo);
        void handleHideCommand(ramses::sceneId_t sceneId);
        void handleUnmapCommand(ramses::sceneId_t sceneId);
        void handleSubscribeCommand(ramses::sceneId_t sceneId);
        void handleUnsubscribeCommand(ramses::sceneId_t sceneId);

        ramses::RamsesRenderer& m_ramsesRenderer;

        const bool m_autoShow;

        std::unordered_map<ramses::sceneId_t, SceneInfo> m_scenesInfo;
        ramses_capu::HashSet<ramses::displayId_t> m_createdDisplays;

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
