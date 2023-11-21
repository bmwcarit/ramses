//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RendererSceneControlImpl.h"
#include "impl/RamsesRendererImpl.h"
#include "impl/RamsesFrameworkTypesImpl.h"
#include "impl/ErrorReporting.h"
#include "ramses/renderer/IRendererSceneControlEventHandler.h"
#include "internal/RendererLib/Types.h"
#include "internal/Core/Utils/LogMacros.h"

namespace ramses::internal
{
    RendererSceneControlImpl::RendererSceneControlImpl(RamsesRendererImpl& renderer)
        : m_renderer(renderer)
    {
    }

    RendererSceneControlImpl::~RendererSceneControlImpl() = default;

    bool RendererSceneControlImpl::setSceneState(sceneId_t sceneId, RendererSceneState state)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererSceneControl::setSceneState: scene {} {}", sceneId, EnumToString(state));

        if (state == RendererSceneState::Unavailable)
        {
            m_renderer.getErrorReporting().set("RendererSceneControl::setSceneState: Can not set scene state Unavailable. In order to release the scene from renderer set Available state.");
            return false;
        }

        auto& sceneInfo = m_sceneInfos[sceneId];
        if (state >= RendererSceneState::Ready && !sceneInfo.mappingSet)
        {
            m_renderer.getErrorReporting().set("RendererSceneControl::setSceneState: cannot get scene to ready/rendered without mapping info, set mapping info via RendererSceneControl::setSceneMapping first!");
            return false;
        }
        sceneInfo.targetState = state;

        RendererCommand::SetSceneState cmd{ SceneId{ sceneId.getValue() }, state };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::setSceneMapping(sceneId_t sceneId, displayId_t displayId)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererSceneControl::setSceneMapping: scene {} display {}", sceneId, displayId);

        auto& sceneInfo = m_sceneInfos[sceneId];
        if (sceneInfo.currState >= RendererSceneState::Ready || sceneInfo.targetState >= RendererSceneState::Ready)
        {
            m_renderer.getErrorReporting().set("RendererSceneControl::setSceneMapping: cannot change mapping properties, scene's current or desired state already set to READY/RENDERED."
                " Set scene state to AVAILABLE first, adjust mapping properties and then it can be made READY/RENDERED with new mapping properties.");
            return false;
        }

        sceneInfo.mappingSet = true;

