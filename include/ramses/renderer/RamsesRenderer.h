//  -------------------------------------------------------------------------
//  Copyright (C) 2023 BMW AG
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/framework/RamsesFramework.h"
#include "ramses/framework/DataTypes.h"
#include "ramses/renderer/RendererConfig.h"

#include <string_view>

/**
 * @defgroup RendererAPI The Ramses Renderer API
 * This group contains all of the Ramses Renderer API types.
 */

namespace ramses
{
    namespace internal
    {
        class RendererFactory;
        class RamsesRendererImpl;
    }

    class SystemCompositorController;
    class DisplayConfig;
    class IRendererEventHandler;
    class RendererSceneControl;

    /**
    * @brief RamsesRenderer is the main renderer component which provides API to configure
    *        and control the way content will be rendered on display(s).
    * @details All the commands in this class are put to a queue and submitted only when #ramses::RamsesRenderer::flush is called,
    *          they are then executed asynchronously in the renderer core, the order of execution is preserved.
    *          Most of the commands have a corresponding callback which reports the result back to the caller
    *          via #ramses::RamsesRenderer::dispatchEvents.
    *          Some commands can fail immediately by returning false, in such case there will be no callback,
    *          because the command will not even be submitted.
    *          #ramses::RamsesRenderer API is not thread-safe.
    * @ingroup RendererAPI
    */
    class RAMSES_API RamsesRenderer
    {
    public:
        /**
        * @brief   Prepare content to be rendered in next frame and render next frame.
        * @details Update and render of all displays is executed sequentially in caller's thread.
        *          For active rendering it is recommended to use threaded mode instead (#startThread).
        *          Once this method is called a threaded mode cannot be used anymore.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool doOneLoop();

        /**
        * @brief   Starts update and render loop in threaded mode.
        * @details Each display will update and render in its own thread.
        *          First call to this method enables threaded mode, afterwards it is not possible to call
        *          #doOneLoop anymore.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool startThread();

        /**
        * @brief   Stops thread(s) running the update and render of displays.
        * @details This function can only be used if startThread was successfully called before.
        *          The looping can be restarted by calling #startThread again.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool stopThread();

        /**
        * @brief Get the current state of rendering thread(s) running
        *
        * @return Returns true if thread is running (started and not stopped), false otherwise.
        */
        [[nodiscard]] bool isThreadRunning() const;

        /**
        * @brief   Sets the maximum frame rate per second for the update/render loop when in threaded mode.
        * @details The parameter is of type float in order to specify any desired frame time (e.g. below 1 FPS).
        *          This function can only be used in threaded mode (#startThread).
        *          The default value is 60 FPS.
        *
        *          Note that FPS limit can be set also for a display that was not yet created internally
        *          (i.e. #ramses::IRendererEventHandler::displayCreated event has not been dispatched yet),
        *          this makes it easier to set a FPS limit on a display right after creation without having to wait
        *          for the result. If setting FPS limit on display which was previously destroyed this method
        *          will return true but will have no effect.
        * @param displayId The ID of the display the framerate should be applied to. The default value is 60 FPS.
        * @param fpsLimit The maximum frame rate per second to set for the render loop.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setFramerateLimit(displayId_t displayId, float fpsLimit);

        /**
        * @brief   Get the maximum frame rate per second set for given display using #setFramerateLimit.
        * @details This method returns the FPS limit set by user using #setFramerateLimit, or the default (60 FPS)
        *          if not modified by user. In both cases regardless if the display exists or not.
        *
        * @param displayId The ID of the display to query the maximum framerate.
        *
        * @return The FPS limit for given display set by user or default FPS limit.
        */
        [[nodiscard]] float getFramerateLimit(displayId_t displayId) const;

        /**
        * @brief   Sets the mode of operation for render loop.
        * @details Mode can be changed during run-time, in case of threaded mode also while running
        *          (no need to #stopThread).
        *          By default loop mode is set to render and update.
        *
        * @param loopMode The mode to be used for render loop.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setLoopMode(ELoopMode loopMode);

        /**
        * @brief Get the current value for loop mode set using #setLoopMode
        *
        * @return The loop mode
        */
        [[nodiscard]] ELoopMode getLoopMode() const;

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
        *            2. Uploading scene resources
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
        * @param[in] limitForOffscreenBufferRender Time limit in microseconds (since beginning of frame) for rendering scenes that are mapped to interruptible offscreen buffers
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setFrameTimerLimits(uint64_t limitForSceneResourcesUpload, uint64_t limitForClientResourcesUpload, uint64_t limitForOffscreenBufferRender);

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
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setPendingFlushLimits(uint32_t forceApplyFlushLimit, uint32_t forceUnsubscribeSceneLimit);

