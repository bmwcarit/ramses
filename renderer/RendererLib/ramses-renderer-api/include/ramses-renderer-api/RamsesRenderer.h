//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RAMSESRENDERER_H
#define RAMSES_RAMSESRENDERER_H

#include "ramses-framework-api/StatusObject.h"
#include "ramses-renderer-api/RendererConfig.h"
#include "ramses-framework-api/RamsesFramework.h"

namespace ramses
{
    class SystemCompositorController;
    class DisplayConfig;
    class IRendererEventHandler;
    class WarpingMeshData;

    /**
    * @brief RamsesRenderer is the main renderer component
    *        which provides API to configure and control the way
    *        content will be rendered on display(s).
    */
    class RAMSES_API RamsesRenderer : public StatusObject
    {
    public:
        /**
        * @brief Constructor of RamsesRenderer
        *
        * @param[in] framework Reference to shared ramses framework components (potentially shared with ramses client)
        * @param[in] config Set of configuration flags and attributes
        */
        RamsesRenderer(RamsesFramework& framework, const RendererConfig& config);

        /**
        * @brief Destructor of RamsesRenderer
        */
        virtual ~RamsesRenderer() override;

        /**
        * @brief Prepare content to be rendered in next frame and render next frame.
        *        This function can not be used in combination with startThread, stopThread and setMaximumFramerate.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t doOneLoop();

        /**
        * @brief Runs the renderer loop in a separate thread created and owned by the renderer.
        *        This function can not be used in combination with doOneLoop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t startThread();

        /**
        * @brief Stops the thread running the renderer loop that was started by calling startThread.
        *        A call to this function joins the renderer thread to wait for its termination.
        *        This function can only be used if startThread was successfully called before.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t stopThread();

        /**
        * @brief Get the current state of render thread running
        *
        * @return Returns true if thread is running (started and not stopped), false otherwise.
        */
        bool isThreadRunning() const;

        /**
        * @brief Sets the maximum frame rate per second for the render loop when it is running in renderer
        *        thread using startThread. The parameter is of type float because it is possible to
        *        specify numbers between 0 and 1 as well, which will result in FPS lower than 1, e.g. 0.5
        *
        *        This function can not be used in combination with doOneLoop.
        * @param maximumFramerate The maximum frame rate per second to set for the render loop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setMaximumFramerate(float maximumFramerate);

        /**
        * @brief Get the current value for maximum frame rate per second set by the user.
        *        if the user did not set a value then it returs the default value.
        *
        * @return Maximum frame rate per second
        */
        float getMaximumFramerate() const;

        /**
        * @brief Sets the mode of operation for render loop. This function affects the behavior of
        *        BOTH doOneLoop and startThread. Mode can be changed during run-time, in case of thread
        *        rendering also while the thread is running.
        *        By default loop mode is set to render and update.
        *
        *        For more details about each loop mode check enum.
        *
        * @param loopMode The mode to be used for render loop.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setLoopMode(ELoopMode loopMode);

        /**
        * @brief Get the current value for loop mode set using setLoopMode
        *
        * @return The loop mode
        */
        ELoopMode getLoopMode() const;

