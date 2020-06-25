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
        : RendererConfig(0, nullptr)
    {
    }

    RendererConfig::RendererConfig(int32_t argc, char const* const* argv)
        : StatusObject(*new RendererConfigImpl(argc, argv))
        , impl(static_cast<RendererConfigImpl&>(StatusObject::impl))
    {
    }

    RendererConfig::RendererConfig(const RendererConfig& other)
        : StatusObject(*new RendererConfigImpl(other.impl))
        , impl(static_cast<RendererConfigImpl&>(StatusObject::impl))
    {
    }

    RendererConfig::~RendererConfig()
    {
    }

    ramses::status_t RendererConfig::setBinaryShaderCache(IBinaryShaderCache& cache)
    {
        const status_t status = impl.setBinaryShaderCache(cache);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(cache));
        return status;
    }

    status_t RendererConfig::setRendererResourceCache(IRendererResourceCache& cache)
    {
        const status_t status = impl.setRendererResourceCache(cache);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_OBJECT_STRING(cache));
        return status;
    }

    status_t RendererConfig::enableSystemCompositorControl()
    {
        const status_t status = impl.enableSystemCompositorControl();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t RendererConfig::setFrameCallbackMaxPollTime(uint64_t waitTimeInUsec)
    {
        const status_t status = impl.setFrameCallbackMaxPollTime(waitTimeInUsec);
        LOG_HL_RENDERER_API1(status, waitTimeInUsec);
        return status;
    }

    status_t RendererConfig::setWaylandEmbeddedCompositingSocketGroup(const char* groupname)
    {
        const status_t status = impl.setWaylandEmbeddedCompositingSocketGroup(groupname);
        LOG_HL_RENDERER_API1(status, groupname);
        return status;
    }

    status_t RendererConfig::setWaylandEmbeddedCompositingSocketName(const char* socketname)
    {
        const status_t status = impl.setWaylandEmbeddedCompositingSocketName(socketname);
        LOG_HL_RENDERER_API1(status, socketname);
        return status;
    }

    const char* RendererConfig::getWaylandEmbeddedCompositingSocketName() const
    {
        return impl.getWaylandEmbeddedCompositingSocketName();
    }

    status_t RendererConfig::setWaylandEmbeddedCompositingSocketFD(int socketFileDescriptor)
    {
        const status_t status = impl.setWaylandEmbeddedCompositingSocketFD(socketFileDescriptor);
        LOG_HL_RENDERER_API1(status, socketFileDescriptor);
        return status;
    }

    status_t RendererConfig::setSystemCompositorWaylandDisplay(const char* waylandDisplay)
    {
        return impl.setSystemCompositorWaylandDisplay(waylandDisplay);
    }

    const char* RendererConfig::getSystemCompositorWaylandDisplay() const
    {
        return impl.getSystemCompositorWaylandDisplay();
    }

    status_t RendererConfig::setRenderThreadLoopTimingReportingPeriod(std::chrono::milliseconds period)
    {
        const status_t status = impl.setRenderThreadLoopTimingReportingPeriod(period);
        LOG_HL_RENDERER_API1(status, period.count());
        return status;
    }

    std::chrono::milliseconds RendererConfig::getRenderThreadLoopTimingReportingPeriod() const
    {
        return impl.getRenderThreadLoopTimingReportingPeriod();
    }

}
