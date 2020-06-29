//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneControlImpl_legacy.h"
#include "RamsesRendererImpl.h"
#include "RendererAPI/Types.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler_legacy.h"
#include "RamsesRendererUtils.h"

namespace ramses
{
    RendererSceneControlImpl_legacy::RendererSceneControlImpl_legacy(RamsesRendererImpl& renderer)
        : StatusObjectImpl()
        , m_renderer(renderer)
    {
    }

    status_t RendererSceneControlImpl_legacy::subscribeScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.subscribeScene(ramses_internal::SceneId(sceneId.getValue()));
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::unsubscribeScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.unsubscribeScene(ramses_internal::SceneId(sceneId.getValue()), false);
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::mapScene(displayId_t displayId, sceneId_t sceneId)
    {
        m_pendingRendererCommands.mapSceneToDisplay(ramses_internal::SceneId{ sceneId.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::unmapScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.unmapScene(ramses_internal::SceneId{ sceneId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::showScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.showScene(ramses_internal::SceneId{ sceneId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::hideScene(sceneId_t sceneId)
    {
        m_pendingRendererCommands.hideScene(ramses_internal::SceneId{ sceneId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to assign to is display's framebuffer pass invalid OB to internal renderer
        const auto& frameBuffers = m_renderer.getDisplayFrameBuffers();
        if (std::any_of(frameBuffers.cbegin(), frameBuffers.cend(), [displayBuffer](const auto& d) { return d.second == displayBuffer; }))
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        m_pendingRendererCommands.assignSceneToDisplayBuffer(ramses_internal::SceneId{ sceneId.getValue() }, bufferHandle, sceneRenderOrder);

        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, float r, float g, float b, float a)
    {
        const auto& frameBuffers = m_renderer.getDisplayFrameBuffers();
        const auto it = frameBuffers.find(display);
        if (it == frameBuffers.cend())
            return addErrorEntry("RendererSceneControl_legacy::setDisplayBufferClearColor failed: display does not exist.");

        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to clear is display's framebuffer pass invalid OB to internal renderer
        if (displayBuffer == it->second)
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        m_pendingRendererCommands.setClearColor(ramses_internal::DisplayHandle{ display.getValue() }, bufferHandle, { r, g, b, a });

        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::linkData(sceneId_t providerSceneId, dataProviderId_t providerDataSlotId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        if (providerSceneId == consumerSceneId)
            return addErrorEntry("RendererSceneControl_legacy::linkData failed: provider- and consumer scene must not be identical");

        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const ramses_internal::SceneId internalProviderSceneId{ providerSceneId.getValue() };
        m_pendingRendererCommands.linkSceneData(internalProviderSceneId, ramses_internal::DataSlotId{ providerDataSlotId.getValue() }, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() });

        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::OffscreenBufferHandle providerBuffer{ offscreenBufferId.getValue() };
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        m_pendingRendererCommands.linkBufferToSceneData(providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() });

        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        m_pendingRendererCommands.unlinkSceneData(ramses_internal::SceneId{ consumerSceneId.getValue() }, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::flush()
    {
        m_renderer.submitRendererCommands(m_pendingRendererCommands);
        m_pendingRendererCommands.clear();
        return StatusOK;
    }

    status_t RendererSceneControlImpl_legacy::dispatchEvents(IRendererSceneControlEventHandler_legacy& eventHandler)
    {
        ramses_internal::RendererEventVector events;
        m_renderer.getRenderer().dispatchSceneControlEvents(events);

        for (const auto& event : events)
        {
            switch (event.eventType)
            {
            case ramses_internal::ERendererEventType_ScenePublished:
                eventHandler.scenePublished(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneUnpublished:
                eventHandler.sceneUnpublished(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneFlushed:
                eventHandler.sceneFlushed(sceneId_t(event.sceneId.getValue()), event.sceneVersionTag.getValue(),
                    RamsesRendererUtils::GetResourceStatus(event.resourceStatus));
                break;
            case ramses_internal::ERendererEventType_SceneSubscribed:
                eventHandler.sceneSubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneSubscribeFailed:
                eventHandler.sceneSubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribed:
                eventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribedIndirect:
                eventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnsubscribeFailed:
                eventHandler.sceneUnsubscribed(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneMapped:
                eventHandler.sceneMapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneMapFailed:
                eventHandler.sceneMapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapped:
                eventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneUnmappedIndirect:
                eventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneUnmapFailed:
                eventHandler.sceneUnmapped(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBuffer:
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBufferFailed:
            {
                displayBufferId_t bufferId{ event.offscreenBuffer.asMemoryHandle() };
                if (!event.offscreenBuffer.isValid())
                {
                    // if not assigned to offscreen buffer, it means it was assigned to display's framebuffer - find its HL id
                    const auto it = m_renderer.getDisplayFrameBuffers().find(displayId_t{ event.displayHandle.asMemoryHandle() });
                    if (it != m_renderer.getDisplayFrameBuffers().cend())
                        bufferId = it->second;
                }
                eventHandler.sceneAssignedToDisplayBuffer(sceneId_t(event.sceneId.getValue()), bufferId,
                    (event.eventType == ramses_internal::ERendererEventType_SceneAssignedToDisplayBuffer ? ERendererEventResult_OK : ERendererEventResult_FAIL));
                break;
            }
            case ramses_internal::ERendererEventType_SceneShown:
                eventHandler.sceneShown(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneShowFailed:
                eventHandler.sceneShown(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneHidden:
                eventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneHiddenIndirect:
                eventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneHideFailed:
                eventHandler.sceneHidden(sceneId_t(event.sceneId.getValue()), ramses::ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneExpired:
                eventHandler.sceneExpired(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneRecoveredFromExpiration:
                eventHandler.sceneRecoveredFromExpiration(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneDataLinked:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataLinkFailed:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinked:
                eventHandler.offscreenBufferLinkedToSceneData(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinkFailed:
                eventHandler.offscreenBufferLinkedToSceneData(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinked:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_OK);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_INDIRECT);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkFailed:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), ERendererEventResult_FAIL);
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderCreated:
                eventHandler.dataProviderCreated(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderDestroyed:
                eventHandler.dataProviderDestroyed(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerCreated:
                eventHandler.dataConsumerCreated(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerDestroyed:
                eventHandler.dataConsumerDestroyed(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()));
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceAvailable:
                eventHandler.streamAvailabilityChanged(ramses::streamSource_t(event.streamSourceId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceUnavailable:
                eventHandler.streamAvailabilityChanged(ramses::streamSource_t(event.streamSourceId.getValue()), false);
                break;
            default:
                assert(false);
                return addErrorEntry("RamsesRenderer::dispatchEvents failed - unknown renderer event type!");
            }
        }

        return StatusOK;
    }

    const ramses_internal::RendererCommands& RendererSceneControlImpl_legacy::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }
}