        RendererCommand::SetSceneMapping cmd{ SceneId{ sceneId.getValue() }, DisplayHandle{ displayId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::setSceneDisplayBufferAssignment(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder)
    {
        LOG_INFO(CONTEXT_RENDERER, "RendererSceneControl::setSceneDisplayBufferAssignment: scene {} displayBuffer {} renderOrder {}", sceneId, displayBuffer, sceneRenderOrder);

        if (!m_sceneInfos[sceneId].mappingSet)
        {
            m_renderer.getErrorReporting().set("RendererSceneControl::setSceneDisplayBufferAssignment: scene does not have valid mapping information, set its mapping first.");
            return false;
        }

        OffscreenBufferHandle bufferHandle{ displayBuffer.getValue() };
        // if buffer to assign to is display's framebuffer pass invalid OB to internal renderer
        const auto& frameBuffers = m_renderer.getDisplayFrameBuffers();
        if (std::any_of(frameBuffers.cbegin(), frameBuffers.cend(), [displayBuffer](const auto& d) { return d.second == displayBuffer; }))
            bufferHandle = OffscreenBufferHandle::Invalid();

        RendererCommand::SetSceneDisplayBufferAssignment cmd{ SceneId{ sceneId.getValue() }, bufferHandle, sceneRenderOrder };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::linkOffscreenBuffer(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const OffscreenBufferHandle providerBuffer{ offscreenBufferId.getValue() };
        const SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        RendererCommand::LinkOffscreenBuffer cmd{ providerBuffer, internalConsumerSceneId, DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::linkStreamBuffer(streamBufferId_t streamBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const StreamBufferHandle providerBuffer{ streamBufferId.getValue() };
        const SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        RendererCommand::LinkStreamBuffer cmd{ providerBuffer, internalConsumerSceneId, DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::linkExternalBuffer(externalBufferId_t externalBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId)
    {
        const ExternalBufferHandle externalTexHandle{externalBufferId.getValue()};
        const SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        RendererCommand::LinkExternalBuffer cmd{ externalTexHandle, internalConsumerSceneId, DataSlotId{ consumerDataSlotId.getValue() } };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::linkData(sceneId_t providerSceneId, dataProviderId_t providerId, sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        if (providerSceneId == consumerSceneId)
        {
            m_renderer.getErrorReporting().set("RendererSceneControl::linkData failed: provider and consumer scene must not be identical");
            return false;
        }

        const SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const SceneId internalProviderSceneId{ providerSceneId.getValue() };
        const DataSlotId providerSlot{ providerId.getValue() };
        const DataSlotId consumerSlot{ consumerId.getValue() };
        RendererCommand::LinkData cmd{ internalProviderSceneId, providerSlot, internalConsumerSceneId, consumerSlot };
        m_pendingRendererCommands.push_back(std::move(cmd));

        return true;
    }

    bool RendererSceneControlImpl::unlinkData(sceneId_t consumerSceneId, dataConsumerId_t consumerId)
    {
        const SceneId internalConsumerSceneId{ consumerSceneId.getValue() };
        const DataSlotId consumerSlot{ consumerId.getValue() };
        m_pendingRendererCommands.push_back(RendererCommand::UnlinkData{ internalConsumerSceneId, consumerSlot });
        return true;
    }

    bool RendererSceneControlImpl::handlePickEvent(sceneId_t scene, float bufferNormalizedCoordX, float bufferNormalizedCoordY)
    {
        const glm::vec2 coords(bufferNormalizedCoordX, bufferNormalizedCoordY);
        const SceneId sceneId(scene.getValue());
        m_pendingRendererCommands.push_back(RendererCommand::PickEvent{ sceneId, coords });
        return true;
    }

    bool RendererSceneControlImpl::flush()
    {
        m_renderer.pushAndConsumeRendererCommands(m_pendingRendererCommands);
        return true;
    }

    bool RendererSceneControlImpl::dispatchEvents(IRendererSceneControlEventHandler& eventHandler)
    {
        m_tempRendererEvents.clear();
        m_renderer.getDisplayDispatcher().dispatchSceneControlEvents(m_tempRendererEvents);

        for (const auto& event : m_tempRendererEvents)
        {
            switch (event.eventType)
            {
            case ERendererEventType::SceneStateChanged:
            {
                const sceneId_t sceneId{ event.sceneId.getValue() };
                const auto state = static_cast<RendererSceneState>(event.state);
                m_sceneInfos[sceneId].currState = state;
                if (state == RendererSceneState::Unavailable)
                {
                    // reset stored target state if scene became unavailable
                    m_sceneInfos[sceneId].targetState = RendererSceneState::Unavailable;
                }
                eventHandler.sceneStateChanged(sceneId, state);
                break;
            }
            case ERendererEventType::SceneDataBufferLinked:
                assert(event.offscreenBuffer.isValid()? (!event.streamBuffer.isValid() && !event.externalBuffer.isValid())
                                                        : (event.streamBuffer.isValid() != event.externalBuffer.isValid()));

                if (event.offscreenBuffer.isValid())
                    eventHandler.offscreenBufferLinked(displayBufferId_t{ event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, true);
                if (event.streamBuffer.isValid())
                    eventHandler.streamBufferLinked(streamBufferId_t(event.streamBuffer.asMemoryHandle()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                if (event.externalBuffer.isValid())
                    eventHandler.externalBufferLinked(externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, true);

                break;
            case ERendererEventType::SceneDataBufferLinkFailed:
                assert(event.offscreenBuffer.isValid() ? (!event.streamBuffer.isValid() && !event.externalBuffer.isValid())
                    : (event.streamBuffer.isValid() != event.externalBuffer.isValid()));

                if (event.offscreenBuffer.isValid())
                    eventHandler.offscreenBufferLinked(displayBufferId_t { event.offscreenBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, false);
                if (event.streamBuffer.isValid())
                    eventHandler.streamBufferLinked(streamBufferId_t(event.streamBuffer.asMemoryHandle()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                if (event.externalBuffer.isValid())
                    eventHandler.externalBufferLinked(externalBufferId_t{ event.externalBuffer.asMemoryHandle() }, sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() }, false);

                break;
            case ERendererEventType::SceneDataLinked:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ERendererEventType::SceneDataLinkFailed:
                eventHandler.dataLinked(sceneId_t(event.providerSceneId.getValue()), dataProviderId_t(event.providerdataId.getValue()), sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ERendererEventType::SceneDataUnlinked:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), true);
                break;
            case ERendererEventType::SceneDataUnlinkedAsResultOfClientSceneChange:
                // TODO vaclav remove this event, not useful
                break;
            case ERendererEventType::SceneDataUnlinkFailed:
                eventHandler.dataUnlinked(sceneId_t(event.consumerSceneId.getValue()), dataConsumerId_t(event.consumerdataId.getValue()), false);
                break;
            case ERendererEventType::SceneDataSlotProviderCreated:
                eventHandler.dataProviderCreated(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ERendererEventType::SceneDataSlotProviderDestroyed:
                eventHandler.dataProviderDestroyed(sceneId_t{ event.providerSceneId.getValue() }, dataProviderId_t{ event.providerdataId.getValue() });
                break;
            case ERendererEventType::SceneDataSlotConsumerCreated:
                eventHandler.dataConsumerCreated(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ERendererEventType::SceneDataSlotConsumerDestroyed:
                eventHandler.dataConsumerDestroyed(sceneId_t{ event.consumerSceneId.getValue() }, dataConsumerId_t{ event.consumerdataId.getValue() });
                break;
            case ERendererEventType::ObjectsPicked:
                static_assert(sizeof(pickableObjectId_t) == sizeof(std::remove_pointer<decltype(event.pickedObjectIds.data())>::type));
                eventHandler.objectsPicked(sceneId_t(event.sceneId.getValue()), reinterpret_cast<const pickableObjectId_t*>(event.pickedObjectIds.data()), event.pickedObjectIds.size());
                break;
            case ERendererEventType::SceneFlushed:
                eventHandler.sceneFlushed(sceneId_t(event.sceneId.getValue()), event.sceneVersionTag.getValue());
                break;
            case ERendererEventType::SceneExpirationMonitoringEnabled:
                eventHandler.sceneExpirationMonitoringEnabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ERendererEventType::SceneExpirationMonitoringDisabled:
                eventHandler.sceneExpirationMonitoringDisabled(sceneId_t(event.sceneId.getValue()));
                break;
            case ERendererEventType::SceneExpired:
                eventHandler.sceneExpired(sceneId_t(event.sceneId.getValue()));
                break;
            case ERendererEventType::SceneRecoveredFromExpiration:
                eventHandler.sceneRecoveredFromExpiration(sceneId_t(event.sceneId.getValue()));
                break;
            case ERendererEventType::StreamSurfaceAvailable:
                eventHandler.streamAvailabilityChanged(waylandIviSurfaceId_t(event.streamSourceId.getValue()), true);
                break;
            case ERendererEventType::StreamSurfaceUnavailable:
                eventHandler.streamAvailabilityChanged(waylandIviSurfaceId_t(event.streamSourceId.getValue()), false);
                break;
            default:
                assert(false);
                break;
            }
        }

        return true;
    }

    const RendererCommands& RendererSceneControlImpl::getPendingCommands() const
    {
        return m_pendingRendererCommands;
    }
}
