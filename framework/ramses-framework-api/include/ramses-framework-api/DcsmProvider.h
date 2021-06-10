//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMPROVIDER_H
#define RAMSES_DCSMPROVIDER_H

#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/RamsesFramework.h"
#include "ramses-framework-api/IDcsmProviderEventHandler.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/DcsmMetadataCreator.h"
#include "ramses-framework-api/EDcsmOfferingMode.h"

namespace ramses
{
    class DcsmProviderImpl;

    /**
     * @brief Class used to offer ramses content and meta infos to a consumer and
     *        synchronize actions between client and renderer side applications
     */
    class RAMSES_API DcsmProvider : public StatusObject
    {
    public:
        /**
         * @brief Assigns a ramses scene ID to a contentID and category and offers
         *        that content to listening consumers. Should only be called if content
         *        could and should currently be shown.
         *        The ramses scene belonging to the scene ID does not have to exist yet.
         *
         *        A content must only be offered in a way that one scene is not associated with
         *        two categories which get assigned to different ramses displays on the
         *        renderer/consumer side. Otherwise contents might never be able to
         *        reach ready state.
         *
         *        A failing offerContent call might trigger a stopOfferAccepted event,
         *        which can be safely ignored.
         *
         * @param contentID The ID of the content to be offered
         * @param category The category the content is made for
         * @param scene The ramses scene ID containing the content.
         * @param mode Indicates if content should be offered within same process only
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t offerContent(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode);

        /**
         * @brief Same behavior as offerContent() but additionally send provided
         *        metadata to consumers that assigned content to themselves.
         *
         *        This method should be used to attach metadata immediately on offer to a
         *        content but is no prerequisite for later calls to updateContentMetadata().
         *
         *        A failing offerContentWithMetadata call might trigger a stopOfferAccepted
         *        event, which can be safely ignored.
         *
         * @param contentID The ID of the content to be offered
         * @param category The category the content is made for
         * @param scene The ramses scene ID containing the content.
         * @param mode Indicates if content should be offered within same process only
         * @param metadata metadata creator filled with metadata
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t offerContentWithMetadata(ContentID contentID, Category category, sceneId_t scene, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata);

        /**
        * @brief Assigns a Wayland IVI Surface ID to a contentID and category and offers
        *        that content to listening consumers. Should only be called if content
        *        could and should currently be shown.
        *        The Wayland IVI Surface belonging to the surface ID must not exist yet.
        *
        *        A failing offerContent call might trigger a stopOfferAccepted event,
        *        which can be safely ignored.
        *
        * @param contentID The ID of the content to be offered
        * @param category The category the content is made for
        * @param surfaceId The Wayland IVI Surface id containing the content.
        * @param mode Indicates if content should be offered within same process only
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t offerContent(ContentID contentID, Category category, waylandIviSurfaceId_t surfaceId, EDcsmOfferingMode mode);

        /**
        * @brief Same behavior as offerContent() but additionally send provided
        *        metadata to consumers that assigned content to themselves.
        *
        *        This method should be used to attach metadata immediately on offer to a
        *        content but is no prerequisite for later calls to updateContentMetadata().
        *
        *        A failing offerContentWithMetadata call might trigger a stopOfferAccepted
        *        event, which can be safely ignored.
        *
        * @param contentID The ID of the content to be offered
        * @param category The category the content is made for
        * @param surfaceId The Wayland IVI Surface id containing the content.
        * @param mode Indicates if content should be offered within same process only
        * @param metadata metadata creator filled with metadata
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t offerContentWithMetadata(ContentID contentID, Category category, waylandIviSurfaceId_t surfaceId, EDcsmOfferingMode mode, const DcsmMetadataCreator& metadata);

        /**
         * @brief Send metadata updates to consumers content is assigned to. The content is earliest
         *        sent to consumer on change change offered to assigned.
         *
         * contentID must belong to a content currently offered by this provider. A consumer initially gets
         * the last combined state of all metadata updates (later updated values overwrite earlier values)
         * when they become assigned. The initial state is given by offerContentWithMetadata() or
         * empty if offerContent() is used.
         * After the initial send updates are directly provided to the assigned consumer.
         *
         * @param contentID The ID of the content for which metadata should be updated
         * @param metadata metadata creator filled with metadata
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t updateContentMetadata(ContentID contentID, const DcsmMetadataCreator& metadata);

        /**
         * @brief Request to stop offering a content. A successful request
         *        will trigger a call to stopOfferAccepted in the handler.
         *
         * @param contentID The ID of the content to be stopped offering.
         * @return StatusOK for a successful request, otherwise the returned status
         *         can be used to resolve error message using getStatusMessage().
         */
        status_t requestStopOfferContent(ContentID contentID);

