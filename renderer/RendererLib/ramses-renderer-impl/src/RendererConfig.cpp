//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-renderer-api/RendererConfig.h"

// internal
#include "RendererConfigImpl.h"
#include "APILoggingMacros.h"
#include "ramses-renderer-api/IRendererResourceCache.h"
#include <chrono>

namespace ramses
{
    RendererConfig::RendererConfig()
        : StatusObject{ std::make_unique<RendererConfigImpl>() }
        , m_impl{ static_cast<RendererConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RendererConfig::~RendererConfig() = default;

    RendererConfig::RendererConfig(const RendererConfig& other)
        : StatusObject{ std::make_unique<RendererConfigImpl>(other.m_impl) }
        , m_impl{ static_cast<RendererConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RendererConfig::RendererConfig(RendererConfig&& other) noexcept
        : StatusObject{ std::move(other.StatusObject::m_impl) }
        , m_impl{ static_cast<RendererConfigImpl&>(*StatusObject::m_impl) }
    {
    }

    RendererConfig& RendererConfig::operator=(const RendererConfig& other)
    {
        StatusObject::m_impl = std::make_unique<RendererConfigImpl>(other.m_impl);
        m_impl = static_cast<RendererConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    RendererConfig& RendererConfig::operator=(RendererConfig&& other) noexcept
    {
        StatusObject::m_impl = std::move(other.StatusObject::m_impl);
        m_impl = static_cast<RendererConfigImpl&>(*StatusObject::m_impl);
        return *this;
    }

    ramses::status_t RendererConfig::setBinaryShaderCache(IBinaryShaderCache& cache)
    {
        const status_t status = m_impl.get().setBinaryShaderCache(cache);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(cache));
        return status;
    }

    status_t RendererConfig::setRendererResourceCache(IRendererResourceCache& cache)
    {
        const status_t status = m_impl.get().setRendererResourceCache(cache);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(cache));
        return status;
    }

    status_t RendererConfig::enableSystemCompositorControl()
    {
        const status_t status = m_impl.get().enableSystemCompositorControl();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t RendererConfig::setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec)
    {
        const status_t status = m_impl.get().setFrameCallbackMaxPollTime(waitTimeInUsec);
        LOG_HL_RENDERER_API1(status, waitTimeInUsec);
        return status;
    }

    status_t RendererConfig::setSystemCompositorWaylandDisplay(std::string_view waylandDisplay)
    {
        return m_impl.get().setSystemCompositorWaylandDisplay(waylandDisplay);
    }

    std::string_view RendererConfig::getSystemCompositorWaylandDisplay() const
    {
        return m_impl.get().getSystemCompositorWaylandDisplay();
    }

    status_t RendererConfig::setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period)
    {
        const status_t status = m_impl.get().setRenderThreadLoopTimingReportingPeriod(period);
        LOG_HL_RENDERER_API1(status, period.count());
        return status;
    }

    std::chrono::milliseconds RendererConfig::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_impl.get().getRenderThreadLoopTimingReportingPeriod();
    }

}
