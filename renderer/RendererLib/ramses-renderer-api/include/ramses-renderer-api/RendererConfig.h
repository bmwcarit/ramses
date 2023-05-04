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
    class RendererConfigImpl;

    /**
    * @brief The RendererConfig holds a set of parameters to be used
    * to initialize a renderer.
    * @ingroup RendererAPI
    */
    class RendererConfig : public StatusObject
    {
    public:
        /**
        * @brief Default constructor of RendererConfig
        */
        RAMSES_API RendererConfig();

        /**
        * @brief Destructor of RendererConfig
        */
        RAMSES_API ~RendererConfig() override;

        /**
        * @brief Set the Binary Shader Cache to be used in Renderer.
        * @param[in] cache the binary shader cache to be used by the Renderer
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setBinaryShaderCache(IBinaryShaderCache& cache);

        /**
        * @brief Set the resource cache implementation to be used by the renderer.
        * @param[in] cache the resource cache to be used by the renderer.
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setRendererResourceCache(IRendererResourceCache& cache);

        /**
        * @brief Enable the renderer to communicate with the system compositor.
        *        This flag needs to be enabled before calling any of the system compositor
        *        related calls, otherwise an error will be reported when issuing such commands
        *
        * @return StatusOK for success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t enableSystemCompositorControl();

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
        RAMSES_API status_t setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        /**
        * @brief Set the Wayland display name to connect system compositor to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setSystemCompositorWaylandDisplay(std::string_view waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        [[nodiscard]] RAMSES_API std::string_view getSystemCompositorWaylandDisplay() const;

        /**
        * @brief   Set the desired reporting period for first display loop timings.
        * @details The values are reported periodically via the renderer callback
        *          ramses::IRendererEventHandler::renderThreadLoopTimings.
        *          Only the first display is measured.
        *          A value of zero disables reporting and is the default.
        *
        * @param[in] period Cyclic time period after which timing information should be reported
        * @return StatusOK on success, otherwise the returned status can be used
        *         to resolve error message using getStatusMessage().
        */
        RAMSES_API status_t setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);

        /**
        * @brief Get the current reporting period for renderThread loop timings
        *
        * @return Reporting period for renderThread loop timings
        */
        [[nodiscard]] RAMSES_API std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RAMSES_API RendererConfig(const RendererConfig& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RAMSES_API RendererConfig(RendererConfig&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RAMSES_API RendererConfig& operator=(const RendererConfig& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RAMSES_API RendererConfig& operator=(RendererConfig&& other) noexcept;

        /**
        * Stores internal data for implementation specifics of RendererConfig.
        */
        std::reference_wrapper<RendererConfigImpl> m_impl;
    };
}

#endif
