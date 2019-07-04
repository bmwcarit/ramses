//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONSUMER_H
#define RAMSES_DCSMCONSUMER_H

#include "ramses-framework-api/APIExport.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/IDcsmConsumerEventHandler.h"
#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    class DcsmConsumerImpl;

    /**
    * @brief Class representing DCSM consumer side.
    */
    class RAMSES_API DcsmConsumer : public StatusObject
    {
    public:
        /**
         * @brief Calls handler functions synchronously in the caller context for DCSM events which were received asynchronously.
         *        This function must be called regularly to avoid buffer overflow of the internal queue.
         *
         * @param handler User class that implements the callbacks that can be triggered if a corresponding event happened.
         *                Check IDcsmConsumerEventHandler documentation for more details.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t dispatchEvents(IDcsmConsumerEventHandler& handler);

        /**
         * @brief Exclusively assign an offered content this consumer.
         *        Sends a DCSM canvasSizeEvent for the given content ID. May only be called for offered content IDs.
         *
         * @param contentID  content to assign to this consumer
         * @param size       expected size of the assigned category
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t assignContentToConsumer(ContentID contentID, SizeInfo size);

        /**
         * @brief Send a DCSM contentSizeEvent for the given content ID. May only be called on assigned content IDs.
         *        Informs provider about canvas size change so it can adapt its content if needed. To allow
         *        a smooth transition animation start and end timestamp can be given during which the size change should
         *        be applied.
         *
         *        This method has to be called after this consumer is assigned for the content and every time the rendering
         *        viewport size for this content changes while it is still using the content.
         *
         * @param contentID  content which is affected by the canvas size change
         * @param size       expected size after the transition
         * @param animationInformation start and end times of the transition. May be null for immediate change.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t contentSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation);

        /**
         * @brief Send a DCSM contentStateChange for the given content ID. May only be called on assigned content IDs.
         *        Requests switch to a different content state from provider side. For more information refer to EDcsmState
         *        enum and DCSM documentation.
         *
         * @param contentID  content for which the state is about to change
         * @param state      expected content state after the transition
         * @param animationInformation start and end times of the transition. May be null for immediate change.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t contentStateChange(ContentID contentID, EDcsmState state, AnimationInformation animationInformation);

        /**
         * @brief Accept a provider requesting to stop offering a content assigned to this consumer.
         *        May only be called as an answer to contentStopOfferRequest callback.
         *        The content may not be used after the given animation time any longer and
         *        also may not be assigned again until offered again.
         *
         * @param contentID  content for which the stopOffer is being accepted
         * @param animationInformation start and end times of the transition. May be null for immediate change.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t acceptStopOffer(ContentID contentID, AnimationInformation animationInformation);

        /**
         * @brief Deleted default constructor
         */
        DcsmConsumer() = delete;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        DcsmConsumer(const DcsmConsumer& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DcsmConsumer& operator=(const DcsmConsumer& other) = delete;

        /**
         * @brief Constructor from impl
         * @param impl_ impl
         */
        DcsmConsumer(DcsmConsumerImpl& impl_);

        /**
         * @brief Destructor
         */
        ~DcsmConsumer() override;

        /**
         * Stores internal data for implementation specifics of DcsmConsumer
         */
        DcsmConsumerImpl& impl;
    };
}

#endif
