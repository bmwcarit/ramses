//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses/renderer/RendererConfig.h"

// internal
#include "impl/RendererConfigImpl.h"
#include "impl/APILoggingMacros.h"
#include <chrono>

namespace ramses
{
    RendererConfig::RendererConfig()
        : m_impl{ std::make_unique<internal::RendererConfigImpl>() }
    {
    }

    RendererConfig::~RendererConfig() = default;

    RendererConfig::RendererConfig(const RendererConfig& other)
        : m_impl{ std::make_unique<internal::RendererConfigImpl>(*other.m_impl) }
    {
    }

    RendererConfig::RendererConfig(RendererConfig&& other) noexcept = default;

    RendererConfig& RendererConfig::operator=(const RendererConfig& other)
    {
        m_impl = std::make_unique<internal::RendererConfigImpl>(*other.m_impl);
        return *this;
    }

    RendererConfig& RendererConfig::operator=(RendererConfig&& other) noexcept = default;

    bool RendererConfig::setBinaryShaderCache(IBinaryShaderCache& cache)
    {
        const auto status = m_impl->setBinaryShaderCache(cache);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(cache));
        return status;
    }

    bool RendererConfig::enableSystemCompositorControl()
    {
        const auto status = m_impl->enableSystemCompositorControl();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    bool RendererConfig::setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec)
    {
        const auto status = m_impl->setFrameCallbackMaxPollTime(waitTimeInUsec);
        LOG_HL_RENDERER_API1(status, waitTimeInUsec);
        return status;
    }

    bool RendererConfig::setSystemCompositorWaylandDisplay(std::string_view waylandDisplay)
    {
        return m_impl->setSystemCompositorWaylandDisplay(waylandDisplay);
    }

    std::string_view RendererConfig::getSystemCompositorWaylandDisplay() const
    {
        return m_impl->getSystemCompositorWaylandDisplay();
    }

    bool RendererConfig::setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period)
    {
        const auto status = m_impl->setRenderThreadLoopTimingReportingPeriod(period);
        LOG_HL_RENDERER_API1(status, period.count());
        return status;
    }

    std::chrono::milliseconds RendererConfig::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_impl->getRenderThreadLoopTimingReportingPeriod();
    }

    internal::RendererConfigImpl& RendererConfig::impl()
    {
        return *m_impl;
    }

    const internal::RendererConfigImpl& RendererConfig::impl() const
    {
        return *m_impl;
    }
}
