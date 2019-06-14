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
         * @brief Provider registered new content that can be controlled
         *
         * @param contentID newly available content
         * @param category content category
         */
        virtual void registerContent(ramses::ContentID contentID, ramses::Category category) = 0;

        /**
         * @brief Provider made content available to use. This is an answer to DcsmConsumer::sendContentStatusChange with
         *        EDcsmStatus::Ready
         *
         * @param contentID available content
         * @param contentType what kind of content
         * @param contentDescriptor descriptor/id of this content
         */
        virtual void contentAvailable(ramses::ContentID contentID, ramses::ETechnicalContentType contentType, ramses::TechnicalContentDescriptor contentDescriptor) = 0;

        /**
         * @brief Provider requested to switch to this content. Consumer may or may not follow this request.
         *
         * @param contentID content that provider wants to switch ot
         */
        virtual void categoryContentSwitchRequest(ramses::ContentID contentID) = 0;

        /**
         * @brief Provider wants to remove its content and requests consumer to initiate controlled state change
         *        via DcsmConsumer::sendContentStatusChange.
         *
         * @param contentID which content should get to unregistered state
         */
        virtual void requestUnregisterContent(ramses::ContentID contentID) = 0;

        /**
         * @brief Content disappeared unexpectedly. Consumer should take immediate actions to handle.
         *        It is not possible to send further commands for this content.
         *
         * @param contentID which content id affected
         */
        virtual void forceUnregisterContent(ramses::ContentID contentID) = 0;
    };
}

#endif
