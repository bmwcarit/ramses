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
    class DcsmMetadataUpdate;

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
         * @param contentType what kind of content
         */
        virtual void contentOffered(ramses::ContentID contentID, ramses::Category category, ramses::ETechnicalContentType contentType) = 0;

        /**
         * @brief Provides the content description. It will be triggered by provider after the content is assigned
         *        to consumer. It will always be called before the contentReady callback.
         *
         * @param contentID described content
         * @param contentDescriptor descriptor/id of this content
         */
        virtual void contentDescription(ramses::ContentID contentID, ramses::TechnicalContentDescriptor contentDescriptor) = 0;

        /**
         * @brief Provider made content ready and available to use. This is an answer to DcsmConsumer::sendContentStatusChange with
         *        EDcsmState::Ready. Will be called after receiving a contentDescription callback with the necessary content description.
         *
         * @param contentID available content
         */
        virtual void contentReady(ramses::ContentID contentID) = 0;

        /**
         * @brief Provider requested to switch to/focus this content within the category. Consumer may or may not follow this request.
         *
         * @param contentID content that provider wants to switch focus to
         * @param focusRequest identifier of the focus request
         */
        virtual void contentEnableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) = 0;

        /**
         * @brief Provider requested to no longer request focusing this content within the category. Consumer may or may not follow this request.
         *
         * @param contentID content that provider should no longer focus
         * @param focusRequest identifier of the focus request
         */
        virtual void contentDisableFocusRequest(ramses::ContentID contentID, int32_t focusRequest) = 0;

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

        /**
         * @brief Update metadata for given content. This callback provides metadata given to DcsmProvider::offerContentWithMetadata()
         *        and DcsmProvider::updateContentMetadata(). A consumer will get the combined state of all past metadata updates
         *        from the whole lifecycle of the content as first event after it assigned to content to itself. Later events only contain
         *        delta updates. When the provider never attached metadata to this content, this callback will never be called.
         *
         * @param contentID which content is affected
         * @param metadataUpdate object to get metadata update from. valid for the lifetime of the callback.
         */
        virtual void contentMetadataUpdated(ramses::ContentID contentID, const DcsmMetadataUpdate& metadataUpdate) = 0;
    };
}

#endif