        /**
        * @brief Sets time limits for time-out of different sections of render and update loop.
        * @details Setting time limits for render/update loop sections instructs the renderer to monitor the amount of time
        *          consumed by the sections and interrupt their execution if the set time limit was exceeded.
        *          The time limit for every section is calculated since beginning of frame.
        *          If a section is interrupted the renderer will execute the sections in the same order in the next frame,
        *          but it will continue to execute the interrupted section(s) from the point where it stopped.
        *
        *          Since time limits are calculated relative to the start of the frame, the values set
        *          should be monotonically increasing in the order of:
        *            1. Uploading client resources
        *            2. Applying scene actions
        *            3. Rendering scenes mapped to interruptible offscreen buffers
        *
        *          By default sections have infinite time limit, so renderer would not try to interrupt their execution.
        *
        * !! IMPORTANT !! Scene resource actions can not be interrupted like other resources. Therefore, if this timer is exceeded, a scene will be
        * force-unsubscribed. Use this timer with caution and merely as a sanity check, NOT as a performance measure! Scenes should not be over-using
        * scene resources, precisely because they can not be interrupted.
        *
        * @param[in] limitForSceneResourcesUpload  Time limit in microseconds (since beginning of frame) for uploading scene resources to GPU
        * @param[in] limitForClientResourcesUpload Time limit in microseconds (since beginning of frame) for uploading client resources to GPU
        * @param[in] limitForSceneActionsApply     Time limit in microseconds (since beginning of frame) for applying scene changes coming from Ramses client side
        * @param[in] limitForOffscreenBufferRender Time limit in microseconds (since beginning of frame) for rendering scenes that are mapped to interruptible offscreen buffers
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForSceneActionsApply, uint64_t limitForOffscreenBufferRender);

        /**
        * @brief Sets the number of pending flushes accepted before force-applying them to their scene, or forcefully insubscribing the scene.
        *
        * @details RAMSES guarantees that a scene flush is only going to be rendered if all resources (textures,
        * shaders etc.) are received and uploaded to the GPU. This allows a misbehaving client to flood a renderer
        * with flushes which are never executed, causing the renderer to exhaust heap memory. If too many pending flushes
        * are received and queued, they are force-applied to the scene, even if some resources are missing, thus potentially
        * causing flickering. This method allows overriding the number of flushes after a scene is updated to the latest
        * flush and all pending flushes wiped from memory. A low number (e.g. 1) will cause frequent flickering,
        * whereas a high number (e.g. 100000) could cause the renderer to go out of memory. The second parameter
        * (forceUnsubscribeSceneLimit) controls after how many pending flushes a scene is completely unsubscribed.
        * This can be used as a protection against malicious remote scenes. It does not affect local scenes though.
        * It is advisable to set forceUnsubscribeSceneLimit to higher number than forceApplyFlushLimit,
        * because re-subscribing a scene is causing a lot of network traffic and unnecessary memory operations, not to mention the
        * scene is then also not visible until re-subscribed, mapped and shown.
        *
        * @param[in] forceApplyFlushLimit Number of flushes that can be pending before force applying occurs.
        * @param[in] forceUnsubscribeSceneLimit Number of flushes that can be pending before force un-subscribe occurs.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);

        /**
        * @brief     Enable or disable skipping of rendering of unmodified buffers.
        *            By default the renderer does not re-render buffers (framebuffer or offscreen buffer)
        *            if there was no change to any of the content assigned to the buffer.
        *            This can save hardware resources or free up resources for rendering of other buffers.
        *            It can be however desired to disable such optimization for profiling of worst case scenario
        *            or debugging of a graphical glitch.
        *
        * @param[in] enable  Enable or disable the feature (enabled initially)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t setSkippingOfUnmodifiedBuffers(bool enable = true);

        /**
         * @brief Creates a display based on provided display config.
         *        Creation of a display is an asynchronous action and the display will be created during the next render loop.
         *        The result of the createDisplay can be retrieved via dispatchEvents which will trigger a callback
         *        after the asynchronous action was processed.
         *
         * @param config The display config to create and configure the new display.
         * @return Display id that can be used to refer to the created display.
         *         InvalidDisplayId in case of error. Display creation can still fail even
         *         if a valid display id is returned, the result of the actual creation
         *         can be retrieved via dispatchEvents.
         */
        displayId_t createDisplay(const DisplayConfig& config);

