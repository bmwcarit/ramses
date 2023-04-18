//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_DISPLAYCONFIG_H
#define RAMSES_RENDERERAPI_DISPLAYCONFIG_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/StatusObject.h"
#include "ramses-framework-api/RamsesFrameworkTypes.h"
#include "ramses-framework-api/DataTypes.h"

namespace ramses
{
    /**
    * @brief The DisplayConfig holds a set of parameters to be used to initialize a display.
    */
    class RAMSES_API DisplayConfig : public StatusObject
    {
    public:
        /**
        * @brief Default constructor of DisplayConfig
        */
        DisplayConfig();

        /**
        * @brief Copy constructor of DisplayConfig
        * @param[in] other Other instance of DisplayConfig
        */
        DisplayConfig(const DisplayConfig& other);

        /**
        * @brief Destructor of DisplayConfig
        */
        ~DisplayConfig() override;

        /**
        * @brief Sets the window size and position in display pixel space.
        * This is ignored if window is set fullscreen.
        *
        * @param[in] x Horizontal offset (distance from left border of the display)
        * @param[in] y Vertical offset (distance from top border of the display)
        * @param[in] width Width of the window
        * @param[in] height Height of the window
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height);

        /**
        * @brief Get the window size and position in display pixel space, as it was specified by setWindowRectangle() or command line options
        * These values have no relevance if window is set fullscreen.
        *
        * @param[out] x Horizontal offset (distance from left border of the display)
        * @param[out] y Vertical offset (distance from top border of the display)
        * @param[out] width Width of the window
        * @param[out] height Height of the window
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const;

        /**
        * @brief Automatically sets the window size so that it fills the entire display.
        * Overrides DisplayConfig::setWindowRectangle() when set to true.
        *
        * @param[in] fullscreen Flag specifying whether window should be fullscreen
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowFullscreen(bool fullscreen);

        /**
        * @brief Gets the currently set fullscreen state, which was set
        *        either via DisplayConfig::setWindowFullscreen or parsed from command line arguments.
        * @return True if this DisplayConfig is set to use fullscreen window, false otherwise.
        */
        [[nodiscard]] bool isWindowFullscreen() const;

        /**
        * @brief Sets window hints/properties to tell the window manager to disable window borders
        *
        * @param[in] borderless flag specifying whether window should be drawn without window-border
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowBorderless(bool borderless);

        /**
        * @brief   Set number of samples to be used for multisampled rendering.
        * @details Valid values are 1, 2, 4 or 8. Default value is 1 meaning multisampling is disabled.
        *          This value is just a hint for the device, the actual number of samples is guaranteed
        *          to be at least the given value but can be also more than that depending on device driver
        *          implementation.
        *          Ramses cannot check the device capability before the display is created, therefore
        *          exceeding the maximum number of samples supported by the device will likely result
        *          in fatal error.
        *
        * @param[in] numSamples Number of samples per pixel.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setMultiSampling(uint32_t numSamples);

        /**
        * @brief Get number of samples currently set.
        *
        * @param[out] numSamples Number of samples per pixel.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getMultiSamplingSamples(uint32_t& numSamples) const;

        /**
        * @brief [Mandatory on Wayland] Set IVI layer ID to use for attaching the IVI surface created by the display.
        *
        * RAMSES does not try to create the layer, instead the layer must be already existing before creating the display.
        * Trying to create a display on Wayland without setting this variable or giving it a non-valid
        * layer ID, e.g., the ID of a non-existing layer, will result in display creation failure.
        *
        * This values is ignored on other systems than Wayland.
        *
        * @param[in] waylandIviLayerID IVI layer ID to use for attaching the surface used by the display
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID);

        /**
        * @brief Get the ID of the Wayland IVI layer to which the IVI surface used by the display is attached.
        *
        * @return the current setting of wayland IVI layer ID, returns waylandIviLayerId_t::Invalid() if no value has been set yet
        */
        [[nodiscard]] waylandIviLayerId_t getWaylandIviLayerID() const;

        /**
        * @brief [Mandatory on Wayland] Set IVI surface ID to use when creating the display window on Wayland.
        *
        * - This is the Wayland IVI surface ID, i.e. the ID that the display reports to Wayland so that the surface (=window)
        *   can be controlled over the system compositor controller
        * - If the surface is not already existing it will be created during display creation
        *
        * @param[in] waylandIviSurfaceID IVI surface ID to use for the display window
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID);

        /**
        * @brief Get the current setting of IVI surface ID
        *
        * @return the current setting of IVI surface ID, returns waylandIviSurfaceId_t::Invalid() if no value has been set yet
        */
        [[nodiscard]] waylandIviSurfaceId_t getWaylandIviSurfaceID() const;

