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

        /** @brief Called when Dcsm provider requests focus change of its content.
        *          This is a purely Dcsm related message forwarded via #ramses::DcsmContentControl event mechanism,
        *          see ramses::DcsmProvider::requestContentFocus for more details.
        *          Application logic should react accordingly if it decides to accept the request,
        *          typically this means requesting the content to be ready and show afterwards using #ramses::DcsmContentControl::requestContentReady
        *          and #ramses::DcsmContentControl::showContent.
        * @param contentID Unique ID of content for which the request was made.
        */
        virtual void contentFocusRequested(ContentID contentID) = 0;

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
        *
        * @param offscreenBufferId ID of offscreen buffer which was linked to consumer.
        * @param consumerContent ID of content using scene that consumes the linked offscreen buffer.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if offscreen buffer successfully linked, false otherwise.
        */
        virtual void offscreenBufferLinked(displayBufferId_t offscreenBufferId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /** @brief Data provider and consumer were linked (or failed to be linked) as result of calling #ramses::DcsmContentControl::linkData.
        * @details Note that there is a possibility that given consumerContent ID or providerContent ID is different from the one passed
        * when calling #ramses::DcsmContentControl::linkOffscreenBuffer. This can happen if the Ramses scene involved in this data linking is used
        * by multiple contents, in that case simply one of the contents using the scene will be given here.
        *
        * @param providerContent ID of content using scene that provides the linked data.
        * @param providerId ID of data provider in the provider scene.
        * @param consumerContent ID of content using scene that consumes the linked data.
        * @param consumerId ID of data consumer in the consumer scene.
        * @param success True if data successfully linked, false otherwise.
        */
        virtual void dataLinked(ContentID providerContent, dataProviderId_t providerId, ContentID consumerContent, dataConsumerId_t consumerId, bool success) = 0;

        /** @brief Scene associated with content was flushed with a version tag.
        * @details Every DCSM content has a #ramses::Scene associated with it, whenever that scene is flushed by
        *          content provider (#ramses::Scene::flush) with a version tag this callback will be triggered.
        *          If the scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        *
        * @param content ID of content that has the flushed scene associated.
        * @param version User version tag given when flushing the scene.
        */
        virtual void contentFlushed(ContentID content, sceneVersionTag_t version) = 0;

        /** @brief This method will be called if a content's scene which has an expiration timestamp set (#ramses::Scene::setExpirationTimestamp)
        *          is on renderer (not necessarily rendered) at a state that expired, i.e. current time is after the expiration timestamp.
        * @details This callback is called only once when the scene expires even if scene stays expired in subsequent frames.
        *          When the content's scene is updated again with a new not anymore expired timestamp, #contentRecoveredFromExpiration is called.
        *          If the expired scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param content ID of content that has the expired scene associated.
        */
        virtual void contentExpired(ContentID content) = 0;

        /** @brief This method will be called if a content's scene which previously expired (#contentExpired)
        *          was updated with a new expiration timestamp that is not expired anymore.
        * @details This callback is called only once when the scene switches state from expired to not expired.
        *          If the expired scene is associated with multiple contents (at the time of receiving this event)
        *          this callback will be triggered for all those contents.
        * @param content ID of content that has the recovered scene associated.
        */
        virtual void contentRecoveredFromExpiration(ContentID content) = 0;

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
        virtual void streamAvailabilityChanged(streamSource_t streamId, bool available) = 0;
    };
}

#endif
