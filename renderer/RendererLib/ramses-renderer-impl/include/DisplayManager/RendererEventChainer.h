//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDEREREVENTCHAINER_H
#define RAMSES_RENDEREREVENTCHAINER_H

#include "ramses-renderer-api/IRendererEventHandler.h"

namespace ramses_display_manager
{
    class RendererEventChainer final : public ramses::IRendererEventHandler
    {
    public:
        RendererEventChainer(ramses::IRendererEventHandler& handler1, ramses::IRendererEventHandler& handler2)
            : m_handler1(handler1)
            , m_handler2(handler2)
        {
        }

        virtual void scenePublished(ramses::sceneId_t sceneId) override
        {
            m_handler1.scenePublished(sceneId);
            m_handler2.scenePublished(sceneId);
        }

        virtual void sceneSubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneSubscribed(sceneId, result);
            m_handler2.sceneSubscribed(sceneId, result);
        }

        virtual void sceneMapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneMapped(sceneId, result);
            m_handler2.sceneMapped(sceneId, result);
        }

        virtual void sceneShown(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneShown(sceneId, result);
            m_handler2.sceneShown(sceneId, result);
        }

        virtual void sceneUnpublished(ramses::sceneId_t sceneId) override
        {
            m_handler1.sceneUnpublished(sceneId);
            m_handler2.sceneUnpublished(sceneId);
        }

        virtual void sceneUnsubscribed(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneUnsubscribed(sceneId, result);
            m_handler2.sceneUnsubscribed(sceneId, result);
        }

        virtual void sceneUnmapped(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneUnmapped(sceneId, result);
            m_handler2.sceneUnmapped(sceneId, result);
        }

        virtual void sceneHidden(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneHidden(sceneId, result);
            m_handler2.sceneHidden(sceneId, result);
        }

        virtual void dataLinked(ramses::sceneId_t providerScene, ramses::dataProviderId_t providerId, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, ramses::ERendererEventResult result) override
        {
            m_handler1.dataLinked(providerScene, providerId, consumerScene, consumerId, result);
            m_handler2.dataLinked(providerScene, providerId, consumerScene, consumerId, result);
        }

        virtual void offscreenBufferLinkedToSceneData(ramses::offscreenBufferId_t providerOffscreenBuffer, ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, ramses::ERendererEventResult result) override
        {
            m_handler1.offscreenBufferLinkedToSceneData(providerOffscreenBuffer, consumerScene, consumerId, result);
            m_handler2.offscreenBufferLinkedToSceneData(providerOffscreenBuffer, consumerScene, consumerId, result);
        }

        virtual void dataUnlinked(ramses::sceneId_t consumerScene, ramses::dataConsumerId_t consumerId, ramses::ERendererEventResult result) override
        {
            m_handler1.dataUnlinked(consumerScene, consumerId, result);
            m_handler2.dataUnlinked(consumerScene, consumerId, result);
        }

        virtual void offscreenBufferCreated(ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
        {
            m_handler1.offscreenBufferCreated(displayId, offscreenBufferId, result);
            m_handler2.offscreenBufferCreated(displayId, offscreenBufferId, result);
        }

        virtual void offscreenBufferDestroyed(ramses::displayId_t displayId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
        {
            m_handler1.offscreenBufferDestroyed(displayId, offscreenBufferId, result);
            m_handler2.offscreenBufferDestroyed(displayId, offscreenBufferId, result);
        }

        virtual void sceneAssignedToOffscreenBuffer(ramses::sceneId_t sceneId, ramses::offscreenBufferId_t offscreenBufferId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneAssignedToOffscreenBuffer(sceneId, offscreenBufferId, result);
            m_handler2.sceneAssignedToOffscreenBuffer(sceneId, offscreenBufferId, result);
        }

        virtual void sceneAssignedToFramebuffer(ramses::sceneId_t sceneId, ramses::ERendererEventResult result) override
        {
            m_handler1.sceneAssignedToFramebuffer(sceneId, result);
            m_handler2.sceneAssignedToFramebuffer(sceneId, result);
        }

        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            m_handler1.framebufferPixelsRead(pixelData, pixelDataSize, displayId, result);
            m_handler2.framebufferPixelsRead(pixelData, pixelDataSize, displayId, result);
        }

        virtual void warpingMeshDataUpdated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            m_handler1.warpingMeshDataUpdated(displayId, result);
            m_handler2.warpingMeshDataUpdated(displayId, result);
        }

        virtual void displayCreated(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            m_handler1.displayCreated(displayId, result);
            m_handler2.displayCreated(displayId, result);
        }

        virtual void displayDestroyed(ramses::displayId_t displayId, ramses::ERendererEventResult result) override
        {
            m_handler1.displayDestroyed(displayId, result);
            m_handler2.displayDestroyed(displayId, result);
        }

        virtual void dataProviderCreated(ramses::sceneId_t sceneId, ramses::dataProviderId_t dataProviderId) override
        {
            m_handler1.dataProviderCreated(sceneId, dataProviderId);
            m_handler2.dataProviderCreated(sceneId, dataProviderId);
        }

        virtual void dataProviderDestroyed(ramses::sceneId_t sceneId, ramses::dataProviderId_t dataProviderId) override
        {
            m_handler1.dataProviderDestroyed(sceneId, dataProviderId);
            m_handler2.dataProviderDestroyed(sceneId, dataProviderId);
        }

        virtual void dataConsumerCreated(ramses::sceneId_t sceneId, ramses::dataConsumerId_t dataConsumerId) override
        {
            m_handler1.dataConsumerCreated(sceneId, dataConsumerId);
            m_handler2.dataConsumerCreated(sceneId, dataConsumerId);
        }

        virtual void dataConsumerDestroyed(ramses::sceneId_t sceneId, ramses::dataConsumerId_t dataConsumerId) override
        {
            m_handler1.dataConsumerDestroyed(sceneId, dataConsumerId);
            m_handler2.dataConsumerDestroyed(sceneId, dataConsumerId);
        }

        virtual void sceneFlushed(ramses::sceneId_t sceneId, ramses::sceneVersionTag_t sceneVersionTag, ramses::ESceneResourceStatus resourceStatus) override
        {
            m_handler1.sceneFlushed(sceneId, sceneVersionTag, resourceStatus);
            m_handler2.sceneFlushed(sceneId, sceneVersionTag, resourceStatus);
        }

        virtual void sceneExpired(ramses::sceneId_t sceneId) override
        {
            m_handler1.sceneExpired(sceneId);
            m_handler2.sceneExpired(sceneId);
        }

        virtual void sceneRecoveredFromExpiration(ramses::sceneId_t sceneId) override
        {
            m_handler1.sceneRecoveredFromExpiration(sceneId);
            m_handler2.sceneRecoveredFromExpiration(sceneId);
        }

        virtual void streamAvailabilityChanged(ramses::streamSource_t streamId, bool available) override
        {
            m_handler1.streamAvailabilityChanged(streamId, available);
            m_handler2.streamAvailabilityChanged(streamId, available);
        }

        virtual void keyEvent(ramses::displayId_t displayId, ramses::EKeyEvent eventType, uint32_t keyModifiers, ramses::EKeyCode keyCode) override
        {
            m_handler1.keyEvent(displayId, eventType, keyModifiers, keyCode);
            m_handler2.keyEvent(displayId, eventType, keyModifiers, keyCode);
        }

        virtual void mouseEvent(ramses::displayId_t displayId, ramses::EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
        {
            m_handler1.mouseEvent(displayId, eventType, mousePosX, mousePosY);
            m_handler2.mouseEvent(displayId, eventType, mousePosX, mousePosY);
        }

        virtual void windowClosed(ramses::displayId_t displayId) override
        {
            m_handler1.windowClosed(displayId);
            m_handler2.windowClosed(displayId);
        }

        virtual void windowResized(ramses::displayId_t displayId, uint32_t width, uint32_t height) override
        {
            m_handler1.windowResized(displayId, width, height);
            m_handler2.windowResized(displayId, width, height);
        }

    private:
        ramses::IRendererEventHandler& m_handler1;
        ramses::IRendererEventHandler& m_handler2;
    };
}

#endif
