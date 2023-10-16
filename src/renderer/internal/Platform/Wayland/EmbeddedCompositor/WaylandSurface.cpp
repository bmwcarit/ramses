//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandIVISurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandShellSurface.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandClient.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IWaylandBuffer.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCallbackResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandBufferResource.h"
#include "internal/Core/Utils/ThreadLocalLogForced.h"
#include <cassert>

namespace ramses::internal
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

    void WaylandSurface::logSurfaceAttach(ELogLevel level, const char* stage, IWaylandBuffer* buffer, int x, int y)
    {
        if (level <= CONTEXT_RENDERER.getLogLevel())
        {
            auto res = buffer->getResource();
            const auto isShm = (res.bufferGetSharedMemoryData() != nullptr);
            const auto width = res.getWidth();
            const auto height = res.getHeight();
            auto* wlres = res.getLowLevelHandle();
            LOG_COMMON_RP(CONTEXT_RENDERER, level, "WaylandSurface::surfaceAttach{}: {} shm:{} x:{} y:{} w:{} h:{} res:{}",
                          stage, getIviSurfaceId(), isShm, x, y, width, height, static_cast<void*>(wlres));
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
        static std::string emptyString;
        return emptyString;
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
        return {};
    }

    uint32_t WaylandSurface::getNumberOfCommitedFrames() const
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

    void WaylandSurface::surfaceAttach([[maybe_unused]] IWaylandClient& client, WaylandBufferResource& bufferResource, [[maybe_unused]] int x, [[maybe_unused]] int y)
    {
        m_pendingBuffer            = &m_compositor.getOrCreateBuffer(bufferResource);
        m_removeBufferOnNextCommit = false;

        // WaylandSurface::surfaceAttach is called if attached buffer is not null (if null buffer gets attached then WaylandSurface::surfaceDetach gets called instead)
        assert(m_pendingBuffer);
        const bool pendingBufferIsSharedMemoryBuffer = m_pendingBuffer->isSharedMemoryBuffer();
        logSurfaceAttach(ELogLevel::Trace, "", m_pendingBuffer, x, y);
        // if no buffer was attached (e.g. first buffer ever or after detach)
        if(!m_buffer)
        {
            m_bufferTypeChanged = true;
            logSurfaceAttach(ELogLevel::Info, "[new]", m_pendingBuffer, x, y);
        }
        // if new buffer (pending buffer) has different type from current buffer
        else if(pendingBufferIsSharedMemoryBuffer != m_buffer->isSharedMemoryBuffer())
        {
            m_bufferTypeChanged = true;
            logSurfaceAttach(ELogLevel::Info, "[typeChange]", m_pendingBuffer, x, y);
        }
    }

    void WaylandSurface::surfaceDetach([[maybe_unused]] IWaylandClient& client)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDetach");

        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::surfaceAttach: remove current pending buffer from surface with title "
                      << getSurfaceTitle() << ", " << getIviSurfaceId());

        m_pendingBuffer            = nullptr;
        m_removeBufferOnNextCommit = true;
    }

    void
    WaylandSurface::surfaceDamage([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] int x, [[maybe_unused]] int y, [[maybe_unused]] int width, [[maybe_unused]] int height)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDamage");
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

    void WaylandSurface::surfaceSetOpaqueRegion([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] INativeWaylandResource* regionResource)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetOpaqueRegion");
    }

    void WaylandSurface::surfaceSetInputRegion([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] INativeWaylandResource* regionResource)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetInputRegion");
    }

    void WaylandSurface::surfaceCommit([[maybe_unused]] IWaylandClient& client)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceCommit");

        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::surfaceCommit: handling commit message for surface " << getIviSurfaceId());

        // Transfers pending callbacks to list of frame_callbacks.
        for(const auto& callback : m_pendingCallbacks)
            m_frameCallbacks.push_back(callback);

        m_pendingCallbacks.clear();

        // If an attach is pending, current buffer is updated with pending one.
        if (m_pendingBuffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::surfaceCommit: new texture data for surface " << getIviSurfaceId());
            setBufferToSurface(*m_pendingBuffer);
            m_pendingBuffer = nullptr;
        }
        else
        {
            if (m_removeBufferOnNextCommit)
            {
                LOG_TRACE(
                    CONTEXT_RENDERER,
                    "WaylandSurface::surfaceCommit: remove buffer from surface " << getIviSurfaceId()
                        << " because triggered by earlier empty attachsurface");
                unsetBufferFromSurface();
            }
        }
        m_removeBufferOnNextCommit = false;
        m_numberOfCommitedFrames++;
        m_numberOfCommitedFramesSinceBeginningOfTime++;
    }

    void WaylandSurface::surfaceSetBufferTransform([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] int32_t transform)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetBufferTransform");
    }

    void WaylandSurface::surfaceSetBufferScale([[maybe_unused]] IWaylandClient& client, [[maybe_unused]] int32_t scale)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceSetBufferScale");
    }

    void WaylandSurface::surfaceDamageBuffer(
        [[maybe_unused]] IWaylandClient& client, [[maybe_unused]] int32_t x, [[maybe_unused]] int32_t y, [[maybe_unused]] int32_t width, [[maybe_unused]] int32_t height)
    {
        LOG_TRACE(CONTEXT_RENDERER, "WaylandSurface::surfaceDamageBuffer");
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

    void WaylandSurface::SurfaceDestroyCallback([[maybe_unused]] wl_client* client, wl_resource* surfaceResource)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        delete surface;
    }

    void WaylandSurface::SurfaceAttachCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* bufferResource, [[maybe_unused]] int x, [[maybe_unused]] int y)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        if (nullptr != bufferResource)
        {
            WaylandBufferResource waylandBufferResource(bufferResource);
            surface->surfaceAttach(waylandClient, waylandBufferResource, x, y);
        }
        else
        {
            surface->surfaceDetach(waylandClient);
        }
    }

    void WaylandSurface::SurfaceDamageCallback(wl_client* client, wl_resource* surfaceResource, int x, int y, int width, int height)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceDamage(waylandClient, x, y, width, height);
    }

    void WaylandSurface::SurfaceFrameCallback(wl_client* client, wl_resource* surfaceResource, uint32_t id)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceFrame(waylandClient, id);
    }

    void WaylandSurface::SurfaceSetOpaqueRegionCallback(wl_client* client, wl_resource* surfaceResource, wl_resource* regionResource)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
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
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
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
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceCommit(waylandClient);
    }

    void WaylandSurface::SurfaceSetBufferTransformCallback(wl_client* client, wl_resource* surfaceResource, int32_t transform)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceSetBufferTransform(waylandClient, transform);
    }

    void WaylandSurface::SurfaceSetBufferScaleCallback(wl_client* client, wl_resource* surfaceResource, int32_t scale)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceSetBufferScale(waylandClient, scale);
    }

    void WaylandSurface::SurfaceDamageBufferCallback(wl_client *client, wl_resource* surfaceResource, int32_t x, int32_t y, int32_t width, int32_t height)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
        WaylandClient waylandClient(client);
        surface->surfaceDamageBuffer(waylandClient, x, y, width, height);
    }

    void WaylandSurface::ResourceDestroyedCallback(wl_resource* surfaceResource)
    {
        auto* surface = static_cast<WaylandSurface*>(wl_resource_get_user_data(surfaceResource));
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
                      "WaylandSurface::setBufferToSurface client provides content for surface " << getIviSurfaceId());
        }

        setWaylandBuffer(&buffer);
    }

    void WaylandSurface::unsetBufferFromSurface()
    {
        LOG_TRACE(CONTEXT_RENDERER,
                  "WaylandSurface::unsetBufferFromSurface: removing buffer used for surface " << getIviSurfaceId());

        if (m_buffer)
        {
            m_buffer->release();
        }

        setWaylandBuffer(nullptr);
    }

    void WaylandSurface::sendFrameCallbacks(uint32_t time)
    {
        for (auto callback: m_frameCallbacks)
        {
            callback->callbackSendDone(time);
            callback->destroy();
            delete callback;
        }

        m_frameCallbacks.clear();
    }

    void WaylandSurface::logInfos(RendererLogContext& context, const WaylandEGLExtensionProcs &eglExt) const
    {
        context << getIviSurfaceId() << "; title: \"" << getSurfaceTitle()
                << "\"; client" << getClientCredentials()
                << "; commitedFrames: " << m_numberOfCommitedFramesSinceBeginningOfTime
                << RendererLogContext::NewLine;
        if (m_buffer != nullptr)
        {
            context.indent();
            m_buffer->logInfos(context, eglExt);
            context.unindent();
        }

    }

    void WaylandSurface::bufferDestroyed(IWaylandBuffer& buffer)
    {
        if (m_buffer == &buffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::bufferDestroyed(): destroying buffer for surface: " << getIviSurfaceId());

            unsetBufferFromSurface();
        }

        if (m_pendingBuffer == &buffer)
        {
            LOG_TRACE(CONTEXT_RENDERER,
                      "WaylandSurface::bufferDestroyed(): destroying pending buffer for surface: " << getIviSurfaceId());
            m_pendingBuffer = nullptr;
        }
    }
}
