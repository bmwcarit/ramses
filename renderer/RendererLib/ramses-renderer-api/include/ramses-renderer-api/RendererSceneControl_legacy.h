//  -------------------------------------------------------------------------
//  Copyright (C) 2020 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERSCENECONTROL_LEGACY_H
#define RAMSES_RENDERERSCENECONTROL_LEGACY_H

#include "ramses-framework-api/StatusObject.h"

namespace ramses
{
    class IRendererSceneControlEventHandler_legacy;

    /**
    * @deprecated [Use RendererSceneControl instead]
    * @brief   Control states of scenes
    * @details Where RamsesRenderer is used to configure general rendering (create displays, control looping and other
    *          global rendering states), RendererSceneControl_legacy is used to configure which content should be rendered and where.
    *          Scenes can be assigned to display buffers, shown, hidden, data linked, etc.
    *          All the commands in this class are put to a queue and submitted only when RendererSceneControl_legacy::flush is called,
    *          they are then executed asynchronously in the renderer core, the order of execution is preserved.
    *          Most of the commands have a corresponding callback which reports the result back to the caller
    *          via RendererSceneControl_legacy::dispatchEvents.
    *          Some commands can fail immediately by returning a status with value other than StatusOK,
    *          in such case there will be no callback, because the command will not even be submitted.
    */
    class RAMSES_API RendererSceneControl_legacy : public StatusObject
    {
    public:
        /**
        * @brief Subscribes to given scene for receiving its content and future content changes.
        * @details Scene can only be subscribed if it was published (IRendererSceneControlEventHandler_legacy::scenePublished).
        *          IRendererSceneControlEventHandler_legacy::sceneSubscribed will be emitted after subscription finished.
        *          Attempt to subscribe an already subscribed scene, or generally scene in any other state than 'published' will fail.
        *          It is allowed to call RendererSceneControl_legacy::unsubscribeScene after subscription requested but before
        *          the subscription finished, effectively canceling the request.
        *
        * @param sceneId The id of the scene to subscribe to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t subscribeScene(sceneId_t sceneId);

        /**
        * @brief Unsubscribes from receiving content updates of given scene and
        *        removes it from renderer.
        * @details IRendererSceneControlEventHandler_legacy::sceneUnsubscribed will be emitted after unsubscription finished.
        *          Attempt to unsubscribe scene which is not in subscribed state or requested to be subscribed will fail.
        *          It is allowed to call RendererSceneControl_legacy::unsubscribeScene after subscription requested but before
        *          the subscription finished, effectively canceling the request.
        *
        * @param sceneId The id of the scene to unsubscribe from
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t unsubscribeScene(sceneId_t sceneId);

        /**
        * @brief Will map scene to a display and upload resources it is using to video memory.
        * @details IRendererSceneControlEventHandler_legacy::sceneMapped will be emitted after mapping and uploading finished.
        *          Attempt to map an already mapped scene, or generally scene in any other state than 'subscribed' will fail,
        *          as well as mapping to an invalid display.
        *          NOTE: this operation can take longer time depending on number and size of resources in use.
        *          The scene will be assigned to display's framebuffer by default (IRendererSceneControlEventHandler_legacy::sceneAssignedToDisplayBuffer
        *          will not be called), this assignment and render order can be changed at any time using RendererSceneControl_legacy::assignSceneToDisplayBuffer.
        *          It is allowed to call RendererSceneControl_legacy::unmapScene after mapping requested but before
        *          the mapping finished, effectively canceling the request.
        *
        * @param[in] displayId id of display to map scene to
        * @param[in] sceneId id of scene to map
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t mapScene(displayId_t displayId, sceneId_t sceneId);

        /**
        * @brief Unmaps scene from display it is mapped to and unloads its resources.
        * @details IRendererSceneControlEventHandler_legacy::sceneUnmapped will be emitted after unmapping finished.
        *          Attempt to unmap scene which is not in mapped state or requested to be mapped will fail.
        *          It is allowed to call RendererSceneControl_legacy::unmapScene after mapping requested but before
        *          the mapping finished, effectively canceling the request.
        *          NOTE: it is not guaranteed that resources will be freed from video memory depending on renderer caching policy.
        *
        * @param[in] sceneId id of scene to unmap
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t unmapScene(sceneId_t sceneId);

        /**
        * @brief Will start rendering scene into display buffer it is assigned to.
        * @details IRendererSceneControlEventHandler_legacy::sceneShown will be emitted after scene is enabled for rendering.
        *          Attempt to show an already shown scene, or generally scene in any other state than 'mapped' will fail.
        *          It is allowed to call RendererSceneControl_legacy::hideScene after show requested but before
        *          the showing is executed, effectively canceling the request.
        *
        * @param[in] sceneId id of scene to render
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t showScene(sceneId_t sceneId);

        /**
        * @brief Will stop rendering scene.
        * @details IRendererSceneControlEventHandler_legacy::sceneHidden will be emitted after rendering stopped.
        *          Attempt to hide scene which is not in shown state or requested to be shown will fail.
        *          It is allowed to call RendererSceneControl_legacy::hideScene after show requested but before
        *          the showing is executed, effectively canceling the request.
        *          Resources used by the scene will be kept in video memory even when hidden, this allows
        *          fast toggling of show/hide (typically with not more than 1 frame latency).
        *          In order to free up video memory, unmap scene using RendererSceneControl_legacy::unmapScene.
        *
        * @param[in] sceneId id of scene to hide
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t hideScene(sceneId_t sceneId);

        /**
        * @brief When a scene is mapped and shown it is by default rendered directly to display's framebuffer.
        *        However scene can be assigned to any of the display's offscreen buffers at any point in time while mapped or shown.
        *        Scene's shown/hidden state is not affected by this assignment.
        *
        *        Assigning a scene to framebuffer or offscreen buffer changes the way its render order is determined.
        *        The rendering order of following buffer groups is fixed:
        *          1. Offscreen buffers
        *          2. Framebuffer
        *          3. Interruptible offscreen buffers
        *        The scene render order only guarantees the order in scope of the buffer the scene is assigned to.
        *
        *        IRendererSceneControlEventHandler_legacy::sceneAssignedToDisplayBuffer will be emitted when scene is assigned.
        *        The assignment will fail if scene not in mapped or shown state, if trying to assign to a display buffer
        *        that does not exist or does not belong to the display the scene is mapped to.
        *
        * @param[in] sceneId Id of scene that should be assigned to the display buffer.
        * @param[in] displayBuffer Id of display buffer (framebuffer or offscreen buffer) the scene should be assigned to.
        *                          If provided buffer Id is invalid, framebuffer of display where scene is mapped is used.
        * @param[in] sceneRenderOrder Lower value means that a scene is rendered before a scene with higher value. Default is 0.
        *                             The render order is guaranteed only in the scope of the buffer it is mapped to (framebuffer or offscreen buffer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t assignSceneToDisplayBuffer(sceneId_t sceneId, displayBufferId_t displayBuffer, int32_t sceneRenderOrder = 0);

        /**
        * @brief Sets clear color for display's framebuffer or offscreen buffer.
        *        Clear color is used to clear the whole buffer at the beginning of a rendering cycle (typically every frame).
        *        There is no event callback for this operation, the clear color change be assumed to be effective in the next frame.
        *
        * @param[in] display Id of display that the buffer to set clear color belongs to.
        * @param[in] displayBuffer Id of display buffer to set clear color,
        *                          if ramses::displayBufferId_t::Invalid() is passed then the clear color is set for display's framebuffer.
        * @param[in] r Clear color red channel value [0,1]
        * @param[in] g Clear color green channel value [0,1]
        * @param[in] b Clear color blue channel value [0,1]
        * @param[in] a Clear color alpha channel value [0,1]
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer = displayBufferId_t::Invalid(), float r = 0.f, float g = 0.f, float b = 0.f, float a = 1.f);

        /**
        * @brief Links a data provider to a data consumer across two scenes.
        *        Data provider and data consumer must be created via client scene API and their data type must match.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        *        A consumer within a scene can be linked to exactly one provider.
        *        IRendererSceneControlEventHandler_legacy::dataLinked will be emitted after data provider linked to consumer.
        *        If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *        however if the new link fails it is undefined whether previous link was discarded or not.
        * @param providerScene The id of the scene which provides the data.
        * @param providerId The id of the data provider within the providerScene.
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkData(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId);

        /**
        * @brief Links an offscreen buffer to a data consumer.
        *        Same as texture linking where an offscreen buffer acts as texture provider.
        *        Offscreen buffer can be used as texture input in one or more scene's texture sampler(s).
        *        In order to link, the scene must be mapped to the same display which has the offscreen buffer.
        *        If the data consumer is already linked to a provider (data or offscreen buffer), the old link will be discarded,
        *        however if the new link fails it is undefined whether previous link was discarded or not.
        *        Note: To unlink offscreen buffer use \c unlinkData as with any other type of data linking.
        *        IRendererSceneControlEventHandler_legacy::offscreenBufferLinkedToSceneData will be emitted after offscreen buffer linked to consumer.
        * @param[in] offscreenBufferId The id of the offscreen buffer to use as texture provider.
        *                              The display that owns the buffer is determined from where the consumer scene is mapped to.
        * @param[in] consumerSceneId The id of the scene which consumes the data.
        * @param[in] consumerDataSlotId The id of the data consumer within the consumerScene.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t linkOffscreenBufferToSceneData(displayBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        /**
        * @brief Removes an existing link between two scenes (see RendererSceneControl_legacy::linkData()).
        *        Consumer scene and id are enough, since a consumer can be linked to exactly one provider, which is already
        *        known from the call to linkData()
        *        IRendererSceneControlEventHandler_legacy::dataUnlinked will be emitted after data consumer unlinked from provider.
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee success, the result argument in dispatched event has its own status.
        */
        status_t unlinkData(sceneId_t consumerScene, dataConsumerId_t consumerId);

