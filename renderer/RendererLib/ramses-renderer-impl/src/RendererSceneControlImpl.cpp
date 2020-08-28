//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererSceneControlImpl.h"
#include "RamsesRendererImpl.h"
#include "RamsesFrameworkTypesImpl.h"
#include "RendererAPI/Types.h"
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"
#include "Utils/LogMacros.h"

namespace ramses
{
    RendererSceneControlImpl::RendererSceneControlImpl(RamsesRendererImpl& renderer)
        : StatusObjectImpl()
        , m_renderer(renderer)
    {
    }

    status_t RendererSceneControlImpl::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControl::setSceneState: scene " << sceneId << " " << EnumToString(state));

        auto& sceneInfo = m_sceneInfos[sceneId];
        if (state >= RendererSceneState::Ready && !sceneInfo.mappingSet)
            return addErrorEntry("RendererSceneControl::setSceneState: cannot get scene to ready/rendered without mapping info, set mapping info via RendererSceneControl::setSceneMapping first!");
        sceneInfo.targetState = state;

        m_pendingRendererCommands.setSceneState(ramses_internal::SceneId{ sceneId.getValue() }, static_cast<ramses_internal::RendererSceneState>(state));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControl::setSceneMapping: scene " << sceneId << " display " << displayId);

        auto& sceneInfo = m_sceneInfos[sceneId];
        if (sceneInfo.currState >= RendererSceneState::Ready || sceneInfo.targetState >= RendererSceneState::Ready)
            return addErrorEntry("RendererSceneControl::setSceneMapping: cannot change mapping properties, scene's current or desired state already set to READY/RENDERED."
                " Set scene state to AVAILABLE first, adjust mapping properties and then it can be made READY/RENDERED with new mapping properties.");
        sceneInfo.mappingSet = true;

        m_pendingRendererCommands.setSceneMapping(ramses_internal::SceneId{ sceneId.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() });