        /**
        * @brief     Enable or disable skipping of rendering of unmodified buffers.
        *            By default the renderer does not re-render buffers (framebuffer or offscreen buffer)
        *            if there was no change to any of the content assigned to the buffer.
        *            This can save hardware resources or free up resources for rendering of other buffers.
        *            It can be however desired to disable such optimization for profiling of worst case scenario
        *            or debugging of a graphical glitch.
        *
        * @param[in] enable  Enable or disable the feature (enabled initially)
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setSkippingOfUnmodifiedBuffers(bool enable = true);

        /**
         * @brief Creates a display based on provided display config.
         *        Creation of a display is an asynchronous action and the display will be created during the next render loop.
         *        The result of the createDisplay can be retrieved via dispatchEvents which will trigger a callback
         *        after the asynchronous action was processed.
         *
         * @param config The display config to create and configure the new display.
         * @return Display id that can be used to refer to the created display.
         *         displayId_t::Invalid() in case of error. Display creation can still fail even
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
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyDisplay(displayId_t displayId);

        /**
        * @brief Get display's framebuffer ID.
        *        Every display upon creation has one framebuffer which can be referenced by a display buffer ID
        *        to be used in various API methods that work with either a framebuffer or an offscreen buffer (e.g. #ramses::RamsesRenderer::setDisplayBufferClearColor).
        *
        * @param displayId The ID of display for which the framebuffer ID is being queried.
        * @return Display's framebuffer ID or invalid ID if display does not exist.
        */
        [[nodiscard]] displayBufferId_t getDisplayFramebuffer(displayId_t displayId) const;

