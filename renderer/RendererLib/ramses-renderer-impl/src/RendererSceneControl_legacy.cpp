//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses-renderer-api/RendererSceneControl_legacy.h"
#include "RendererSceneControlImpl_legacy.h"
#include "RamsesFrameworkTypesImpl.h"
#include "APILoggingMacros.h"

namespace ramses
{
    RendererSceneControl_legacy::RendererSceneControl_legacy(RendererSceneControlImpl_legacy& impl_)
        : StatusObject(impl_)
        , impl(impl_)
    {
    }

    RendererSceneControl_legacy::~RendererSceneControl_legacy() = default;

    status_t RendererSceneControl_legacy::subscribeScene(sceneId_t sceneId)
    {
        const status_t status = impl.subscribeScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::unsubscribeScene(sceneId_t sceneId)
    {
        const status_t status = impl.unsubscribeScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::mapScene(displayId_t displayId, sceneId_t sceneId)
    {
        const status_t status = impl.mapScene(displayId, sceneId);
        LOG_HL_RENDERER_API2(status, displayId, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::unmapScene(sceneId_t sceneId)
    {
        const status_t status = impl.unmapScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::showScene(sceneId_t sceneId)
    {
        const status_t status = impl.showScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::hideScene(sceneId_t sceneId)
    {
        const status_t status = impl.hideScene(sceneId);
        LOG_HL_RENDERER_API1(status, sceneId);
        return status;
    }

    status_t RendererSceneControl_legacy::assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        const status_t status = impl.assignSceneToDisplayBuffer(sceneId, displayBuffer, sceneRenderOrder);
        LOG_HL_RENDERER_API3(status, sceneId, displayBuffer, sceneRenderOrder);
        return status;
    }

    status_t RendererSceneControl_legacy::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        const status_t status = impl.setDisplayBufferClearColor(display, displayBuffer, r, g, b, a);
        LOG_HL_RENDERER_API6(status, display, displayBuffer, r, g, b, a);
        return status;
    }

    status_t RendererSceneControl_legacy::linkData(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        const status_t status = impl.linkData(providerScene, providerId, consumerScene, consumerId);
        LOG_HL_RENDERER_API4(status, providerScene, providerId, consumerScene, consumerId);
        return status;
    }

    status_t RendererSceneControl_legacy::linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const status_t status = impl.linkOffscreenBufferToSceneData(offscreenBufferId, consumerSceneId, consumerDataSlotId);
        LOG_HL_RENDERER_API3(status, offscreenBufferId, consumerSceneId, consumerDataSlotId);
        return status;
    }

    status_t RendererSceneControl_legacy::unlinkData(sceneId_t consumerScene, dataConsumerId_t consumerId)
    {
        const status_t status = impl.unlinkData(consumerScene, consumerId);
        LOG_HL_RENDERER_API2(status, consumerScene, consumerId);
        return status;
    }

    status_t RendererSceneControl_legacy::flush()
    {
        const status_t result = impl.flush();
        LOG_HL_RENDERER_API_NOARG(result);
        return result;
    }

    status_t RendererSceneControl_legacy::dispatchEvents(IRendererSceneControlEventHandler_legacy& eventHandler)
    {
        const status_t status = impl.dispatchEvents(eventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(eventHandler));
        return status;
    }
}
