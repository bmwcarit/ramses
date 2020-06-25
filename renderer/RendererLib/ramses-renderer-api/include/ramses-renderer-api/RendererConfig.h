//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_RENDERERAPI_RENDERERCONFIG_H
#define RAMSES_RENDERERAPI_RENDERERCONFIG_H

#include "ramses-renderer-api/Types.h"
#include "ramses-framework-api/StatusObject.h"
#include <chrono>

namespace ramses
{
    class IBinaryShaderCache;
    class IRendererResourceCache;

    /**
    * @brief The RendererConfig holds a set of parameters to be used
    * to initialize a renderer.
    */
    class RAMSES_API RendererConfig : public StatusObject
    {
    public:
        /**
        * @brief Default constructor of RendererConfig
        */
        RendererConfig();

        /**
        * @brief Constructor of RendererConfig that takes command line parameters
        * and parses them to initialize the parameters.
        * @param[in] argc Number of arguments in arguments array parameter
        * @param[in] argv Array of arguments as strings
        */
        RendererConfig(int32_t argc, char const* const* argv);

        /**
        * @brief Copy constructor of RendererConfig
        * @param[in] other Other instance of RendererConfig
        */
        RendererConfig(const RendererConfig& other);

        /**
        * @brief Destructor of RendererConfig
        */
        virtual ~RendererConfig();

        /**
        * @brief Set the Binary Shader Cache to be used in Renderer.
        * @param[in] cache the binary shader cache to be used by the Renderer
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setBinaryShaderCache(IBinaryShaderCache& cache);

        /**
        * @brief Set the resource cache implementation to be used by the renderer.
        * @param[in] cache the resource cache to be used by the renderer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRendererResourceCache(IRendererResourceCache& cache);

        /**
        * @brief Enable the renderer to communicate with the system compositor.
        *        This flag needs to be enabled before calling any of the system compositor
        *        related calls, otherwise an error will be reported when issuing such commands
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t enableSystemCompositorControl();

        /**
         * @brief      Set the maximum time to wait for the system compositor frame callback
         *             before aborting and skipping rendering of current frame. This is an
         *             advanced function to be used by experts only. Be warned that the
         *             synchronization of frame callbacks with the system compositor and
         *             the display controller vsync is a sensitive topic and can majorly
         *             influence system performance.
         *
         * @param[in]  waitTimeInUsec  The maximum time wait for a frame callback, in microseconds
         *
         * @return     StatusOK for success, otherwise the returned status can
         *             be used to resolve error message using
         *             getStatusMessage().
         */
        status_t setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        /**
         * @brief      Set the name to be used for the embedded compositing
         *             display socket name.
         *
         *             The embedded compositor communicates with its clients via
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
         *             groupname is set, also the group rights are set.
         *
         *             Be aware that the socket file name is only used if the
         *             file descriptor is set to an invalid value (default), see
         *             RendererConfig::setWaylandEmbeddedCompositingSocketFD
         *
         *             If neither filename nor file descriptor is set display
         *             creation will fail.
         *             If both filename and file descriptor are set display
         *             creation will fail.
         *
         * @param[in]  socketname  The file name of the socket file.
         *
         * @return     StatusOK for success, otherwise the returned status can
         *             be used to resolve error message using
         *             getStatusMessage().
         */
        status_t setWaylandEmbeddedCompositingSocketName(const char* socketname);

        /**
        * @brief Get the current setting of embedded compositing display socket name
        *
        * @return Wayland display name to use for embedded compositing socket
        */
        const char* getWaylandEmbeddedCompositingSocketName() const;

        /**
        * @brief Request that the embedded compositing display socket obtains the group permissions given
        *        by the given name.
        *
        * @param[in] groupname The group name of the socket.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setWaylandEmbeddedCompositingSocketGroup(const char* groupname);

        /**
         * @brief      Set the file descriptor for the embedded compositor
         *             display socket.
         *
         *             The embedded compositor communicates with its clients via
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
         *             If neither filename nor file descriptor is set display
         *             creation will fail.
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
        * @brief Set the Wayland display name to connect system compositor to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setSystemCompositorWaylandDisplay(const char* waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        const char* getSystemCompositorWaylandDisplay() const;

        /**
        * @brief Set the desired reporting period for renderThread loop timings
        *        The values are reported via the renderer event 'RenderThreadPeriodicLoopTimes'
        *        A value of zero disables reporting and is the default.
        *
        * @param[in] period Cyclic time period after which timing information should be reported
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        status_t setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);

        /**
        * @brief Get the current reporting period for renderThread loop timings
        *
        * @return Reporting period for renderThread loop timings
        */
        std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        /**
        * Stores internal data for implementation specifics of RendererConfig.
        */
        class RendererConfigImpl& impl;

        /**
         * @brief Deleted copy assignment
         * @param other unused
         * @return unused
         */
        RendererConfig& operator=(const RendererConfig& other) = delete;
    };
}

#endif
