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
#include "SceneAPI/TextureEnums.h"
#include "SceneAPI/WaylandIviSurfaceId.h"
#include "Math3d/Vector4.h"

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

        AndroidNativeWindowPtr getAndroidNativeWindow() const;
        void setAndroidNativeWindow(AndroidNativeWindowPtr nativeWindowPtr);

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
        AndroidNativeWindowPtr m_androidNativeWindowPtr;
        Bool m_startVisibleIvi = false;
        String m_waylandDisplay;

        UInt32 m_antiAliasingSamples = 1;

        Bool m_keepEffectsUploaded = true;
        UInt64 m_gpuMemoryCacheSize = 0u;
        Vector4 m_clearColor{ 0.f, 0.f, 0.f, 1.0f };
        ERenderBufferType m_depthStencilBufferType = ERenderBufferType_DepthStencilBuffer;
    };
}

#endif
