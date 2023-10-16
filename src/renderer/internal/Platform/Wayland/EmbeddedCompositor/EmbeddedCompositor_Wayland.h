//  -------------------------------------------------------------------------
//  Copyright (C) 2016 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandCompositorGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandShellGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandOutputGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandDisplay.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/WaylandIVIApplicationGlobal.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/IEmbeddedCompositor_Wayland.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/INativeWaylandResource.h"
#include "internal/Platform/Wayland/EmbeddedCompositor/LinuxDmabufGlobal.h"
#include "internal/RendererLib/PlatformInterface/IEmbeddedCompositor.h"
#include "internal/PlatformAbstraction/Collections/HashMap.h"

#include <string>

namespace ramses::internal
{
    class DisplayConfig;
    class Context_EGL;
    class IWaylandCompositorConnection;
    class IWaylandSurface;
    class IWaylandBuffer;
    class WaylandBufferResource;
    class IWaylandRegion;

    class EmbeddedCompositor_Wayland: public IEmbeddedCompositor, public IEmbeddedCompositor_Wayland
    {
    public:
        EmbeddedCompositor_Wayland(const DisplayConfig& displayConfig, Context_EGL& context);
        ~EmbeddedCompositor_Wayland() override;

        bool init();
        [[nodiscard]] wl_display* getEmbeddedCompositingDisplay() const;

        void handleRequestsFromClients() override;
        [[nodiscard]] bool hasUpdatedStreamTextureSources() const override;
        WaylandIviSurfaceIdSet dispatchUpdatedStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchNewStreamTextureSourceIds() override;
        WaylandIviSurfaceIdSet dispatchObsoleteStreamTextureSourceIds() override;
        void endFrame(bool notifyClients) override;
        uint32_t uploadCompositingContentForStreamTexture(WaylandIviSurfaceId streamTextureSourceId, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter) override;

        [[nodiscard]] bool isContentAvailableForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;

        [[nodiscard]] uint64_t getNumberOfCommitedFramesForWaylandIviSurfaceSinceBeginningOfTime(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] bool isBufferAttachedToWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        [[nodiscard]] uint32_t getNumberOfCompositorConnections() const override;
        [[nodiscard]] bool hasSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const override;
        [[nodiscard]] const IWaylandSurface& findSurfaceForStreamTexture(WaylandIviSurfaceId streamTextureSourceId) const;
        [[nodiscard]] std::string getTitleOfWaylandIviSurface(WaylandIviSurfaceId waylandSurfaceId) const override;
        void logInfos(RendererLogContext& context) const override;
        void logPeriodicInfo(StringOutputStream& sos) const override;

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

        static void UploadCompositingContentForWaylandSurface(IWaylandSurface* waylandSurface, DeviceResourceHandle textureHandle, ITextureUploadingAdapter& textureUploadingAdapter);

        bool applyPermissionsGroupToEmbeddedCompositingSocket(const std::string& embeddedSocketName);

        bool addSocketToDisplay();
        bool addSocketToDisplayWithFD(int socketFD);
        bool addSocketToDisplayWithName(const std::string& embeddedSocketName);
        IWaylandBuffer* findWaylandBuffer(WaylandBufferResource& bufferResource);

        const std::string           m_waylandEmbeddedSocketName;
        const std::string           m_waylandEmbeddedSocketGroup;
        const uint32_t              m_waylandEmbeddedSocketPermissions;
        const int                   m_waylandEmbeddedSocketFD;
        Context_EGL&                m_context;

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
