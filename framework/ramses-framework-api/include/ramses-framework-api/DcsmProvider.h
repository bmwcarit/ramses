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
         *        The ramses scene belonging to the scene ID must not exist yet.
         *
         * @param contentID The ID of the content to be offered
         * @param category The category the content is made for
         * @param scene The ramses scene ID containing the content.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t offerContent(ContentID contentID, Category category, sceneId_t scene);

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
         * @param contentID The ID of the content to be marked ready
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         *
         * @pre Scene associated with content is set up and published.
         */
        status_t markContentReady(ContentID contentID);

        /**
         * @brief Requests an assigned DcsmConsumer to switch to/focus this content within a category.
         *        This function must not be called to enable a consumer to use this
         *        content, it is only needed when the provider side wants to influence
         *        the consumer application logic concerning which content to use.
         *
         * @param contentID The ID of the content to request focus for
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t requestContentFocus(ContentID contentID);

        /**
         * @brief Communication from DcsmConsumer will be handled by a
         *        DcsmProvider.  Some of this communication results in
         *        an event. Calls handler funtions synchronously in
         *        the caller context for DCSM events which were
         *        received asynchronously.  This function must be
         *        called regularly to avoid buffer overflow of the
         *        internal queue.
         *
         * @param handler A class which handles feedback from DcsmProvider
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t dispatchEvents(IDcsmProviderEventHandler& handler);

        /**
         * @brief Constructor of DcsmProvider
         */
        DcsmProvider(DcsmProviderImpl&);

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
