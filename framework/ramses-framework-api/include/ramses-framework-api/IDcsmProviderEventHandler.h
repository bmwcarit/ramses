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
         * @brief Called if a content was switched to not being shown.
         *
         * @param contentID The ID of the hidden content
         * @param animInfo animation information for hiding the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content gets hidden
         */
        virtual void contentHidden(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called if a content was switched to being shown.
         *
         * @param contentID The ID of the shown content
         * @param animInfo animation information for showing the content:
         *                 startTime: beginning of a potential animation and
         *                            time point when content gets shown
         *                 endTime: end of a potential animation
         */
        virtual void contentShown(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after a content was being requested to be unregistered.
         *        After animation is finished and content is hidden, content is
         *        unregistered and the associated scene can be safely unpublished.
         *
         * @param contentID The ID of the content to be unregistered
         * @param animInfo animation information for hiding the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content gets hidden
         */
        virtual void contentUnregistered(ContentID contentID, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after the rendering viewport for the content has been
         *        changed. Will be called once after a DcsmConsumer signed up for
         *        the content, and every time the consumer changes the rendering
         *        viewport size.
         *
         * @param contentID The ID of the content to be unregistered
         * @param sizeInfo the new viewport
         * @param animInfo animation information for resizing the content:
         *                 startTime: beginning of a potential animation
         *                 endTime: end of a potential animation and
         *                          time point when content has to have new size
         */
        virtual void canvasSizeChanged(ContentID contentID, SizeInfo sizeInfo, AnimationInformation animInfo) = 0;

        /**
         * @brief Called after a DcsmConsumer requested the content and it has not
         *        been marked ready yet. After this function has been called,
         *        markContentReady() of the DcsmProvider shall be called.
         *
         * @param contentID The ID of the content to be unregistered
         */
        virtual void contentReadyRequest(ContentID contentID) = 0;

        /**
         * @brief Destructor
         */
        virtual ~IDcsmProviderEventHandler() = default;
    };
}

#endif
