//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_IDCSMRENDEREREVENTHANDLER_H
#define RAMSES_IDCSMRENDEREREVENTHANDLER_H

#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"

namespace ramses
{
    /// DcsmRenderer event result used in some event handler callbacks
    enum class DcsmRendererEventResult
    {
        OK,       /// The event (state change) was successful
        TimedOut  /// The event (state change) did not happen in given time period and is considered as failed - no change happened
    };

    /**
    * @brief Callback handler interface for events emitted by DcsmRenderer.
    *
    * These must be implemented in order to use with ramses::DcsmRenderer::update.
    */
    class RAMSES_API IDcsmRendererEventHandler
    {
    public:
        /// Destructor
        virtual ~IDcsmRendererEventHandler() = default;

        /** @brief New content is available.
        *          This is a result of Dcsm content offer that requested category registered in the DcsmRenderer.
        * @param contentID Content that became available.
        * @param categoryID Category that the content was assigned to.
        */
        virtual void contentAvailable(ContentID contentID, Category categoryID) = 0;

        /** @brief Content became ready or timed out to get ready.
        *          See DcsmRenderer::requestContentReady for details.
        * @param contentID Content that became ready or timed out.
        * @param result OK if content became ready, TimedOut otherwise.
        */
        virtual void contentReady(ContentID contentID, DcsmRendererEventResult result) = 0;

        /** @brief Content is shown (its scene is rendered on corresponding display).
        *          This callback comes at start time of animation if any provided,
        *          see DcsmRenderer::showContent for details.
        * @param contentID Content that is shown.
        */
        virtual void contentShown(ContentID contentID) = 0;

        /** @brief Content is hidden (its scene is not rendered anymore).
        *          This callback comes at finish time of animation if any provided,
        *          see DcsmRenderer::hideContent for details.
        * @param contentID Content that is hidden.
        */
        virtual void contentHidden(ContentID contentID) = 0;

        /** @brief Content is released (its scene is not rendered anymore and resources are unloaded).
        *          This callback comes at finish time of animation if any provided,
        *          see DcsmRenderer::releaseContent for details.
        * @param contentID Content that is released.
        */
        virtual void contentReleased(ContentID contentID) = 0;

        /** @brief Called when Dcsm provider requests focus change of its content.
        *          This is a purely Dcsm related message forwarded via DcsmRenderer event mechanism,
        *          see ramses::DcsmProvider::requestContentFocus for more details.
        *          Application logic should react accordingly if it decides to accept the request,
        *          typically this means requesting the content to be ready and show afterwards using ramses::DcsmRenderer::requestContentState.
        * @param contentID Unique ID of content for which the request was made.
        */
        virtual void contentFocusRequested(ContentID contentID) = 0;

        /** @brief Called when Dcsm provider requests that its content is not used anymore
        *          and it would like to stop offering it.
        *          This is a purely Dcsm related message forwarded via DcsmRenderer event mechanism.
        *          Application logic should react by calling ramses::DcsmRenderer::acceptStopOffer (optionally with timing information),
        *          when it decides that the content is indeed no longer needed.
        * @param contentID Unique ID of content for which the request was made.
        */
        virtual void contentStopOfferRequested(ContentID contentID) = 0;

        /** @brief Content became unavailable unexpectedly.
        *          Application logic should react as fast as possible if this affects currently rendered content(s).
        * @param contentID Unique ID of content that became unavailable.
        */
        virtual void contentNotAvailable(ContentID contentID) = 0;
    };
}

#endif
