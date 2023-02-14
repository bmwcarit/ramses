//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererConfigImpl.h"
#include "CLI/CLI.hpp"

namespace ramses
{
    RendererConfigImpl::RendererConfigImpl(int argc, char const* const* argv)
        : StatusObjectImpl()
        , m_binaryShaderCache(nullptr)
        , m_rendererResourceCache(nullptr)
    {
        if (argc > 1)
        {
            CLI::App cli;
            m_internalConfig.registerOptions(cli);
            cli.allow_extras();
            try
            {
                cli.parse(argc, argv);
            }
            catch (CLI::ParseError& e)
            {
                const auto err = cli.exit(e);
                if (err != 0)
                    exit(err);
            }
        }
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

    status_t RendererConfigImpl::setWaylandEmbeddedCompositingSocketGroup(const char* groupname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketGroup(groupname);
        return StatusOK;
    }

    const char* RendererConfigImpl::getWaylandSocketEmbeddedGroup() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedGroup().c_str();
    }

    status_t RendererConfigImpl::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        if (m_internalConfig.setWaylandEmbeddedCompositingSocketPermissions(permissions))
            return StatusOK;
        return addErrorEntry("RendererConfig::setWaylandEmbeddedCompositingSocketPermissions failed");
    }

    uint32_t RendererConfigImpl::getWaylandSocketEmbeddedPermissions() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedPermissions();
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

    status_t RendererConfigImpl::setWaylandEmbeddedCompositingSocketName(const char* socketname)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketName(socketname);
        return StatusOK;
    }

    const char* RendererConfigImpl::getWaylandEmbeddedCompositingSocketName() const
    {
        return m_internalConfig.getWaylandSocketEmbedded().c_str();
    }

    status_t RendererConfigImpl::setWaylandEmbeddedCompositingSocketFD(int fd)
    {
        m_internalConfig.setWaylandEmbeddedCompositingSocketFD(fd);
        return StatusOK;
    }

    int RendererConfigImpl::getWaylandSocketEmbeddedFD() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedFD();
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

    status_t RendererConfigImpl::validate() const
    {
        status_t status = StatusObjectImpl::validate();

        const ramses_internal::String& embeddedCompositorFilename = m_internalConfig.getWaylandSocketEmbedded();
        int embeddedCompositorFileDescriptor                      = m_internalConfig.getWaylandSocketEmbeddedFD();

        if(embeddedCompositorFilename.size() == 0u && embeddedCompositorFileDescriptor < 0)
            status = addValidationMessage(EValidationSeverity_Warning, "no socket information for EmbeddedCompositor set (neither file descriptor nor file name). No embedded compositor available.");
        else if(embeddedCompositorFilename.size() > 0u && embeddedCompositorFileDescriptor >= 0)
            status = addValidationMessage(EValidationSeverity_Warning, "Competing settings for EmbeddedCompositor are set (file descriptor and file name). File descriptor setting will be preferred.");

        return status;
    }

}