        /**
        * @brief   Will create an offscreen buffer that can be used to render scenes into (see #ramses::RendererSceneControl::setSceneDisplayBufferAssignment)
        *          and can be linked as input to a consumer texture sampler (see #ramses::RendererSceneControl::linkOffscreenBuffer).
        * @details The created offscreen buffer always has a color buffer. Depth/stencil buffer can be configured (depth/stencil combined is attached by default).
        *          A multisampled buffer will be created if sampleCount greater than 0, note that the value is just a hint for the device,
        *          the actual number of samples might be different depending on device driver implementation.
        *          If the number of samples exceeds device capabilities the number of samples it will be clamped to its
        *          maximum supported (creation will succeeded with a warning log).
        *
        * @param[in] display id of display for which the buffer should be created
        * @param[in] width width of the buffer to be created (has to be higher than 0)
        * @param[in] height height of the buffer to be created (has to be higher than 0)
        * @param[in] sampleCount Optional sample count for MSAA. Default value is 0 for no MSAA.
        * @param[in] depthBufferType Optional setting to configure depth and stencil buffers, default is depth/stencil buffer will be created in addition to color buffer.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        displayBufferId_t createOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t sampleCount = 0u, EDepthBufferType depthBufferType = EDepthBufferType::DepthStencil);

        /**
        * @brief     Additional API to create an offscreen buffer as interruptible. (see #createOffscreenBuffer)
        * @details   This allows the renderer to interrupt rendering of scenes to such offscreen buffer
        *            if the time budget for rendering is exceeded within a frame (see #setFrameTimerLimits).
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
        *            The created offscreen buffer always has a color buffer. Depth/stencil buffer can be configured (depth/stencil combined is attached by default).
        *
        * @param[in] display  Id of display for which the buffer should be created
        * @param[in] width    Width of the buffer to be created (has to be higher than 0)
        * @param[in] height   Height of the buffer to be created (has to be higher than 0)
        * @param[in] depthBufferType Optional setting to configure depth and stencil buffers, default is depth/stencil buffer will be created in addition to color buffer.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        displayBufferId_t createInterruptibleOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, EDepthBufferType depthBufferType = EDepthBufferType::DepthStencil);

        /**
        * @brief     Additional API to create an offscreen buffer using DMA buffer for internal storage. (see #createOffscreenBuffer)
        * @details   The created offscreen buffer uses a DMA buffer based on a GBM buffer object for its internal storage, which can be mapped to CPU
        *            memory, instead of standard OpenGL textures and render storages. The created offscreen buffer can be rendered into and linked
        *            to texture consumers, like other offscreen buffer types.
        *            This type of offscreen buffers can be created only if the platform provides the necessary support, and if the render node for creation
        *            of GBM device is provided on #ramses::DisplayConfig::setPlatformRenderNode.
        *
        *            DMA offscreen buffer can only have a color component, no depth or stencil.
        *
        *            Notes:
        *            It is of particular importance to avoid CPU operations on the mapped memory while GPU could be accessing the offscreen buffer's memory
        *            for executing asynchronous rendering commands.
        *               * If a DMA offscreen buffer is being used by the GPU for rendering operations, it is the responsibility of the user not to attempt
        *                 accessing the CPU mapped memory for that buffer.
        *               * Due to the asynchronous nature of OpenGL and render pipeline execution, an offscreen buffer can still be in use after the call to
        *                 #doOneLoop is finished. It is important to submit the commands to get the offscreen buffer out of the rendering pipeline, then make
        *                 at least two calls to #doOneLoop (in case of double buffering) before accessing the CPU mapped memory for that buffer
        *               * An offscreen buffer is a part of the rendering pipeline as long as it is either being used as input, i.e., by linking to a
        *                 texture consumer via #ramses::RendererSceneControl::linkOffscreenBuffer, or being used for output by rendering some scenes into it
        *                 via #ramses::RendererSceneControl::setSceneDisplayBufferAssignment
        *
        *            It is only possible to create DMA offscreen buffers if renderer is running using #doOneLoop.
        *            Calling this method on a renderer with display threads will fail right away with error status
        *            without invoking a callback #ramses::IRendererEventHandler::offscreenBufferCreated.
        *
        * @param[in] display  Id of display for which the buffer should be created
        * @param[in] width    Width of the buffer to be created (has to be higher than 0)
        * @param[in] height   Height of the buffer to be created (has to be higher than 0)
        * @param[in] bufferFourccFormat  Format to be used for underlying storage of the buffer, as specified in drm_fourcc.h on the target platform
        * @param[in] usageFlags  Usage flags used for creation of the underlying GBM buffer object, as specific in enum gbm_bo_flags on the target platform
        * @param[in] modifier  Optional format modifier. If not needed set to DRM_FORMAT_MOD_INVALID.
        * @return Identifier of the created offscreen buffer.
        *         In case of unsupported resolution or renderer running in own thread \c displayBufferId_t::Invalid() will be returned with no renderer event generated.
        *         Note that the buffer will be created asynchronously and there will be a renderer event once the operation is finished.
        */
        displayBufferId_t createDmaOffscreenBuffer(displayId_t display, uint32_t width, uint32_t height, uint32_t bufferFourccFormat, uint32_t usageFlags, uint64_t modifier);