        /**
        * @brief Get the current setting of Android native window
        *
        * @return the current setting of Android native window, returns nullptr if no value has been set yet
        */
        [[nodiscard]] void* getAndroidNativeWindow() const;

        /**
        * @brief [Mandatory on Android] Set native window to use for rendering on Android.
        *
        * @param[in] nativeWindowPtr ANativeWindow* which can be obtained with ANativeWindow_fromSurface() from a Java Surface object
        *
        * No ownership is transferred, the user is responsible to call ANativeWindow_release after destroying the RAMSES Renderer.
        *
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setAndroidNativeWindow(void* nativeWindowPtr);

        /**
        * @brief Set IVI window to be visible right after window creation
        *
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowIviVisible();

        /**
        * @brief By default uploaded effects are kept forever in VRAM to avoid
        *        recompiling and reuploading. As their size is mostly negligible
        *        this should not have any disadvantages.
        *        If however it is desired to delete unused effects same as any other resources
        *        use this method to disable that optimization.
        *        Note that GPU cache can still prevent effect deletion (see ramses::DisplayConfig::setGPUMemoryCacheSize).
        *
        * @param[in] enable Set to true if effects should be always kept in VRAM (default), false if they should be deleted if unused as other resources
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t keepEffectsUploaded(bool enable);

        /**
        * @brief Set the amount of GPU memory in bytes that will be used as cache for resources.
        *        Uploaded resources are kept in GPU memory even if not in use by any scene anymore.
        *        They are only freed from memory in order to make space for new resources to be uploaded
        *        which would not fit in the cache otherwise.
        *        First in first out method is used when deciding which unused resource to remove from cache.
        *
        *        Note that the cache size does not act as hard limit, the renderer can still upload
        *        resources taking up more space. As long as cache limit is exceeded, newly unused resources are unloaded
        *        immediately.
        *
        *        Only client resources are considered for this cache, not scene resources (eg. render targets/render buffers).
        *        Cache is disabled by default (size is 0).
        *
        * @param[in] size GPU resource cache size in bytes. Disabled if 0 (default)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setGPUMemoryCacheSize(uint64_t size);

        /**
         * @brief Enables/disables resizing of the window (Default=Disabled)
         * @param[in] resizable The resizable flag
         *
         * @return  StatusOK on success, otherwise the returned status can be used to resolve
         *          to resolve error message using getStatusMessage()
         */
        status_t setResizable(bool resizable);

        /**
        * @brief Sets the clear color of the displays framebuffer (Default=0.0, 0.0, 0.0, 1.0)
        * @param[in] color clear color (rgba, channel values in range [0,1])
        *
        * @return  StatusOK on success, otherwise the returned status can be used to resolve
        *          to resolve error message using getStatusMessage()
        */
        status_t setClearColor(const vec4f& color);

        /**
        * @brief Sets whether depth/stencil buffer should be created for the framebuffer of the display.
        *        The framebuffer always has a color buffer. By default depth-stencil buffer is
        *        created as well.
        *
        *        The set configuration will be passed to WGL/EGL, which uses this information as a lower-limit
        *        for the chosen configuration. If the configuration is not available exactly as requested, WGL/EGL
        *        will try the nearest configuration with the requested capabilities or more, e.g., if depth buffer is requested
        *        but stencil buffer is not it can happen that a stencil buffer will still be created because WGL/EGL does
        *        not have a configuration with that specific description.
        *
        * @param[in] depthBufferType Configure depth and stencil buffers.
        *
        * @return  StatusOK on success, otherwise the returned status can be used to resolve
        *          to resolve error message using getStatusMessage()
        */
        status_t setDepthStencilBufferType(EDepthBufferType depthBufferType);

