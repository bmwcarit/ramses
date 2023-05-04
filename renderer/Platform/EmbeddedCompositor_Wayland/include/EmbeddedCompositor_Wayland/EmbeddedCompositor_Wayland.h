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
        ~EmbeddedCompositor_Wayland() override;

        bool init();
        [[nodiscard]] wl_display* getEmbeddedCompositingDisplay() const;

        void handleRequestsFromClients() override;
        [[nodiscard]] bool hasUpdatedStreamTextureSources() const override;
        WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        void endFrame(bool notifyClients) override;
        UInt32 uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;

        [[nodiscard]] UInt64 getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] UInt32 getNumberOfCompositorConnections() const override;
        [[nodiscard]] bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] const IWaylandSurface& findSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const;
        [[nodiscard]] String getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        void logInfos(RendererLogContext& context) const override;

        void addWaylandSurface(IWaylandSurface& waylandSurface) override;
        void removeWaylandSurface(IWaylandSurface& waylandSurface) override;

        void handleBufferDestroyed(IWaylandBuffer& buffer) override;

        void addWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) override;
        void removeWaylandCompositorConnection(IWaylandCompositorConnection& waylandCompositorConnection) override;

        void addWaylandRegion(IWaylandRegion& waylandRegion) override;
        void removeWaylandRegion(IWaylandRegion& waylandRegion) override;

        void removeFromUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id);
        void addToUpdatedStreamTextureSourceIds(WaylandIviSurfaceId id);

        IWaylandBuffer& getOrCreateBuffer(WaylandBufferResource& bufferResource) override;

        [[nodiscard]] bool isRealCompositor() const override; //TODO Mohamed: remove this when dummy EC is removed

    private:
        [[nodiscard]] IWaylandSurface* findWaylandSurfaceByIviSurfaceId(WaylandIviSurfaceId iviSurfaceId) const;

        void uploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter);

        bool applyPermissionsGroupToEmbeddedCompositingSocket(const String& embeddedSocketName);

        bool addSocketToDisplay();
        bool addSocketToDisplayWithFD(int socketFD);
        bool addSocketToDisplayWithName(const String& embeddedSocketName);
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
