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
        * @brief Constructor of DisplayConfig that takes command line parameters
        * and parses them to initialize the parameters.
        * @param[in] argc Number of arguments in arguments array parameter
        * @param[in] argv Array of arguments as strings
        */
        DisplayConfig(int32_t argc, char const* const* argv);

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
        bool isWindowFullscreen() const;

        /**
        * @brief Sets window hints/properties to tell the window manager to disable window borders
        *
        * @param[in] borderless flag specifying whether window should be drawn without window-border
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWindowBorderless(bool borderless);

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
        waylandIviLayerId_t getWaylandIviLayerID() const;

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
        waylandIviSurfaceId_t getWaylandIviSurfaceID() const;

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
        * @brief Get the current setting of Android native window
        *
        * @return the current setting of Android native window, returns nullptr if no value has been set yet
        */
        void* getAndroidNativeWindow() const;

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
