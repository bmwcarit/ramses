//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROLIMPL_LEGACY_H
#define RAMSES_RENDERERSCENECONTROLIMPL_LEGACY_H

#include "StatusObjectImpl.h"
#include "RendererLib/RendererCommands.h"

namespace ramses
{
    class RamsesRendererImpl;
    class IRendererSceneControlEventHandler_legacy;

    class IRendererSceneControlImpl_legacy
    {
    public:
        virtual status_t subscribeScene(sceneId_t sceneId) = 0;
        virtual status_t unsubscribeScene(sceneId_t sceneId) = 0;

        virtual status_t mapScene(displayId_t displayId, sceneId_t sceneId) = 0;
        virtual status_t unmapScene(sceneId_t sceneId) = 0;

        virtual status_t showScene(sceneId_t sceneId) = 0;
        virtual status_t hideScene(sceneId_t sceneId) = 0;

        virtual status_t assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) = 0;
        virtual status_t setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a) = 0;

        virtual status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual status_t linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;
        virtual status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) = 0;

        virtual status_t flush() = 0;
        virtual status_t dispatchEvents(IRendererSceneControlEventHandler_legacy& eventHandler) = 0;

        virtual ~IRendererSceneControlImpl_legacy() = default;
    };

    class RendererSceneControlImpl_legacy final : public IRendererSceneControlImpl_legacy, public StatusObjectImpl
    {
    public:
        explicit RendererSceneControlImpl_legacy(RamsesRendererImpl& renderer);

        virtual status_t subscribeScene(sceneId_t sceneId) override;
        virtual status_t unsubscribeScene(sceneId_t sceneId) override;

        virtual status_t mapScene(displayId_t displayId, sceneId_t sceneId) override;
        virtual status_t unmapScene(sceneId_t sceneId) override;

        virtual status_t showScene(sceneId_t sceneId) override;
        virtual status_t hideScene(sceneId_t sceneId) override;

        virtual status_t assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder) override;
        virtual status_t setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a) override;

        virtual status_t linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        virtual status_t linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;
        virtual status_t unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId) override;

        virtual status_t flush() override;
        virtual status_t dispatchEvents(IRendererSceneControlEventHandler_legacy& eventHandler) override;

        const ramses_internal::RendererCommands& getPendingCommands() const;

    private:
        RamsesRendererImpl& m_renderer;
        ramses_internal::RendererCommands m_pendingRendererCommands;
    };
}

#endif