        /**
        * @brief   Get the FD and stride for a DMA offscreen buffer previously created on the given display
        * @details Get the file descriptor and stride for the underlying GBM buffer object used for a DMA offscreenbuffer
        *          that was created using #ramses::RamsesRenderer::createDmaOffscreenBuffer.
        *          This function can be safely called only after a successful offscreen buffer event
        *          is disptched (using #ramses::IRendererEventHandler::offscreenBufferCreated) for the meant offscreen buffer.
        *
        *          The file descriptor could be used for mapping the underlying memory used by
        *          the offscreen buffer to CPU. As long the mapped memory is in use it is important to watch
        *          the mentioned considerations in #createDmaOffscreenBuffer. It is the responsibility of the user
        *          to unmap that memory on offscreen buffer destruction or when the CPU operations do not need to be applied
        *          to that memory any more.
        *
        *          Stride could be used for calculating addresses of specific pixels within mapped memory,
        *          where the data for each row in the image starts at a multiple of stride.
        *          Buffer stride can be different from the calculatable row size in bytes relying
        *          only on buffer width and format pixel size.
        *
        * @param[in] display  Id of display for which the buffer was created
        * @param[in] displayBufferId  Id of the DMA offscreen buffer for which the FD should be returned
        * @param[out] fd File descriptor of underlying GBM buffer object for DMA offscreen buffer
        * @param[out] stride Stride of DMA offsceen buffer in bytes
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool getDmaOffscreenBufferFDAndStride(displayId_t display, displayBufferId_t displayBufferId, int& fd, uint32_t& stride) const;

        /**
        * @brief Will destroy a previously created offscreen buffer.
        *        If there are any consumer texture samplers linked to this buffer, these links will be removed.
        *        Offscreen buffer will fail to be destroyed if there are any scenes assigned to it, these scenes
        *        have to be first assigned to another buffer or framebuffer or unmapped from display.
        *
        * @param[in] display id of display which the buffer belongs to
        * @param[in] offscreenBuffer id of buffer to destroy
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyOffscreenBuffer(displayId_t display, displayBufferId_t offscreenBuffer);

        /**
        * @brief   Creates a buffer using OpenGL External textures for storage.
        * @details The created buffer uses OpenGL External textures according to the OpenGL
        *          extension OES_EGL_image_external. The created texture can be used for
        *          compositing platform dependent content from native processes and applications
        *          on the target platforms.
        *
        *          In order to provide content to the created buffer, the callback for external buffer's creation
        *          should be handled in #ramses::IRendererEventHandler, where on creation success the buffer's OpenGL Id
        *          should be provided. This texture Id should be passed to the native platform mechanisms.
        *          On Android the texture Id should be passed to a SurfaceTexture object which connects the texture with
        *          other Android platform constructs.
        *
        *          The created external buffer should be linked to a texture consumer created from
        *          #ramses::TextureSamplerExternal. This can be used only with GLSL shader sampler
        *          of type "samplerExternalOES".
        *
        *          External buffers can be used with #doOneLoop, since the user is expected to
        *          make platform specific calls to update the content of the external texture.
        *          Using a render thread could lead to race conditions and to unexpected and
        *          undesirable behavior on target platform, so it is prohibited.
        *          Trying to call this function if the renderer is not using #doOneLoop will
        *          lead to failure, and an invalid external buffer id will be returned.
        *
        * @param[in] display Id of display that the buffer should be created on.
        * @return Identifier of the created external buffer.
        *         In case renderer is running in own thread \c externalBufferId_t::Invalid() will be returned.
        */
        externalBufferId_t createExternalBuffer(displayId_t display);

        /**
        * @brief Will destroy a previously created external buffer.
        *        If there are any consumer texture samplers linked to this buffer, these links will be removed.
        *
        * @param[in] display id of display which the buffer belongs to
        * @param[in] externalBuffer id of buffer to destroy
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyExternalBuffer(displayId_t display, externalBufferId_t externalBuffer);

        /**
        * @brief   Creates a buffer for viewing wayland surfaces from the embedded compositor.
        *          The created buffer can be linked as input to a consumer texture sampler (see #ramses::RendererSceneControl::linkStreamBuffer).
        *
        * @param[in] display Id of display that the buffer should be created on.
        * @param[in] surfaceId Id of the wayland surface that the buffer should render from.
        * @return Identifier of the created external buffer.
        */
        streamBufferId_t createStreamBuffer(displayId_t display, ramses::waylandIviSurfaceId_t surfaceId);

