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
         * @brief Calls handler funtions synchronously in the caller context for DCSM events which were received asynchronously.
         *        This function must be called regularly to avoid buffer overflow of the internal queue.
         *
         * @param handler User class that implements the callbacks that can be triggered if a corresponding event happened.
         *                Check IDcsmConsumerEventHandler documentation for more details.
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t dispatchEvents(IDcsmConsumerEventHandler& handler);

        /**
         * @brief Send a DCSM canvasSizeEvent for the given content ID. May only be called on registered content IDs.
         *        Informs provider about canvas size change so it can adapt its content if needed. To allow
         *        a smooth transition animation start and end timestamp can be given during which the size change should
         *        be applied.
         *
         *        This method has to be called after this consumer signs up for the content and every time the rendering
         *        viewport size for this content changes while it is still using the content.
         *
         * @param contentID  content which is affected by the canvas size change
         * @param size       expected size after the transition
         * @param animationInformation start and end times of the transition. May be null for immediate change.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t sendCanvasSizeChange(ContentID contentID, SizeInfo size, AnimationInformation animationInformation);

        /**
         * @brief Send a DCSM contentStatusChange for the given content ID. May only be called on registered content IDs.
         *        Requests switch to a different content status from provider side. For more information refer to EDCsmStatus
         *        enum and DCSM documentation.
         *
         * @param contentID  content for which the status is about to change
         * @param status     expected content state after the transition
         * @param animationInformation start and end times of the transition. May be null for immediate change.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t sendContentStatusChange(ContentID contentID, EDcsmStatus status, AnimationInformation animationInformation);

        DcsmConsumer() = delete;
        DcsmConsumer(const DcsmConsumer&) = delete;
        DcsmConsumer& operator=(const DcsmConsumer&) = delete;

        DcsmConsumer(DcsmConsumerImpl&);
        ~DcsmConsumer() override;
        DcsmConsumerImpl& impl;
    };
}

#endif