        /**
        * @brief Most RendererSceneControl_legacy methods push commands to an internal queue which is submitted
        *        when calling RendererSceneControl_legacy::flush. The commands are then executed during a render loop
        *        (RamsesRenderer::doOneLoop or in a render thread if used RamsesRenderer::startThread).
        *        Some of these calls result in an event (can be both informational and data).
        *        Such events and their result can be retrieved using the dispatchEvents call.
        *        *IMPORTANT* Scene control events must be regularly consumed by calling dispatchEvents()
        *        in order to prevent buffer overflow of the internal event queue,
        *        even if the application is not interested in those events.
        *
        * @param eventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                     Check ramses::IRendererSceneControlEventHandler_legacy documentation for more details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IRendererSceneControlEventHandler_legacy& eventHandler);

        /**
        * @brief Submits scene control commands (API calls on RendererSceneControl_legacy)
        *        since previous flush to be executed in the next renderer update loop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t flush();

        /**
        * Stores internal data for implementation specifics
        */
        class RendererSceneControlImpl_legacy& impl;

        /// Deleted default constructor
        RendererSceneControl_legacy() = delete;

        /// Deleted copy constructor
        RendererSceneControl_legacy(const RendererSceneControl_legacy&) = delete;

        /**
        * @brief Deleted copy assignment
        * @param other unused
        * @return unused
        */
        RendererSceneControl_legacy& operator=(const RendererSceneControl_legacy& other) = delete;

    private:
        /**
        * @brief RendererSceneControl_legacy can only be instantiated through RamsesRenderer
        */
        friend class RamsesRendererImpl;

        /**
        * @brief Destructor
        */
        virtual ~RendererSceneControl_legacy();

        /**
        * @brief Constructor
        */
        explicit RendererSceneControl_legacy(RendererSceneControlImpl_legacy&);
    };
}

#endif
