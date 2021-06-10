//  -------------------------------------------------------------------------
//  Copyright (C) 2019 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DCSMCONTENTCONTROL_H
#define RAMSES_DCSMCONTENTCONTROL_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/DcsmApiTypes.h"
#include "ramses-framework-api/CategoryInfoUpdate.h"

namespace ramses
{
    class RamsesRenderer;
    class RamsesFramework;
    class IDcsmContentControlEventHandler;
    class IRendererEventHandler;
    class DcsmStatusMessage;

    /** @brief DcsmContentControl provides way to interact with both Dcsm (as consumer) and renderer content control
    *          (replaces #ramses::RendererSceneControl).
    *
    * @details DcsmContentControl's main purpose is to simplify the handling of combination of Dcsm content state
    *          and state of its renderer scene, which can become rather complicated if dealt with separately.
    *          DcsmContentControl presents an alternative to #ramses::RendererSceneControl together with a layer
    *          on top of #ramses::DcsmConsumer. It unifies control of content state in Dcsm context and content's
    *          scene state in renderer context.
    *          At initialization user must specify list of Dcsm categories that should be managed by it,
    *          DcsmContentControl will then automatically accept Dcsm content offers for those categories.
    *
    *          \section TimingInformation
    *          Several methods (#ramses::DcsmContentControl::showContent, #ramses::DcsmContentControl::hideContent, #ramses::DcsmContentControl::releaseContent, ...) allow user to give timing information,
    *          these timestamps are sent 'as is' via Dcsm to provider and it is assumed that provider and consumer
    *          agreed on how to interpret the values. DcsmContentControl only does less/equal comparison to determine
    *          which one comes first and when to execute scheduled operations, it is therefore mandated to provide
    *          the current time when using #ramses::DcsmContentControl::update.
    *          Timing information has different meaning for different state transitions but only affect content's renderer scene state request timing.
    *          If there is a Dcsm messages involved in the transition it is sent always right away and carries the given timing information as is,
    *          Dcsm provider is required to react on them accordingly, see DcsmProvider documentation for details.
    *          Content's scene state change becomes effective at finishTime from given timing for most transitions
    *          except for showContent which uses the startTime instead (this is to allow fade in animation).
    *
    *          \section RequestTimeout
    *          Some methods (#ramses::DcsmContentControl::requestContentReady) have a \c timeOut parameter.
    *          Requesting content to be ready is potentially a heavy operation that consists of preparing, transferring and uploading
    *          content data and resources. All these operations are executed asynchronously and many of them do not fail directly,
    *          but might never finish (e.g. missing resource or some other unresolved dependency of content).
    *          The \c timeOut parameter can be useful to handle such situations, application can decide if it retries the request
    *          or take other measures.
    *          Similar to timing information described above, the timeOut depends on \c timeStampNow passed to
    *          #ramses::DcsmContentControl::update. A ready request times out when #ramses::DcsmContentControl::update is called with \c timeStampNow,
    *          which is greater than \c requestTimeOutTimeStamp:
    *               \c requestTimeOutTimeStamp = \c timeStampNowAtRequest + \c timeOut.
    *
    *          Example:
    *               \code{.cpp}
    *               update(10);
    *               update(20);
    *               requestContentReady(myContentID, 30);  // current time is 20, timeOut in 30 time units, requestTimeOutTimeStamp = 20 + 30 = 50
    *               update(30);
    *               update(40);
    *               update(50); // if not reached ready till this point, request timed out
    *               \endcode
    */
    class RAMSES_API DcsmContentControl : public StatusObject
    {
    public:
        /** @brief Add a content category
        * @details Adds a content category for receiving content offers. At least RenderSize and CategoryRect have to be set (width or height cannot be 0) on the
        *          CategoryInfoUpdate or else the category will not be added and error status will be returned.
        *
        *          Be aware that assigning categories to different displays will disable their contents ability to share the same
        *          technical content with each other, since technical content can only be shown on one display at a time.
        *
        * @param category Category to add
        * @param display display that category should be mapped to
        * @param categoryInformation information about the category being added
        */
        status_t addContentCategory(Category category, displayId_t display, const CategoryInfoUpdate& categoryInformation);

        /** @brief Remove a content category
        * @details Removes a content category. No more offers will be received for this category, any content assigned to this category will
        *          be dropped (Hide, Release)
        *
        * @param category Category to remove
        */
        status_t removeContentCategory(Category category);

