//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDCSMCONTENTCONTROLEVENTHANDLER_H
#define RAMSES_IDCSMCONTENTCONTROLEVENTHANDLER_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    class DcsmMetadataUpdate;

    /// #ramses::DcsmContentControl event result used in some event handler callbacks
    enum class DcsmContentControlEventResult
    {
        OK,       ///< The event (state change) was successful
        TimedOut  ///< The event (state change) did not happen in given time period and is considered as failed - no change happened
    };

    /**
    * @brief Callback handler interface for events emitted by #ramses::DcsmContentControl.
    *
    * These must be implemented in order to use with #ramses::DcsmContentControl::update.
    */
    class RAMSES_API IDcsmContentControlEventHandler
    {
    public:
        /// Destructor
        virtual ~IDcsmContentControlEventHandler() = default;

        /** @brief New content is available.
        *          This is a result of Dcsm content offer that requested category registered in the #ramses::DcsmContentControl.
        *          This can also mean that content is released (its scene is not rendered anymore and resources are unloaded),
        *          as result of #ramses::DcsmContentControl::releaseContent at finish time of animation if any provided,
        *          or if content state drops to available as result of its scene state change.
        * @param contentID Content that became available.
        * @param categoryID Category that the content was assigned to.
        */
        virtual void contentAvailable(ContentID contentID, Category categoryID) = 0;

        /** @brief Content became ready or timed out to get ready.
        *          See #ramses::DcsmContentControl::requestContentReady for details.
        *          This can also mean content has become hidden (its scene is not rendered anymore),
        *          as result of #ramses::DcsmContentControl::hideContent at finish time of animation if any provided.
        * @param contentID Content that became ready or timed out.
        * @param result OK if content became ready, TimedOut otherwise.
        */
        virtual void contentReady(ContentID contentID, DcsmContentControlEventResult result) = 0;

        /** @brief Content is shown (its scene is rendered on corresponding display).
        *          This callback comes at start time of animation if any provided,
        *          see #ramses::DcsmContentControl::showContent for details.
        * @param contentID Content that is shown.
        */
        virtual void contentShown(ContentID contentID) = 0;

        /**
         * @brief Provider requested to switch to/focus this content within the category. Consumer may or may not follow this request.
         *        This is a purely Dcsm related message forwarded via #ramses::DcsmContentControl event mechanism,
         *        see ramses::DcsmProvider::enableFocusRequest for more details.
         *        Application logic should react accordingly if it decides to accept the request,
         *        typically this means requesting the content to be ready and show afterwards using #ramses::DcsmContentControl::requestContentReady
         *        and #ramses::DcsmContentControl::showContent.
         *
         * @param contentID content that provider wants to switch focus to
         * @param focusRequest identifier of the focus request
         */
        virtual void contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) = 0;

        /**
         * @brief Provider requested to no longer focus this content within the category. Consumer may or may not follow this request.
         *
         * @param contentID content that provider should no longer focus
         * @param focusRequest identifier of the focus request
         */
        virtual void contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) = 0;

        /** @brief Called when Dcsm provider requests that its content is not used anymore
        *          and it would like to stop offering it.
        *          This is a purely Dcsm related message forwarded via #ramses::DcsmContentControl event mechanism.
        *          Application logic should react by calling ramses::#ramses::DcsmContentControl::acceptStopOffer (optionally with timing information),
        *          when it decides that the content is indeed no longer needed.
        * @param contentID Unique ID of content for which the request was made.
        */
        virtual void contentStopOfferRequested(ContentID contentID) = 0;

        /** @brief Content became unavailable unexpectedly.
        *          Application logic should react as fast as possible if this affects currently rendered content(s).
        * @param contentID Unique ID of content that became unavailable.
        */
        virtual void contentNotAvailable(ContentID contentID) = 0;

        /**
         * @brief Update metadata for given content. This callback provides metadata given to by DcsmProvider::offerContentWithMetadata()
         *        and DcsmProvider::updateContentMetadata(). A consumer will get the combined state of all past metadata updates
         *        from the whole lifecycle of the content as first event after it successfully acquired control over content (state Assigned).
         *        Later events only contain delta updates.
         *        When the provider never attached metadata to this content, this callback will never be called.
         *
         * @param contentID which content is affected
         * @param metadataUpdate object to get metadata update from. valid for the lifetime of the callback.
         */
        virtual void contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) = 0;

        /** @brief Offscreen buffer and consumer were linked (or failed to be linked) as result of calling #ramses::DcsmContentControl::linkOffscreenBuffer.
        * @details Note that there is a possibility that given consumerContent ID is different from the one passed when calling
        *          #ramses::DcsmContentControl::linkOffscreenBuffer. This can happen if the Ramses scene involved in this data linking is used
        *          by multiple contents, in that case simply one of the contents using the scene will be given here.
        *          Also note that given consumerContent ID will be invalid if the consumer content or its scene became unavailable
        *          before this callback is emitted.
        *
        * @param offscreenBufferId ID of offscreen buffer which was linked to consumer.
        * @param consumerContent ID of content using scene that consumes the linked offscreen buffer.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if offscreen buffer successfully linked, false otherwise.
        */
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /** @brief Content and consumer were linked (or failed to be linked) as result of calling #ramses::DcsmContentControl::linkContentToTextureConsumer.
        * @details Note that there is a possibility that given consumerContent ID is different from the one passed when calling
        *          #ramses::DcsmContentControl::linkContentToTextureConsumer. This can happen if the underlying type involved in this data linking is used
        *          by multiple contents, in that case simply one of the contents will be given here.
        *          Also note that given consumerContent ID will be invalid if the consumer content or its scene became unavailable
        *          before this callback is emitted.
        *
        * @param providerContent ID of content which is providing texture (scene on offscreenbuffer or wayland surface).
        * @param consumerContent ID of content using scene that consumes the linked content.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if content successfully linked, false otherwise.
        */
        virtual void contentLinkedToTextureConsumer(ContentID providerContent, ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /** @brief Data provider and consumer were linked (or failed to be linked) as result of calling #ramses::DcsmContentControl::linkData.
        * @details Note that there is a possibility that given consumerContent ID or providerContent ID is different from the one passed
        *          when calling #ramses::DcsmContentControl::linkData. This can happen if the Ramses scene involved in this data linking is used
        *          by multiple contents, in that case simply one of the contents using the scene will be given here.
        *          Also note that given consumerContent/providerContent ID will be invalid if the consumer/provider content or its scene became unavailable
        *          before this callback is emitted.
        *
        * @param providerContent ID of content using scene that provides the linked data.
        * @param providerId ID of data provider in the provider scene.
        * @param consumerContent ID of content using scene that consumes the linked data.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if data successfully linked, false otherwise.
        */
        virtual void dataLinked(ContentID providerContent, dataProviderId_t providerId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /** @brief Data consumer was unlinked from provider (or failed to be linked) as result of calling #ramses::DcsmContentControl::unlinkData.
        * @details Note that there is a possibility that given consumerContent ID is different from the one passed
        *          when calling #ramses::DcsmContentControl::unlinkData. This can happen if the Ramses scene involved in this data linking is used
        *          by multiple contents, in that case simply one of the contents using the scene will be given here.
        *          Also note that given consumerContent ID will be invalid if the consumer content or its scene became unavailable
        *          before this callback is emitted.
        *
        * @param consumerContent ID of content using scene with data that was unlinked.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if data successfully unlinked, false otherwise.
        */
        virtual void dataUnlinked(ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /**
        * @brief This method will be called when there were scene objects picked as a result of #ramses::DcsmContentControl::handlePickEvent.
        *        A ramses::PickableObject can be 'picked' via a pick input event
        *        which is passed to #ramses::DcsmContentControl when the scene is rendered.
        *
        * @param content Content using scene to which the picked objects belong.
        * @param pickedObjects Pointer to first ID of the picked objects array.
        *        This array is valid only for the time of calling this method.
        * @param pickedObjectsCount Number of picked object IDs in the \c pickedObjects array.
        */
        virtual void objectsPicked(ContentID content, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) = 0;

        /** @brief This method will be called whenever a data provider is created in content's scene.
        * @details The event is emitted also for every data provider in a newly available content.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param contentID ID of content that has the data slot change associated.
        * @param dataProviderId The created data provider id
        */
        virtual void dataProviderCreated(ContentID contentID, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief   This method will be called when a data provider is removed from content's scene.
        * @details The event is emitted only when data provider explicitly destroyed,
        *          not if content becomes unavailable as a whole.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param contentID ID of content that has the data slot change associated.
        * @param dataProviderId The destroyed data provider id
        */
        virtual void dataProviderDestroyed(ContentID contentID, dataProviderId_t dataProviderId) = 0;

        /**
        * @brief   This method will be called whenever a data consumer is created in content's scene.
        * @details The event is emitted also for every data consumer in a newly available content.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param contentID ID of content that has the data slot change associated.
        * @param dataConsumerId The created data consumer id
        */
        virtual void dataConsumerCreated(ContentID contentID, dataConsumerId_t dataConsumerId) = 0;

        /**
        * @brief   This method will be called when a data consumer is removed from content's scene.
        * @details The event is emitted only when data consumer explicitly destroyed,
        *          not if scene becomes unavailable as a whole.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param contentID ID of content that has the data slot change associated.
        * @param dataConsumerId The destroyed data consumer id
        */
        virtual void dataConsumerDestroyed(ContentID contentID, dataConsumerId_t dataConsumerId) = 0;

        /** @brief Scene associated with content was flushed with a version tag.
        * @details Every DCSM content has a #ramses::Scene associated with it, whenever that scene is flushed by
        *          content provider (#ramses::Scene::flush) with a version tag this callback will be triggered.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        *
        * @param contentID ID of content that has the flushed scene associated.
        * @param version User version tag given when flushing the scene.
        */
        virtual void contentFlushed(ContentID contentID, sceneVersionTag_t version) = 0;

        /**
        * @brief This method will be called whenever a content's scene which was not previously monitored for expiration has requested expiration
        *        monitoring by sending a scene flush with valid expiration timestamp (#ramses::Scene::setExpirationTimestamp)
        *        and that flush was applied on renderer side.
        *        From this point on, the content's scene will be monitored, can expire and recover (#contentExpired, #contentRecoveredFromExpiration)
        *        until monitoring disabled again (#contentExpirationMonitoringDisabled).
        *        If the scene is associated with multiple contents (at the time of receiving this event)
        *        this callback will be triggered for all those contents.
        * @param contentID ID of content that has associated the scene, which will be monitored for expiration
        */
        virtual void contentExpirationMonitoringEnabled(ContentID contentID) = 0;

        /**
        * @brief This method will be called whenever a content's scene which was previously monitored for expiration has requested
        *        to stop being monitored by sending a scene flush with invalid expiration timestamp (#ramses::Scene::setExpirationTimestamp)
        *        and that flush was applied on renderer side.
        *        From this point on, the content's scene will not be monitored anymore, regardless if it previously expired or not,
        *        i.e. there will be no expiration events (#contentExpired, #contentRecoveredFromExpiration) until monitoring
        *        enabled again (#contentExpirationMonitoringEnabled).
        *        If the scene is associated with multiple contents (at the time of receiving this event)
        *        this callback will be triggered for all those contents.
        * @param contentID ID of content that has associated the scene, which will not be monitored for expiration anymore
        */
        virtual void contentExpirationMonitoringDisabled(ContentID contentID) = 0;

        /**
        * @brief This method will be called if a content's scene which is enabled for expiration monitoring (#contentExpirationMonitoringEnabled)
        *        is on renderer (not necessarily rendered) at a state that expired, i.e. current time is after the expiration timestamp
        *        set via #ramses::Scene::setExpirationTimestamp.
        *        This callback is called only once when the content's scene expires even if the scene stays expired in subsequent frames.
        *        When the scene is updated again with a new not anymore expired timestamp, #contentRecoveredFromExpiration is called.
        *        If the scene is associated with multiple contents (at the time of receiving this event)
        *        this callback will be triggered for all those contents.
        * @param contentID ID of content that has the expired scene associated.
        */
        virtual void contentExpired(ContentID contentID) = 0;

        /**
        * @brief This method will be called if a content's scene which previously expired (#ramses::Scene::setExpirationTimestamp and #contentExpired)
        *        was updated with a new expiration timestamp that is not expired anymore.
        *        This callback is called only once when the content's scene switches state from expired to not expired.
        *        This callback is not called when monitoring becomes disabled (#contentExpirationMonitoringDisabled) while content's scene
        *        is expired (#contentExpired).
        *        If the scene is associated with multiple contents (at the time of receiving this event)
        *        this callback will be triggered for all those contents.
        * @param contentID ID of content that has the recovered scene associated.
        */
        virtual void contentRecoveredFromExpiration(ContentID contentID) = 0;

        /** @brief This method will be called when a new IVI video stream becomes available, or when an existing stream disappears
        * @details In terms of Wayland protocol, a stream is available if an "ivi_application" exists which has created a Wayland surface
        *          (wl_surface) with ivi_id=streamId, and the surface has at least one attached non-nullptr buffer (i.e. renderable content).
        *
        *          It is possible that the ivi_application does not update its surface (by providing new buffers/frames), but RAMSES has
        *          no way of knowing that, hence a stream is not reported unavailable in that case.
        *
        *          A surface becomes unavailable whenever either the ivi_application is closed or when it attached
        *          a nullptr buffer to the stream surface with id=streamId (i.e. actively told Wayland that it should not render contents to
        *          this IVI surface).
        *
        * @param streamId The IVI stream id
        * @param available True if the stream became available, and false if it disappeared
        */
        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) = 0;
    };

    /**
    * @brief Convenience empty implementation of IDcsmContentControlEventHandler that can be used to derive from
    *        when only subset of event handling methods need to be implemented.
    */
    class RAMSES_API DcsmContentControlEventHandlerEmpty : public IDcsmContentControlEventHandler
    {
    public:
        /// @copydoc ramses::IDcsmContentControlEventHandler::contentAvailable
        virtual void contentAvailable(ContentID contentID, Category categoryID) override
        {
            (void)contentID;
            (void)categoryID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentReady
        virtual void contentReady(ContentID contentID, DcsmContentControlEventResult result) override
        {
            (void)contentID;
            (void)result;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentShown
        virtual void contentShown(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentStopOfferRequested
        virtual void contentStopOfferRequested(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentNotAvailable
        virtual void contentNotAvailable(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentMetadataUpdated
        virtual void contentMetadataUpdated(ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) override
        {
            (void)contentID;
            (void)metadataUpdate;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::offscreenBufferLinked
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) override
        {
            (void)offscreenBufferId;
            (void)consumerContent;
            (void)consumerId;
            (void)success;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentLinkedToTextureConsumer
        void contentLinkedToTextureConsumer(ContentID providerContent, ContentID consumerContent, dataConsumerId_t consumerId, bool success) override
        {
            (void)providerContent;
            (void)consumerContent;
            (void)consumerId;
            (void)success;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::dataLinked
        virtual void dataLinked(ContentID providerContent, dataProviderId_t providerId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) override
        {
            (void)providerContent;
            (void)providerId;
            (void)consumerContent;
            (void)consumerId;
            (void)success;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::dataUnlinked
        virtual void dataUnlinked(ContentID consumerContent, dataConsumerId_t consumerId, bool success) override
        {
            (void)consumerContent;
            (void)consumerId;
            (void)success;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::objectsPicked
        virtual void objectsPicked(ContentID content, const pickableObjectId_t* pickedObjects, uint32_t pickedObjectsCount) override
        {
            (void)content;
            (void)pickedObjects;
            (void)pickedObjectsCount;
        }

        /**
        * @copydoc ramses::IDcsmContentControlEventHandler::dataProviderCreated
        */
        virtual void dataProviderCreated(ContentID contentID, dataProviderId_t dataProviderId) override
        {
            (void)contentID;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IDcsmContentControlEventHandler::dataProviderDestroyed
        */
        virtual void dataProviderDestroyed(ContentID contentID, dataProviderId_t dataProviderId) override
        {
            (void)contentID;
            (void)dataProviderId;
        }

        /**
        * @copydoc ramses::IDcsmContentControlEventHandler::dataConsumerCreated
        */
        virtual void dataConsumerCreated(ContentID contentID, dataConsumerId_t dataConsumerId) override
        {
            (void)contentID;
            (void)dataConsumerId;
        }

        /**
        * @copydoc ramses::IDcsmContentControlEventHandler::dataConsumerDestroyed
        */
        virtual void dataConsumerDestroyed(ContentID contentID, dataConsumerId_t dataConsumerId) override
        {
            (void)contentID;
            (void)dataConsumerId;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentFlushed
        virtual void contentFlushed(ContentID contentID, sceneVersionTag_t version) override
        {
            (void)contentID;
            (void)version;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentExpirationMonitoringEnabled
        virtual void contentExpirationMonitoringEnabled(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentExpirationMonitoringDisabled
        virtual void contentExpirationMonitoringDisabled(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentExpired
        virtual void contentExpired(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentRecoveredFromExpiration
        virtual void contentRecoveredFromExpiration(ContentID contentID) override
        {
            (void)contentID;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::streamAvailabilityChanged
        virtual void streamAvailabilityChanged(waylandIviSurfaceId_t streamId, bool available) override
        {
            (void)streamId;
            (void)available;
        }
        /// @copydoc ramses::IDcsmContentControlEventHandler::contentEnableFocusRequest
        void contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override
        {
            (void) contentID;
            (void) focusRequest;
        }

        /// @copydoc ramses::IDcsmContentControlEventHandler::contentDisableFocusRequest
        void contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) override
        {
            (void) contentID;
            (void) focusRequest;
        }
    };
}

#endif
