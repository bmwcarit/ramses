//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITOR_WAYLAND_H
#define RAMSES_EMBEDDEDCOMPOSITOR_WAYLAND_H

#include "RendererAPI/IEmbeddedCompositor.h"
#include "EmbeddedCompositor_Wayland/WaylandCompositorGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandShellGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandIVIApplicationGlobal.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/IWaylandResource.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class RendererConfig;
    class IWaylandCompositorConnection;
    class IWaylandSurface;
    class IWaylandBuffer;
    class WaylandBufferResource;
    class IWaylandRegion;

    class EmbeddedCompositor_Wayland: public IEmbeddedCompositor, public IEmbeddedCompositor_Wayland
    {
    public:
        EmbeddedCompositor_Wayland(const RendererConfig& config);
        virtual ~EmbeddedCompositor_Wayland();

        Bool init();
        wl_display* getEmbeddedCompositingDisplay() const;

        virtual void handleRequestsFromClients() override;
        virtual Bool hasUpdatedStreamTextureSources() const override;
        virtual StreamTextureSourceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        virtual StreamTextureSourceIdSet dispatchNewStreamTextureSourceIds() override;
        virtual StreamTextureSourceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        virtual void endFrame(Bool notifyClients) override;
        virtual UInt32 uploadCompositingContentForStreamTexture(StreamTextureSourceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        virtual Bool isContentAvailableForStreamTexture(StreamTextureSourceId streamTextureSourceId) const override;

        virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual UInt32 getNumberOfCompositorConnections() const override;
        virtual Bool hasSurfaceForStreamTexture(StreamTextureSourceId streamTextureSourceId) const override;
        virtual String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual void logInfos(RendererLogContext& context) const override;

        virtual void addWaylandSurface(IWaylandSurface& waylandSurface) override;
        virtual void removeWaylandSurface(IWaylandSurface& waylandSurface) override;

        virtual void handleBufferDestroyed(IWaylandBuffer& buffer) override;

        virtual void addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) override;
        virtual void removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) override;

        virtual void addWaylandRegion(IWaylandRegion& waylandRegion) override;
        virtual void removeWaylandRegion(IWaylandRegion& waylandRegion) override;

        void removeFromUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id);
        void addToUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id);

        virtual IWaylandBuffer& getOrCreateBuffer(WaylandBufferResource& bufferResource) override;

        virtual Bool isRealCompositor() const override; //TODO Mohamed: remove this when dummy EC is removed

    private:
        IWaylandSurface* findWaylandSurfaceByIviSurfaceId(WaylandIviSurfaceId iviSurfaceId) const;

        void uploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter);

        Bool applyPermissionsGroupToEmbeddedCompositingSocket(const String& embeddedSocketName);

        Bool addSocketToDisplay();
        Bool addSocketToDisplayWithFD(int socketFD);
        Bool addSocketToDisplayWithName(const String& embeddedSocketName);
        IWaylandBuffer* findWaylandBuffer(WaylandBufferResource& bufferResource);

        const RendererConfig* m_rendererConfig;

        WaylandDisplay              m_serverDisplay;

        WaylandCompositorGlobal     m_compositorGlobal;
        WaylandShellGlobal          m_shellGlobal;
        WaylandIVIApplicationGlobal m_iviApplicationGlobal;

        typedef std::vector<IWaylandSurface*> WaylandSurfaces;
        WaylandSurfaces m_surfaces;
        StreamTextureSourceIdSet m_knownStreamTextureSoruceIds;
        StreamTextureSourceIdSet m_updatedStreamTextureSourceIds;
        StreamTextureSourceIdSet m_newStreamTextureSourceIds;
        StreamTextureSourceIdSet m_obsoleteStreamTextureSourceIds;

        typedef HashSet<IWaylandBuffer*> WaylandBuffers;
        WaylandBuffers m_waylandBuffers;

        typedef HashSet<IWaylandCompositorConnection*> WaylandCompositorConnections;
        WaylandCompositorConnections m_compositorConnections;

        typedef HashSet<IWaylandRegion*> WaylandRegions;
        WaylandRegions m_regions;
    };
}

#endif
