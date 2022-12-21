//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONFIG_H
#define RAMSES_DISPLAYCONFIG_H

#include "RendererAPI/Types.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/WaylandIviSurfaceId.h"
#include "Math3d/Vector4.h"
#include <unordered_map>

namespace ramses_internal
{
    class DisplayConfig
    {
    public:
        DisplayConfig() {}

        Bool getFullscreenState() const;
        void setFullscreenState(Bool state);

        Bool getBorderlessState() const ;
        void setBorderlessState(Bool state);

        UInt32 getAntialiasingSampleCount() const;
        void setAntialiasingSampleCount(UInt32 samples);

        UInt32 getDesiredWindowWidth() const;
        void setDesiredWindowWidth(UInt32 width);

        UInt32 getDesiredWindowHeight() const;
        void setDesiredWindowHeight(UInt32 height);

        Int32 getWindowPositionX() const;
        void setWindowPositionX(Int32 posx);

        Int32 getWindowPositionY() const;
        void setWindowPositionY(Int32 posy);

        WaylandIviLayerId getWaylandIviLayerID() const;
        void setWaylandIviLayerID(WaylandIviLayerId waylandIviLayerID);

        WaylandIviSurfaceId getWaylandIviSurfaceID() const;
        void setWaylandIviSurfaceID(WaylandIviSurfaceId waylandIviSurfaceID);

        void setWaylandDisplay(const String& waylandDisplay);
        const String& getWaylandDisplay() const;

        IntegrityRGLDeviceUnit getIntegrityRGLDeviceUnit() const;
        void setIntegrityRGLDeviceUnit(IntegrityRGLDeviceUnit rglDeviceUnit);

        void setWindowsWindowHandle(WindowsWindowHandle hwnd);
        WindowsWindowHandle getWindowsWindowHandle() const;

        void setX11WindowHandle(X11WindowHandle x11WindowHandle);
        X11WindowHandle getX11WindowHandle() const;

        MobilePlatformNativeWindowPtr getMobilePlatformNativeWindow() const;
        void setMobilePlatformNativeWindow(MobilePlatformNativeWindowPtr nativeWindowPtr);

        Bool getStartVisibleIvi() const;
        void setStartVisibleIvi(bool startVisible);

        Bool isWarpingEnabled() const;
        void setWarpingEnabled(Bool enabled);

        Bool getKeepEffectsUploaded() const;
        void setKeepEffectsUploaded(Bool enable);

        Bool isResizable() const;
        void setResizable(Bool resizable);

        UInt64 getGPUMemoryCacheSize() const;
        void setGPUMemoryCacheSize(UInt64 size);

        void setClearColor(const Vector4& clearColor);
        const Vector4& getClearColor() const;

        void setDepthStencilBufferType(ERenderBufferType depthStencilBufferType);
        ERenderBufferType getDepthStencilBufferType() const;

        void setAsyncEffectUploadEnabled(bool enabled);
        bool isAsyncEffectUploadEnabled() const;

        void setWaylandEmbeddedCompositingSocketName(const String& socket);
        const String& getWaylandSocketEmbedded() const;

        void setWaylandEmbeddedCompositingSocketGroup(const String& groupNameForSocketPermissions);
        const String& getWaylandSocketEmbeddedGroup() const;

        void setWaylandEmbeddedCompositingSocketFD(int fd);
        int getWaylandSocketEmbeddedFD() const;

        bool setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        uint32_t getWaylandSocketEmbeddedPermissions() const;

        void setPlatformRenderNode(const String& renderNode);
        const String& getPlatformRenderNode() const;

        void setSwapInterval(int32_t interval);
        int32_t getSwapInterval() const;

        void setScenePriority(SceneId sceneId, int32_t priority);
        int32_t getScenePriority(SceneId sceneId) const;
        const std::unordered_map<SceneId, int32_t>& getScenePriorities() const;

        Bool operator==(const DisplayConfig& other) const;
        Bool operator!=(const DisplayConfig& other) const;

    private:
        Bool m_fullscreen = false;
        Bool m_borderless = false;
        Bool m_warpingEnabled = false;
        Bool m_resizable = false;

        UInt32 m_desiredWindowWidth = 1280;
        UInt32 m_desiredWindowHeight = 480;
        Int32 m_windowPositionX = 0;
        Int32 m_windowPositionY = 0;

        WaylandIviLayerId m_waylandIviLayerID;
        WaylandIviSurfaceId m_waylandIviSurfaceID;
        IntegrityRGLDeviceUnit m_integrityRGLDeviceUnit;
        WindowsWindowHandle m_windowsWindowHandle;
        X11WindowHandle m_x11WindowHandle;
        MobilePlatformNativeWindowPtr m_mobilePlatformNativeWindowPtr;
        Bool m_startVisibleIvi = false;
        String m_waylandDisplay;

        UInt32 m_antiAliasingSamples = 1;

        Bool m_keepEffectsUploaded = true;
        UInt64 m_gpuMemoryCacheSize = 0u;
        Vector4 m_clearColor{ 0.f, 0.f, 0.f, 1.0f };
        ERenderBufferType m_depthStencilBufferType = ERenderBufferType_DepthStencilBuffer;
        bool m_asyncEffectUploadEnabled = true;

        String m_waylandSocketEmbedded;
        String m_waylandSocketEmbeddedGroupName;
        uint32_t m_waylandSocketEmbeddedPermissions = 0;
        int m_waylandSocketEmbeddedFD = -1;
        String m_platformRenderNode;

        int32_t m_swapInterval = -1;
        std::unordered_map<SceneId, int32_t> m_scenePriorities;
    };
}

#endif