        /**
        * @brief Destroy a display.
        *        Destruction of a display is an asynchronous action and the actual display components (window, device, etc.)
        *        will be released during the next render loop.
        *        The result of the destroyDisplay can be retrieved via dispatchEvents which will trigger a callback with the result
        *        after the asynchronous action was processed.
        *
        * @param displayId The display id of the display to destroy.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t destroyDisplay(displayId_t displayId);

        /**
        * @brief Subscribes to receiving scene actions from a scene.
        *
        * @param sceneId The id of the scene to subscribe to
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t subscribeScene(sceneId_t sceneId);

        /**
        * @brief Unsubscribes from receiving scene actions from a scene and
        * removes it from renderer.
        *
        * @param sceneId The id of the scene to unsubscribe from
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unsubscribeScene(sceneId_t sceneId);

        /**
        * @brief Links a data provider to a data consumer across two scenes.
        *        Data provider and data consumer must be created via client scene API
        *        and their data type must match.
        *        Linking data means that the consumer's data property will be overridden by provider's data property.
        *        A consumer within a scene can be linked to exactly one provider.
        * @param providerScene The id of the scene which provides the data.
        * @param providerId The id of the data provider within the providerScene.
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkData(sceneId_t providerScene, dataProviderId_t providerId, sceneId_t consumerScene, dataConsumerId_t consumerId);

        /**
        * @brief Links an offscreen buffer to a data consumer.
        *        Same as texture linking where an offscreen buffer acts as texture provider.
        *        Offscreen buffer can be used as texture input in one or more scene's texture sampler(s).
        *        In order to link, the scene must be mapped to the same display which has the offscreen buffer.
        *        Note: To unlink offscreen buffer use \c unlinkData as with any other type of data linking.
        * @param[in] offscreenBufferId The id of the offscreen buffer to use as texture provider.
        *                              The display that owns the buffer is determined from where the consumer scene is mapped to.
        * @param[in] consumerSceneId The id of the scene which consumes the data.
        * @param[in] consumerDataSlotId The id of the data consumer within the consumerScene.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t linkOffscreenBufferToSceneData(offscreenBufferId_t offscreenBufferId, sceneId_t consumerSceneId, dataConsumerId_t consumerDataSlotId);

        /**
        * @brief Removes an existing link between two scenes (see RamsesRenderer::linkData()).
        *        Consumer scene and id are enough, since a consumer can be linked to exactly one provider, which is already
        *        known from the call to linkData()
        * @param consumerScene The id of the scene which consumes the data.
        * @param consumerId The id of the data consumer within the consumerScene
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unlinkData(sceneId_t consumerScene, dataConsumerId_t consumerId);

        /**
        * @brief Triggers an asynchronous map of scene with given scene id to a display with given display id.
        * @details Scene and display ids must refer to valid and existing instances.
        *          The result of the asynchronous operation can be retrieved via dispatchEvents.
        *
        * @param[in] displayId id of display to map scene to
        * @param[in] sceneId id of scene to map
        * @param[in] sceneRenderOrder Lower value means that a scene is rendered before a scene with higher value. Default is 0.
        *                             The render order is guaranteed only in the scope of the buffer it is mapped to (framebuffer or offscreen buffer).
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful map, the result event has its own status.
        */
        status_t mapScene(displayId_t displayId, sceneId_t sceneId, int32_t sceneRenderOrder = 0);

        /**
        * @brief Triggers an asynchronous unmap of scene with given scene id from any display it is mapped to.
        * @details The unmap operation will trigger fail event if the scene is not mapped to any display.
        *          The result of the asynchronous operation can be retrieved via dispatchEvents.
        *
        * @param[in] sceneId id of scene to unmap
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t unmapScene(sceneId_t sceneId);

        /**
        * @brief Triggers a scene with given scene id to be rendered.
        * @details Scene id must refer to a valid and existing instance.
        *          The scene to be rendered must be in mapped state.
        *          The result of this asynchronous operation can be retrieved via dispatchEvents.
        *
        * @param[in] sceneId id of scene to render
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful show, the result event has its own status.
        */
        status_t showScene(sceneId_t sceneId);

        /**
        * @brief Triggers a scene with given scene id to be not rendered anymore.
        * @details Scene id must refer to a valid and existing instance.
        *          The scene to be rendered must be in rendered state.
        *          The result of this asynchronous operation can be retrieved via dispatchEvents.
        *
        * @param[in] sceneId id of scene to hide
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful hide, the result event has its own status.
        */
        status_t hideScene(sceneId_t sceneId);

