//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "impl/RendererConfigImpl.h"

namespace ramses::internal
{
    RendererConfigImpl::RendererConfigImpl() = default;

    bool RendererConfigImpl::enableSystemCompositorControl()
    {
        m_internalConfig.enableSystemCompositorControl();
        return true;
    }

    bool RendererConfigImpl::setBinaryShaderCache(IBinaryShaderCache& cache)
    {
        m_binaryShaderCache = &cache;
        return true;
    }

    bool RendererConfigImpl::setSystemCompositorWaylandDisplay(std::string_view waylandDisplay)
    {
        m_internalConfig.setWaylandDisplayForSystemCompositorController(waylandDisplay);
        return true;
    }

    std::string_view RendererConfigImpl::getSystemCompositorWaylandDisplay() const
    {
        return m_internalConfig.getWaylandDisplayForSystemCompositorController();
    }

    bool RendererConfigImpl::setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec)
    {
        m_internalConfig.setFrameCallbackMaxPollTime(std::chrono::microseconds{waitTimeInUsec});
        return true;
    }

    ramses::IBinaryShaderCache* RendererConfigImpl::getBinaryShaderCache() const
    {
        return m_binaryShaderCache;
    }

    bool RendererConfigImpl::setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period)
    {
        m_internalConfig.setRenderthreadLooptimingReportingPeriod(period);
        return true;
    }

    std::chrono::milliseconds RendererConfigImpl::getRenderThreadLoopTimingReportingPeriod() const
    {
        return m_internalConfig.getRenderThreadLoopTimingReportingPeriod();
    }

    const ramses::internal::RendererConfigData& RendererConfigImpl::getInternalRendererConfig() const
    {
        return m_internalConfig;
    }
}
