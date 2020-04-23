//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_HL_RENDERERSCENECONTROLLOGIC_H
#define RAMSES_HL_RENDERERSCENECONTROLLOGIC_H

#include "ramses-framework-api/RendererSceneState.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"
#include "Utils/LoggingUtils.h"
#include <vector>
#include <unordered_map>

namespace ramses
{
    static const char* RendererSceneStateNames[] =
    {
        "Unavailable",
        "Available",
        "Ready",
        "Rendered"
    };
    ENUM_TO_STRING(ramses::RendererSceneState, RendererSceneStateNames, 4);

    class IRendererSceneControlImpl_legacy;

    class RendererSceneControlLogic final : public RendererSceneControlEventHandlerEmpty_legacy
    {
    public:
        RendererSceneControlLogic(IRendererSceneControlImpl_legacy& sceneControl);

        void setSceneState(sceneId_t sceneId, RendererSceneState state);
        void setSceneMapping(sceneId_t sceneId, displayId_t displayId);
        void setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder = 0);
        void setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a);
        void linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        void linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId);
        void unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId);

        struct Event
        {
            enum class Type
            {
                ScenePublished,
                SceneStateChanged,
                OffscreenBufferLinked,
                DataLinked,
                DataUnlinked,
                SceneFlushed,
                SceneExpired,
                SceneRecoveredFromExpiration,
                StreamAvailable
            };

            Type type = Type::DataLinked;
            sceneId_t sceneId = {};
            RendererSceneState state = RendererSceneState::Unavailable;
            displayBufferId_t displayBufferId = {};

            sceneId_t providerSceneId = {};
            sceneId_t consumerSceneId = {};
            dataProviderId_t providerId = {};
            dataConsumerId_t consumerId = {};
            bool dataLinked = false;
            sceneVersionTag_t sceneVersion = 0;
            streamSource_t streamSourceId = {};
            bool streamAvailable = false;
        };
        using Events = std::vector<Event>;
        Events consumeEvents();

    private:
        // RendererSceneControlEventHandlerEmpty_legacy methods
        virtual void scenePublished(sceneId_t sceneId) override;
        virtual void sceneUnpublished(sceneId_t sceneId) override;
        virtual void sceneSubscribed(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void sceneMapped(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void sceneUnmapped(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void sceneShown(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void sceneHidden(sceneId_t sceneId, ERendererEventResult result) override;
        virtual void offscreenBufferLinkedToSceneData(displayBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override;
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override;
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override;
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag, ESceneResourceStatus resourceStatus) override;
        virtual void sceneExpired(sceneId_t sceneId) override;
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override;
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) override;

        struct MappingInfo
        {
            displayId_t display;
            displayBufferId_t displayBuffer;
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
            Mapped,            // mapped to display with resources uploaded
            MappedAndAssigned, // assigned to display's buffer and set render order
            Rendered
        };

        struct SceneInfo
        {
            ESceneStateInternal currentState = ESceneStateInternal::Unpublished;
            ESceneStateInternal targetState = ESceneStateInternal::Unpublished;
            ESceneStateCommand lastCommandWaitigForReply = ESceneStateCommand::None;
            MappingInfo mappingInfo;
        };

        ESceneStateInternal getCurrentSceneState(sceneId_t sceneId) const;
        ESceneStateInternal getTargetSceneState(sceneId_t sceneId) const;
        ESceneStateCommand getLastSceneStateCommandWaitingForReply(sceneId_t sceneId) const;
        void goToTargetState(sceneId_t sceneId);
        void goToMappedAndAssignedState(sceneId_t sceneId);
        void setCurrentSceneState(sceneId_t sceneId, ESceneStateInternal state);

        static RendererSceneState GetSceneStateFromInternal(ESceneStateInternal internalState);
        static ESceneStateInternal GetInternalSceneState(RendererSceneState state);

        IRendererSceneControlImpl_legacy& m_sceneControl;

        std::unordered_map<sceneId_t, SceneInfo> m_scenesInfo;

        std::vector<Event> m_pendingEvents;
    };
}

#endif
