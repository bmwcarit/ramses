//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabuf.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/WaylandEGLExtensionProcs.h"
#include "internal/Core/Utils/LogMacros.h"
#include "internal/Core/Utils/Warnings.h"
#include "internal/RendererLib/RendererLogContext.h"
#include <cassert>

namespace ramses::internal
{
    WaylandBuffer::WaylandBuffer(WaylandBufferResource& bufferResource, IEmbeddedCompositor_Wayland& compositor)
        : m_bufferResource(*bufferResource.clone())
        , m_compositor(compositor)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandBuffer::WaylandBuffer");

        m_destroyListener.notify = ClientBufferDestroyed;
        m_bufferResource.addDestroyListener(&m_destroyListener);
    }

    WaylandBuffer::~WaylandBuffer()
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandBuffer::~WaylandBuffer");
        delete &m_bufferResource;
    }

    WaylandBufferResource& WaylandBuffer::getResource() const
    {
        return m_bufferResource;
    }

    void WaylandBuffer::ClientBufferDestroyed(wl_listener* listener, [[maybe_unused]] void* data)
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandBuffer::ClientBufferDestroyed data: {}", data);

        WaylandBuffer* buffer(nullptr);

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Winvalid-offsetof)
WARNING_DISABLE_LINUX(-Wcast-align)
WARNING_DISABLE_LINUX(-Wold-style-cast)

        buffer = wl_container_of(listener, buffer, m_destroyListener); // NOLINT(cppcoreguidelines-pro-type-cstyle-cast)

WARNINGS_POP

        assert(buffer != nullptr);

        buffer->m_compositor.handleBufferDestroyed(*buffer);
        delete buffer;
    }

    /** Increase reference counter of buffer. */
    void WaylandBuffer::reference()
    {
        m_refCount++;
    }

    /** Decrease reference counter of buffer. */
    void WaylandBuffer::release()
    {
        assert(m_refCount > 0);

        if (--m_refCount == 0)
        {
            m_bufferResource.bufferSendRelease();
        }
    }

    bool WaylandBuffer::isSharedMemoryBuffer() const
    {
        return m_bufferResource.bufferGetSharedMemoryData() != nullptr;
    }

    void WaylandBuffer::logInfos(RendererLogContext& contexti, const WaylandEGLExtensionProcs &eglExt) const
    {
        auto res = m_bufferResource.getLowLevelHandle();
        contexti << "Buffer: res:" << static_cast<const void*>(res);
        wl_shm_buffer* shm = wl_shm_buffer_get(res);
        if (shm != nullptr)
        {
            contexti << " type:shm w:" << wl_shm_buffer_get_width(shm) << " h:" << wl_shm_buffer_get_height(shm);
            contexti << " format:" << wl_shm_buffer_get_format(shm);
        }
        else if ((res != nullptr) && (wl_resource_instance_of(res, &wl_buffer_interface, &LinuxDmabufBuffer::m_bufferInterface) != 0))
        {
            contexti << " type:dma";
            auto dmabuf = static_cast<LinuxDmabufBufferData*>(wl_resource_get_user_data(res));
            if (dmabuf != nullptr)
            {
                contexti << " w:" << dmabuf->getWidth() << " h:" << dmabuf->getHeight();
                contexti << " planes:" << dmabuf->getNumPlanes() << " format:" << dmabuf->getFormat();
                contexti << " flags:" << dmabuf->getFlags();
            }
        }
        else
        {
            EGLint textureFormat =  0;
            EGLint textureWidth =  0;
            EGLint textureHeight =  0;
            auto bufferResource = m_bufferResource.getLowLevelHandle();
            contexti << " type:EGL_WAYLAND_BUFFER_WL";
            EGLBoolean ret = eglExt.eglQueryWaylandBufferWL(bufferResource, EGL_TEXTURE_FORMAT, &textureFormat);
            if (ret == EGL_FALSE)
            {
                contexti <<  "eglQueryWaylandBufferWL(EGL_TEXTURE_FORMAT) failed" << RendererLogContext::NewLine;
            }
            ret = eglExt.eglQueryWaylandBufferWL(bufferResource, EGL_WIDTH, &textureWidth);
            if (ret == EGL_FALSE)
            {
                contexti <<  "eglQueryWaylandBufferWL(EGL_WIDTH) failed" << RendererLogContext::NewLine;
            }
            ret = eglExt.eglQueryWaylandBufferWL(bufferResource, EGL_HEIGHT, &textureHeight);
            if (ret == EGL_FALSE)
            {
                contexti <<  "eglQueryWaylandBufferWL(EGL_HEIGHT) failed" << RendererLogContext::NewLine;
            }
            contexti << " w:" << textureWidth << " h:" << textureHeight
                     << " format:" << WaylandEGLExtensionProcs::getTextureFormatName(textureFormat) << " (" << textureFormat << ")";
        }
        contexti << RendererLogContext::NewLine;
    }
}
