//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/RendererSceneControl.h"
#include "RendererSceneControlImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{
    RendererSceneControl::RendererSceneControl(RendererSceneControlImpl& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
    }

    RendererSceneControl::~RendererSceneControl() = default;

    status_t RendererSceneControl::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        const status_t status = impl.setSceneState(sceneId, state);
        LOG_HL_RENDERER_API2(status, sceneId, EnumToString(state));
        return status;
    }

    status_t RendererSceneControl::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        const status_t status = impl.setSceneMapping(sceneId, displayId);
        LOG_HL_RENDERER_API2(status, sceneId, displayId);
        return status;
    }

    status_t RendererSceneControl::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        const status_t status = impl.setSceneDisplayBufferAssignment(sceneId, displayBuffer, sceneRenderOrder);
        LOG_HL_RENDERER_API3(status, sceneId, displayBuffer, sceneRenderOrder);
        return status;
    }

    status_t RendererSceneControl::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        const status_t status = impl.setDisplayBufferClearColor(display, displayBuffer, r, g, b, a);
        LOG_HL_RENDERER_API6(status, display, displayBuffer, r, g, b, a);
        return status;
    }

    status_t RendererSceneControl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const status_t status = impl.linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerDataSlotId);
        LOG_HL_RENDERER_API3(status, offscreenBufferId, consumerSceneId, consumerDataSlotId);
        return status;
    }

    status_t RendererSceneControl::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const status_t status = impl.linkData(providerSceneId, providerId, consumerSceneId, consumerId);
        LOG_HL_RENDERER_API4(status, providerSceneId, providerId, consumerSceneId, consumerId);
        return status;
    }

    status_t RendererSceneControl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const status_t status = impl.unlinkData(consumerSceneId, consumerId);
        LOG_HL_RENDERER_API2(status, consumerSceneId, consumerId);
        return status;
    }

    status_t RendererSceneControl::flush()
    {
        const status_t status = impl.flush();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t RendererSceneControl::dispatchEvents(IRendererSceneControlEventHandler& eventHandler)
    {
        const status_t status = impl.dispatchEvents(eventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(eventHandler));
        return status;
    }
}
