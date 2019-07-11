//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDEREREVENTHANDLER_H
#define RAMSES_IRENDEREREVENTHANDLER_H

#include "Types.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
    * @brief Provides an interface for handling the result of renderer events.
    *        Implementation of this interface must be passed to RamsesRenderer::dispatchEvents
    *        which will in return invoke methods of the interface according to events that occurred since last dispatching.
    */
    class RAMSES_API IRendererEventHandler
    {
    public:
        /**
        * @brief This method will be called when a scene gets published by the client.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void scenePublished(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called when a scene subscription result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the subscription operation. The value is ERendererEventResult_OK
        * if subscription was successful or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneSubscribed(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a scene mapping result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the mapping operation. The value is ERendererEventResult_OK
        * if mapping was successful or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneMapped(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a scene show result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the show operation. The value is ERendererEventResult_OK
        * if mapping was successful or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneShown(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a scene gets unpublished by the client.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneUnpublished(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called when a scene unsubscription result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the unsubscription operation. The value is ERendererEventResult_OK
        * if unsubscription was successful as a result of explicit unsubscription request from the renderer, ERendererEventResult_INDIRECT
        * if unsubscription was successful as a result of unpublish event received from the client or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a scene unmapping result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the unmapping operation. The value is ERendererEventResult_OK
        * if unmapping was successful as a result of explicit unmapping request from the renderer, ERendererEventResult_INDIRECT
        * if unmapping was successful as a result of unpublish event received from the client or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneUnmapped(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a scene hide result is available
        * @param sceneId The scene id of the scene on which the event occurred
        * @param result The result of the hide operation. The value is ERendererEventResult_OK
        * if unmapping was successful as a result of explicit unmapping request from the renderer, ERendererEventResult_INDIRECT
        * if unmapping was successful as a result of unpublish event received from the client or ERendererEventResult_FAIL otherwise
        */
        virtual void sceneHidden(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a data link is created (or failed to create) as a result of RamsesRenderer API linkData call.
        *
        * @param providerScene The id of the scene which provides the data.
        * @param providerId The id of the data provider within the providerScene.
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a data link between offscreen buffer as provider and consumer scene's data is created (or failed to create) as a result of RamsesRenderer API \c linkOffscreenBufferToSceneData call.
        *        Note: Unlinking offscreen buffer uses the same event as any other type of data linking - \c dataUnlinked.
        *
        * @param providerOffscreenBuffer The id of the offscreen buffer which provides the data.
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void offscreenBufferLinkedToSceneData(offscreenBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a data link is removed (or failed to remove) as a result of RamsesRenderer API unlinkData call.
        *
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed,
        * ERendererEventResult_INDIRECT if the unlink happened as a result of client scene change - eg. node or data provider/consumer that was involved in the data link was deleted on client side.
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after an offscreen buffer is created (or failed to be created) as a result of RamsesRenderer API \c createOffscreenBuffer call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param offscreenBufferId The id of the offscreen buffer that was created or failed to be created.
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void offscreenBufferCreated(displayId_t displayId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after an offscreen buffer is destroyed (or failed to be destroyed) as a result of RamsesRenderer API \c destroyOffscreenBuffer call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param offscreenBufferId The id of the offscreen buffer that was destroyed or failed to be destroyed.
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void offscreenBufferDestroyed(displayId_t displayId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a scene is assigned to an offscreen buffer (or failed to be assigned) as a result of RamsesRenderer API \c assignSceneToOffscreenBuffer call.
        *
        * @param sceneId The id of the scene that was assigned to the offscreen buffer (or failed to be assigned).
        * @param offscreenBufferId The id of the offscreen buffer that the scene was assigned to (or failed to be assigned to).
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void sceneAssignedToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a scene is assigned to display's framebuffer (or failed to be assigned) as a result of RamsesRenderer API \c assignSceneToFramebuffer call.
        *
        * @param sceneId The id of the scene that was assigned to the framebuffer (or failed to be assigned).
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void sceneAssignedToFramebuffer(sceneId_t sceneId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a read back of pixels from framebuffer
        *        was finished. This is the result of RamsesRenderer::readPixels call which
        *        triggers an asynchronous read back from the internal device.
        * @param pixelData Pointer to the pixel data in uncompressed RGBA8 format.
        *                  Check result and pixelDataSize first to determine the state and size of the data.
        *                  The data is available at the pointer only during the dispatch of this event.
        *                  The pointer is NULL in case of failure.
        * @param pixelDataSize The number of elements in the data array pixelData, ie. number of pixels * 4 (color channels).
        *                      The size is 0 in case of failure.
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when update of warping mesh data was finished.
        *        This is the result of RamsesRenderer::updateWarpingMeshData call which
        *        triggers an asynchronous update of warping data used by internal display.
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void warpingMeshDataUpdated(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a display was created (or failed to create) as a result of RamsesRenderer API createDisplay call.
        *
        * @param displayId id of the display that was created and initialized (or failed in case of error).
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called after a display was destroyed (or failed to destroy) as a result of RamsesRenderer API destroyDisplay call.
        *
        * @param displayId Display id of display that the callback refers to.
        * @param result Can be ERendererEventResult_OK if succeeded or ERendererEventResult_FAIL if failed.
        */
        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) = 0;

        /**
        * @brief This method will be called when a data provider is created
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataProviderId The created data provider id
        */
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief This method will be called when a data provider is destroyed
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataProviderId The destroyed data provider id
        */
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief This method will be called when a data consumer is created
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataConsumerId The created data consumer id
        */
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) = 0;

        /**
        * @brief This method will be called when a data consumer is destroyed
        * @param sceneId The scene id of the scene on which the event occurred
        * @param dataConsumerId The destroyed data consumer id
        */
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) = 0;

        /**
        * @brief This method will be called when a named scene transaction (aka flush) has been applied
        * @param sceneId The scene id of the scene on which the event occurred
        * @param sceneVersionTag The name of the scene transaction
        * @param resourceStatus The current state of the resources to be fetched/updated for the scene transaction
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag, ESceneResourceStatus resourceStatus) = 0;

        /**
        * @brief This method will be called if a scene which has an expiration timestamp set (see \c Scene::setExpirationTimestamp)
        *        is on renderer (rendered or just subscribed) at a state that expired, i.e. current time is after the expiration timestamp.
        *        This callback is called only once when the scene expires even if scene stays expired in subsequent frames.
        *        When the scene is updated again with a new valid expiration timestamp, \c sceneRecoveredFromExpiration is called.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneExpired(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called if a scene which previously expired (see \c Scene::setExpirationTimestamp and \c sceneExpired)
        *        was updated with a new expiration timestamp that is not expired anymore.
        *        This callback is called only once when the scene switches state from expired to not expired.
        * @param sceneId The scene id of the scene on which the event occurred
        */
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) = 0;

        /**
        * @brief This method will be called when a new IVI video stream becomes available, or when an existing stream disappears
        * In terms of Wayland protocol, a stream is available if an "ivi_application" exists which has created a wayland surface
        * (wl_surface) with ivi_id=streamId, and the surface has at least one attached non-NULL buffer (i.e. renderable content).
        *
        * It is possible that the ivi_application does not update its surface (by providing new buffers/frames), but RAMSES has
        * no way of knowing that, hence a stream is _NOT_ reported unavailable in that case.
        *
        * A surface becomes unavailable whenever either the ivi_application is destroyed, or when it crashed, or when it attached
        * a NULL buffer to the stream surface with id=streamId (i.e. actively told wayland that it should not render contents to
        * this ivi surface).
        *
        * @param streamId The IVI stream id
        * @param available True if the stream became available, and false if it disappeared
        */
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) = 0;

        /**
        * @brief This method will be called when a key has been pressed while a display's window was focused
        * @param displayId The display on which the event occurred
        * @param eventType Specifies which type of key event has occurred
        * @param keyModifiers Modifiers used while pressing a key (bit mask of EKeyModifier)
        * @param keyCode The actual key which was pressed
        */
        virtual void keyEvent(displayId_t displayId, EKeyEvent eventType, uint32_t keyModifiers, EKeyCode keyCode) = 0;


        /**
        * @brief This method will be called when a mouse event action has occured while a display's window was focused
        * @param displayId The display on which the event occurred
        * @param eventType Specifies which kind of mouse action has occurred
        * @param mousePosX Horizontal mouse position related to window (left = 0)
        * @param mousePosY Vertical mouse position related to window (top = 0)
        */
        virtual void mouseEvent(displayId_t displayId, EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) = 0;

        /**
        * @brief This method will be called when a display's window has been resized
        * @param displayId The ramses display whose corresponding window was resized
        * @param width The new width of the window
        * @param height The new height of the window
        */
        virtual void windowResized(displayId_t displayId, uint32_t width, uint32_t height) = 0;

        /**
        * @brief This method will be called when a display's window has been closed
        * @param displayId The display on which the event occurred
        */
        virtual void windowClosed(displayId_t displayId) = 0;

        /**
        * @brief Empty destructor
        */
        virtual ~IRendererEventHandler() = default;
    };

    /**
    * @brief Convenience empty implementation of IRendererEventHandler that can be used to derive from
    *        when only subset of event handling methods need to be implemented.
    */
    class RAMSES_API RendererEventHandlerEmpty : public IRendererEventHandler
    {
    public:
        /**
        * @copydoc ramses::IRendererEventHandler::scenePublished
        */
        virtual void scenePublished(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneSubscribed
        */
        virtual void sceneSubscribed(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneMapped
        */
        virtual void sceneMapped(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneShown
        */
        virtual void sceneShown(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneUnpublished
        */
        virtual void sceneUnpublished(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneUnsubscribed
        */
        virtual void sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneUnmapped
        */
        virtual void sceneUnmapped(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneHidden
        */
        virtual void sceneHidden(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataLinked
        */
        virtual void dataLinked(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override
        {
            (void)providerScene;
            (void)providerId;
            (void)consumerScene;
            (void)consumerId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::offscreenBufferLinkedToSceneData
        */
        virtual void offscreenBufferLinkedToSceneData(offscreenBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override
        {
            (void)providerOffscreenBuffer;
            (void)consumerScene;
            (void)consumerId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataUnlinked
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override
        {
            (void)consumerScene;
            (void)consumerId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::offscreenBufferCreated
        */
        virtual void offscreenBufferCreated(displayId_t displayId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)offscreenBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::offscreenBufferDestroyed
        */
        virtual void offscreenBufferDestroyed(displayId_t displayId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)offscreenBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneAssignedToOffscreenBuffer
        */
        virtual void sceneAssignedToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBufferId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)offscreenBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneAssignedToFramebuffer
        */
        virtual void sceneAssignedToFramebuffer(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::framebufferPixelsRead
        */
        virtual void framebufferPixelsRead(const uint8_t* pixelData, const uint32_t pixelDataSize, displayId_t displayId, ERendererEventResult result) override
        {
            (void)pixelData;
            (void)pixelDataSize;
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::warpingMeshDataUpdated
        */
        virtual void warpingMeshDataUpdated(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::displayCreated
        */
        virtual void displayCreated(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::displayDestroyed
        */
        virtual void displayDestroyed(displayId_t displayId, ERendererEventResult result) override
        {
            (void)displayId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataProviderCreated
        */
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataProviderDestroyed
        */
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataConsumerCreated
        */
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::dataConsumerDestroyed
        */
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneFlushed
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag, ESceneResourceStatus resourceStatus) override
        {
            (void)sceneId;
            (void)sceneVersionTag;
            (void)resourceStatus;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneExpired
        */
        virtual void sceneExpired(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::sceneRecoveredFromExpiration
        */
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::streamAvailabilityChanged
        */
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) override
        {
            (void)streamId;
            (void)available;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::keyEvent
        */
        virtual void keyEvent(displayId_t displayId, EKeyEvent eventType, uint32_t keyModifiers, EKeyCode keyCode) override
        {
            (void)displayId;
            (void)eventType;
            (void)keyModifiers;
            (void)keyCode;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::mouseEvent
        */
        virtual void mouseEvent(displayId_t displayId, EMouseEvent eventType, int32_t mousePosX, int32_t mousePosY) override
        {
            (void)displayId;
            (void)eventType;
            (void)mousePosX;
            (void)mousePosY;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::windowResized
        */
        virtual void windowResized(displayId_t displayId, uint32_t width, uint32_t height) override
        {
            (void)displayId;
            (void)width;
            (void)height;
        }

        /**
        * @copydoc ramses::IRendererEventHandler::windowClosed
        */
        virtual void windowClosed(displayId_t displayId) override
        {
            (void)displayId;
        }
    };
}

#endif
