//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "ramses/renderer/RendererSceneControl.h"
#include "impl/RendererSceneControlImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/APILoggingMacros.h"

namespace ramses
{
    RendererSceneControl::RendererSceneControl(std::unique_ptr<internal::RendererSceneControlImpl> impl)
        : m_impl{ std::move(impl) }
    {
    }

    RendererSceneControl::~RendererSceneControl() = default;

    bool RendererSceneControl::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        const bool status = m_impl->setSceneState(sceneId, state);
        LOG_HL_RENDERER_API2(status, sceneId, internal::EnumToString(state));
        return status;
    }

    bool RendererSceneControl::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        const bool status = m_impl->setSceneMapping(sceneId, displayId);
        LOG_HL_RENDERER_API2(status, sceneId, displayId);
        return status;
    }

    bool RendererSceneControl::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        const bool status = m_impl->setSceneDisplayBufferAssignment(sceneId, displayBuffer, sceneRenderOrder);
        LOG_HL_RENDERER_API3(status, sceneId, displayBuffer, sceneRenderOrder);
        return status;
    }

    bool RendererSceneControl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const bool status = m_impl->linkOffscreenBuffer(offscreenBufferId, consumerSceneId, consumerDataSlotId);
        LOG_HL_RENDERER_API3(status, offscreenBufferId, consumerSceneId, consumerDataSlotId);
        return status;
    }

    bool RendererSceneControl::linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        return m_impl->linkStreamBuffer(streamBufferId, consumerSceneId, consumerDataSlotId);
    }

    bool RendererSceneControl::linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const bool status = m_impl->linkExternalBuffer(externalBufferId, consumerSceneId, consumerDataSlotId);
        LOG_HL_RENDERER_API3(status, externalBufferId, consumerSceneId, consumerDataSlotId);
        return status;
    }

    bool RendererSceneControl::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const bool status = m_impl->linkData(providerSceneId, providerId, consumerSceneId, consumerId);
        LOG_HL_RENDERER_API4(status, providerSceneId, providerId, consumerSceneId, consumerId);
        return status;
    }

    bool RendererSceneControl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const bool status = m_impl->unlinkData(consumerSceneId, consumerId);
        LOG_HL_RENDERER_API2(status, consumerSceneId, consumerId);
        return status;
    }

    bool RendererSceneControl::handlePickEvent(sceneId_t sceneId, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const bool status = m_impl->handlePickEvent(sceneId, bufferNormalizedCoordX, bufferNormalizedCoordY);
        LOG_HL_RENDERER_API3(status, sceneId, bufferNormalizedCoordX, bufferNormalizedCoordY);
        return status;
    }

    bool RendererSceneControl::flush()
    {
        const bool status = m_impl->flush();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RendererSceneControl::dispatchEvents(IRendererSceneControlEventHandler& eventHandler)
    {
        const bool status = m_impl->dispatchEvents(eventHandler);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(eventHandler));
        return status;
    }

    internal::RendererSceneControlImpl& RendererSceneControl::impl()
    {
        return *m_impl;
    }

    const internal::RendererSceneControlImpl& RendererSceneControl::impl() const
    {
        return *m_impl;
    }
}