        return StatusOK;
    }

    status_t RendererSceneControlImpl::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        LOG_INFO(ramses_internal::CONTEXT_RENDERER, "RendererSceneControl::setSceneDisplayBufferAssignment: scene " << sceneId << " displayBuffer " << displayBuffer << " renderOrder " << sceneRenderOrder);

        if (!m_sceneInfos[sceneId].mappingSet)
            return addErrorEntry("RendererSceneControl::setSceneDisplayBufferAssignment: scene does not have valid mapping information, set its mapping first.");

        ramses_internal::OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to assign to is display's framebuffer pass invalid OB to internal renderer
        const auto& frameBuffers = m_renderer.getDisplayFrameBuffers();
        if (std::any_of(frameBuffers.cbegin(), frameBuffers.cend(), [displayBuffer](const auto& d) { return d.second == displayBuffer; }))
            bufferHandle = ramses_internal::OffscreenBufferHandle::Invalid();

        m_pendingRendererCommands.setSceneDisplayBufferAssignment(ramses_internal::SceneId{ sceneId.getValue() }, bufferHandle, sceneRenderOrder);

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::OffscreenBufferHandle providerBuffer{ offscreenBufferId.getValue() };
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        m_pendingRendererCommands.linkBufferToSceneData(providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() });

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        if (providerSceneId == consumerSceneId)
            return addErrorEntry("RendererSceneControl::linkData failed: provider and consumer scene must not be identical");

        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const ramses_internal::SceneId internalProviderSceneId{ providerSceneId.getValue() };
        m_pendingRendererCommands.linkSceneData(internalProviderSceneId, ramses_internal::DataSlotId{ providerId.getValue() }, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerId.getValue() });

        return StatusOK;
    }

    status_t RendererSceneControlImpl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        m_pendingRendererCommands.unlinkSceneData(ramses_internal::SceneId{ consumerSceneId.getValue() }, ramses_internal::DataSlotId{ consumerId.getValue() });
        return StatusOK;
    }

    status_t RendererSceneControlImpl::handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const ramses_internal::Vector2 coords(bufferNormalizedCoordX, bufferNormalizedCoordY);
        const ramses_internal::SceneId sceneId(scene.getValue());
        m_pendingRendererCommands.handlePickEvent(sceneId, coords);
        return StatusOK;
    }

    status_t RendererSceneControlImpl::flush()
    {
        m_renderer.submitRendererCommands(m_pendingRendererCommands);
        m_pendingRendererCommands.clear();
        return StatusOK;
    }

    status_t RendererSceneControlImpl::dispatchEvents(IRendererSceneControlEventHandler& eventHandler)
    {
        m_tempRendererEvents.clear();
        m_renderer.getRenderer().dispatchSceneControlEvents(m_tempRendererEvents);

        for (const auto& event : m_tempRendererEvents)
        {
            switch (event.eventType)
            {
            case ramses_internal::ERendererEventType_ScenePublished:
                eventHandler.scenePublished(sceneId_t{ event.sceneId.getValue() });
                break;
            case ramses_internal::ERendererEventType_SceneStateChanged:
            {
                const sceneId_t sceneId{ event.sceneId.getValue() };
                const auto state = static_cast<RendererSceneState>(event.state);
                m_sceneInfos[sceneId].currState = state;
                if (state == RendererSceneState::Unavailable)
                    // reset stored target state if scene became unavailable
                    m_sceneInfos[sceneId].targetState = RendererSceneState::Unavailable;
                eventHandler.sceneStateChanged(sceneId, state);
                break;
            }
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBuffer:
            case ramses_internal::ERendererEventType_SceneAssignedToDisplayBufferFailed:
                // TODO vaclav - decide if needed, does not have callback atm
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinked:
                eventHandler.offscreenBufferLinked(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, true);
                break;
            case ramses_internal::ERendererEventType_SceneDataBufferLinkFailed:
                eventHandler.offscreenBufferLinked(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, false);
                break;
            case ramses_internal::ERendererEventType_SceneDataLinked:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_SceneDataLinkFailed:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinked:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkedAsResultOfClientSceneChange:
                // TODO vaclav remove this event, not useful
                break;
            case ramses_internal::ERendererEventType_SceneDataUnlinkFailed:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderCreated:
                eventHandler.dataProviderCreated(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotProviderDestroyed:
                eventHandler.dataProviderDestroyed(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerCreated:
                eventHandler.dataConsumerCreated(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType_SceneDataSlotConsumerDestroyed:
                eventHandler.dataConsumerDestroyed(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType_ObjectsPicked:
                static_assert(sizeof(ramses::pickableObjectId_t) == sizeof(std::remove_pointer<decltype(event.pickedObjectIds.data())>::type), "");
                eventHandler.objectsPicked(ramses::sceneId_t(event.sceneId.getValue()), reinterpret_cast<const ramses::pickableObjectId_t*>(event.pickedObjectIds.data()), static_cast<uint32_t>(event.pickedObjectIds.size()));
                break;
            case ramses_internal::ERendererEventType_SceneFlushed:
                eventHandler.sceneFlushed(sceneId_t(event.sceneId.getValue()), event.sceneVersionTag.getValue());
                break;
            case ramses_internal::ERendererEventType_SceneExpirationMonitoringEnabled:
                eventHandler.sceneExpirationMonitoringEnabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneExpirationMonitoringDisabled:
                eventHandler.sceneExpirationMonitoringDisabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneExpired:
                eventHandler.sceneExpired(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_SceneRecoveredFromExpiration:
                eventHandler.sceneRecoveredFromExpiration(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceAvailable:
                eventHandler.streamAvailabilityChanged(ramses::waylandIviSurfaceId_t(event.streamSourceId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType_StreamSurfaceUnavailable:
                eventHandler.streamAvailabilityChanged(ramses::waylandIviSurfaceId_t(event.streamSourceId.getValue()), false);
                break;

            case ramses_internal::ERendererEventType_SceneUnpublished:
            case ramses_internal::ERendererEventType_SceneSubscribed:
            case ramses_internal::ERendererEventType_SceneSubscribeFailed:
            case ramses_internal::ERendererEventType_SceneUnsubscribed:
            case ramses_internal::ERendererEventType_SceneUnsubscribedIndirect:
            case ramses_internal::ERendererEventType_SceneUnsubscribeFailed:
            case ramses_internal::ERendererEventType_SceneMapped:
            case ramses_internal::ERendererEventType_SceneMapFailed:
            case ramses_internal::ERendererEventType_SceneUnmapped:
            case ramses_internal::ERendererEventType_SceneUnmappedIndirect:
            case ramses_internal::ERendererEventType_SceneUnmapFailed:
            case ramses_internal::ERendererEventType_SceneShown:
            case ramses_internal::ERendererEventType_SceneShowFailed:
            case ramses_internal::ERendererEventType_SceneHidden:
            case ramses_internal::ERendererEventType_SceneHiddenIndirect:
            case ramses_internal::ERendererEventType_SceneHideFailed:
                // these events are emitted from internal renderer all the way here to support legacy scene control
                break;

            default:
                assert(false);
                break;
            }
        }

        return StatusOK;
    }

    const ramses_internal::RendererCommands& RendererSceneControlImpl::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }
}
