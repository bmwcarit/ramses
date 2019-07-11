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

namespace ramses
{
    /**
    * @brief The DisplayConfig holds a set of parameters to be used
    * to initialize a display.
    */
    class RAMSES_API DisplayConfig : public StatusObject
    {
    public:
        /**
        * @brief Constructor of DisplayConfig that takes command line parameters
        * and parses them to initialize the parameters.
        * @param[in] argc Number of arguments in arguments array parameter
        * @param[in] argv Array of arguments as strings
        */
        DisplayConfig(int32_t argc = 0, char const* const* argv = 0);

        /**
        * @brief Constructor of DisplayConfig that takes command line parameters
        * and parses them to initialize the parameters.
        * @param[in] argc Number of arguments in arguments array parameter
        * @param[in] argv Array of arguments as strings
        */
        DisplayConfig(int32_t argc, char* argv[]);

        /**
        * @brief Copy constructor of DisplayConfig
        * @param[in] other Other instance of DisplayConfig
        */
        DisplayConfig(const DisplayConfig& other);

        /**
        * @brief Destructor of DisplayConfig
        */
        virtual ~DisplayConfig();

        /**
        * @brief Sets the view position of the display.
        * The view denotes the logical location of the display in the global RAMSES world.
        *
        * @param[in] x The 'x' component of the position
        * @param[in] y The 'y' component of the position
        * @param[in] z The 'z' component of the position
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setViewPosition(float x, float y, float z);

        /**
        * @brief Gets the view position of the display.
        * @param[out] x The 'x' component of the position
        * @param[out] y The 'y' component of the position
        * @param[out] z The 'z' component of the position
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getViewPosition(float& x, float& y, float& z) const;

        /**
        * @brief Sets the view rotation of the display.
        * See DisplayConfig::setViewPosition().
        *
        * @param[in] x The 'x' component of the rotation
        * @param[in] y The 'y' component of the rotation
        * @param[in] z The 'z' component of the rotation
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setViewRotation(float x, float y, float z);

        /**
        * @brief Gets the view rotation of the display.
        * See DisplayConfig::getViewPosition().
        *
        * @param[in] x The 'x' component of the rotation
        * @param[in] y The 'y' component of the rotation
        * @param[in] z The 'z' component of the rotation
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getViewRotation(float& x, float& y, float& z) const;

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
        * @brief Automatically sets the window size so that it fills the entire display.
        * Overrides DisplayConfig::setWindowRectangle() when set to true.
        *
        * @param[in] fullscreen Flag specifying whether window should be fullscreen
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowFullscreen(bool fullscreen);

        /**
        * @brief Sets window hints/properties to tell the window manager to disable window borders
        *
        * @param[in] borderless flag specifying whether window should be drawn without window-border
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowBorderless(bool borderless);

        /**
        * @brief Sets a projection based on a perspective view frustum.
        *
        * @param[in] fieldOfViewY The vertical field of view of the frustum
        * @param[in] aspectRatio The aspect ratio between the frustum's width and height
        * @param[in] nearPlane The near plane of the frustum
        * @param[in] farPlane The far plane of the frustum
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPerspectiveProjection(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane);

        /**
        * @brief Sets a projection based on a perspective view frustum.
        *        This represents an alternative setter of perspective projection parameters,
        *        where the opening angles of the vertical and horizontal planes of the camera
        *        frustum can be given explicitly.
        *
        * @param[in] leftPlane Opening angle of the left camera frustum plane
        * @param[in] rightPlane Opening angle of the left camera frustum plane
        * @param[in] bottomPlane Opening angle of the bottom camera frustum plane
        * @param[in] topPlane Opening angle of the top camera frustum plane
        * @param[in] nearPlane Distance from center to the 'near' wall of the frustum
        * @param[in] farPlane Distance from center to the 'far' wall of the frustum
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setPerspectiveProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);

        /**
        * @brief Sets an orthographic projection based on a cube.
        *
        * @param[in] leftPlane Distance from center to the left wall of the cube
        * @param[in] rightPlane Distance from center to the right wall of the cube
        * @param[in] bottomPlane Distance from center to the bottom wall of the cube
        * @param[in] topPlane Distance from center to the top wall of the cube
        * @param[in] nearPlane Distance from center to the 'near' wall of the cube (in analogy to near plane in perspective projection)
        * @param[in] farPlane Distance from center to the 'far' wall of the cube (in analogy to far plane in perspective projection)
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setOrthographicProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane);

        /**
        * @brief Set number of samples to be used with multisampled rendering.
        *
        * @param[in] numSamples Number of samples per pixel, valid values are 1, 2 and 4
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setMultiSampling(uint32_t numSamples);

        /**
        * @brief Get number of samples used with multisampled rendering.
        *
        * @param[out] numSamples Number of samples per pixel, valid values are 1, 2 and 4
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t getMultiSamplingSamples(uint32_t& numSamples) const;

        /**
        * @brief Enable warping post effect. User has to set warping mesh data later in the display,
        * otherwise the mesh is a fullscreen quad.
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t enableWarpingPostEffect();

        /**
        * @brief Enable stereo display.
        *        Will create a stereo display that can be used to render left and right eye.
        *        Stereo display does not support anti aliasing and any post processing.
        *
        * @return RendererStatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t enableStereoDisplay();

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
        status_t setWaylandIviLayerID(uint32_t waylandIviLayerID);

        /**
        * @brief Get the ID of the Wayland IVI layer to which the IVI surface used by the display is attached.
        *
        * @return the current setting of wayland IVI layer ID, returns 0xFFFFFFFF if no value has been set yet
        */
        uint32_t getWaylandIviLayerID() const;


