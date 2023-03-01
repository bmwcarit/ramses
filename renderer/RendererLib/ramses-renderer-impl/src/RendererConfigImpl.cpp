//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererConfigImpl.h"

namespace ramses
{
    RendererConfigImpl::RendererConfigImpl()
        : StatusObjectImpl()
        , m_binaryShaderCache(nullptr)
        , m_rendererResourceCache(nullptr)
    {
    }

    void RendererConfigImpl::registerOptions(CLI::App& cli)
    {
        m_internalConfig.registerOptions(cli);
    }

    status_t RendererConfigImpl::enableSystemCompositorControl()
    {
        m_internalConfig.enableSystemCompositorControl();
        return StatusOK;
    }

    status_t RendererConfigImpl::setBinaryShaderCache(IBinaryShaderCache& cache)
    {
        m_binaryShaderCache = &cache;
        return StatusOK;
    }

    status_t RendererConfigImpl::setRendererResourceCache(IRendererResourceCache& cache)
    {
        m_rendererResourceCache = &cache;
        return StatusOK;
    }

    status_t RendererConfigImpl::setSystemCompositorWaylandDisplay(const char* waylandDisplay)
    {
        m_internalConfig.setWaylandDisplayForSystemCompositorController(waylandDisplay);
        return StatusOK;
    }

    const char* RendererConfigImpl::getSystemCompositorWaylandDisplay() const
    {
        return m_internalConfig.getWaylandDisplayForSystemCompositorController().c_str();
    }

    status_t RendererConfigImpl::setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec)
    {
        m_internalConfig.setFrameCallbackMaxPollTime(std::chrono::microseconds{waitTimeInUsec});
        return StatusOK;
    }

    IBinaryShaderCache* RendererConfigImpl::getBinaryShaderCache() const
    {
        return m_binaryShaderCache;
    }

    IRendererResourceCache* RendererConfigImpl::getRendererResourceCache() const
    {
        return m_rendererResourceCache;
    }

    status_t RendererConfigImpl::setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period)
    {
        m_internalConfig.setRenderthreadLooptimingReportingPeriod(period);
        return StatusOK;
    }

    std::chrono::milliseconds RendererConfigImpl::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_internalConfig.getRenderThreadLoopTimingReportingPeriod();
    }

    const ramses_internal::RendererConfig& RendererConfigImpl::getInternalRendererConfig() const
    {
        return m_internalConfig;
    }
}
