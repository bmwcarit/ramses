//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IRENDERERSCENECONTROLEVENTHANDLER_LEGACY_H
#define RAMSES_IRENDERERSCENECONTROLEVENTHANDLER_LEGACY_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
    * @deprecated [Use RendererSceneControl and IRendererSceneControlEventHandler instead]
    * @brief Provides an interface for handling the result of renderer scene control events.
    *        Implementation of this interface must be passed to RendererSceneControl_legacy::dispatchEvents
    *        which will in return invoke methods of the interface according to events that occurred since last dispatching.
    */
    class RAMSES_API IRendererSceneControlEventHandler_legacy
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
        virtual void offscreenBufferLinkedToSceneData(displayBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) = 0;

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
        * @brief This method will be called after a scene is assigned to a display buffer (or failed to be assigned) as a result of RamsesRenderer API \c assignSceneToDisplayBuffer call.
        *
        * @param sceneId The id of the scene that was assigned to the display buffer (or failed to be assigned).
        * @param displayBufferId The id of the display buffer that the scene was assigned to (or failed to be assigned to).
        * @param result Can be ERendererEventResult_OK if succeeded, ERendererEventResult_FAIL if failed.
        */
        virtual void sceneAssignedToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBufferId, ERendererEventResult result) = 0;

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
        * (wl_surface) with ivi_id=streamId, and the surface has at least one attached non-nullptr buffer (i.e. renderable content).
        *
        * It is possible that the ivi_application does not update its surface (by providing new buffers/frames), but RAMSES has
        * no way of knowing that, hence a stream is _NOT_ reported unavailable in that case.
        *
        * A surface becomes unavailable whenever either the ivi_application is destroyed, or when it crashed, or when it attached
        * a nullptr buffer to the stream surface with id=streamId (i.e. actively told wayland that it should not render contents to
        * this ivi surface).
        *
        * @param streamId The IVI stream id
        * @param available True if the stream became available, and false if it disappeared
        */
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) = 0;

        /**
        * @brief Empty destructor
        */
        virtual ~IRendererSceneControlEventHandler_legacy() = default;
    };

    /**
    * @deprecated [Use RendererSceneControl and RendererSceneControlEventHandlerEmpty instead]
    * @brief Convenience empty implementation of IRendererSceneControlEventHandler_legacy that can be used to derive from
    *        when only subset of event handling methods need to be implemented.
    */
    class RAMSES_API RendererSceneControlEventHandlerEmpty_legacy : public IRendererSceneControlEventHandler_legacy
    {
    public:
        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::scenePublished
        */
        virtual void scenePublished(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneSubscribed
        */
        virtual void sceneSubscribed(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneMapped
        */
        virtual void sceneMapped(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneShown
        */
        virtual void sceneShown(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneUnpublished
        */
        virtual void sceneUnpublished(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneUnsubscribed
        */
        virtual void sceneUnsubscribed(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneUnmapped
        */
        virtual void sceneUnmapped(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneHidden
        */
        virtual void sceneHidden(sceneId_t sceneId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataLinked
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
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::offscreenBufferLinkedToSceneData
        */
        virtual void offscreenBufferLinkedToSceneData(displayBufferId_t providerOffscreenBuffer, sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override
        {
            (void)providerOffscreenBuffer;
            (void)consumerScene;
            (void)consumerId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataUnlinked
        */
        virtual void dataUnlinked(sceneId_t consumerScene, dataConsumerId_t consumerId, ERendererEventResult result) override
        {
            (void)consumerScene;
            (void)consumerId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneAssignedToDisplayBuffer
        */
        virtual void sceneAssignedToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBufferId, ERendererEventResult result) override
        {
            (void)sceneId;
            (void)displayBufferId;
            (void)result;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataProviderCreated
        */
        virtual void dataProviderCreated(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataProviderDestroyed
        */
        virtual void dataProviderDestroyed(sceneId_t sceneId, dataProviderId_t dataProviderId) override
        {
            (void)sceneId;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataConsumerCreated
        */
        virtual void dataConsumerCreated(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::dataConsumerDestroyed
        */
        virtual void dataConsumerDestroyed(sceneId_t sceneId, dataConsumerId_t dataConsumerId) override
        {
            (void)sceneId;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneFlushed
        */
        virtual void sceneFlushed(sceneId_t sceneId, sceneVersionTag_t sceneVersionTag, ESceneResourceStatus resourceStatus) override
        {
            (void)sceneId;
            (void)sceneVersionTag;
            (void)resourceStatus;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneExpired
        */
        virtual void sceneExpired(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::sceneRecoveredFromExpiration
        */
        virtual void sceneRecoveredFromExpiration(sceneId_t sceneId) override
        {
            (void)sceneId;
        }

        /**
        * @copydoc ramses::IRendererSceneControlEventHandler_legacy::streamAvailabilityChanged
        */
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) override
        {
            (void)streamId;
            (void)available;
        }
    };
}

#endif
