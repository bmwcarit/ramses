//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "internal/RendererLib/DisplayConfigData.h"
#include <array>

namespace ramses::internal
{
    EDeviceType DisplayConfigData::getDeviceType() const
    {
        return m_deviceType;
    }

    void DisplayConfigData::setDeviceType(EDeviceType deviceType)
    {
        m_deviceType = deviceType;
    }

    EWindowType DisplayConfigData::getWindowType() const
    {
        return m_windowType;
    }

    void DisplayConfigData::setWindowType(EWindowType windowType)
    {
        m_windowType = windowType;
    }

    const std::string& DisplayConfigData::getWindowTitle() const
    {
        return m_windowTitle;
    }

    void DisplayConfigData::setWindowTitle(std::string_view title)
    {
        m_windowTitle = title;
    }

    void DisplayConfigData::setAntialiasingSampleCount(uint32_t samples)
    {
        assert(contains_c<uint32_t>({ 1u, 2u, 4u, 8u }, samples));
        m_antiAliasingSamples = samples;
    }

    WaylandIviLayerId DisplayConfigData::getWaylandIviLayerID() const
    {
        return m_waylandIviLayerID;
    }

    void DisplayConfigData::setWaylandIviLayerID(WaylandIviLayerId waylandIviLayerID)
    {
        m_waylandIviLayerID = waylandIviLayerID;
    }

    WaylandIviSurfaceId DisplayConfigData::getWaylandIviSurfaceID() const
    {
        return m_waylandIviSurfaceID;
    }

    void DisplayConfigData::setWaylandIviSurfaceID(WaylandIviSurfaceId waylandIviSurfaceID)
    {
        m_waylandIviSurfaceID = waylandIviSurfaceID;
    }

    AndroidNativeWindowPtr DisplayConfigData::getAndroidNativeWindow() const
    {
        return m_androidNativeWindowPtr;
    }

    void DisplayConfigData::setAndroidNativeWindow(AndroidNativeWindowPtr nativeWindowPtr)
    {
        m_androidNativeWindowPtr = nativeWindowPtr;
    }

    IOSNativeWindowPtr DisplayConfigData::getIOSNativeWindow() const
    {
        return m_iOSNativeWindowPtr;
    }

    void DisplayConfigData::setIOSNativeWindow(IOSNativeWindowPtr nativeWindowPtr)
    {
        m_iOSNativeWindowPtr = nativeWindowPtr;
    }

    bool DisplayConfigData::getStartVisibleIvi() const
    {
        return m_startVisibleIvi;
    }

    void DisplayConfigData::setStartVisibleIvi(bool startVisible)
    {
        m_startVisibleIvi = startVisible;
    }

    bool DisplayConfigData::getFullscreenState() const
    {
        return m_fullscreen;
    }

    void DisplayConfigData::setFullscreenState(bool state)
    {
        m_fullscreen = state;
    }

    uint32_t DisplayConfigData::getAntialiasingSampleCount() const
    {
        return m_antiAliasingSamples;
    }

    uint32_t DisplayConfigData::getDesiredWindowWidth() const
    {
        return m_desiredWindowWidth;
    }

    void DisplayConfigData::setDesiredWindowWidth(uint32_t width)
    {
        assert(width != 0u);
        m_desiredWindowWidth = width;
    }

    uint32_t DisplayConfigData::getDesiredWindowHeight() const
    {
        return m_desiredWindowHeight;
    }

    void DisplayConfigData::setDesiredWindowHeight(uint32_t height)
    {
        assert(height != 0u);
        m_desiredWindowHeight = height;
    }

    int32_t DisplayConfigData::getWindowPositionX() const
    {
        return m_windowPositionX;
    }

    void DisplayConfigData::setWindowPositionX(int32_t posx)
    {
        m_windowPositionX = posx;
    }

    int32_t DisplayConfigData::getWindowPositionY() const
    {
        return m_windowPositionY;
    }

    void DisplayConfigData::setWindowPositionY(int32_t posy)
    {
        m_windowPositionY = posy;
    }

    bool DisplayConfigData::isResizable() const
    {
        return m_resizable;
    }

    void DisplayConfigData::setResizable(bool resizable)
    {
        m_resizable = resizable;
    }

    uint64_t DisplayConfigData::getGPUMemoryCacheSize() const
    {
        return m_gpuMemoryCacheSize;
    }

    void DisplayConfigData::setGPUMemoryCacheSize(uint64_t size)
    {
        m_gpuMemoryCacheSize = size;
    }

    void DisplayConfigData::setClearColor(const glm::vec4& clearColor)
    {
        m_clearColor = clearColor;
    }

    const glm::vec4& DisplayConfigData::getClearColor() const
    {
        return m_clearColor;
    }

    void DisplayConfigData::setDepthStencilBufferType(EDepthBufferType depthStencilBufferType)
    {
        m_depthStencilBufferType = depthStencilBufferType;
    }

