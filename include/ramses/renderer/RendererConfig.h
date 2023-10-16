//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/Types.h"
#include <chrono>
#include <memory>

namespace ramses
{
    namespace internal
    {
        class RendererConfigImpl;
    }

    class IBinaryShaderCache;

    /**
    * @brief The RendererConfig holds a set of parameters to be used
    * to initialize a renderer.
    * @ingroup RendererAPI
    */
    class RAMSES_API RendererConfig
    {
    public:
        /**
        * @brief Default constructor of RendererConfig
        */
        RendererConfig();

        /**
        * @brief Destructor of RendererConfig
        */
        ~RendererConfig();

        /**
        * @brief Set the Binary Shader Cache to be used in Renderer.
        * @param[in] cache the binary shader cache to be used by the Renderer
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setBinaryShaderCache(IBinaryShaderCache& cache);

        /**
        * @brief Enable the renderer to communicate with the system compositor.
        *        This flag needs to be enabled before calling any of the system compositor
        *        related calls, otherwise an error will be reported when issuing such commands
        *
        * @return true on success, false if an error occurred (error is logged)
        */
        bool enableSystemCompositorControl();

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
         * @return     true on success, false if an error occurred (error is logged)
         */
        bool setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec);

        /**
        * @brief Set the Wayland display name to connect system compositor to.
        *        This will override the default behavior which is to use WAYLAND_DISPLAY environment variable
        *
        * @param[in] waylandDisplay Wayland display name to use for connection
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setSystemCompositorWaylandDisplay(std::string_view waylandDisplay);

        /**
        * @brief Get the current setting of Wayland display name
        *
        * @return Wayland display name to use for connection, empty means default
        */
        [[nodiscard]] std::string_view getSystemCompositorWaylandDisplay() const;

        /**
        * @brief   Set the desired reporting period for first display loop timings.
        * @details The values are reported periodically via the renderer callback
        *          ramses::IRendererEventHandler::renderThreadLoopTimings.
        *          Only the first display is measured.
        *          A value of zero disables reporting and is the default.
        *
        * @param[in] period Cyclic time period after which timing information should be reported
        * @return true on success, false if an error occurred (error is logged)
        */
        bool setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period);

        /**
        * @brief Get the current reporting period for renderThread loop timings
        *
        * @return Reporting period for renderThread loop timings
        */
        [[nodiscard]] std::chrono::milliseconds getRenderThreadLoopTimingReportingPeriod() const;

        /**
         * @brief Copy constructor
         * @param other source to copy from
         */
        RendererConfig(const RendererConfig& other);

        /**
         * @brief Move constructor
         * @param other source to move from
         */
        RendererConfig(RendererConfig&& other) noexcept;

        /**
         * @brief Copy assignment
         * @param other source to copy from
         * @return this instance
         */
        RendererConfig& operator=(const RendererConfig& other);

        /**
         * @brief Move assignment
         * @param other source to move from
         * @return this instance
         */
        RendererConfig& operator=(RendererConfig&& other) noexcept;

        /**
         * Get the internal data for implementation specifics of RendererConfig.
         */
        [[nodiscard]] internal::RendererConfigImpl& impl();

        /**
         * Get the internal data for implementation specifics of RendererConfig.
         */
        [[nodiscard]] const internal::RendererConfigImpl& impl() const;

    protected:
        /**
        * Stores internal data for implementation specifics of RendererConfig.
        */
        std::unique_ptr<internal::RendererConfigImpl> m_impl;
    };
}