        /**
        * @brief Will destroy a previously created stream buffer.
        *        If there are any consumer texture samplers linked to this buffer, these links will be removed.
        *
        * @param[in] display id of display which the buffer belongs to
        * @param[in] bufferId id of buffer to destroy
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool destroyStreamBuffer(displayId_t display, ramses::streamBufferId_t bufferId);

        /**
        * @brief   Sets clear flags for a display buffer (display's framebuffer or offscreen buffer).
        * @details By default all display buffers' color, depth and stencil are cleared every frame when they are rendered to.
        *          This can be overridden for performance or special effect reasons.
        *          There is no event callback for this operation, the change can be assumed to be effective
        *          in the next frame rendered after flushed.
        *
        * @param[in] display Id of display that the buffer to set clearing belongs to.
        * @param[in] displayBuffer Id of display buffer to set clearing,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then the clearing is set for display's framebuffer.
        * @param[in] clearFlags Bitmask of the #ramses::EClearFlag, use bit OR to select which buffer component to clear
        *                       or #ramses::EClearFlag::All to clear all (default).
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setDisplayBufferClearFlags(displayId_t display, displayBufferId_t displayBuffer, ClearFlags clearFlags);

        /**
        * @brief   Sets clear color of a display buffer (display's framebuffer or offscreen buffer).
        * @details Clear color is used to clear the whole buffer at the beginning of a rendering cycle (typically every frame).
        *          Default clear color is (0, 0, 0, 1).
        *          There is no event callback for this operation, the clear color change can be assumed to be effective
        *          in the next frame rendered after flushed.
        *
        * @param[in] display Id of display that the buffer to set clear color belongs to.
        * @param[in] displayBuffer Id of display buffer to set clear color,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then the clear color is set for display's framebuffer.
        * @param[in] color Clear color (rgba, channel values in range [0,1])
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setDisplayBufferClearColor(displayId_t display, displayBufferId_t displayBuffer, const vec4f& color);

        /**
        * @brief   Updates display window size after a resize event on windows not owned by renderer.
        * @details Sets the new display window size after a resize event is externally handled for the window.
        *          RAMSES renderer handles window events within its render loop for windows that are created and owned by
        *          the renderer. Typically this can not be performed for windows that are created externally by the user, and provided to RAMSES renderer during
        *          display creation as a native handle, since the user handles window events explicitly.
        *
        *          This applies also to resize events, i.e., if an externally owned window gets resized RAMSES renderer does not
        *          handle the resize event internally and does not get to know about it. In this case API users are expected to call this function to let
        *          RAMSES update its information about display window size. The display window size can have visible effects, e.g., if the shaders
        *          use #ramses::EEffectUniformSemantic::DisplayBufferResolution
        *
        *          This function will log an error if the platform does not support this feature, or if the display window was not explicitly
        *          provided by the user as a native handle.
        *
        * @param[in] display Id of display that the resized window belongs to.
        * @param[in] width New width of the resized window.
        * @param[in] height New height of the resized window.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool setExternallyOwnedWindowSize(displayId_t display, uint32_t width, uint32_t height);

        /**
        * @brief Triggers an asynchronous read back of a display buffer memory from GPU to system memory.
        * @details The color data from the provided rectangle coordinates
        *          will be read back and stored as RGBA8. If the coordinates
        *          lie outside the rendered region the result is undefined.
        *
        *          If a read pixels command is issued for a display buffer
        *          while a previous read pixels command for the same buffer was not
        *          yet executed only the last submitted read pixel command gets executed.
        *
        *          The pixel data can be obtained as a renderer event after the asynchronous read back is finished,
        *          see #ramses::RamsesRenderer::dispatchEvents for details.
        * @param[in] displayId id of display to read pixels from.
        * @param[in] displayBuffer Id of display buffer to read pixels from,
        *                          if #ramses::displayBufferId_t::Invalid() is passed then pixels are read from the display's framebuffer.
        * @param[in] x The starting offset in the original image (i.e. left border) in pixels.
        * @param[in] y The starting offset in the original image (i.e. lower border) in pixels.
        *          The origin of the image is supposed to be in the lower left corner.
        * @param[in] width The width of the read image in pixels. Must be greater than Zero.
        * @param[in] height The height of the read image in pixels. Must be greater than Zero.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        *         true does not guarantee successful read back, the result event has its own status.
        */
        bool readPixels(displayId_t displayId, displayBufferId_t displayBuffer, uint32_t x, uint32_t y, uint32_t width, uint32_t height);

        /**
        * @brief Get scene control API
        * @details Typical application using Ramses has different components controlling the renderer
        *          (display management, frame limits and looping control, etc.) and controlling the states
        *          of content to be rendered (show/hide scene, data link, assign to display buffer, etc.).
        *          The scene control part can be obtained using this method, calling this method
        *          multiple times is allowed and will always return the same pointer, i.e. there is only
        *          a single instance per #ramses::RamsesRenderer.
        *          This method will return nullptr in case an internal policy disallows controlling of scenes
        *          through this API - this could mean that there is another, incompatible scene control
        *          mechanism in use.
        *          Scene control API has its own independent flush and event dispatching,
        *          see #ramses::RendererSceneControl for details.
        *
        *          #ramses::RamsesRenderer is owner of the #ramses::RendererSceneControl API and the pointer
        *          stays valid as long as this #ramses::RamsesRenderer instance is alive. It cannot be destroyed
        *          without destroying the #ramses::RamsesRenderer.
        *
        * @return Pointer to scene control API, or nullptr on error
        */
        RendererSceneControl* getSceneControlAPI();

