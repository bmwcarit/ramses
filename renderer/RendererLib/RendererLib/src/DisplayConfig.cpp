//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayConfig.h"

namespace ramses_internal
{
    void DisplayConfig::setAntialiasingSampleCount(UInt32 samples)
    {
        assert(ramses_internal::contains_c<uint32_t>({ 1u, 2u, 4u, 8u }, samples));
        m_antiAliasingSamples = samples;
    }

    WaylandIviLayerId DisplayConfig::getWaylandIviLayerID() const
    {
        return m_waylandIviLayerID;
    }

    void DisplayConfig::setWaylandIviLayerID(WaylandIviLayerId waylandIviLayerID)
    {
        m_waylandIviLayerID = waylandIviLayerID;
    }

    WaylandIviSurfaceId DisplayConfig::getWaylandIviSurfaceID() const
    {
        return m_waylandIviSurfaceID;
    }

    void DisplayConfig::setWaylandIviSurfaceID(WaylandIviSurfaceId waylandIviSurfaceID)
    {
        m_waylandIviSurfaceID = waylandIviSurfaceID;
    }

    IntegrityRGLDeviceUnit DisplayConfig::getIntegrityRGLDeviceUnit() const
    {
        return m_integrityRGLDeviceUnit;
    }

    void DisplayConfig::setIntegrityRGLDeviceUnit(IntegrityRGLDeviceUnit rglDeviceUnit)
    {
        m_integrityRGLDeviceUnit = rglDeviceUnit;
    }

    AndroidNativeWindowPtr DisplayConfig::getAndroidNativeWindow() const
    {
        return m_androidNativeWindowPtr;
    }

    void DisplayConfig::setAndroidNativeWindow(AndroidNativeWindowPtr nativeWindowPtr)
    {
        m_androidNativeWindowPtr = nativeWindowPtr;
    }

    Bool DisplayConfig::getStartVisibleIvi() const
    {
        return m_startVisibleIvi;
    }

    void DisplayConfig::setStartVisibleIvi(bool startVisible)
    {
        m_startVisibleIvi = startVisible;
    }

    Bool DisplayConfig::getFullscreenState() const
    {
        return m_fullscreen;
    }

    void DisplayConfig::setFullscreenState(Bool state)
    {
        m_fullscreen = state;
    }

    Bool DisplayConfig::getBorderlessState() const
    {
        return m_borderless;
    }

    void DisplayConfig::setBorderlessState(Bool state)
    {
        m_borderless = state;
    }

    UInt32 DisplayConfig::getAntialiasingSampleCount() const
    {
        return m_antiAliasingSamples;
    }

    UInt32 DisplayConfig::getDesiredWindowWidth() const
    {
        return m_desiredWindowWidth;
    }

    void DisplayConfig::setDesiredWindowWidth(UInt32 width)
    {
        assert(width != 0u);
        m_desiredWindowWidth = width;
    }

    UInt32 DisplayConfig::getDesiredWindowHeight() const
    {
        return m_desiredWindowHeight;
    }

    void DisplayConfig::setDesiredWindowHeight(UInt32 height)
    {
        assert(height != 0u);
        m_desiredWindowHeight = height;
    }

    Int32 DisplayConfig::getWindowPositionX() const
    {
        return m_windowPositionX;
    }

    void DisplayConfig::setWindowPositionX(Int32 posx)
    {
        m_windowPositionX = posx;
    }

    Int32 DisplayConfig::getWindowPositionY() const
    {
        return m_windowPositionY;
    }

    void DisplayConfig::setWindowPositionY(Int32 posy)
    {
        m_windowPositionY = posy;
    }

    void DisplayConfig::setWarpingEnabled(Bool enabled)
    {
        m_warpingEnabled = enabled;
    }

    Bool DisplayConfig::isWarpingEnabled() const
    {
        return m_warpingEnabled;
    }

    void DisplayConfig::setKeepEffectsUploaded(Bool enabled)
    {
        m_keepEffectsUploaded = enabled;
    }

    Bool DisplayConfig::getKeepEffectsUploaded() const
    {
        return m_keepEffectsUploaded;
    }

    Bool DisplayConfig::isResizable() const
    {
        return m_resizable;
    }

    void DisplayConfig::setResizable(Bool resizable)
    {
        m_resizable = resizable;
    }

    UInt64 DisplayConfig::getGPUMemoryCacheSize() const
    {
        return m_gpuMemoryCacheSize;
    }

    void DisplayConfig::setGPUMemoryCacheSize(UInt64 size)
    {
        m_gpuMemoryCacheSize = size;
    }

    void DisplayConfig::setClearColor(const Vector4& clearColor)
    {
        m_clearColor = clearColor;
    }

    const Vector4& DisplayConfig::getClearColor() const
    {
        return m_clearColor;
    }

    void DisplayConfig::setDepthStencilBufferType(ERenderBufferType depthStencilBufferType)
    {
        m_depthStencilBufferType = depthStencilBufferType;
    }

    ERenderBufferType DisplayConfig::getDepthStencilBufferType() const
    {
        return m_depthStencilBufferType;
    }

    void DisplayConfig::setWaylandDisplay(const String& waylandDisplay)
    {
        m_waylandDisplay = waylandDisplay;
    }

    const String& DisplayConfig::getWaylandDisplay() const
    {
        return m_waylandDisplay;
    }

    void DisplayConfig::setWindowsWindowHandle(WindowsWindowHandle hwnd)
    {
        m_windowsWindowHandle = hwnd;
    }

