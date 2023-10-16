//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/RendererSceneControl.h"
#include "internal/RendererLib/RendererCommands.h"
#include "internal/RendererLib/RendererEvent.h"
#include <unordered_map>

namespace ramses::internal
{
    class RamsesRendererImpl;

    class IRendererSceneControl
    {
    public:
        virtual bool setSceneState(sceneId_t sceneId, RendererSceneState state) = 0;
        virtual bool setSceneMapping(sceneId_t sceneId, displayId_t displayId) = 0;
        virtual bool setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) = 0;
        virtual bool linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual bool linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual bool linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId) = 0;
        virtual bool unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId) = 0;
        virtual bool handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY) = 0;
        virtual bool flush() = 0;
        virtual bool dispatchEvents(IRendererSceneControlEventHandler& eventHandler) = 0;

        virtual ~IRendererSceneControl() = default;
    };

    class RendererSceneControlImpl final : public IRendererSceneControl
    {
    public:
        explicit RendererSceneControlImpl(RamsesRendererImpl& renderer);
        ~RendererSceneControlImpl() override;

        bool setSceneState(sceneId_t sceneId, RendererSceneState state) override;
        bool setSceneMapping(sceneId_t sceneId, displayId_t displayId) override;
        bool setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) override;
        bool linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        bool linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        virtual bool linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);
        bool linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId) override;
        bool unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId) override;
        bool handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY) override;

        bool flush() override;
        bool dispatchEvents(IRendererSceneControlEventHandler& eventHandler) override;

        const RendererCommands& getPendingCommands() const;

    private:
        RamsesRendererImpl& m_renderer;
        RendererCommands m_pendingRendererCommands;

        struct SceneInfo
        {
            bool mappingSet = false;
            RendererSceneState currState = RendererSceneState::Unavailable;
            RendererSceneState targetState = RendererSceneState::Unavailable;
        };
        std::unordered_map<sceneId_t, SceneInfo> m_sceneInfos;

        // keep allocated containers which are used to swap internal data
        RendererEventVector m_tempRendererEvents;
    };
}
