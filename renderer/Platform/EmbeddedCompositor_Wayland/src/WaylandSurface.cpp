//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandIVISurface.h"
#include "EmbeddedCompositor_Wayland/IWaylandShellSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandClient.h"
#include "EmbeddedCompositor_Wayland/INativeWaylandResource.h"
#include "EmbeddedCompositor_Wayland/IWaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/WaylandCallbackResource.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses_internal
{
    WaylandSurface::WaylandSurface(IEmbeddedCompositor_Wayland& compositor, IWaylandClient& client, int version, uint32_t id)
    : m_compositor(compositor)
    , m_clientCredentials(client.getCredentials())
    {
        LOG_DEBUG(CONTEXT_RENDERER, "WaylandSurface::WaylandSurface  " << m_clientCredentials);

        m_surfaceResource = client.resourceCreate(&wl_surface_interface, version, id);
        if (m_surfaceResource != nullptr)
        {
            LOG_INFO(CONTEXT_RENDERER, "WaylandSurface::WaylandSurface: created successfully  " << m_clientCredentials);

            m_surfaceResource->setImplementation(&m_surfaceInterface, this, ResourceDestroyedCallback);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandSurface::WaylandSurface Could not create wayland resource  " << m_clientCredentials);
            client.postNoMemory();
        }

        m_compositor.addWaylandSurface(*this);
    }

    WaylandSurface::~WaylandSurface()
    {
        LOG_INFO(CONTEXT_RENDERER, "WaylandSurface::~WaylandSurface: wayland surface destroyed  " << m_clientCredentials);

        unsetBufferFromSurface();
        m_compositor.removeWaylandSurface(*this);

        if (m_iviSurface != nullptr)
        {
            m_iviSurface->surfaceWasDeleted();
        }

        if (m_shellSurface != nullptr)
        {
            m_shellSurface->surfaceWasDeleted();
        }

        if (m_surfaceResource != nullptr)
        {
            // Remove ResourceDestroyedCallback
            m_surfaceResource->setImplementation(&m_surfaceInterface, this, nullptr);
            m_surfaceResource->destroy();
            delete m_surfaceResource;
        }

        for (auto callback : m_frameCallbacks)
        {
            delete callback;
        }

        for (auto callback : m_pendingCallbacks)
        {
            delete callback;
        }
    }

    void WaylandSurface::setWaylandBuffer(IWaylandBuffer* buffer)
    {
        m_buffer = buffer;

        if (m_iviSurface != nullptr)
        {
            m_iviSurface->bufferWasSetToSurface(buffer);
        }
    }

    IWaylandBuffer* WaylandSurface::getWaylandBuffer() const
    {
        return m_buffer;
    }

    bool WaylandSurface::hasPendingBuffer() const
    {
        return m_pendingBuffer != nullptr;
    }

    void WaylandSurface::setShellSurface(IWaylandShellSurface* shellSurface)
    {
        assert(m_shellSurface == nullptr || shellSurface == nullptr);
        m_shellSurface = shellSurface;
    }

    bool WaylandSurface::hasShellSurface() const
    {
        return m_shellSurface != nullptr;
    }

    const std::string& WaylandSurface::getSurfaceTitle() const
    {
        if (m_shellSurface)
        {
            return m_shellSurface->getTitle();
        }
        else
        {
            static std::string emptyString;
            return emptyString;
        }
    }

    void WaylandSurface::setIviSurface(IWaylandIVISurface* iviSurface)
    {
        assert(m_iviSurface == nullptr || iviSurface == nullptr);
        m_iviSurface = iviSurface;
    }

    bool WaylandSurface::hasIviSurface() const
    {
        return m_iviSurface != nullptr;
    }

    WaylandIviSurfaceId WaylandSurface::getIviSurfaceId() const
    {
        if (m_iviSurface)
        {
            return m_iviSurface->getIviId();
        }
        else
        {
            return {};
        }
    }

    UInt32 WaylandSurface::getNumberOfCommitedFrames() const
    {
        return m_numberOfCommitedFrames;
    }

    void WaylandSurface::resetNumberOfCommitedFrames()
    {
        m_numberOfCommitedFrames = 0;
    }

    uint64_t WaylandSurface::getNumberOfCommitedFramesSinceBeginningOfTime() const
    {
        return m_numberOfCommitedFramesSinceBeginningOfTime;
    }

    void WaylandSurface::resourceDestroyed()
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceResourceDestroyed");
        assert(nullptr != m_surfaceResource);

        // wl_resource is destroyed outside by the Wayland library, m_surfaceResource loses the ownership of the
        // Wayland resource, so that we don't call wl_resource_destroy.
        delete m_surfaceResource;
        m_surfaceResource = nullptr;
    }

    void WaylandSurface::surfaceAttach(IWaylandClient& client, WaylandBufferResource& bufferResource, int x, int y)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceAttach");

        UNUSED(client);
        UNUSED(x);
        UNUSED(y);

        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::surfaceAttach: set new pending buffer to surface with title "
                      << getSurfaceTitle() << ", ivi surface id " << getIviSurfaceId().getValue());

        m_pendingBuffer            = &m_compositor.getOrCreateBuffer(bufferResource);
        m_removeBufferOnNextCommit = false;

        //WaylandSurface::surfaceAttach is called iff attached buffer is not null (if null buffer gets attached then WaylandSurface::surfaceDetach gets called instead)
        assert(m_pendingBuffer);
        //if no buffer was attached (e.g. first buffer ever or after detach)
        const bool pendingBufferIsSharedMemoryBuffer = m_pendingBuffer->isSharedMemoryBuffer();
        if(!m_buffer)
        {
            m_bufferTypeChanged = true;

            LOG_INFO(CONTEXT_RENDERER, "WaylandSurface::surfaceAttach: Mark buffer type as changed for surface with ivi-id :" << getIviSurfaceId()
                     << " due to attach after no buffer attached to surface. New buffer is Shared mem buffer :" << pendingBufferIsSharedMemoryBuffer);
        }
        //if new buffer (pending buffer) has different type from current buffer
        else if(pendingBufferIsSharedMemoryBuffer != m_buffer->isSharedMemoryBuffer())
        {
            m_bufferTypeChanged = true;

            LOG_INFO(CONTEXT_RENDERER, "WaylandSurface::surfaceAttach: Mark buffer type as changed for surface with ivi-id :" << getIviSurfaceId()
                     << " because newly attached buffer has differen type from current. New buffer is Shared mem buffer :" << pendingBufferIsSharedMemoryBuffer);
        }
    }

    void WaylandSurface::surfaceDetach(IWaylandClient& client)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDetach");

        UNUSED(client);

        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::surfaceAttach: remove current pending buffer from surface with title "
                      << getSurfaceTitle() << ", ivi surface id " << getIviSurfaceId().getValue());

        m_pendingBuffer            = nullptr;
        m_removeBufferOnNextCommit = true;
    }

    void WaylandSurface::surfaceDamage(IWaylandClient& client, int x, int y, int width, int height)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDamage");

        UNUSED(client);
        UNUSED(x);
        UNUSED(y);
        UNUSED(width);
        UNUSED(height);
    }

    void WaylandSurface::surfaceFrame(IWaylandClient& client, uint32_t id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceFrame");

        // Create server side resource for new callback
        WaylandCallbackResource* cb = client.callbackResourceCreate(&wl_callback_interface, 1, id);
        if (cb)
        {
            // Add it to pending_callbacks until it gets committed
            m_pendingCallbacks.push_back(cb);
        }
        else
        {
            LOG_ERROR(CONTEXT_RENDERER, "WaylandCallbacks::surface_frame Could not create wayland resource!");
            client.postNoMemory();
        }
    }

    void WaylandSurface::surfaceSetOpaqueRegion(IWaylandClient& client, INativeWaylandResource* regionResource)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetOpaqueRegion");

        UNUSED(client);
        UNUSED(regionResource);
    }

    void WaylandSurface::surfaceSetInputRegion(IWaylandClient& client, INativeWaylandResource* regionResource)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetInputRegion");

        UNUSED(client);
        UNUSED(regionResource);
    }

    void WaylandSurface::surfaceCommit(IWaylandClient& client)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceCommit");

        UNUSED(client);

        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::surfaceCommit: handling commit message for surface with ivi surface id "
                      << getIviSurfaceId().getValue());

        // Transfers pending callbacks to list of frame_callbacks.
        for(const auto& callback : m_pendingCallbacks)
            m_frameCallbacks.push_back(callback);

        m_pendingCallbacks.clear();

        // If an attach is pending, current buffer is updated with pending one.
        if (m_pendingBuffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::surfaceCommit: new texture data for surface with ivi surface id "
                          << getIviSurfaceId().getValue());
            setBufferToSurface(*m_pendingBuffer);
            m_pendingBuffer = nullptr;
        }
        else
        {
            if (m_removeBufferOnNextCommit)
            {
                LOG_TRACE(
                    CONTEXT_RENDERER,
                    "WaylandSurface::surfaceCommit: remove buffer from surface with ivi surface id "
                        << getIviSurfaceId().getValue()
                        << " because triggered by earlier empty attachsurface");
                unsetBufferFromSurface();
            }
        }
        m_removeBufferOnNextCommit = false;
        m_numberOfCommitedFrames++;
        m_numberOfCommitedFramesSinceBeginningOfTime++;
    }

    void WaylandSurface::surfaceSetBufferTransform(IWaylandClient& client, int32_t transform)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetBufferTransform");

        UNUSED(client);
        UNUSED(transform);
    }

    void WaylandSurface::surfaceSetBufferScale(IWaylandClient& client, int32_t scale)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetBufferScale");

        UNUSED(client);
        UNUSED(scale);
    }

    void WaylandSurface::surfaceDamageBuffer(IWaylandClient& client, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDamageBuffer");

        UNUSED(client);
        UNUSED(x);
        UNUSED(y);
        UNUSED(width);
        UNUSED(height);
    }

    WaylandClientCredentials WaylandSurface::getClientCredentials() const
    {
        return m_clientCredentials;
    }

    bool WaylandSurface::dispatchBufferTypeChanged()
    {
        const auto result = m_bufferTypeChanged;
        m_bufferTypeChanged = false;
        return result;
    }

    void WaylandSurface::SurfaceDestroyCallback(wl_client* client, wl_resource* surfaceResource)
    {
        UNUSED(client);
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        delete surface;
    }

    void WaylandSurface::SurfaceAttachCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* bufferResource, int x, int y)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        if (nullptr != bufferResource)
        {
            WaylandBufferResource waylandBufferResource(bufferResource);
            surface->surfaceAttach(waylandClient, waylandBufferResource, x, y);
        }
        else
        {
            UNUSED(x);
            UNUSED(y);
            surface->surfaceDetach(waylandClient);
        }

    }

    void WaylandSurface::SurfaceDamageCallback(wl_client* client, wl_resource* surfaceResource, int x, int y, int width, int height)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceDamage(waylandClient, x, y, width, height);
    }

    void WaylandSurface::SurfaceFrameCallback(wl_client* client, wl_resource* surfaceResource, uint32_t id)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceFrame(waylandClient, id);
    }

    void WaylandSurface::SurfaceSetOpaqueRegionCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* regionResource)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        if (nullptr != regionResource)
        {
            NativeWaylandResource waylandRegionResource(regionResource);
            surface->surfaceSetOpaqueRegion(waylandClient, &waylandRegionResource);
        }
        else
        {
            surface->surfaceSetOpaqueRegion(waylandClient, nullptr);
        }
    }

    void WaylandSurface::SurfaceSetInputRegionCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* regionResource)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        if (nullptr != regionResource)
        {
            NativeWaylandResource waylandRegionResource(regionResource);
            surface->surfaceSetInputRegion(waylandClient, &waylandRegionResource);
        }
        else
        {
            surface->surfaceSetInputRegion(waylandClient, nullptr);
        }
    }

    void WaylandSurface::SurfaceCommitCallback(wl_client* client, wl_resource* surfaceResource)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceCommit(waylandClient);
    }

    void WaylandSurface::SurfaceSetBufferTransformCallback(wl_client* client, wl_resource* surfaceResource, int32_t transform)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceSetBufferTransform(waylandClient, transform);
    }

    void WaylandSurface::SurfaceSetBufferScaleCallback(wl_client* client, wl_resource* surfaceResource, int32_t scale)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceSetBufferScale(waylandClient, scale);
    }

    void WaylandSurface::SurfaceDamageBufferCallback(wl_client *client, wl_resource* surfaceResource, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceDamageBuffer(waylandClient, x, y, width, height);
    }

    void WaylandSurface::ResourceDestroyedCallback(wl_resource* surfaceResource)
    {
        WaylandSurface* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        surface->resourceDestroyed();
        delete surface;
    }

    void WaylandSurface::setBufferToSurface(IWaylandBuffer& buffer)
    {
        buffer.reference();

        if (m_buffer)
        {
            m_buffer->release();
        }
        else
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::setBufferToSurface client provides content for surface "
                          << getIviSurfaceId().getValue());
        }

        setWaylandBuffer(&buffer);
    }

    void WaylandSurface::unsetBufferFromSurface()
    {
        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::unsetBufferFromSurface: removing buffer used for surface with ivi surface id "
                      << getIviSurfaceId().getValue());

        if (m_buffer)
        {
            m_buffer->release();
        }

        setWaylandBuffer(nullptr);
    }

    void WaylandSurface::sendFrameCallbacks(UInt32 time)
    {
        for (auto callback: m_frameCallbacks)
        {
            callback->callbackSendDone(time);
            callback->destroy();
            delete callback;
        }

        m_frameCallbacks.clear();
    }

    void WaylandSurface::logInfos(RendererLogContext& context) const
    {
        context << "[ivi-surface-id: " << getIviSurfaceId().getValue() << "; title: \"" << getSurfaceTitle() << "\"]"
                << RendererLogContext::NewLine;
    }

    void WaylandSurface::bufferDestroyed(IWaylandBuffer& buffer)
    {
        if (m_buffer == &buffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::bufferDestroyed(): destroying buffer for surface with ivi "
                      "surface id :"
                          << getIviSurfaceId().getValue());

            unsetBufferFromSurface();
        }

        if (m_pendingBuffer == &buffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::bufferDestroyed(): destroying pending buffer for surface with "
                      "ivi surface id :"
                          << getIviSurfaceId().getValue());
            m_pendingBuffer = nullptr;
        }
    }
}