    EDepthBufferType DisplayConfigData::getDepthStencilBufferType() const
    {
        return m_depthStencilBufferType;
    }

    void DisplayConfigData::setWaylandDisplay(std::string_view waylandDisplay)
    {
        m_waylandDisplay.assign(waylandDisplay);
    }

    std::string_view DisplayConfigData::getWaylandDisplay() const
    {
        return m_waylandDisplay;
    }

    void DisplayConfigData::setWindowsWindowHandle(WindowsWindowHandle hwnd)
    {
        m_windowsWindowHandle = hwnd;
    }

    WindowsWindowHandle DisplayConfigData::getWindowsWindowHandle() const
    {
        return m_windowsWindowHandle;
    }

    void DisplayConfigData::setX11WindowHandle(X11WindowHandle windowHandle)
    {
        m_x11WindowHandle = windowHandle;
    }

    X11WindowHandle DisplayConfigData::getX11WindowHandle() const
    {
        return m_x11WindowHandle;
    }

    void DisplayConfigData::setAsyncEffectUploadEnabled(bool enabled)
    {
        m_asyncEffectUploadEnabled = enabled;
    }

    bool DisplayConfigData::isAsyncEffectUploadEnabled() const
    {
        return m_asyncEffectUploadEnabled;
    }
    void DisplayConfigData::setWaylandEmbeddedCompositingSocketName(std::string_view socket)
    {
        m_waylandSocketEmbedded = socket;
    }

    void DisplayConfigData::setWaylandEmbeddedCompositingSocketFD(int fd)
    {
        m_waylandSocketEmbeddedFD = fd;
    }

    std::string_view DisplayConfigData::getWaylandSocketEmbedded() const
    {
        return m_waylandSocketEmbedded;
    }

    std::string_view DisplayConfigData::getWaylandSocketEmbeddedGroup() const
    {
        return m_waylandSocketEmbeddedGroupName;
    }

    int DisplayConfigData::getWaylandSocketEmbeddedFD() const
    {
        return m_waylandSocketEmbeddedFD;
    }

    void DisplayConfigData::setWaylandEmbeddedCompositingSocketGroup(std::string_view groupNameForSocketPermissions)
    {
        m_waylandSocketEmbeddedGroupName = groupNameForSocketPermissions;
    }

    bool DisplayConfigData::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        if (permissions == 0)
            return false;
        m_waylandSocketEmbeddedPermissions = permissions;
        return true;
    }

    uint32_t DisplayConfigData::getWaylandSocketEmbeddedPermissions() const
    {
        return m_waylandSocketEmbeddedPermissions;
    }

    void DisplayConfigData::setPlatformRenderNode(std::string_view renderNode)
    {
        m_platformRenderNode = renderNode;
    }

    std::string_view DisplayConfigData::getPlatformRenderNode() const
    {
        return m_platformRenderNode;
    }

    void DisplayConfigData::setSwapInterval(int32_t interval)
    {
        m_swapInterval = interval;
    }

    int32_t DisplayConfigData::getSwapInterval() const
    {
        return m_swapInterval;
    }

    void DisplayConfigData::setScenePriority(SceneId sceneId, int32_t priority)
    {
        m_scenePriorities[sceneId] = priority;
    }

    int32_t DisplayConfigData::getScenePriority(SceneId sceneId) const
    {
        const auto it = m_scenePriorities.find(sceneId);
        if (it != m_scenePriorities.end())
        {
            return it->second;
        }
        return 0;
    }

    const std::unordered_map<SceneId, int32_t>& DisplayConfigData::getScenePriorities() const
    {
        return m_scenePriorities;
    }

    void DisplayConfigData::setResourceUploadBatchSize(uint32_t batchSize)
    {
        m_resourceUploadBatchSize = batchSize;
    }

    uint32_t DisplayConfigData::getResourceUploadBatchSize() const
    {
        return m_resourceUploadBatchSize;
    }

    bool DisplayConfigData::operator == (const DisplayConfigData& other) const
    {
        return
            m_deviceType                 == other.m_deviceType &&
            m_windowType                 == other.m_windowType &&
            m_windowTitle                == other.m_windowTitle &&
            m_fullscreen                 == other.m_fullscreen &&
            m_antiAliasingSamples        == other.m_antiAliasingSamples &&
            m_desiredWindowWidth         == other.m_desiredWindowWidth &&
            m_desiredWindowHeight        == other.m_desiredWindowHeight &&
            m_windowPositionX            == other.m_windowPositionX &&
            m_windowPositionY            == other.m_windowPositionY &&
            m_waylandIviLayerID          == other.m_waylandIviLayerID &&
            m_waylandIviSurfaceID        == other.m_waylandIviSurfaceID &&
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

    bool DisplayConfigData::operator != (const DisplayConfigData& other) const
    {
        return !operator==(other);
    }
}
