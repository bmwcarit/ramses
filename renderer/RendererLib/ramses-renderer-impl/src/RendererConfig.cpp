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

namespace ramses
{
    RendererConfig::RendererConfig(int32_t argc, char const* const* argv)
        : StatusObject(*new RendererConfigImpl(argc, argv))
        , impl(static_cast<RendererConfigImpl&>(StatusObject::impl))
    {
    }

    RendererConfig::RendererConfig(int32_t argc, char* argv[])
        : StatusObject(*new RendererConfigImpl(argc, const_cast<const char**>(argv)))
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

    status_t RendererConfig::setWaylandSocketEmbeddedGroup(const char* groupname)
    {
        const status_t status = impl.setWaylandSocketEmbeddedGroup(groupname);
        LOG_HL_RENDERER_API1(status, groupname);
        return status;
    }

    status_t RendererConfig::setWaylandSocketEmbedded(const char* socketname)
    {
        const status_t status = impl.setWaylandSocketEmbedded(socketname);
        LOG_HL_RENDERER_API1(status, socketname);
        return status;
    }

    status_t RendererConfig::setWaylandSocketEmbeddedFD(int socketFileDescriptor)
    {
        const status_t status = impl.setWaylandSocketEmbeddedFD(socketFileDescriptor);
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
}
