//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMRENDERER_H
#define RAMSES_DCSMRENDERER_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/DcsmApiTypes.h"

namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
    class DcsmRendererConfig;
    class IDcsmRendererEventHandler;
    class IRendererEventHandler;

    /** @brief DcsmRenderer provides way to interact with both Dcsm (as consumer) and renderer (as owner)
    *          in a unified way.
    *
    * @details DcsmRenderer's main purpose is to simplify the handling of combination of Dcsm content state
    *          and state of a its renderer scene, which can become rather difficult.
    *          DcsmRenderer presents a layer on top of RamsesRenderer and DcsmConsumer, provides a ContentState
    *          which is essentially a 'compound' state of Dcsm content and its scene state. It also allows user
    *          to define list of categories that should be managed by it, i.e. it should accept content offers
    *          for those categories.
    *
    *          Several methods (showContent, hideContent, releaseContent) allow user to give timing information,
    *          these timestamps are sent 'as is' via Dcsm to provider and it is assumed that provider and consumer
    *          agreed on how to interpret the values. DcsmRenderer only does less/equal comparison to determine
    *          which one comes first and when to execute scheduled operations, it is therefore mandated to provide
    *          the current time when using DcsmRenderer::update.
    *          Timing information has different meaning for different state transitions but only affect content's renderer scene state request timing.
    *          If there is a Dcsm messages involved in the transition it is sent always right away and carries the given timing information as is,
    *          Dcsm provider is required to react on them accordingly, see DcsmProvider documentation for details.
    *          Content's scene state change becomes effective at finishTime from given timing for most transitions
    *          except for showContent which uses the startTime instead (this is to allow fade in animation).
    *
    *          Disclaimer (TODO): DcsmRenderer operates on top of RamsesRenderer, it uses only the part of renderer's API that
    *          controls scene states, nothing else. It also internally dispatches (and consumes) renderer events, all of them,
    *          it is likely that there is part of application logic that needs to control other renderer features
    *          and needs to react on other (non scene state related) events. For that reason the DcsmRenderer::update
    *          provides way to dispatch all renderer events, additionally, to a custom handler. This way it should
    *          be possible to add DcsmRenderer to an existing application logic that already uses RamsesRenderer directly.
    *          Note that it is strongly recommended to control scene states using one API only, either DcsmRenderer or
    *          RamsesRenderer, not both.
    *          Future Ramses release should provide modified APIs where the scene control is separated from the rest
    *          and avoid any possibility of wrong doing on this level. Also it should allow application design to separate
    *          logic owning renderer and logic controlling content states.
    */
    class RAMSES_API DcsmRenderer : public StatusObject
    {
    public:
        /** @brief Constructor
        *   Disclaimer (TODO): there should never be more than single instance of DcsmRenderer, creating more will
        *               fail or result in undefined behavior. Also, given Ramses framework must be the one that
        *               was used to instantiate given renderer.
        *               Future Ramses release should provide API where it is not possible to do the instantiation wrong.
        * @param renderer Ramses renderer to operate on. See ramses::DcsmRenderer for details.
        * @param framework Ramses framework used to instantiate given renderer and will be used for Dcsm messaging.
        * @param config Parameters to be used to instantiate the DcsmRenderer.
        *               These are used only at instantiation time, modifications to config done later have no effect on this DcsmRenderer instance.
        */
        DcsmRenderer(RamsesRenderer& renderer, RamsesFramework& framework, const DcsmRendererConfig& config);

        /** @brief Requests that the provided content is ready to show.
        * @details This involves a corresponding Dcsm message sent to content provider and also request to map the content's scene to the display defined in its category.
        *          This call will fail right away (check return status) if transition is not valid (e.g. content is already shown).
        *          When content becomes ready (from both Dcsm and renderer scene perspective) an event/callback is emitted - IDcsmRendererEventHandler::contentReady,
        *          the content can be shown right after.
        *          (TODO) This call may time out, i.e. not being able to finish the transition in given time, the event/callback's result is DcsmRendererEventResult::TimedOut.
        *
        * @param contentID Content to request ready.
        * @param timeOut Time out period, state change will be canceled if not reached within this period, 0 to disable time out.
        *                The time duration units are same as time given to DcsmRenderer::update.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t requestContentReady(ContentID contentID, uint64_t timeOut);

        /** @brief Requests that the provided content is ready to show via an offscreen buffer linked to another content's texture sampler.
        * @details This is an extended version of DcsmRenderer::requestContentReady, it does all that DcsmRenderer::requestContentReady would do
        *          and in addition:
        *            - offscreen buffer with given resolution is created on display specified in the category the content is assigned to
        *            - content is assigned to the offscreen buffer (it will be rendered there when shown)
        *            - the offscreen buffer is data-linked to another content's texture sampler identified by its data ID (see Scene::createTextureConsumer)
        *          Only when all those steps are done the IDcsmRendererEventHandler::contentReady event callback is emitted.
        *
        *          Requirements for making content ready and linked:
        *            - content must be available (not ready or shown)
        *            - the contentToLinkTo must be ready or shown (not necessarily to same category but must be same display)
        *            - content and contentToLinkTo cannot be same
        *            - offscreen buffer dimensions must be non-zero
        *            - texture sampler with given ID must exist in contentToLinkTo's scene
        *
        *          When content is released or unavailable the created offscreen buffer will be destroyed and texture sampler unlinked.
        *
        * @param contentID Content to request ready and linked.
        * @param width Width of the offscreen buffer to create.
        * @param height Height of the offscreen buffer to create.
        * @param contentIDToLinkTo ID of content containing texture sampler to link the offscreen buffer to.
        * @param textureSamplerDataIDToLinkTo Data consumer ID (see Scene::createTextureConsumer) of texture sampler to link the offscreen buffer to.
        * @param timeOut Time out period, state change will be canceled if not reached within this period, 0 to disable time out.
        *                The time duration units are same as time given to DcsmRenderer::update.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t requestContentReadyAndLinkedViaOffscreenBuffer(ContentID contentID, uint32_t width, uint32_t height, ContentID contentIDToLinkTo, dataConsumerId_t textureSamplerDataIDToLinkTo, uint64_t timeOut);

        /** @brief Shows the content. The content must be ready to be shown - see DcsmRenderer::requestContentReady.
        * @details This involves a corresponding Dcsm message sent to content provider and also starts content's scene rendering.
        *          This call will fail right away (check return status) if transition is not valid (e.g. content is not ready).
        *          When content is shown an event/callback is emitted - IDcsmRendererEventHandler::contentShown.
        *
        *          This method is one of those supporting timing of state change, see ramses::DcsmRenderer for general description.
        *          The content's scene will start rendering at AnimationInformation::startTime (IDcsmRendererEventHandler::contentShown
        *          is emitted when rendering is confirmed from RamsesRenderer), AnimationInformation::finishTime sent to DcsmProvider
        *          to mark the end time of fade in animation if any.
        *
        * @param contentID Content to show.
        * @param timingInfo Timing information, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t showContent(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Hides the content. The content must be shown.
        * @details This involves a corresponding Dcsm message sent to content provider and also stops content's scene rendering.
        *          This call will fail right away (check return status) if transition is not valid (e.g. content is not shown).
        *          When content is hidden an event/callback is emitted - IDcsmRendererEventHandler::contentHidden.
        *
        *          This method is one of those supporting timing of state change, see ramses::DcsmRenderer for general description.
        *          The content's scene will stop rendering at AnimationInformation::finishTime (unlike showContent),
        *          IDcsmRendererEventHandler::contentHidden is emitted when hide is confirmed from RamsesRenderer),
        *          AnimationInformation::startTime sent to DcsmProvider to mark the start time of fade out animation if any.
        *
        * @param contentID Content to hide.
        * @param timingInfo Timing information, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Stops using content.
        * @details This call will behave similar to hideContent if the content is currently shown,
        *          after it is not rendered anymore it will unload resources that were used to render this content.
        *          The content is still known and assigned to its category and can be requested to be ready again.
        *          This involves a corresponding Dcsm message sent to content provider.
        *
        *          This method is one of those supporting timing of state change, see ramses::DcsmRenderer for general description.
        *          If the content was shown its scene will stop rendering at AnimationInformation::finishTime (similar to hideContent),
        *          and it will be unloaded from renderer (IDcsmRendererEventHandler::contentReleased is emitted when unload/unmap is confirmed
        *          from RamsesRenderer), AnimationInformation::startTime sent to DcsmProvider to mark the start time of fade out animation if any.
        *
        * @param contentID Content to release.
        * @param timingInfo Timing information, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Sets new size for given category.
        * @details Timing information is sent immediately via Dcsm to consumer to be able to react in time to this change.
        *          In case there is a new content offered for this category after this call, it will receive the new size
        *          regardless of given timing information. The timing information is assumed to be used for transition only
        *          and therefore in the rare case of new content being offered at the same time it should be configured
        *          for the new size right away.
        * @param categoryId Unique ID of the category to change size.
        * @param size New category size.
        * @param timingInfo Timing to be sent to provider, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setCategorySize(Category categoryId, SizeInfo size, AnimationInformation timingInfo);

        /** @brief Stops using given content and accepts the request from provider.
        * @details The Dcsm message is sent immediately to provider with given timing information.
        *          In addition to that there is a content's scene state change scheduled at finishTime
        *          to hide and unload the scene.
        *          This allows a fade out effect to be executed if needed.
        *          The content becomes unknown at AnimationInformation::finishTime
        *          and cannot be used until offered again (IDcsmRendererEventHandler::contentAvailable).
        *          Any attempt to change state of the content between calling this and finishTime is undefined,
        *          it is however possible to call this method again if timing needs to be adjusted.
        * @param contentID Content to stop using.
        * @param timingInfo Timing to be sent to provider, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Do one update cycle which flushes any pending renderer commands, dispatches events from both Dcsm and renderer
        *          and executes any scheduled operations to be done at given time.
        * @details If any of the actions caused a state change or an event to be reported, a corresponding callback will be called
        *          on given handler.
        *          Additionally, renderer events will execute corresponding callbacks on customRendererEventHandler if provided
        *          (see ramses::DcsmRenderer for details).
        * @param timeStampNow Timestamp which is used to execute any operations scheduled for a time at or before this timestamp (see DcsmRenderer::requestContentState for details).
        * @param eventHandler Implementation of callback handler interface to react on events. It is allowed to call DcsmRenderer API directly from the callbacks.
        * @param customRendererEventHandler Custom renderer event handler implementation which will receive all the renderer events emitted since last call (see ramses::DcsmRenderer for details).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t update(uint64_t timeStampNow, IDcsmRendererEventHandler& eventHandler, IRendererEventHandler* customRendererEventHandler = nullptr);

        /// Deleted copy constructor
        /// @param other Other
        DcsmRenderer(const DcsmRenderer& other) = delete;
        /// Deleted assignment operator
        /// @param other Other
        /// @return Instance
        DcsmRenderer& operator=(const DcsmRenderer& other) = delete;

        /// Implementation
        class DcsmRendererImpl& m_impl;
    };
}

#endif
