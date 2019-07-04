//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDCSMCONSUMEREVENTHANDLER_H
#define RAMSES_IDCSMCONSUMEREVENTHANDLER_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/APIExport.h"

namespace ramses
{
    /**
    * @brief Callback interface for recived DCSM consumer side events. Used with DcsmConsumer::dispatchEvents().
    */
    class RAMSES_API IDcsmConsumerEventHandler
    {
    public:
        virtual ~IDcsmConsumerEventHandler() = default;

        /**
         * @brief Provider offered new content that can be controlled
         *
         * @param contentID newly available content
         * @param category content category
         */
        virtual void contentOffered(ramses::ContentID contentID, ramses::Category category) = 0;

        /**
         * @brief Provider made content ready and available to use. This is an answer to DcsmConsumer::sendContentStatusChange with
         *        EDcsmState::Ready
         *
         * @param contentID available content
         * @param contentType what kind of content
         * @param contentDescriptor descriptor/id of this content
         */
        virtual void contentReady(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor) = 0;

        /**
         * @brief Provider requested to switch to/focus this content within the category. Consumer may or may not follow this request.
         *
         * @param contentID content that provider wants to switch ot
         */
        virtual void contentFocusRequest(ramses::ContentID contentID) = 0;

        /**
         * @brief Provider no longer wants to offer the content and wants to remove it.
         *        Requests consumer to release the content and reply with DcsmConsumer::acceptStopOffer.
         *        Optionally the consumer may trigger contentStateChanges to release content in a controlled manner.
         *
         * @param contentID content for which the stopOffer is requested
         */
        virtual void contentStopOfferRequest(ramses::ContentID contentID) = 0;

        /**
         * @brief Content disappeared unexpectedly. Consumer should take immediate actions to handle.
         *        It is not possible to send further commands for this content.
         *
         * @param contentID which content id affected
         */
        virtual void forceContentOfferStopped(ramses::ContentID contentID) = 0;
    };
}

#endif