        /** @brief Requests that the provided content is ready to show.
        * @details This involves a corresponding Dcsm message sent to content provider and also request to map the content's scene to the display defined in its category.
        *          This call will fail right away (check return status) if transition is not valid (e.g. content is already shown).
        *          When content becomes ready (from both Dcsm and renderer scene perspective) an event/callback is emitted - #ramses::IDcsmContentControlEventHandler::contentReady,
        *          the content can be shown right after.
        *          This call may time out, i.e. not being able to finish the transition in given time, the event/callback's result is #ramses::DcsmContentControlEventResult::TimedOut.
        *          See \ref RequestTimeout for more details.
        *          It is valid to call this method more than once to change the \c timeOut value, but only if the ready state was not reached yet.
        *
        * @param contentID Content to request ready.
        * @param timeOut Time out period, state change will be canceled if not reached within this period, 0 to disable time out.
        *                The timeout period is measured from \c timeStampNow passed to most recent call of #ramses::DcsmContentControl::update and uses same units.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t requestContentReady(ContentID contentID, uint64_t timeOut);

        /** @brief Shows the content. The content must be ready to be shown - see #ramses::DcsmContentControl::requestContentReady.
        * @details This involves a corresponding Dcsm message sent to content provider and also starts content's scene rendering.
        *          This call will fail right away (check return status) if transition is not valid (e.g. content is not ready).
        *          When content is shown an event/callback is emitted - #ramses::IDcsmContentControlEventHandler::contentShown.
        *
        *          This method is one of those supporting timing of state change, see #ramses::DcsmContentControl for general description.
        *          The content's scene will start rendering at AnimationInformation::startTime (#ramses::IDcsmContentControlEventHandler::contentShown
        *          is emitted when rendering is confirmed from RamsesRenderer), AnimationInformation::finishTime sent to DcsmProvider
        *          to mark the end time of fade in animation if any. See \ref TimingInformation for more details.
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
        *          Once content is not shown anymore an event/callback is emitted - #ramses::IDcsmContentControlEventHandler::contentReady.
        *          The content can be requested to be shown again anytime later as long as it stays ready.
        *
        *          This method is one of those supporting timing of state change, see #ramses::DcsmContentControl for general description.
        *          The content's scene will stop rendering at AnimationInformation::finishTime (unlike showContent),
        *          #ramses::IDcsmContentControlEventHandler::contentReady is emitted when hide is confirmed by the renderer),
        *          AnimationInformation::startTime sent to DcsmProvider to mark the start time of fade out animation if any.
        *          See \ref TimingInformation for more details.
        *
        * @param contentID Content to hide.
        * @param timingInfo Timing information, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t hideContent(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Stops using content.
        * @details This call will behave similar to hideContent if the content is currently shown,
        *          but in addition after it is not rendered anymore it will unload resources that were used to render this content.
        *          Once content is released an event/callback is emitted - #ramses::IDcsmContentControlEventHandler::contentAvailable.
        *          The content is still known and assigned to its category and can be requested to be ready again.
        *          This involves a corresponding Dcsm message sent to content provider.
        *
        *          This method is one of those supporting timing of state change, see #ramses::DcsmContentControl for general description.
        *          If the content was shown its scene will stop rendering at AnimationInformation::finishTime (similar to hideContent),
        *          and it will be unloaded from renderer (#ramses::IDcsmContentControlEventHandler::contentAvailable is emitted when unload/unmap is confirmed
        *          by the renderer), AnimationInformation::startTime sent to DcsmProvider to mark the start time of fade out animation if any.
        *          See \ref TimingInformation for more details.
        *
        * @param contentID Content to release.
        * @param timingInfo Timing information, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t releaseContent(ContentID contentID, AnimationInformation timingInfo);

        /** @brief Sets new category info for given category.
        * @details Timing information is sent immediately via Dcsm to consumer to be able to react in time to this change.
        *          In case there is a new content offered for this category after this call, it will receive the new info
        *          regardless of given timing information. The timing information is assumed to be used for transition only
        *          and therefore in the rare case of new content being offered at the same time it should be configured
        *          for the new info right away.
        * @param categoryId Unique ID of the category to change info.
        * @param categoryInfo New category info.
        * @param timingInfo Timing to be sent to provider, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setCategoryInfo(Category categoryId, const CategoryInfoUpdate& categoryInfo, AnimationInformation timingInfo);

        /** @brief Stops using given content and accepts the request from provider.
        * @details The Dcsm message is sent immediately to provider with given timing information.
        *          In addition to that there is a content's scene state change scheduled at finishTime
        *          to hide and unload the scene.
        *          This allows a fade out effect to be executed if needed.
        *          The content becomes unknown immediately and cannot be used until
        *          offered again (#ramses::IDcsmContentControlEventHandler::contentAvailable).
        *          Any attempt to change state of the content between calling this and finishTime is undefined,
        *          it is however possible to call this method again if timing needs to be adjusted.
        *          See \ref TimingInformation for more details.
        * @param contentID Content to stop using.
        * @param timingInfo Timing to be sent to provider, see above for details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t acceptStopOffer(ContentID contentID, AnimationInformation timingInfo);

        /**
        * @brief Redirects rendering output of a content to a display buffer.
        * @details When content is shown it is by default rendered to a framebuffer that belongs to a display associated with content's category.
        *          However a content can be assigned to any of the display's offscreen buffers at any point in time after the content became ready.
        *          Content's ready/shown/hidden state is not affected by this assignment.
        *
        *          Assigning content to a framebuffer or an offscreen buffer changes the way its render order is determined.
        *          The rendering order of following buffer groups is fixed:
        *            1. Offscreen buffers
        *            2. Framebuffer
        *            3. Interruptible offscreen buffers
        *          The content render order only guarantees the order in scope of the buffer the content is assigned to.
        *
        *          The assignment will fail if content unknown or not ready (at least DCSM ready reported by DCSM provider),
        *          if trying to assign to a display buffer that does not exist or does not belong to the display associated with content's category.
        *          Assignment must be repeated if content state drops to available or unknown and becomes ready again.
        *
        * @param[in] contentID Id of content that should be assigned to the display buffer.
        * @param[in] displayBuffer Id of display buffer (framebuffer or offscreen buffer) the content should be assigned to.
        * @param[in] renderOrder Lower value means that a content is rendered before a content with higher value. Default is 0.
        *                        The render order is guaranteed only in the scope of the buffer it is assigned to (framebuffer or offscreen buffer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t assignContentToDisplayBuffer(ContentID contentID, displayBufferId_t displayBuffer, int32_t renderOrder = 0);

        /** @brief Creates a data link between offscreen buffer and a consumer data slot defined in Ramses scene.
        * @details When linked, the offscreen buffer contents can be used as a texture input on the consumer side (see ramses::Scene::createTextureConsumer).
        *          This way a consumer scene exposes its data slots and \c dataConsumerId_t are then needed to create the link here on renderer side.
        *          Offscreen buffer can be linked to multiple consumer slots.
        *          The consumer data slot type must be of type texture consumer (see ramses::Scene::createTextureConsumer)
        *          in order to successfully link them. Also the consumer content must be ready (#ramses::DcsmContentControl::requestContentReady).
        *          This call results in an event which can be dispatched via #ramses::IDcsmContentControlEventHandler::offscreenBufferLinked.
        *          If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *
        * @param offscreenBufferId Offscreen buffer ID to be linked to consumer.
        * @param consumerContentID Content with scene containing data consumer with given \c consumerId.
        * @param consumerId Scene's data consumer ID.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkOffscreenBuffer(displayBufferId_t offscreenBufferId, ContentID consumerContentID, dataConsumerId_t consumerId);

        /** @brief Creates a data link between given content as texture and a consumer data slot defined in Ramses scene.
        * @details When linked, the content can be used as a texture input on the consumer side (see #ramses::Scene::createTextureConsumer).
        *          This can only be done if
        *          - content comes from a ramses scene and has been assigned to on offscreen buffer before.
        *          - content comes from a wayland surface
        *          Content can be linked to multiple consumer slots.
        *          The consumer data slot type must be of type texture consumer (see #ramses::Scene::createTextureConsumer)
        *          in order to successfully link them.
        *          Both the consumer content and provider content must at least ready (#ramses::DcsmContentControl::requestContentReady).
        *          This call results in an event which can be dispatched via #ramses::IDcsmContentControlEventHandler::contentLinkedToTextureConsumer.
        *          If the data consumer is already linked to a provider, the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *          This method is more generic and powerful than #ramses::DcsmContentControl::linkOffscreenBuffer and supercedes it.
        *
        * @param contentID Content to be linked as a texture
        * @param consumerContentID Content with scene containing data consumer with given \c consumerId.
        * @param consumerId Scene's data consumer ID.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkContentToTextureConsumer(ContentID contentID, ContentID consumerContentID, dataConsumerId_t consumerId);

        /** @brief Creates a data link between data slots defined in Ramses scene.
        * @details The purpose of data linking is to provide data from one content's scene to another content's scene.
        *          While linked, the data flows from provider to consumer, i.e. a data value set on provider's side
        *          overwrites the linked data value on consumer's side.
        *          Various data slots can be marked as data provider/consumer in Ramses scene,
        *          see ramses::Scene for details (e.g. ramses::Scene::createDataProvider).
        *          This way a scene exposes its data slots and \c dataProviderId_t and \c dataConsumerId_t are then needed
        *          to create the link here on renderer side.
        *          Only data slot marked as provider/consumer can be used as source/destination respectively. A provider slot
        *          can be linked to multiple consumer slots, a consumer slot can be linked to exactly one provider.
        *          The data link type and underlying data type must match in order to successfully link them.
        *          Both provider content's and consumer content's categories have to be mapped to the same display
        *          with #ramses::DcsmContentControl::addContentCategory. Also both the contents must be known,
        *          i.e. it was made ready (#ramses::DcsmContentControl::requestContentReady) at least once.
        *          This call results in an event which can be dispatched via ##ramses::IDcsmContentControlEventHandler::dataLinked.
        *          If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *          however if the new link fails it is undefined whether previous link was discarded or not.
        *
        * @param providerContentID Content with scene containing data provider with given \c providerId.
        * @param providerId Scene's data provider ID.
        * @param consumerContentID Content with scene containing data consumer with given \c consumerId.
        * @param consumerId Scene's data consumer ID.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkData(ContentID providerContentID, dataProviderId_t providerId, ContentID consumerContentID, dataConsumerId_t consumerId);

        /**
        * @brief   Removes an existing link between data provider and consumer (#linkData)
        *          or offscreen buffer and consumer (#linkOffscreenBuffer).
        * @details Data link is fully defined by consumer content and its data slot as there can only be one link from consumer to provider.
        *          #ramses::IDcsmContentControlEventHandler::dataUnlinked will be emitted after consumer unlinked from provider.
        *          If successful the operation can be assumed to be effective in the next frame consumer content is rendered after #update.
        *
        * @param consumerContentID Consumer content containing consumer slot to unlink.
        * @param consumerId ID of consumer slot to unlink.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unlinkData(ContentID consumerContentID, dataConsumerId_t consumerId);

        /*
        * @brief Trigger renderer to test if given pick event with coordinates intersects with any instances
        *        of ramses::PickableObject contained in given content's scene. If so, the intersected PickableObjects are
        *        reported to the ramses::DcsmContentControl (see ramses::IDcsmContentControlEventHandler::objectsPicked) using their user IDs
        *        given at creation time (see ramses::Scene::createPickableObject).
        *
        * @details \section Coordinates
        *          Coordinates normalized to range <-1, 1> where (-1, -1) is bottom left corner of the buffer where content is mapped to
        *          and (1, 1) is top right corner.
        *          If the content to test is rendered directly to framebuffer then display size should be used,
        *          i.e. (-1, -1) is bottom left corner of the display and (1, 1) top right corner of display.
        *          If the content is mapped to an offscreen buffer and rendered as a texture mapped
        *          on a mesh in another content, the given coordinates need to be mapped to the offscreen buffer
        *          dimensions in the same way.
        *          For example if the content's offscreen buffer is mapped on a 2D quad placed somewhere on screen
        *          then the coordinates provided need to be within the region of the 2D quad, i.e. (-1, -1) at bottom left corner of the quad and (1, 1) at top right corner.
        *
        * @param contentID Content to check for intersected PickableObjects.
        * @param bufferNormalizedCoordX Normalized X pick coordinate within buffer size (see \ref Coordinates).
        * @param bufferNormalizedCoordY Normalized Y pick coordinate within buffer size (see \ref Coordinates).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t handlePickEvent(ContentID contentID, float bufferNormalizedCoordX, float bufferNormalizedCoordY);

        /** @brief Do one update cycle which flushes any pending commands, dispatches events from both Dcsm and renderer's scene control
        *          and executes any scheduled operations to be done at given time.
        * @details If any of the actions caused a state change or an event to be reported, a corresponding callback will be called
        *          on given handler.
        * @param timeStampNow Timestamp which is used to execute any operations scheduled for a time at or before this timestamp (see \ref TimingInformation and \ref RequestTimeout for details).
        *                     Timestamp provided must be greater than or equal to timestamp from last update call.
        * @param eventHandler Implementation of callback handler interface to react on events. It is allowed to call #ramses::DcsmContentControl API directly from the callbacks.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t update(uint64_t timeStampNow, IDcsmContentControlEventHandler& eventHandler);

        /**
         * @brief Send a message to the provider of the content assigned to this content control.
         *
         * @param contentID Content for which the message should be sent.
         * @param message An implementation of DcsmStatusMessage, containing data to be carried to the provider.
         *
         * @return StatusOK for success, otherwise the returned status can be used
         *         to resolve error message using getStatusMessage().
         */
        status_t sendContentStatus(ContentID contentID, DcsmStatusMessage const& message);

        /// Deleted default constructor
        DcsmContentControl() = delete;

        /// Deleted copy constructor
        DcsmContentControl(const DcsmContentControl&) = delete;

        /**
        * @brief Deleted copy assignment
        * @param other unused
        * @return unused
        */
        DcsmContentControl& operator=(const DcsmContentControl& other) = delete;

        /// Implementation
        class DcsmContentControlImpl& m_impl;

    private:
        /// DcsmContentControl can only be instantiated through RamsesRenderer
        friend class RamsesRendererImpl;

        /// Destructor
        virtual ~DcsmContentControl();

        /// Constructor
        explicit DcsmContentControl(DcsmContentControlImpl&);
    };
}

#endif
