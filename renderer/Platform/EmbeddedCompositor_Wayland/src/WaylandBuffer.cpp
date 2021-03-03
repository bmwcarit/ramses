//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Warnings.h"
#include <cassert>

namespace ramses_internal
{
    WaylandBuffer::WaylandBuffer(WaylandBufferResource& bufferResource, IEmbeddedCompositor_Wayland& compositor)
        : m_bufferResource(*bufferResource.clone())
        , m_refCount(0)
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

    void WaylandBuffer::ClientBufferDestroyed(wl_listener* listener, void* data)
    {
        UNUSED(data);
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandBuffer::ClientBufferDestroyed data: " << data);

        WaylandBuffer* buffer(nullptr);

WARNINGS_PUSH
WARNING_DISABLE_LINUX(-Winvalid-offsetof)
WARNING_DISABLE_LINUX(-Wcast-align)
WARNING_DISABLE_LINUX(-Wold-style-cast)

        buffer = wl_container_of(listener, buffer, m_destroyListener);

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
}
