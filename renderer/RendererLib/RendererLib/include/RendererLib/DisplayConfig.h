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

namespace CLI
{
    class App;
}

namespace ramses_internal
{
    class DisplayConfig
    {
    public:
        DisplayConfig() {}

        void registerOptions(CLI::App& cli);

        [[nodiscard]] Bool getFullscreenState() const;
        void setFullscreenState(Bool state);

        [[nodiscard]] Bool getBorderlessState() const ;
        void setBorderlessState(Bool state);

        [[nodiscard]] UInt32 getAntialiasingSampleCount() const;
        void setAntialiasingSampleCount(UInt32 samples);

        [[nodiscard]] UInt32 getDesiredWindowWidth() const;
        void setDesiredWindowWidth(UInt32 width);

        [[nodiscard]] UInt32 getDesiredWindowHeight() const;
        void setDesiredWindowHeight(UInt32 height);

        [[nodiscard]] Int32 getWindowPositionX() const;
        void setWindowPositionX(Int32 posx);

        [[nodiscard]] Int32 getWindowPositionY() const;
        void setWindowPositionY(Int32 posy);

        [[nodiscard]] WaylandIviLayerId getWaylandIviLayerID() const;
        void setWaylandIviLayerID(WaylandIviLayerId waylandIviLayerID);

        [[nodiscard]] WaylandIviSurfaceId getWaylandIviSurfaceID() const;
        void setWaylandIviSurfaceID(WaylandIviSurfaceId waylandIviSurfaceID);

        void setWaylandDisplay(const String& waylandDisplay);
        [[nodiscard]] const String& getWaylandDisplay() const;

        void setWindowsWindowHandle(WindowsWindowHandle hwnd);
        [[nodiscard]] WindowsWindowHandle getWindowsWindowHandle() const;

        void setX11WindowHandle(X11WindowHandle x11WindowHandle);
        [[nodiscard]] X11WindowHandle getX11WindowHandle() const;

        [[nodiscard]] AndroidNativeWindowPtr getAndroidNativeWindow() const;
        void setAndroidNativeWindow(AndroidNativeWindowPtr nativeWindowPtr);

        [[nodiscard]] Bool getStartVisibleIvi() const;
        void setStartVisibleIvi(bool startVisible);

        [[nodiscard]] Bool getKeepEffectsUploaded() const;
        void setKeepEffectsUploaded(Bool enable);

        [[nodiscard]] Bool isResizable() const;
        void setResizable(Bool resizable);

        [[nodiscard]] UInt64 getGPUMemoryCacheSize() const;
        void setGPUMemoryCacheSize(UInt64 size);

        void setClearColor(const Vector4& clearColor);
        [[nodiscard]] const Vector4& getClearColor() const;

        void setDepthStencilBufferType(ERenderBufferType depthStencilBufferType);
        [[nodiscard]] ERenderBufferType getDepthStencilBufferType() const;

        void setAsyncEffectUploadEnabled(bool enabled);
        [[nodiscard]] bool isAsyncEffectUploadEnabled() const;

        void setWaylandEmbeddedCompositingSocketName(const String& socket);
        [[nodiscard]] const String& getWaylandSocketEmbedded() const;

        void setWaylandEmbeddedCompositingSocketGroup(const String& groupNameForSocketPermissions);
        [[nodiscard]] const String& getWaylandSocketEmbeddedGroup() const;

        void setWaylandEmbeddedCompositingSocketFD(int fd);
        [[nodiscard]] int getWaylandSocketEmbeddedFD() const;

        bool setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        [[nodiscard]] uint32_t getWaylandSocketEmbeddedPermissions() const;

        void setPlatformRenderNode(const String& renderNode);
        [[nodiscard]] const String& getPlatformRenderNode() const;

        void setSwapInterval(int32_t interval);
        [[nodiscard]] int32_t getSwapInterval() const;

        void setScenePriority(SceneId sceneId, int32_t priority);
        [[nodiscard]] int32_t getScenePriority(SceneId sceneId) const;
        [[nodiscard]] const std::unordered_map<SceneId, int32_t>& getScenePriorities() const;

        void setResourceUploadBatchSize(uint32_t batchSize);
        [[nodiscard]] uint32_t getResourceUploadBatchSize() const;

        Bool operator==(const DisplayConfig& other) const;
        Bool operator!=(const DisplayConfig& other) const;

    private:
        Bool m_fullscreen = false;
        Bool m_borderless = false;
        Bool m_resizable = false;

        UInt32 m_desiredWindowWidth = 1280;
        UInt32 m_desiredWindowHeight = 480;
        Int32 m_windowPositionX = 0;
        Int32 m_windowPositionY = 0;

        WaylandIviLayerId m_waylandIviLayerID;
        WaylandIviSurfaceId m_waylandIviSurfaceID;
        WindowsWindowHandle m_windowsWindowHandle;
        X11WindowHandle m_x11WindowHandle;
        AndroidNativeWindowPtr m_androidNativeWindowPtr;
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
        uint32_t m_resourceUploadBatchSize = 10u;
    };
}

#endif
