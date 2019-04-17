//  -------------------------------------------------------------------------
//  Copyright (C) 2014 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#include "RendererLib/DisplayConfig.h"
#include "Utils/LogMacros.h"

namespace ramses_internal
{
    void DisplayConfig::setAntialiasingSampleCount(UInt32 samples)
    {
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

    EAntiAliasingMethod DisplayConfig::getAntialiasingMethod() const
    {
        return m_antiAliasingMethod;
    }

    void DisplayConfig::setAntialiasingMethod(EAntiAliasingMethod method)
    {
        m_antiAliasingMethod = method;
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

    const Vector3& DisplayConfig::getCameraPosition() const
    {
        return m_cameraPosition;
    }

    void DisplayConfig::setCameraPosition(const Vector3& position)
    {
        m_cameraPosition = position;
    }

    const Vector3& DisplayConfig::getCameraRotation() const
    {
        return m_cameraRotation;
    }

    void DisplayConfig::setCameraRotation(const Vector3& rotation)
    {
        m_cameraRotation = rotation;
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

    Bool DisplayConfig::isStereoDisplay() const
    {
        return m_stereoDisplay;
    }

    void DisplayConfig::setStereoDisplay(Bool enabled)
    {
        m_stereoDisplay = enabled;
    }

    void DisplayConfig::setProjectionParams(const ProjectionParams& params)
    {
        m_projectionParams = params;
    }

    const ProjectionParams& DisplayConfig::getProjectionParams() const
    {
        return m_projectionParams;
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

    void DisplayConfig::setOffscreen(Bool offscreenFlag)
    {
        m_offscreen = offscreenFlag;
    }

    Bool DisplayConfig::getOffscreen() const
    {
        return m_offscreen;
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

    Bool DisplayConfig::operator == (const DisplayConfig& other) const
    {
        return
            m_fullscreen                 == other.m_fullscreen &&
            m_borderless                 == other.m_borderless &&
            m_warpingEnabled             == other.m_warpingEnabled &&
            m_stereoDisplay              == other.m_stereoDisplay &&
            m_antiAliasingMethod         == other.m_antiAliasingMethod &&
            m_antiAliasingSamples        == other.m_antiAliasingSamples &&
            m_desiredWindowWidth         == other.m_desiredWindowWidth &&
            m_desiredWindowHeight        == other.m_desiredWindowHeight &&
            m_windowPositionX            == other.m_windowPositionX &&
            m_windowPositionY            == other.m_windowPositionY &&
            m_cameraPosition             == other.m_cameraPosition &&
            m_cameraRotation             == other.m_cameraRotation &&
            m_projectionParams           == other.m_projectionParams &&
            m_waylandIviLayerID          == other.m_waylandIviLayerID &&
            m_waylandIviSurfaceID        == other.m_waylandIviSurfaceID &&
            m_integrityRGLDeviceUnit     == other.m_integrityRGLDeviceUnit &&
            m_startVisibleIvi            == other.m_startVisibleIvi &&
            m_resizable                  == other.m_resizable &&
            m_gpuMemoryCacheSize         == other.m_gpuMemoryCacheSize &&
            m_clearColor                 == other.m_clearColor &&
            m_offscreen                  == other.m_offscreen &&
            m_windowsWindowHandle        == other.m_windowsWindowHandle &&
            m_waylandDisplay             == other.m_waylandDisplay;
    }

    Bool DisplayConfig::operator != (const DisplayConfig& other) const
    {
        return !operator==(other);
    }
}