        /**
        * @brief Will create an offscreen buffer that can be used to render scenes into (see RamsesRenderer::assignSceneToOffscreenBuffer)
        *        and can be linked as input to a consumer texture sampler (see RamsesRenderer::linkOffscreenBufferToConsumer).
        *
        * @param[in] display id of display for which the buffer should be created
        * @param[in] width width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c InvalidOffscreenBufferId will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        offscreenBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height);

        /**
        * @brief     Additional API to create an offscreen buffer as interruptible.
        *            This allows the renderer to interrupt rendering of scenes to such offscreen buffer
        *            if the time budget for rendering is exceeded within a frame (see SetFrameTimerLimits).
        *            The rendering continues next frame starting from the interruption point.
        *
        *            The renderer creates two render targets on GPU (front and back) for every
        *            interruptible offscreen buffer. It then renders into the back render target
        *            of the offscreen buffer, while it is possible to read the content of front render target
        *            with content from previous frame (or older if interrupted for several frames).
        *
        *            Note that whenever a scene gets assigned to interruptible offscreen buffer,
        *            it is not guaranteed anymore that it will be fully rendered every frame.
        *            Essentially it is rendered with lower priority.
        *
        * @param[in] display  Id of display for which the buffer should be created
        * @param[in] width    Width of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @param[in] height   Height of the buffer to be created (has to be higher than 0 and lower than 4096)
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c InvalidOffscreenBufferId will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        offscreenBufferId_t createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height);

        /**
        * @brief Will destroy a previously created offscreen buffer.
        *        If there are any consumer texture samplers linked to this buffer, these links will be removed.
        *        Offscreen buffer will fail to be destroyed if there are any scenes assigned to it, these scenes
        *        have to be first assigned to another buffer or framebuffer or unmapped from display.
        *
        * @param[in] display id of display which the buffer belongs to
        * @param[in] offscreenBuffer id of buffer to destroy
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful hide, the result event has its own status.
        */
        status_t destroyOffscreenBuffer(displayId_t display, offscreenBufferId_t offscreenBuffer);

        /**
        * @brief When a scene is mapped and shown it is by default rendered directly to display's framebuffer.
        *        Assigning scene to an offscreen buffer means that scene will not be rendered to framebuffer but
        *        to the offscreen buffer instead.
        *        Offscreen buffer can be then used as texture input in a consumer texture sampler of another scene (see \c linkOffscreenBufferToSceneData).
        *        In order to assign a scene to offscreen buffer the scene has to be mapped to the same display where the offscreen buffer was created.
        *        Other properties like scene's shown or hidden state are not affected by this assignment.
        *
        *        Assigning a scene to framebuffer or offscreen buffer changes the way its render order is determined.
        *        The rendering order of following buffer groups is fixed:
        *          1. Offscreen buffers
        *          2. Framebuffer
        *          3. Interruptible offscreen buffers
        *        The render order given as part of mapping command only guarantees the order in scope of the buffer the scene is mapped to.
        *
        * @param[in] sceneId Id of scene that should be assigned to the offscreen buffer.
        * @param[in] offscreenBuffer Id of offscreen buffer that the scene should be assigned to.
        *                            The display that the buffer belongs to is determined by where the scene is mapped.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful hide, the result event has its own status.
        */
        status_t assignSceneToOffscreenBuffer(sceneId_t sceneId, offscreenBufferId_t offscreenBuffer);

        /**
        * @brief When a scene is assigned to an offscreen buffer using \c assignSceneToOffscreenBuffer it can be
        *        assigned back to display's framebuffer using this method.
        *        In order to assign a scene to framebuffer buffer the scene has to be mapped.
        *        Which display's framebuffer to map the scene to is determined by where the scene is mapped.
        *        Other properties like scene's shown or hidden state are not affected by this assignment.
        *        When a scene is mapped to display it is by default assigned to the display's framebuffer.
        *
        *        Assigning a scene to framebuffer or offscreen buffer changes the way its render order is determined.
        *        The rendering order of following buffer groups is fixed:
        *          1. Offscreen buffers
        *          2. Framebuffer
        *          3. Interruptible offscreen buffers
        *        The render order given as part of mapping command only guarantees the order in scope of the buffer the scene is mapped to.
        *
        * @param[in] sceneId Id of scene that should be assigned to the framebuffer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful hide, the result event has its own status.
        */
        status_t assignSceneToFramebuffer(sceneId_t sceneId);

