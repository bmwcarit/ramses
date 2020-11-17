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
#include "ramses-renderer-api/IRendererSceneControlEventHandler.h"

namespace ramses
{
    class RendererEventChainer final : public IRendererEventHandler
    {
    public:
        RendererEventChainer(IRendererEventHandler& handler1, IRendererEventHandler& handler2)
            : m_handler1(handler1)
            , m_handler2(handler2)
        {
        }

        virtual void offscreenBufferCreated(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            m_handler1.offscreenBufferCreated(displayId, offscreenBufferId, result);
            m_handler2.offscreenBufferCreated(displayId, offscreenBufferId, result);
        }

        virtual void offscreenBufferDestroyed(displayId_t displayId, displayBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            m_handler1.offscreenBufferDestroyed(displayId, offscreenBufferId, result);
            m_handler2.offscreenBufferDestroyed(displayId, offscreenBufferId, result);
        }

        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, displayBufferId_t displayBuffer, ERendererEventResult result) override
        {
            m_handler1.framebufferPixelsRead(pixelData, pixelDataSize, displayId, displayBuffer, result);
            m_handler2.framebufferPixelsRead(pixelData, pixelDataSize, displayId, displayBuffer, result);
        }

        virtual void warpingMeshDataUpdated(displayId_t displayId, ERendererEventResult result) override
        {
            m_handler1.warpingMeshDataUpdated(displayId, result);
            m_handler2.warpingMeshDataUpdated(displayId, result);
        }

        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) override
        {
            m_handler1.displayCreated(displayId, result);
            m_handler2.displayCreated(displayId, result);
        }

        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) override
        {
            m_handler1.displayDestroyed(displayId, result);
            m_handler2.displayDestroyed(displayId, result);
        }

        virtual void keyEvent(displayId_t displayId, EKeyEvent eventType, uint32_t keyModifiers, EKeyCode keyCode) override
        {
            m_handler1.keyEvent(displayId, eventType, keyModifiers, keyCode);
            m_handler2.keyEvent(displayId, eventType, keyModifiers, keyCode);
        }

        virtual void mouseEvent(displayId_t displayId, EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
        {
            m_handler1.mouseEvent(displayId, eventType, mousePosX, mousePosY);
            m_handler2.mouseEvent(displayId, eventType, mousePosX, mousePosY);
        }

        virtual void windowClosed(displayId_t displayId) override
        {
            m_handler1.windowClosed(displayId);
            m_handler2.windowClosed(displayId);
        }

        virtual void windowResized(displayId_t displayId, uint32_t width, uint32_t height) override
        {
            m_handler1.windowResized(displayId, width, height);
            m_handler2.windowResized(displayId, width, height);
        }

        virtual void windowMoved(displayId_t displayId, int32_t posX, int32_t posY) override
        {
            m_handler1.windowMoved(displayId, posX, posY);
            m_handler2.windowMoved(displayId, posX, posY);
        }

        virtual void renderThreadLoopTimings(std::chrono::microseconds maximumLoopTime, std::chrono::microseconds averageLooptime) override
        {
            m_handler1.renderThreadLoopTimings(maximumLoopTime, averageLooptime);
            m_handler2.renderThreadLoopTimings(maximumLoopTime, averageLooptime);
        }

    private:
        IRendererEventHandler& m_handler1;
        IRendererEventHandler& m_handler2;
    };

    class RendererSceneControlEventChainer final : public IRendererSceneControlEventHandler
    {
    public:
        RendererSceneControlEventChainer(IRendererSceneControlEventHandler& handler1, IRendererSceneControlEventHandler& handler2)
            : m_handler1(handler1)
            , m_handler2(handler2)
        {
        }

        virtual void sceneStateChanged(sceneId_t sceneId, RendererSceneState state) override
        {
            m_handler1.sceneStateChanged(sceneId, state);
            m_handler2.sceneStateChanged(sceneId, state);
        }

        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, bool result) override
        {
            m_handler1.dataLinked(providerScene, providerId, consumerScene, consumerId, result);
            m_handler2.dataLinked(providerScene, providerId, consumerScene, consumerId, result);
        }

        virtual void objectsPicked(sceneId_t sceneId, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
        {
            m_handler1.objectsPicked(sceneId, pickedObjects, pickedObjectsCount);
            m_handler2.objectsPicked(sceneId, pickedObjects, pickedObjectsCount);
        }

        virtual void offscreenBufferLinked(displayBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, bool result) override
        {
            m_handler1.offscreenBufferLinked(providerOffscreenBuffer, consumerScene, consumerId, result);
            m_handler2.offscreenBufferLinked(providerOffscreenBuffer, consumerScene, consumerId, result);
        }

        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, bool result) override
        {
            m_handler1.dataUnlinked(consumerScene, consumerId, result);
            m_handler2.dataUnlinked(consumerScene, consumerId, result);
        }

        void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            m_handler1.dataProviderCreated(sceneId, dataProviderId);
            m_handler2.dataProviderCreated(sceneId, dataProviderId);
        }

        void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            m_handler1.dataProviderDestroyed(sceneId, dataProviderId);
            m_handler2.dataProviderDestroyed(sceneId, dataProviderId);
        }

        void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            m_handler1.dataConsumerCreated(sceneId, dataConsumerId);
            m_handler2.dataConsumerCreated(sceneId, dataConsumerId);
        }

        void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            m_handler1.dataConsumerDestroyed(sceneId, dataConsumerId);
            m_handler2.dataConsumerDestroyed(sceneId, dataConsumerId);
        }

        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag) override
        {
            m_handler1.sceneFlushed(sceneId, sceneVersionTag);
            m_handler2.sceneFlushed(sceneId, sceneVersionTag);
        }

        virtual void sceneExpirationMonitoringEnabled(sceneId_t sceneId) override
        {
            m_handler1.sceneExpirationMonitoringEnabled(sceneId);
            m_handler2.sceneExpirationMonitoringEnabled(sceneId);
        }

        virtual void sceneExpirationMonitoringDisabled(sceneId_t sceneId) override
        {
            m_handler1.sceneExpirationMonitoringDisabled(sceneId);
            m_handler2.sceneExpirationMonitoringDisabled(sceneId);
        }

        virtual void sceneExpired(sceneId_t sceneId) override
        {
            m_handler1.sceneExpired(sceneId);
            m_handler2.sceneExpired(sceneId);
        }

        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override
        {
            m_handler1.sceneRecoveredFromExpiration(sceneId);
            m_handler2.sceneRecoveredFromExpiration(sceneId);
        }

        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override
        {
            m_handler1.streamAvailabilityChanged(streamId, available);
            m_handler2.streamAvailabilityChanged(streamId, available);
        }

    private:
        IRendererSceneControlEventHandler& m_handler1;
        IRendererSceneControlEventHandler& m_handler2;
    };
}

#endif
