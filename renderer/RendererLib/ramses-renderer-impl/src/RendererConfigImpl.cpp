//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererConfigImpl.h"
#include "RendererLib/RendererConfigUtils.h"

namespace ramses
{
    RendererConfigImpl::RendererConfigImpl(int argc, char const* const* argv)
        : StatusObjectImpl()
        , m_binaryShaderCache(nullptr)
        , m_rendererResourceCache(nullptr)
    {
        ramses_internal::CommandLineParser parser(argc, argv);
        ramses_internal::RendererConfigUtils::ApplyValuesFromCommandLine(parser, m_internalConfig);
    }

    status_t RendererConfigImpl::enableSystemCompositorControl()
    {
        m_internalConfig.enableSystemCompositorControl();
        return StatusOK;
    }

    status_t RendererConfigImpl::setWaylandSocketEmbeddedGroup(const char* groupname)
    {
        m_internalConfig.setWaylandSocketEmbeddedGroup(groupname);
        return StatusOK;
    }

    const char* RendererConfigImpl::getWaylandSocketEmbeddedGroup() const
    {
        return m_internalConfig.getWaylandSocketEmbeddedGroup().c_str();
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

    status_t RendererConfigImpl::setWaylandSocketEmbedded(const char* socketname)
    {
        m_internalConfig.setWaylandSocketEmbedded(socketname);
        return StatusOK;
    }

    const char* RendererConfigImpl::getWaylandSocketEmbedded() const
    {
        return m_internalConfig.getWaylandSocketEmbedded().c_str();
    }

    status_t RendererConfigImpl::setWaylandSocketEmbeddedFD(int fd)
    {
        m_internalConfig.setWaylandSocketEmbeddedFD(fd);
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

    const ramses_internal::RendererConfig& RendererConfigImpl::getInternalRendererConfig() const
    {
        return m_internalConfig;
    }

    status_t RendererConfigImpl::validate(uint32_t indent) const
    {
        status_t status = StatusObjectImpl::validate(indent);
        indent += IndentationStep;

        const ramses_internal::String& embeddedCompositorFilename = m_internalConfig.getWaylandSocketEmbedded();
        int embeddedCompositorFileDescriptor                      = m_internalConfig.getWaylandSocketEmbeddedFD();

        if(embeddedCompositorFilename.getLength() == 0u && embeddedCompositorFileDescriptor < 0)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "no socket information for EmbeddedCompositor set (neither file descriptor nor file name). No embedded compositor available.");
            status = getValidationErrorStatus();
        }
        else if(embeddedCompositorFilename.getLength() > 0u && embeddedCompositorFileDescriptor >= 0)
        {
            addValidationMessage(EValidationSeverity_Warning, indent, "Competing settings for EmbeddedCompositor are set (file descriptor and file name). File descriptor setting will be preferred.");
            status = getValidationErrorStatus();
        }

        return status;
    }
}
