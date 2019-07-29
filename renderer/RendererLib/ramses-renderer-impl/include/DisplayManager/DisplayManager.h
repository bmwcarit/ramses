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

#include "Collections/String.h"
#include "PlatformAbstraction/PlatformLock.h"
#include <unordered_map>
#include <unordered_set>
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
        virtual bool setSceneState(ramses::sceneId_t sceneId, SceneState state, const char* confirmationText = "") override;
        virtual bool setSceneMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, int32_t sceneRenderOrder = 0) override;
        virtual bool setSceneOffscreenBufferMapping(ramses::sceneId_t sceneId, ramses::displayId_t displayId, uint32_t width, uint32_t height, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerTextureSamplerId) override;
        virtual void linkData(ramses::sceneId_t providerSceneId, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerSceneId, ramses::dataConsumerId_t consumerId) override;
        virtual void processConfirmationEchoCommand(const char* text) override;

        virtual void dispatchAndFlush(IEventHandler* eventHandler = nullptr, ramses::IRendererEventHandler* customRendererEventHandler = nullptr) override;

        ramses::displayId_t createDisplay(const ramses::DisplayConfig& config);
        void destroyDisplay(ramses::displayId_t displayId);

        bool isRunning() const;
        bool isDisplayCreated(ramses::displayId_t display) const;

        SceneState getLastReportedSceneState(ramses::sceneId_t sceneId) const;
        ramses::displayId_t getDisplaySceneIsMappedTo(ramses::sceneId_t sceneId) const;

        void enableKeysHandling();

    private:
        // IRendererEventHandler methods for scene state transition
        virtual void scenePublished                    (ramses::sceneId_t sceneId) override;
        virtual void sceneUnpublished                  (ramses::sceneId_t sceneId) override;
        virtual void sceneSubscribed                   (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneUnsubscribed                 (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneMapped                       (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneUnmapped                     (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneShown                        (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void sceneHidden                       (ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override;
        virtual void displayCreated                    (ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
        virtual void displayDestroyed                  (ramses::displayId_t displayId, ramses::ERendererEventResult result) override;
        virtual void offscreenBufferCreated            (ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override;
        virtual void offscreenBufferDestroyed          (ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override;
        virtual void sceneAssignedToOffscreenBuffer    (ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override;
        virtual void offscreenBufferLinkedToSceneData  (ramses::offscreenBufferId_t providerOffscreenBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, ramses::ERendererEventResult result) override;
        virtual void keyEvent                          (ramses::displayId_t displayId, ramses::EKeyEvent keyEvent, uint32_t keyModifiers, ramses::EKeyCode keyCode) override;
        virtual void windowClosed                      (ramses::displayId_t displayId) override;

        struct MappingInfo
        {
            ramses::displayId_t display = ramses::InvalidDisplayId;
            int32_t renderOrder = 0;

            // OB mapping
            ramses::offscreenBufferId_t offscreenBuffer = ramses::InvalidOffscreenBufferId;
            ramses::sceneId_t consumerScene = 0;
            ramses::dataConsumerId_t consumerSamplerId = 0;
            uint32_t obWidth = 0;
            uint32_t obHeight = 0;

            // states used when OB mapping, sub-states of Mapped scene state before reaching MappedAndAssigned
            enum class OBMappingState
            {
                None,
                OBCreationRequested,
                OBCreated,
                AssignmentRequested,
                Assigned,
                LinkRequested,
                Linked
            };
            OBMappingState obMappingState = OBMappingState::None;
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
            Mapped,            // scene is mapped to display
            MappedAndAssigned, // scene is mapped and assigned to FB/OB and linked to consumer if any
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
        void goToMappedAndAssignedState(ramses::sceneId_t sceneId);
        void goToOBLinkedState(ramses::sceneId_t sceneId);
        void setCurrentSceneState(ramses::sceneId_t sceneId, ESceneStateInternal state);
        bool setMappingInternal(ramses::sceneId_t sceneId, const MappingInfo& mappingInfo);
        void resetMappingState(ramses::sceneId_t sceneId);

        static SceneState GetSceneStateFromInternal(ESceneStateInternal internalState);
        static ESceneStateInternal GetInternalSceneState(SceneState state);

        ramses::RamsesRenderer& m_ramsesRenderer;

        const bool m_autoShow;

        std::unordered_map<ramses::sceneId_t, SceneInfo> m_scenesInfo;

        struct DisplayInfo
        {
            std::unordered_set<ramses::offscreenBufferId_t> offscreenBuffers;
        };
        std::unordered_map<ramses::displayId_t, DisplayInfo> m_createdDisplays;

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
        bool                                               m_keysHandling = false;

        mutable ramses_internal::PlatformLock m_lock;
    };
}

#endif