        /**
        * @brief [Mandatory on Waylannd] Set IVI surface ID to use when creating the display window on Wayland.
        *
        * - This is the Wayland IVI surface ID, i.e. the ID that the display reports to Wayland so that the surface (=window)
        *   can be controlled over the system compositor controller
        * - If the surface is not already existing it will be created during display creation
        *
        * @param[in] waylandIviSurfaceID IVI surface ID to use for the display window
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandIviSurfaceID(uint32_t waylandIviSurfaceID);

        /**
        * @brief Get the current setting of IVI surface ID
        *
        * @return the current setting of IVI surface ID, returns 0xFFFFFFFF if no value has been set yet
        */
        uint32_t getWaylandIviSurfaceID() const;

        /**
        * @brief [Mandatory on Integrity] Set device unit number to use when creating the display window on Integrity using RGL Window Manager API.
        *
        *
        * @param[in] rglDeviceUnit Device unit number to use for the display window
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit);

        /**
        * @brief Get the current setting of RGL device unit number
        *
        * @return the current setting of RGL device unit, returns 0xFFFFFFFF if no value has been set yet
        */
        uint32_t getIntegrityRGLDeviceUnit() const;

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
        * @param[in] red clear value for red channel
        * @param[in] green clear value for green channel
        * @param[in] blue clear value for blue channel
        * @param[in] alpha clear value for alpha channel
        *
        * @return  StatusOK on success, otherwise the returned status can be used to resolve
        *          to resolve error message using getStatusMessage()
        */
        status_t setClearColor(float red, float green, float blue, float alpha);

        /**
        * @brief Enables/disables offscreen rendering for the created display. By default displays are
        *        created to be onscreen (not offscreen). An offscreen display will not open a visible
        *        window, but will use an offscreen surface to render the framebuffer contents. Also,
        *        an offscreen display will not block on the system compositor, if created on a
        *        wayland backend.
        * @param[in] offscreenFlag The flag for enabling or disabling offscreen rendering for the display
        *
        * @return  StatusOK on success, otherwise the returned status can be used to resolve
        *          to resolve error message using getStatusMessage()
        */
        status_t setOffscreen(bool offscreenFlag);

        /**
        * @brief [Only for Windows] Set the HWND handle for custom display creation.
        *
        * - This is the HWND variable (very platform dependent)
        * - On other systems except for Windows, the value has no meaning
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
        void* getWindowsWindowHandle() const;

        /**
        * @brief Set the Wayland display name to connect to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandDisplay(const char* waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        const char* getWaylandDisplay() const;

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