        /**
        * @brief [Only for X11] Set the X11 window handle to create a ramses display from an existing X11 window.
        *
        * - This method is platform dependent!
        * - On other backends except for X11, the value has no meaning
        *
        * @param[in] x11WindowHandle native X11 window id to use for the display window.
        *   The type is equivalent to \verbatim ::Window \endverbatim from the X11 headers.
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setX11WindowHandle(unsigned long x11WindowHandle);

        /**
        * @brief [Only for X11] Get the current setting of the X11 window handle
        *
        * @return the current setting of the X11 window handle, returns numerical maximum value if no value has been set yet.
        * The returned type is equivalent to \verbatim ::Window \endverbatim from the X11 headers.
        */
        [[nodiscard]] unsigned long getX11WindowHandle() const;

        /**
        * @brief [Only for Windows] Set the HWND handle to create a ramses display from an existing HWND window on a Window platform.
        *
        * - This method is platform dependent!
        * - On other systems except for Windows, calling this method has no meaning
        *
        * @param[in] hwnd Windows window handle to use for the display window
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowsWindowHandle(void* hwnd);

        /**
        * @brief Get the current setting of the Windows window handle (HWND)
        *
        * @return the current setting of the Windows window handle, returns nullptr if no value has been set yet
        */
        [[nodiscard]] void* getWindowsWindowHandle() const;

        /**
        * @brief Set the Wayland display name to connect to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandDisplay(std::string_view waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        [[nodiscard]] std::string_view getWaylandDisplay() const;

        /**
        * @brief   Sets whether async shader/effect compilation and upload should be enabled.
        *          By default async effect compile and upload is enabled.
        * @details Shader compilation can be a relatively slow and computationally intense task on some platforms.
        *          This can lead to undesirable performance overhead, i.e., if shader compilation stalls rendering
        *          and upload of other resources.
        *
        *          Enabling async effect upload lets the renderer create a shared context and a separate
        *          thread that are used exclusively for shader compilation and upload.
        *
        *          It is recommended to leave async effect compile and upload enabled, and to disable it only for
        *          development and debugging purposes when necessary.
        *          If async effect compile and upload is disabled using this function the renderer will still create
        *          the components normally used for this purpose, i.e., a shared context
        *          and an additional thread, but their logic will not be triggered.
        *          Instead, shaders will be compiled and uploaded within the rendering loop, i.e. potentially stalling rendering.
        *
        * @param[in] enabled Set to true to enable async effect upload, false to disable it.
        *
        * @return  StatusOK on success, otherwise the returned status can be used to resolve
        *          to resolve error message using getStatusMessage()
        */
        status_t setAsyncEffectUploadEnabled(bool enabled);

        /**
         * @brief      Set the name to be used for the embedded compositing
         *             display socket name.
         *
         * @details    The embedded compositor communicates with its clients via
         *             a socket file. There are two distinct ways to connect the
         *             embedded compositor with its socketfile. Either you
         *             provide a name for the socket file or the file descriptor
         *             of the socket file.
         *
         *             This method is used to set the file name of the socket.
         *
         *             Providing the name of the socket file leads to the
         *             embedded compositor searching/creating the socket file in
         *             the directory pointed to by $XDG_RUNTIME_DIR. If a
         *             groupname is set, also the group is set.
         *
         *             Be aware that the socket file name is only used if the
         *             file descriptor is set to an invalid value (default), see
         *             RendererConfig::setWaylandEmbeddedCompositingSocketFD
         *
         *             If both filename and file descriptor are set display
         *             creation will fail.
         *
         * @param[in]  socketname  The file name of the socket file.
         *
         * @return     StatusOK for success, otherwise the returned status can
         *             be used to resolve error message using
         *             getStatusMessage().
         */
        status_t setWaylandEmbeddedCompositingSocketName(std::string_view socketname);

        /**
        * @brief Get the current setting of embedded compositing display socket name
        *
        * @return Wayland display name to use for embedded compositing socket
        */
        [[nodiscard]] std::string_view getWaylandEmbeddedCompositingSocketName() const;

        /**
        * @brief Request that the embedded compositing display socket belongs to the given group.
        *
        * @param[in] groupname The group name of the socket.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname);

        /**
         * @brief      Set the file descriptor for the embedded compositor
         *             display socket.
         *
         * @details    The embedded compositor communicates with its clients via
         *             a socket file. There are two distinct ways to connect the
         *             embedded compositor with its socketfile.
         *
         *             Either you provide a name for the socket file or the file
         *             descriptor of the socket file.
         *
         *             This method is used to set the file descriptor.
         *
         *             When the file descriptor is set, the embedded compositor
         *             will use this file descriptor directly as its socket. It
         *             is expected that this file descriptor is belonging to a
         *             file already open, bind and listen to.
         *
         *             If both filename and file descriptor are set display
         *             creation will fail.
         *
         * @param      socketFileDescriptor  The file descriptor of the socket
         *                                   for the embedded compositor.
         *
         * @return     StatusOK for success, otherwise the returned status can
         *             be used to resolve error message using
         *             getStatusMessage().
         */
        status_t setWaylandEmbeddedCompositingSocketFD(int socketFileDescriptor);

