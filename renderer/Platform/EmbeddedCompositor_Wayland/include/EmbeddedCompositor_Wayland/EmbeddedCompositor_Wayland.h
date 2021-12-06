//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_EMBEDDEDCOMPOSITOR_WAYLAND_H
#define RAMSES_EMBEDDEDCOMPOSITOR_WAYLAND_H

#include "EmbeddedCompositor_Wayland/WaylandCompositorGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandShellGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandOutputGlobal.h"
#include "EmbeddedCompositor_Wayland/WaylandDisplay.h"
#include "EmbeddedCompositor_Wayland/WaylandIVIApplicationGlobal.h"
#include "EmbeddedCompositor_Wayland/IEmbeddedCompositor_Wayland.h"
#include "EmbeddedCompositor_Wayland/INativeWaylandResource.h"
#include "EmbeddedCompositor_Wayland/LinuxDmabufGlobal.h"
#include "RendererAPI/IEmbeddedCompositor.h"
#include "Collections/HashMap.h"

namespace ramses_internal
{
    class DisplayConfig;
    class IContext;
    class IWaylandCompositorConnection;
    class IWaylandSurface;
    class IWaylandBuffer;
    class WaylandBufferResource;
    class IWaylandRegion;

    class EmbeddedCompositor_Wayland: public IEmbeddedCompositor, public IEmbeddedCompositor_Wayland
    {
    public:
        EmbeddedCompositor_Wayland(const DisplayConfig& displayConfig, IContext& context);
        virtual ~EmbeddedCompositor_Wayland() override;

        Bool init();
        wl_display* getEmbeddedCompositingDisplay() const;

        virtual void handleRequestsFromClients() override;
        virtual Bool hasUpdatedStreamTextureSources() const override;
        virtual WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        virtual WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        virtual WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        virtual void endFrame(Bool notifyClients) override;
        virtual UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        virtual Bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;

        virtual UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual Bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        virtual UInt32 getNumberOfCompositorConnections() const override;
        virtual Bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        const IWaylandSurface& findSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const;
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

        const String                m_waylandEmbeddedSocketName;
        const String                m_waylandEmbeddedSocketGroup;
        const uint32_t              m_waylandEmbeddedSocketPermissions;
        const int                   m_waylandEmbeddedSocketFD;
        IContext&                   m_context;

        WaylandDisplay              m_serverDisplay;
        WaylandCompositorGlobal     m_compositorGlobal;
        WaylandShellGlobal          m_shellGlobal;
        WaylandOutputGlobal         m_waylandOutputGlobal;
        WaylandIVIApplicationGlobal m_iviApplicationGlobal;
        LinuxDmabufGlobal           m_linuxDmabufGlobal;

        using WaylandSurfaces = std::vector<IWaylandSurface *>;
        WaylandSurfaces m_surfaces;
        WaylandIviSurfaceIdSet m_knownStreamTextureSoruceIds;
        WaylandIviSurfaceIdSet m_updatedStreamTextureSourceIds;
        WaylandIviSurfaceIdSet m_newStreamTextureSourceIds;
        WaylandIviSurfaceIdSet m_obsoleteStreamTextureSourceIds;

        using WaylandBuffers = HashSet<IWaylandBuffer *>;
        WaylandBuffers m_waylandBuffers;

        using WaylandCompositorConnections = HashSet<IWaylandCompositorConnection *>;
        WaylandCompositorConnections m_compositorConnections;

        using WaylandRegions = HashSet<IWaylandRegion *>;
        WaylandRegions m_regions;
    };
}

#endif