        /**
         * @brief Marks the content ready for displaying. This function might be
         *        called any time after offerContent().
         *        A connected DcsmConsumer might request a content to be marked as
         *        ready, resulting in a call to contentReadyRequest() in the event
         *        handler (see dispatchEvents).
         *        markContentReady shall be called after that.
         *
         *        When a previously marked ready content is released by the consumer
         *        the ready state is automatically reset. Should this or another
         *        consumer request the content ready again, this function has to be
         *        called again for this content to indicate renewed readiness.
         *
         * @param contentID The ID of the content to be marked ready
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         *
         * @pre Ramses scene content: Scene associated with content is set up and published.
         */
        status_t markContentReady(ContentID contentID);

        /**
         * @brief Requests an assigned DcsmConsumer to switch to/focus this content within a category.
         *        This function does not have to be called to enable a consumer to use this
         *        content, it is only needed when the provider side wants to influence
         *        the consumer application logic concerning which content to use.
         *        When the logical reason for the focus is no longer valid, the request should be disabled again.
         *
         * @param contentID The ID of the content to request focus for
         * @param focusRequest An arbitrary identifier for this focusRequest
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t enableFocusRequest(ContentID contentID, int32_t focusRequest);

        /**
         * @brief No longer request an assigned DcsmConsumer to focus this content for given focusrequest within a category.
         *        Should be called, when the logical reason for the focus is no longer given.
         *
         * @param contentID The ID of the content for which to disable the focus request for
         * @param focusRequest The focusRequest to disable
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t disableFocusRequest(ContentID contentID, int32_t focusRequest);

        /**
         * @brief Calls a callback function for every internal event
         *        except the content status event, which will be discarded.
         *
         * Communication from DcsmConsumer will be handled by a
         * DcsmProvider.  Some of this communication results in
         * an event. Calls handler functions synchronously in
         * the caller context for DCSM events which were
         * received asynchronously.  This function must be
         * called regularly to avoid buffer overflow of the
         * internal queue.
         *
         * @param handler A class which handles feedback from DcsmProvider
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t dispatchEvents(IDcsmProviderEventHandler& handler);

        /**
         * @brief Calls a callback function for every internal event
         *        including the content status event.
         *
         * Communication from DcsmConsumer will be handled by a
         * DcsmProvider.  Some of this communication results in
         * an event. Calls handler functions synchronously in
         * the caller context for DCSM events which were
         * received asynchronously.  This function must be
         * called regularly to avoid buffer overflow of the
         * internal queue.
         *
         * @param handler A class which handles feedback from DcsmProvider
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t dispatchEvents(IDcsmProviderEventHandlerExtended& handler);

        /**
         * @brief Constructor of DcsmProvider
         */
        explicit DcsmProvider(DcsmProviderImpl&);

        /**
         * @brief Destructor of DcsmProvider
         */
        ~DcsmProvider();

        /**
         * Stores internal data for implementation specifics of DcsmProvider
         */
        class DcsmProviderImpl& impl;

        /**
         * @brief Deleted default constructor
         */
        DcsmProvider() = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        DcsmProvider(const DcsmProvider& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DcsmProvider& operator=(const DcsmProvider& other) = delete;
    };
}

#endif