        /**
        * @brief       Request that the embedded compositing display socket obtains the permissions given.
        *
        * @details     The format should be the same as expected by chmod() mode argument. Permissions value may not
        *              be 0. If not set "user+group can read/write (0660)" is used as default.
        *
        *              The socket should be readable and writable for the required users, some examples values are:
        *              * Only user r/w:  384u (or in octal 0600)
        *              * User+Group r/w: 432u (or in octal 0660)
        *              * Everyone r/w:   438u (or in octal 0666)
        *
        *              This value is only used when socket is given as name, e.g. via
        *              setWaylandEmbeddedCompositingSocketName(), not when passed in as filedescriptor.
        *
        * @param[in]   permissions The permissions of the socket.
        * @return      StatusOK for success, otherwise the returned status can be used
        *              to resolve error message using getStatusMessage().
        */
        status_t setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);

        /**
        * @brief Set the Wayland display name to connect system compositor to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSystemCompositorWaylandDisplay(std::string_view waylandDisplay);

        /**
        * @brief Set the render node to use for creating GBM buffer objects used for creating DMA Offscreen buffers
        *        on platforms that support creation of DMA buffers.
        *
        * @brief Set the socket of a DRM driver's render node to load and create a GBM (Generic Buffer Manager) device.
        *        If the socket is opened successfully a GBM device is created using gbm_create_device.
        *        The GBM device can be used to create DMA buffers which act as regular offscreen buffers but also allow
        *        direct memory access from application's side. This can be helpful for use cases that require CPU read or write of memory
        *        that is used for rendering as well.
        *
        *        Setting a valid render node is a pre-requisite to creating DMA offscreen buffers using #ramses::RamsesRenderer::createDmaOffscreenBuffer.
        *        Setting an invalid render node, or failing to create a GBM device using the passed render node
        *        will result in display creation failure.
        *
        *
        * @param[in] renderNode Render node used to load for creating of GBM device using gbm_create_device
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPlatformRenderNode(std::string_view renderNode);

        /**
        * @brief Specifies the minimum number of video frames that are displayed before a buffer swap will occur
        *
        * If interval is set to 0, buffer swaps are not synchronized to a video frame.
        * Interval is silently clamped to a platform specific minimum and maximum value.
        * Default value is platform dependent.
        *
        * @param[in] interval Minimum number of video frames that are displayed before a buffer swap will occur
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSwapInterval(int32_t interval);

        /**
        * @brief Specifies the scene's priority on this display
        *
        * The renderer will apply scene updates according to scene priority so that there will be less latency between
        * client scene flush and rendering for scenes with higher priority. This could result in more latency for scenes with less priority.
        * This setting should be used in combination with time budgets for resource uploads (#ramses::RamsesRenderer::setFrameTimerLimits)
        * to get the desired effect.
        *
        * Default priority is 0. Higher values mean less priority.
        * (Use negative values to increase priority)
        *
        * @param[in] sceneId scene id of the preferred scene
        * @param[in] priority scene priority. Higher value means less priority
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setScenePriority(sceneId_t sceneId, int32_t priority);

        /**
        * @brief Sets the batch size for resource uploads
        *
        * The resource upload batch size defines the number of resources that should be uploaded in a single step.
        * Time budgets (#ramses::RamsesRenderer::setFrameTimerLimits) are checked at the latest when a batch was uploaded.
        * It should be safe to configure a batch size of 1 unless time measurement has a significant performance impact
        * on the target device.
        * The batch size may not be 0.
        *
        * @param[in] batchSize the number of resources to upload in a single step (default: 10)
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setResourceUploadBatchSize(uint32_t batchSize);

        /**
        * Stores internal data for implementation specifics of DisplayConfig.
        */
        class DisplayConfigImpl& impl;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        DisplayConfig& operator=(const DisplayConfig& other) = delete;
    };
}

#endif
