//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDCSMPROVIDEREVENTHANDLER_H
#define RAMSES_IDCSMPROVIDEREVENTHANDLER_H

#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    /**
     * @brief An Interface for a class, whose functions are called as reaction to
     *        DcsmConsumer communication after calling dispatchEvents() on a
     *        DcsmProvider object.
     */
    class RAMSES_API IDcsmProviderEventHandler
    {
    public:
        /**
         * @brief Called if a content will be switched to be hidden.
         *
         * @param contentID The ID of the content being hidden
         * @param animInfo animation information for hiding the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content gets hidden
         */
        virtual void contentHide(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called if a content will be switched to shown.
         *
         * @param contentID The ID of the shown content
         * @param animInfo animation information for showing the content:
         *                 startTime: beginning of a potential animation and
         *                            time point when content gets shown
         *                 endTime: end of a potential animation
         */
        virtual void contentShow(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after a content was requested to no longer be offered.
         *        After this function returns, content is no longer offered.
         *        After the animation is finished, the associated scene can be
         *        safely unpublished.
         *
         * @param contentID The ID of the content to be unoffered
         * @param animInfo animation information for hiding the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content gets hidden
         */
        virtual void stopOfferAccepted(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after the rendering viewport for the content has been
         *        changed. Will be called once after a DcsmConsumer assigned itself for
         *        the content, and every time the consumer changes the rendering
         *        viewport size.
         *
         * @param contentID The ID of the content for which the viewport changes
         * @param sizeInfo the new viewport
         * @param animInfo animation information for resizing the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content has to have new size
         */
        virtual void contentSizeChange(ContentID contentID, SizeInfo sizeInfo, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after a DcsmConsumer requested the content and it has not
         *        been marked ready yet. After this function has been called,
         *        markContentReady() of the DcsmProvider shall be called.
         *
         * @param contentID The ID of the content to be marked as ready
         */
        virtual void contentReadyRequested(ContentID contentID) = 0;

        /**
         * @brief Called after an assigned DcsmConsumer is no longer interested in
         *        the content or the scene associated with it. After the animation
         *        has been finished, the associated scene can be safely unpublished.
         *        Note that the content has to be marked ready again after the call
         *        of this callback, should a DcsmConsumer request it to be ready
         *        again.
         *
         * @param animInfo animation information for resizing the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content has to have new size
         * @param contentID The ID of the content to be released
         */
        virtual void contentRelease(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Destructor
         */
        virtual ~IDcsmProviderEventHandler() = default;
    };
}

#endif
