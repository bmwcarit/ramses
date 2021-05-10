//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLLOGIC_H
#define RAMSES_RENDERERSCENECONTROLLOGIC_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include <unordered_map>

namespace ramses_internal
{
    class IRendererSceneStateControl;
    struct InternalSceneStateEvent;

    class IRendererSceneControlLogic
    {
    public:
        virtual void setSceneState(SceneId sceneId, RendererSceneState state) = 0;
        virtual void setSceneDisplayBufferAssignment(SceneId sceneId, OffscreenBufferHandle displayBuffer, int32_t sceneRenderOrder) = 0;
        virtual void getSceneInfo(SceneId sceneId, RendererSceneState& targetState, OffscreenBufferHandle& bufferToAssign, int32_t& renderOrder) const = 0;

        virtual ~IRendererSceneControlLogic() = default;
    };

    class RendererSceneControlLogic : public IRendererSceneControlLogic
    {
    public:
        explicit RendererSceneControlLogic(IRendererSceneStateControl& sceneStateControl);

        virtual void setSceneState(SceneId sceneId, RendererSceneState state) override;
        virtual void setSceneDisplayBufferAssignment(SceneId sceneId, OffscreenBufferHandle displayBuffer, int32_t sceneRenderOrder) override;
        virtual void getSceneInfo(SceneId sceneId, RendererSceneState& targetState, OffscreenBufferHandle& bufferToAssign, int32_t& renderOrder) const override;

        enum class EventResult { OK, Failed, Indirect };

        void scenePublished(SceneId sceneId);
        void sceneUnpublished(SceneId sceneId);
        void sceneSubscribed(SceneId sceneId, EventResult result);
        void sceneUnsubscribed(SceneId sceneId, EventResult result);
        void sceneMapped(SceneId sceneId, EventResult result);
        void sceneUnmapped(SceneId sceneId, EventResult result);
        void sceneShown(SceneId sceneId, EventResult result);
        void sceneHidden(SceneId sceneId, EventResult result);

        void processInternalEvent(const InternalSceneStateEvent& evt);

        struct Event
        {
            SceneId sceneId;
            RendererSceneState state;
        };
        using Events = std::vector<Event>;
        void consumeEvents(Events& eventsOut);

    private:
        struct BufferAssignmentInfo
        {
            OffscreenBufferHandle displayBuffer;
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
            BufferAssignmentInfo assignmentInfo;
        };

        ESceneStateInternal getCurrentSceneState(SceneId sceneId) const;
        ESceneStateInternal getTargetSceneState(SceneId sceneId) const;
        ESceneStateCommand getLastSceneStateCommandWaitingForReply(SceneId sceneId) const;
        void goToTargetState(SceneId sceneId);
        void goToMappedAndAssignedState(SceneId sceneId);
        void setCurrentSceneState(SceneId sceneId, ESceneStateInternal state);

        static RendererSceneState GetSceneStateFromInternal(ESceneStateInternal internalState);
        static ESceneStateInternal GetInternalSceneState(RendererSceneState state);

        IRendererSceneStateControl& m_sceneStateControl;
        std::unordered_map<SceneId, SceneInfo> m_scenesInfo;

        Events m_pendingEvents;
    };
}

#endif