    WindowsWindowHandle DisplayConfig::getWindowsWindowHandle() const
    {
        return m_windowsWindowHandle;
    }

    void DisplayConfig::setX11WindowHandle(X11WindowHandle windowHandle)
    {
        m_x11WindowHandle = windowHandle;
    }

    X11WindowHandle DisplayConfig::getX11WindowHandle() const
    {
        return m_x11WindowHandle;
    }

    void DisplayConfig::setAsyncEffectUploadEnabled(bool enabled)
    {
        m_asyncEffectUploadEnabled = enabled;
    }

    bool DisplayConfig::isAsyncEffectUploadEnabled() const
    {
        return m_asyncEffectUploadEnabled;
    }
    void DisplayConfig::setWaylandEmbeddedCompositingSocketName(const String& socket)
    {
        m_waylandSocketEmbedded = socket;
    }

    void DisplayConfig::setWaylandEmbeddedCompositingSocketFD(int fd)
    {
        m_waylandSocketEmbeddedFD = fd;
    }

    const String& DisplayConfig::getWaylandSocketEmbedded() const
    {
        return m_waylandSocketEmbedded;
    }

    const String& DisplayConfig::getWaylandSocketEmbeddedGroup() const
    {
        return m_waylandSocketEmbeddedGroupName;
    }

    int DisplayConfig::getWaylandSocketEmbeddedFD() const
    {
        return m_waylandSocketEmbeddedFD;
    }

    void DisplayConfig::setWaylandEmbeddedCompositingSocketGroup(const String& groupNameForSocketPermissions)
    {
        m_waylandSocketEmbeddedGroupName = groupNameForSocketPermissions;
    }

    bool DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        if (permissions == 0)
            return false;
        m_waylandSocketEmbeddedPermissions = permissions;
        return true;
    }

    uint32_t DisplayConfig::getWaylandSocketEmbeddedPermissions() const
    {
        return m_waylandSocketEmbeddedPermissions;
    }

    void DisplayConfig::setPlatformRenderNode(const String& renderNode)
    {
        m_platformRenderNode = renderNode;
    }

    const String& DisplayConfig::getPlatformRenderNode() const
    {
        return m_platformRenderNode;
    }

    void DisplayConfig::setSwapInterval(int32_t interval)
    {
        m_swapInterval = interval;
    }

    int32_t DisplayConfig::getSwapInterval() const
    {
        return m_swapInterval;
    }

    void DisplayConfig::setScenePriority(SceneId sceneId, int32_t priority)
    {
        m_scenePriorities[sceneId] = priority;
    }

    int32_t DisplayConfig::getScenePriority(SceneId sceneId) const
    {
        const auto it = m_scenePriorities.find(sceneId);
        if (it != m_scenePriorities.end())
        {
            return it->second;
        }
        return 0;
    }

    const std::unordered_map<SceneId, int32_t>& DisplayConfig::getScenePriorities() const
    {
        return m_scenePriorities;
    }

    void DisplayConfig::setResourceUploadBatchSize(uint32_t batchSize)
    {
        m_resourceUploadBatchSize = batchSize;
    }

    uint32_t DisplayConfig::getResourceUploadBatchSize() const
    {
        return m_resourceUploadBatchSize;
    }

    Bool DisplayConfig::operator == (const DisplayConfig& other) const
    {
        return
            m_fullscreen                 == other.m_fullscreen &&
            m_borderless                 == other.m_borderless &&
            m_warpingEnabled             == other.m_warpingEnabled &&
            m_antiAliasingSamples        == other.m_antiAliasingSamples &&
            m_desiredWindowWidth         == other.m_desiredWindowWidth &&
            m_desiredWindowHeight        == other.m_desiredWindowHeight &&
            m_windowPositionX            == other.m_windowPositionX &&
            m_windowPositionY            == other.m_windowPositionY &&
            m_waylandIviLayerID          == other.m_waylandIviLayerID &&
            m_waylandIviSurfaceID        == other.m_waylandIviSurfaceID &&
            m_integrityRGLDeviceUnit     == other.m_integrityRGLDeviceUnit &&
            m_startVisibleIvi            == other.m_startVisibleIvi &&
            m_resizable                  == other.m_resizable &&
            m_gpuMemoryCacheSize         == other.m_gpuMemoryCacheSize &&
            m_clearColor                 == other.m_clearColor &&
            m_windowsWindowHandle        == other.m_windowsWindowHandle &&
            m_waylandDisplay             == other.m_waylandDisplay &&
            m_depthStencilBufferType     == other.m_depthStencilBufferType &&
            m_asyncEffectUploadEnabled   == other.m_asyncEffectUploadEnabled &&
            m_waylandSocketEmbedded      == other.m_waylandSocketEmbedded &&
            m_waylandSocketEmbeddedGroupName    == other.m_waylandSocketEmbeddedGroupName &&
            m_waylandSocketEmbeddedPermissions  == other.m_waylandSocketEmbeddedPermissions &&
            m_waylandSocketEmbeddedFD    == other.m_waylandSocketEmbeddedFD &&
            m_platformRenderNode         == other.m_platformRenderNode &&
            m_swapInterval               == other.m_swapInterval &&
            m_scenePriorities            == other.m_scenePriorities &&
            m_resourceUploadBatchSize    == other.m_resourceUploadBatchSize;
    }

    Bool DisplayConfig::operator != (const DisplayConfig& other) const
    {
        return !operator==(other);
    }
}
