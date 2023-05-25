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

        if (state == RendererSceneState::Unavailable)
            return addErrorEntry("RendererSceneControl::setSceneState: Can not set scene state Unavailable. In order to release the scene from renderer set Available state.");

        auto& sceneInfo = m_sceneInfos[sceneId];
        if (state >= RendererSceneState::Ready && !sceneInfo.mappingSet)
            return addErrorEntry("RendererSceneControl::setSceneState: cannot get scene to ready/rendered without mapping info, set mapping info via RendererSceneControl::setSceneMapping first!");
        sceneInfo.targetState = state;

        ramses_internal::RendererCommand::SetSceneState cmd{ ramses_internal::SceneId{ sceneId.getValue() }, static_cast<ramses_internal::RendererSceneState>(state) };
        m_pendingRendererCommands.push_back(std::move(cmd));

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

        ramses_internal::RendererCommand::SetSceneMapping cmd{ ramses_internal::SceneId{ sceneId.getValue() }, ramses_internal::DisplayHandle{ displayId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

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

        ramses_internal::RendererCommand::SetSceneDisplayBufferAssignment cmd{ ramses_internal::SceneId{ sceneId.getValue() }, bufferHandle, sceneRenderOrder };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::OffscreenBufferHandle providerBuffer{ offscreenBufferId.getValue() };
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        ramses_internal::RendererCommand::LinkOffscreenBuffer cmd{ providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::StreamBufferHandle providerBuffer{ streamBufferId.getValue() };
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        ramses_internal::RendererCommand::LinkStreamBuffer cmd{ providerBuffer, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ramses_internal::ExternalBufferHandle externalTexHandle{externalBufferId.getValue()};
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        ramses_internal::RendererCommand::LinkExternalBuffer cmd{ externalTexHandle, internalConsumerSceneId, ramses_internal::DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        if (providerSceneId == consumerSceneId)
            return addErrorEntry("RendererSceneControl::linkData failed: provider and consumer scene must not be identical");

        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const ramses_internal::SceneId internalProviderSceneId{ providerSceneId.getValue() };
        const ramses_internal::DataSlotId providerSlot{ providerId.getValue() };
        const ramses_internal::DataSlotId consumerSlot{ consumerId.getValue() };
        ramses_internal::RendererCommand::LinkData cmd{ internalProviderSceneId, providerSlot, internalConsumerSceneId, consumerSlot };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return StatusOK;
    }

    status_t RendererSceneControlImpl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const ramses_internal::SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const ramses_internal::DataSlotId consumerSlot{ consumerId.getValue() };
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::UnlinkData{ internalConsumerSceneId, consumerSlot });
        return StatusOK;
    }

    status_t RendererSceneControlImpl::handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const glm::vec2 coords(bufferNormalizedCoordX, bufferNormalizedCoordY);
        const ramses_internal::SceneId sceneId(scene.getValue());
        m_pendingRendererCommands.push_back(ramses_internal::RendererCommand::PickEvent{ sceneId, coords });
        return StatusOK;
    }

    status_t RendererSceneControlImpl::flush()
    {
        m_renderer.pushAndConsumeRendererCommands(m_pendingRendererCommands);
        return StatusOK;
    }

    status_t RendererSceneControlImpl::dispatchEvents(IRendererSceneControlEventHandler& eventHandler)
    {
        m_tempRendererEvents.clear();
        m_renderer.getDisplayDispatcher().dispatchSceneControlEvents(m_tempRendererEvents);

        for (const auto& event : m_tempRendererEvents)
        {
            switch (event.eventType)
            {
            case ramses_internal::ERendererEventType::SceneStateChanged:
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
            case ramses_internal::ERendererEventType::SceneDataBufferLinked:
                assert(event.offscreenBuffer.isValid()? (!event.streamBuffer.isValid() && !event.externalBuffer.isValid())
                                                        : (event.streamBuffer.isValid() != event.externalBuffer.isValid()));

                if (event.offscreenBuffer.isValid())
                    eventHandler.offscreenBufferLinked(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, true);
                if (event.streamBuffer.isValid())
                    eventHandler.streamBufferLinked(streamBufferId_t(event.streamBuffer.asMemoryHandle()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                if (event.externalBuffer.isValid())
                    eventHandler.externalBufferLinked(externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, true);

                break;
            case ramses_internal::ERendererEventType::SceneDataBufferLinkFailed:
                assert(event.offscreenBuffer.isValid() ? (!event.streamBuffer.isValid() && !event.externalBuffer.isValid())
                    : (event.streamBuffer.isValid() != event.externalBuffer.isValid()));

                if (event.offscreenBuffer.isValid())
                    eventHandler.offscreenBufferLinked(displayBufferId_t { event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, false);
                if (event.streamBuffer.isValid())
                    eventHandler.streamBufferLinked(streamBufferId_t(event.streamBuffer.asMemoryHandle()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                if (event.externalBuffer.isValid())
                    eventHandler.externalBufferLinked(externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, false);

                break;
            case ramses_internal::ERendererEventType::SceneDataLinked:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType::SceneDataLinkFailed:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ramses_internal::ERendererEventType::SceneDataUnlinked:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange:
                // TODO vaclav remove this event, not useful
                break;
            case ramses_internal::ERendererEventType::SceneDataUnlinkFailed:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ramses_internal::ERendererEventType::SceneDataSlotProviderCreated:
                eventHandler.dataProviderCreated(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType::SceneDataSlotProviderDestroyed:
                eventHandler.dataProviderDestroyed(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType::SceneDataSlotConsumerCreated:
                eventHandler.dataConsumerCreated(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType::SceneDataSlotConsumerDestroyed:
                eventHandler.dataConsumerDestroyed(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ramses_internal::ERendererEventType::ObjectsPicked:
                static_assert(sizeof(ramses::pickableObjectId_t) == sizeof(std::remove_pointer<decltype(event.pickedObjectIds.data())>::type), "");
                eventHandler.objectsPicked(ramses::sceneId_t(event.sceneId.getValue()), reinterpret_cast<const ramses::pickableObjectId_t*>(event.pickedObjectIds.data()), static_cast<uint32_t>(event.pickedObjectIds.size()));
                break;
            case ramses_internal::ERendererEventType::SceneFlushed:
                eventHandler.sceneFlushed(sceneId_t(event.sceneId.getValue()), event.sceneVersionTag.getValue());
                break;
            case ramses_internal::ERendererEventType::SceneExpirationMonitoringEnabled:
                eventHandler.sceneExpirationMonitoringEnabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType::SceneExpirationMonitoringDisabled:
                eventHandler.sceneExpirationMonitoringDisabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType::SceneExpired:
                eventHandler.sceneExpired(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType::SceneRecoveredFromExpiration:
                eventHandler.sceneRecoveredFromExpiration(sceneId_t(event.sceneId.getValue()));
                break;
            case ramses_internal::ERendererEventType::StreamSurfaceAvailable:
                eventHandler.streamAvailabilityChanged(ramses::waylandIviSurfaceId_t(event.streamSourceId.getValue()), true);
                break;
            case ramses_internal::ERendererEventType::StreamSurfaceUnavailable:
                eventHandler.streamAvailabilityChanged(ramses::waylandIviSurfaceId_t(event.streamSourceId.getValue()), false);
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