        /////////////////////////////////////////////////
        //      System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Set visibility of given surface at the system compositor
        * @param surfaceId id of the surface to set visibility of
        * @param visibility visibility to set
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        **/
        [[deprecated]] bool setSurfaceVisibility(uint32_t surfaceId, bool visibility);

        /**
        * @brief Set opacity of given surface at the system compositor
        * @param surfaceId id of the surface to set opacity of
        * @param opacity Opacity in the range 0.0 (fully transparent) to 1.0 (fully opaque)
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        **/
        [[deprecated]] bool setSurfaceOpacity(uint32_t surfaceId, float opacity);

        /**
        * @brief Set output rectangle of given surface at the system compositor
        * @param surfaceId id of the surface to set the rectangle for
        * @param x Output position of surface along the x-axis
        * @param y Output position of surface along the y-axis
        * @param width Output width of surface
        * @param height Output height of surface
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        [[deprecated]] bool setSurfaceRectangle(uint32_t surfaceId, int32_t x, int32_t y, int32_t width, int32_t height);

        /**
        * @brief Set visibility of given layer at the system compositor
        * @param layerId id identifying the layer
        * @param visibility If \c true the layer's visibility will be enabled, otherwise disabled
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        **/
        [[deprecated]] bool setLayerVisibility(uint32_t layerId, bool visibility);

        /**
        * @brief Trigger the System Compositor to take a screenshot and store it in a file.
        * @param fileName File name including path, for storing the screenshot.
        * @param screenIviId >= 0 to trigger a screenshot on the given IVI screen id, -1 to trigger screenshot
        *             on a single existing screen (fails asynchronously if more than one screen exists)
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        [[deprecated]] bool takeSystemCompositorScreenshot(std::string_view fileName, int32_t screenIviId);

        /////////////////////////////////////////////////
        //      End of System Compositor API
        /////////////////////////////////////////////////

        /**
        * @brief Most RamsesRenderer methods push commands to an internal queue which is submitted
        *        when calling #ramses::RamsesRenderer::flush. The commands are then executed during a render loop
        *        (#ramses::RamsesRenderer::doOneLoop or in a render thread if used #ramses::RamsesRenderer::startThread).
        *        Some of these calls result in an event (can be both informational and data).
        *        Such events and their result can be retrieved using the dispatchEvents call.
        *        *IMPORTANT* Renderer events must be regularly consumed by calling dispatchEvents()
        *        in order to prevent buffer overflow of the internal event queue,
        *        even if the application is not interested in those events.
        *
        * @param rendererEventHandler User class that implements the callbacks that can be triggered if a corresponding event happened.
        *                             Check ramses::IRendererEventHandler documentation for more details.
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool dispatchEvents(IRendererEventHandler& rendererEventHandler);

        /**
        * @brief Submits renderer commands (API calls on this instance of RamsesRenderer)
        *        since previous flush to be executed in the next renderer update loop.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool flush();

        /**
        * @brief Prints detailed information about renderer state and contents to the log output.
        *
        * @return true for success, false otherwise (check log or #ramses::RamsesFramework::getLastError for details).
        */
        bool logRendererInfo();

        /**
         * @brief Deleted copy constructor
         */
        RamsesRenderer(const RamsesRenderer&) = delete;

        /**
         * @brief Deleted move constructor
         */
        RamsesRenderer(RamsesRenderer&&) = delete;

        /**
         * @brief Deleted copy assignment
         * @return unused
         */
        RamsesRenderer& operator=(const RamsesRenderer&) = delete;

        /**
         * @brief Deleted move assignment
         * @return unused
         */
        RamsesRenderer& operator=(RamsesRenderer&&) = delete;

        /**
         * Get the internal data for implementation specifics of RamsesRenderer.
         */
        [[nodiscard]] internal::RamsesRendererImpl& impl();

        /**
         * Get the internal data for implementation specifics of RamsesRenderer.
         */
        [[nodiscard]] const internal::RamsesRendererImpl& impl() const;

    private:
        /**
         * @brief Constructor of RamsesRenderer
         */
        explicit RamsesRenderer(std::unique_ptr<internal::RamsesRendererImpl> impl);

        /**
         * Destructor
         */
        ~RamsesRenderer();

        /**
        * Stores internal data for implementation specifics of RamsesRenderer
        */
        std::unique_ptr<internal::RamsesRendererImpl> m_impl;

        /**
        * @brief RendererFactory is the factory for RamsesRenderer
        */
        friend class internal::RendererFactory;
    };
}
