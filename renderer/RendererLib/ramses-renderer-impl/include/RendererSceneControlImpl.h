//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLIMPL_H
#define RAMSES_RENDERERSCENECONTROLIMPL_H

#include "ramses-renderer-api/RendererSceneControl.h"
#include "StatusObjectImpl.h"
#include "RendererLib/RendererCommands.h"
#include "RendererLib/RendererEvent.h"
#include "Math3d/Vector4.h"
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

    class RamsesRendererImpl;

    class IRendererSceneControlEventHandler_SpecialForWayland
    {
    public:
        virtual void streamBufferLinked(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId, bool success) = 0;
        virtual void streamBufferEnabled(streamBufferId_t streamBufferId, bool success) = 0;
        virtual ~IRendererSceneControlEventHandler_SpecialForWayland() = default;
    };

    class IRendererSceneControl
    {
    public:
        virtual status_t setSceneState(sceneId_t sceneId, RendererSceneState state) = 0;
        virtual status_t setSceneMapping(sceneId_t sceneId, displayId_t displayId) = 0;
        virtual status_t setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) = 0;
        virtual status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual status_t linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId) = 0;
        virtual status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId) = 0;
        virtual status_t handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY) = 0;
        virtual status_t flush() = 0;
        virtual status_t dispatchEvents(IRendererSceneControlEventHandler& eventHandler) = 0;
        virtual status_t dispatchSpecialEvents(IRendererSceneControlEventHandler_SpecialForWayland& eventHandler) = 0;
        virtual streamBufferId_t createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source) = 0;
        virtual status_t destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer) = 0;
        virtual status_t setStreamBufferState(displayId_t display, streamBufferId_t streamBufferId, bool state) = 0;

        virtual ~IRendererSceneControl() = default;
    };

    class RendererSceneControlImpl final : public IRendererSceneControl, public StatusObjectImpl
    {
    public:
        explicit RendererSceneControlImpl(RamsesRendererImpl& renderer);

        virtual status_t setSceneState(sceneId_t sceneId, RendererSceneState state) override;
        virtual status_t setSceneMapping(sceneId_t sceneId, displayId_t displayId) override;
        virtual status_t setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) override;
        virtual status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        virtual status_t linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        virtual status_t linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        virtual status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId) override;
        virtual status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId) override;
        virtual status_t handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY) override;

        virtual status_t flush() override;
        virtual status_t dispatchEvents(IRendererSceneControlEventHandler& eventHandler) override;

        const ramses_internal::RendererCommands& getPendingCommands() const;

        virtual status_t dispatchSpecialEvents(IRendererSceneControlEventHandler_SpecialForWayland& eventHandler) override;

        streamBufferId_t createStreamBuffer(displayId_t display, waylandIviSurfaceId_t source) override;
        status_t destroyStreamBuffer(displayId_t display, streamBufferId_t streamBuffer) override;
        status_t setStreamBufferState(displayId_t display, streamBufferId_t streamBufferId, bool state) override;
    private:
        RamsesRendererImpl& m_renderer;
        ramses_internal::RendererCommands m_pendingRendererCommands;

        struct SceneInfo
        {
            bool mappingSet = false;
            RendererSceneState currState = RendererSceneState::Unavailable;
            RendererSceneState targetState = RendererSceneState::Unavailable;
        };
        std::unordered_map<sceneId_t, SceneInfo> m_sceneInfos;

        // keep allocated containers which are used to swap internal data
        ramses_internal::RendererEventVector m_tempRendererEvents;
        // todo(jonathan) cleanup with next major version
        ramses_internal::RendererEventVector m_waylandEvents;
    };
}

#endif
