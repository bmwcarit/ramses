//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "EmbeddedCompositor_Wayland/EmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandCompositorConnection.h"
#include "EmbeddedCompositor_Wayland/WaylandSurface.h"
#include "EmbeddedCompositor_Wayland/WaylandBuffer.h"
#include "EmbeddedCompositor_Wayland/WaylandBufferResource.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufGlobal.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufBuffer.h"
#include "EmbeddedCompositor_Wayland/TextureUploadingAdapter_Wayland.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabuf.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputParams.h"
#include "RendererLib/DisplayConfig.h"
#include "RendererLib/RendererLogContext.h"
#include "Utils/ThreadLocalLogForced.h"
#include "Utils/Warnings.h"
#include "PlatformAbstraction/PlatformTime.h"
#include <algorithm>
#include <unistd.h>

namespace ramses_internal
{
    EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(const DisplayConfig& displayConfig, IContext& context)
        : m_waylandEmbeddedSocketName(displayConfig.getWaylandSocketEmbedded())
        , m_waylandEmbeddedSocketGroup(displayConfig.getWaylandSocketEmbeddedGroup())
        , m_waylandEmbeddedSocketPermissions(displayConfig.getWaylandSocketEmbeddedPermissions())
        , m_waylandEmbeddedSocketFD(displayConfig.getWaylandSocketEmbeddedFD())
        , m_context(context)
        , m_compositorGlobal(*this)
        , m_waylandOutputGlobal({ displayConfig.getDesiredWindowWidth(), displayConfig.getDesiredWindowHeight() })
        , m_iviApplicationGlobal(*this)
        , m_linuxDmabufGlobal(*this)
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::EmbeddedCompositor_Wayland(): Created EmbeddedCompositor_Wayland...(not initialized yet)");
    }

    EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland()
    {
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::~EmbeddedCompositor_Wayland(): Destroying EmbeddedCompositor_Wayland");

        m_compositorGlobal.destroy();
        m_shellGlobal.destroy();
        m_waylandOutputGlobal.destroy();
        m_iviApplicationGlobal.destroy();
        m_linuxDmabufGlobal.destroy();
    }

    bool EmbeddedCompositor_Wayland::init()
    {
        if (!m_serverDisplay.init(m_waylandEmbeddedSocketName, m_waylandEmbeddedSocketGroup, m_waylandEmbeddedSocketPermissions, m_waylandEmbeddedSocketFD))
        {
            return false;
        }

        if (!m_compositorGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if (!m_shellGlobal.init(m_serverDisplay))
        {
            return false;
        }

        if(!m_waylandOutputGlobal.init(m_serverDisplay))
            return false;

        if (!m_iviApplicationGlobal.init(m_serverDisplay))
        {
            return false;
        }

        // Not all EGL implementations support the extensions necessary for dmabuf import
        if (!m_linuxDmabufGlobal.init(m_serverDisplay, m_context))
        {
            LOG_WARN(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): EGL_EXT_image_dma_buf_import not supported, skipping zwp_linux_dmabuf_v1.");
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::init(): Embedded compositor created successfully!");

        return true;
    }

    wl_display* EmbeddedCompositor_Wayland::getEmbeddedCompositingDisplay() const
    {
        return m_serverDisplay.get();
    }

    void EmbeddedCompositor_Wayland::handleRequestsFromClients()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleRequestsFromClients(): handling pending events and requests from clients");

        m_serverDisplay.dispatchEventLoop();
    }

    bool EmbeddedCompositor_Wayland::hasUpdatedStreamTextureSources() const
    {
        return 0u != m_updatedStreamTextureSourceIds.size();
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds()
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::dispatchUpdatedStreamTextureSourceIds(): count of pending updates for dispatching :" << m_updatedStreamTextureSourceIds.size());
        WaylandIviSurfaceIdSet result = m_updatedStreamTextureSourceIds;
        m_updatedStreamTextureSourceIds.clear();
        return result;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchNewStreamTextureSourceIds()
    {
        auto result = m_newStreamTextureSourceIds;
        m_newStreamTextureSourceIds.clear();
        return result;
    }

    WaylandIviSurfaceIdSet EmbeddedCompositor_Wayland::dispatchObsoleteStreamTextureSourceIds()
    {
        auto result = m_obsoleteStreamTextureSourceIds;
        m_obsoleteStreamTextureSourceIds.clear();
        return result;
    }

    void EmbeddedCompositor_Wayland::logInfos(RendererLogContext& context) const
    {
        context << m_surfaces.size() << " connected wayland client(s)" << RendererLogContext::NewLine;
        context.indent();
        for (auto surface: m_surfaces)
        {
            surface->logInfos(context);
        }
        context.unindent();
    }

    void EmbeddedCompositor_Wayland::addWaylandSurface(IWaylandSurface& waylandSurface)
    {
        m_surfaces.push_back(&waylandSurface);
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addWaylandSurface: Client created surface. Count surfaces :" << m_surfaces.size());
    }

    void EmbeddedCompositor_Wayland::removeWaylandSurface(IWaylandSurface& waylandSurface)
    {
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface destroyed");
        // It's safe to call remove even if surface has not been mapped and
        // therefore not been added into any list, since link got initialized at
        // construction in compositor_create_surface().
        for(auto surface = m_surfaces.begin(); surface != m_surfaces.end(); ++surface)
        {
            if(*surface == &waylandSurface)
            {
                m_surfaces.erase(surface);
                break;
            }
        }

        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::removeWaylandSurface() Client destroyed surface, showing fallback texture for ivi surface " << waylandSurface.getIviSurfaceId()
                 << ". Count surfaces :" << m_surfaces.size());
    }

    void EmbeddedCompositor_Wayland::addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        m_compositorConnections.put(&waylandCompositorConnection);
        LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addWaylandCompositorConnection: embedded-compositing connection created. Count connections :" << m_compositorConnections.size());
    }

    IWaylandSurface* EmbeddedCompositor_Wayland::findWaylandSurfaceByIviSurfaceId(WaylandIviSurfaceId iviSurfaceId) const
    {
        for (auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == iviSurfaceId)
            {
                return surface;
            }
        }
        return nullptr;
    }

    void EmbeddedCompositor_Wayland::endFrame(bool notifyClients)
    {
        if (notifyClients)
        {
            LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): will send surface frame callbacks to clients");
            const UInt32 time = static_cast<UInt32>(PlatformTime::GetMillisecondsAbsolute());

            for (auto surface: m_surfaces)
            {
                surface->sendFrameCallbacks(time);
                surface->resetNumberOfCommitedFrames();
            }
        }

        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::endFrame(): flusing clients");

        m_serverDisplay.flushClients();
    }

    UInt32 EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        assert(streamTextureSourceId.isValid());
        IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        assert(nullptr != waylandClientSurface);

        LOG_DEBUG(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::uploadCompositingContentForStreamTexture(): Stream texture with source Id " << streamTextureSourceId);
        LOG_INFO(CONTEXT_SMOKETEST, "embedded-compositing client surface found for existing streamtexture: " << streamTextureSourceId);

        uploadCompositingContentForWaylandSurface(waylandClientSurface, textureHandle, textureUploadingAdapter);
        return waylandClientSurface->getNumberOfCommitedFrames();
    }

    void EmbeddedCompositor_Wayland::uploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter)
    {
        IWaylandBuffer* waylandBuffer = waylandSurface->getWaylandBuffer();
        assert(nullptr != waylandBuffer);

        WaylandBufferResource& waylandBufferResource = waylandBuffer->getResource();

        const UInt8* sharedMemoryBufferData = static_cast<const UInt8*>(waylandBufferResource.bufferGetSharedMemoryData());
        LinuxDmabufBufferData* linuxDmabufBuffer = LinuxDmabufBuffer::fromWaylandBufferResource(waylandBufferResource);

        const bool surfaceBufferTypeChanged = waylandSurface->dispatchBufferTypeChanged();

        if(surfaceBufferTypeChanged)
        {
            //reset swizzle
            const TextureSwizzleArray swizzle = {ETextureChannelColor::Red, ETextureChannelColor::Green, ETextureChannelColor::Blue, ETextureChannelColor::Alpha};
            uint8_t dummyData { 0u };
            textureUploadingAdapter.uploadTexture2D(textureHandle, 1u, 1u, ETextureFormat::R8, &dummyData, swizzle);

            LOG_INFO(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::uploadCompositingContentForWaylandSurface(): resetting swizzle for stream source id :" << waylandSurface->getIviSurfaceId());
        }

        if (nullptr != sharedMemoryBufferData)
        {
            const TextureSwizzleArray swizzle = {ETextureChannelColor::Blue, ETextureChannelColor::Green, ETextureChannelColor::Red, ETextureChannelColor::Alpha};
            textureUploadingAdapter.uploadTexture2D(textureHandle, waylandBufferResource.bufferGetSharedMemoryWidth(), waylandBufferResource.bufferGetSharedMemoryHeight(), ETextureFormat::RGBA8, sharedMemoryBufferData, swizzle);
        }
        else if (nullptr != linuxDmabufBuffer)
        {
            const auto success = static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromLinuxDmabuf(textureHandle, linuxDmabufBuffer);

            if(!success)
            {
                StringOutputStream message;
                message << "Failed creating EGL image from dma buffer for stream source id: " << waylandSurface->getIviSurfaceId();

                waylandBufferResource.postError(ZWP_LINUX_BUFFER_PARAMS_V1_ERROR_INVALID_WL_BUFFER, String(message.release()));
            }
        }
        else
        {
            static_cast<TextureUploadingAdapter_Wayland&>(textureUploadingAdapter).uploadTextureFromWaylandResource(textureHandle, waylandBufferResource.getLowLevelHandle());
        }
    }

    bool EmbeddedCompositor_Wayland::isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(streamTextureSourceId);
        if(waylandClientSurface)
        {
            return nullptr != waylandClientSurface->getWaylandBuffer();
        }

        return false;
    }

    UInt64 EmbeddedCompositor_Wayland::getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getNumberOfCommitedFramesSinceBeginningOfTime();
        }
        else
        {
            return 0;
        }
    }

    bool EmbeddedCompositor_Wayland::isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->hasPendingBuffer() || waylandClientSurface->getWaylandBuffer() != nullptr;
        }
        else
        {
            return false;
        }
    }

    UInt32 EmbeddedCompositor_Wayland::getNumberOfCompositorConnections() const
    {
        return m_compositorConnections.size();
    }

    bool EmbeddedCompositor_Wayland::hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        for (const auto surface: m_surfaces)
        {
            if (surface->getIviSurfaceId() == streamTextureSourceId)
            {
                return true;
            }
        }

        return false;
    }

    const IWaylandSurface& EmbeddedCompositor_Wayland::findSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const
    {
        const auto it = std::find_if(std::cbegin(m_surfaces), std::cend(m_surfaces), [&](const auto surface){ return surface->getIviSurfaceId() == streamTextureSourceId;});
        return **it;
    }

    String EmbeddedCompositor_Wayland::getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const
    {
        const IWaylandSurface* waylandClientSurface = findWaylandSurfaceByIviSurfaceId(waylandSurfaceId);
        if (waylandClientSurface)
        {
            return waylandClientSurface->getSurfaceTitle();
        }
        else
        {
            return String();
        }
    }

    IWaylandBuffer* EmbeddedCompositor_Wayland::findWaylandBuffer(WaylandBufferResource& bufferResource)
    {
        for (auto i: m_waylandBuffers)
        {
            if (i->getResource().getLowLevelHandle() == bufferResource.getLowLevelHandle())
            {
                return i;
            }
        }
        return nullptr;
    }

    IWaylandBuffer& EmbeddedCompositor_Wayland::getOrCreateBuffer(WaylandBufferResource& bufferResource)
    {
        IWaylandBuffer* buffer = findWaylandBuffer(bufferResource);
        if (nullptr == buffer)
        {
            buffer = new WaylandBuffer(bufferResource, *this);
            m_waylandBuffers.put(buffer);
        }
        return *buffer;
    }

    bool EmbeddedCompositor_Wayland::isRealCompositor() const
    {
        return true;
    }

    void EmbeddedCompositor_Wayland::handleBufferDestroyed(IWaylandBuffer& buffer)
    {
        if (!m_waylandBuffers.remove(&buffer))
        {
            LOG_ERROR(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::handleBufferDestroyed m_waylandBuffers.remove failed");
            assert(false);
        }

        for (auto surface: m_surfaces)
        {
            surface->bufferDestroyed(buffer);
        }
    }

    void EmbeddedCompositor_Wayland::removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection)
    {
        const bool removed = m_compositorConnections.remove(&waylandCompositorConnection);
        assert(removed);

        LOG_INFO(CONTEXT_SMOKETEST, "EmbeddedCompositor_Wayland::removeWaylandCompositorConnection: embedded-compositing connection removed. Count connections :"
                 << m_compositorConnections.size() << ", was removed " << removed);
    }

    void EmbeddedCompositor_Wayland::removeFromUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        if (m_newStreamTextureSourceIds.count(id))
        {
            m_newStreamTextureSourceIds.erase(id);
        }
        else if (m_knownStreamTextureSoruceIds.count(id))
        {
            m_obsoleteStreamTextureSourceIds.insert(id);
        }

        m_updatedStreamTextureSourceIds.erase(id);
        m_knownStreamTextureSoruceIds.erase(id);
    }

    void EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id)
    {
        LOG_TRACE(CONTEXT_RENDERER, "EmbeddedCompositor_Wayland::addToUpdatedStreamTextureSourceIds: new texture data for stream texture with source id " << id);
        m_updatedStreamTextureSourceIds.insert(id);

        if(!m_knownStreamTextureSoruceIds.count(id))
        {
            m_newStreamTextureSourceIds.insert(id);
            m_knownStreamTextureSoruceIds.insert(id);
        }
    }

    void EmbeddedCompositor_Wayland::addWaylandRegion(IWaylandRegion& waylandRegion)
    {
        m_regions.put(&waylandRegion);
    }

    void EmbeddedCompositor_Wayland::removeWaylandRegion(IWaylandRegion& waylandRegion)
    {
        const bool removed = m_regions.remove(&waylandRegion);
        UNUSED(removed);
        assert(removed);
    }
}