        /**
        * @brief Triggers an asynchronous read back of framebuffer memory from GPU to system memory.
        * @details The color data from the provided rectangle coordinates
        *          will be read back and stored as RGBA8. If the coordinates
        *          lie outside the rendered region the result is undefined.
        *          The pixel data can be obtained as a renderer event after the asynchronous read back is finished,
        *          see RamsesRenderer::dispatchEvents for details.
        *
        * @param[in] displayId id of display to read pixels from.
        * @param[in] x The starting offset in the original image (i.e. left border) in pixels.
        * @param[in] y The starting offset in the original image (i.e. lower border) in pixels.
        *          The origin of the image is supposed to be in the lower left corner.
        * @param[in] width The width of the read image in pixels.
        * @param[in] height The height of the read image in pixels.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        *         StatusOK does not guarantee successful read back, the result event has its own status.
        */
        status_t readPixels(displayId_t displayId, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /////////////////////////////////////////////////
        //      System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Set visibility of given surface at the system compositor
        * @param surfaceId id of the surface to set visibility of
        * @param visibility visibility to set
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setSurfaceVisibility(uint32_t surfaceId, bool visibility);

        /**
        * @brief Set opacity of given surface at the system compositor
        * @param surfaceId id of the surface to set opacity of
        * @param opacity Opacity in the range 0.0 (fully transparent) to 1.0 (fully opaque)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setSurfaceOpacity(uint32_t surfaceId, float opacity);

        /**
        * @brief Set output rectangle of given surface at the system compositor
        * @param surfaceId id of the surface to set the rectangle for
        * @param x Output position of surface along the x-axis
        * @param y Output position of surface along the y-axis
        * @param width Output width of surface
        * @param height Output height of surface
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);

        /**
        * @brief Set visibility of given layer at the system compositor
        * @param layerId id identifying the layer
        * @param visibility If \c true the layer's visibility will be enabled, otherwise disabled
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        **/
        status_t setLayerVisibility(uint32_t layerId, bool visibility);

        /**
        * @brief Trigger the System Compositor to take a screenshot and store it in a file.
        * @param fileName File name including path, for storing the screenshot.
        * @param screenIviId >= 0 to trigger a screenshot on the given IVI screen id, -1 to trigger screenshot
        *             on a single existing screen (fails asynchronously if more than one screen exists)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t takeSystemCompositorScreenshot(const char* fileName, int32_t screenIviId);

        /////////////////////////////////////////////////
        //      End of System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Updates the warping mesh for the warping postprocessing based on the new config.
        * @details Display must be created with warping enabled in order for this operation to succeed.
        *
        * @param[in] displayId id of display to update.
        * @param[in] newWarpingMeshData Holds the geometry needed to create the mesh for display warping.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t updateWarpingMeshData(displayId_t displayId, const WarpingMeshData& newWarpingMeshData);

        /**
        * @brief Most RamsesRenderer methods trigger asynchronous actions in the internal renderer,
        *        which are executed during a render loop (doOneLoop or startThread).
        *        Some of these calls result in an event (can be both informational and data).
        *        Such events and their result can be retrieved using the dispatchEvents call.
        *        *IMPORTANT* In the threaded version of the renderer (RamsesRenderer::startThread)
        *        events must be regularly polled by calling dispatchEvents() in order to prevent
        *        buffer overflow of the internal event queue, even if the application is not interested
        *        in those events.
        *
        * @param rendererEventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                         Check IRendererEventHandler documentation for more details.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t dispatchEvents(IRendererEventHandler& rendererEventHandler);

        /**
        * @brief Marks renderer calls made since previous call to the function (if any) as to be executed in the next frame
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t flush();

        /**
        * @brief Prints detailed information about renderer state and contents to the log output.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t logRendererInfo();

        /**
        * Stores internal data for implementation specifics of RamsesRenderer
        */
        class RamsesRendererImpl& impl;

        /**
         * @brief Deleted copy constructor
         * @param other unused
         */
        RamsesRenderer(const RamsesRenderer& other) = delete;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RamsesRenderer& operator=(const RamsesRenderer& other) = delete;
    };
}

#endif
