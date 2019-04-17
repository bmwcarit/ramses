//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

// API
#include "ramses-renderer-api/DisplayConfig.h"

// internal
#include "DisplayConfigImpl.h"

namespace ramses
{
    DisplayConfig::DisplayConfig(int32_t argc, char const* const* argv)
        : StatusObject(*new DisplayConfigImpl(argc, argv))
        , impl(static_cast<DisplayConfigImpl&>(StatusObject::impl))
    {
    }

    DisplayConfig::DisplayConfig(int32_t argc, char* argv[])
        : StatusObject(*new DisplayConfigImpl(argc, const_cast<char const* const*>(argv)))
        , impl(static_cast<DisplayConfigImpl&>(StatusObject::impl))
    {
    }

    DisplayConfig::DisplayConfig(const DisplayConfig& other)
        : StatusObject(*new DisplayConfigImpl(other.impl))
        , impl(static_cast<DisplayConfigImpl&>(StatusObject::impl))
    {
    }

    DisplayConfig::~DisplayConfig()
    {
    }

    status_t DisplayConfig::setViewPosition(float x, float y, float z)
    {
        const status_t status = impl.setViewPosition(x, y, z);
        LOG_HL_RENDERER_API3(status, x, y, z);
        return status;
    }

    status_t DisplayConfig::getViewPosition(float& x, float& y, float& z) const
    {
        return impl.getViewPosition(x, y, z);
    }

    status_t DisplayConfig::setViewRotation(float x, float y, float z)
    {
        const status_t status = impl.setViewRotation(x, y, z);
        LOG_HL_RENDERER_API3(status, x, y, z);
        return status;
    }

    status_t DisplayConfig::getViewRotation(float& x, float& y, float& z) const
    {
        return impl.getViewRotation(x, y, z);
    }

    status_t DisplayConfig::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.setWindowRectangle(x, y, width, height);
        LOG_HL_RENDERER_API4(status, x, y, width, height);
        return status;
    }

    status_t DisplayConfig::setWindowFullscreen(bool fullscreen)
    {
        const status_t status = impl.setFullscreen(fullscreen);
        LOG_HL_RENDERER_API1(status, fullscreen);
        return status;
    }

    status_t DisplayConfig::setWindowBorderless(bool borderless)
    {
        const status_t status = impl.setBorderless(borderless);
        LOG_HL_RENDERER_API1(status, borderless);
        return status;
    }

    status_t DisplayConfig::setPerspectiveProjection(float fieldOfViewY, float aspectRatio, float nearPlane, float farPlane)
    {
        const status_t status = impl.setPerspectiveProjection(fieldOfViewY, aspectRatio, nearPlane, farPlane);
        LOG_HL_RENDERER_API4(status, fieldOfViewY, aspectRatio, nearPlane, farPlane);
        return status;
    }

    status_t DisplayConfig::setPerspectiveProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const status_t status = impl.setProjection(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane, false);
        LOG_HL_RENDERER_API6(status, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        return status;
    }

    status_t DisplayConfig::setOrthographicProjection(float leftPlane, float rightPlane, float bottomPlane, float topPlane, float nearPlane, float farPlane)
    {
        const status_t status = impl.setProjection(leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane, true);
        LOG_HL_RENDERER_API6(status, leftPlane, rightPlane, bottomPlane, topPlane, nearPlane, farPlane);
        return status;
    }

    status_t DisplayConfig::setMultiSampling(uint32_t numSamples)
    {
        const status_t status = impl.setMultiSampling(numSamples);
        LOG_HL_RENDERER_API1(status, numSamples);
        return status;
    }

    status_t DisplayConfig::getMultiSamplingSamples(uint32_t& numSamples) const
    {
        return impl.getMultiSamplingSamples(numSamples);
    }

    status_t DisplayConfig::enableWarpingPostEffect()
    {
        const status_t status = impl.enableWarpingPostEffect();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::enableStereoDisplay()
    {
        const status_t status = impl.enableStereoDisplay();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::setWaylandIviLayerID(uint32_t waylandIviLayerID)
    {
        const status_t status = impl.setWaylandIviLayerID(waylandIviLayerID);
        LOG_HL_RENDERER_API1(status, waylandIviLayerID);
        return status;
    }

    uint32_t DisplayConfig::getWaylandIviLayerID() const
    {
        return impl.getWaylandIviLayerID();
    }

    status_t DisplayConfig::setWaylandIviSurfaceID(uint32_t waylandIviSurfaceID)
    {
        const status_t status = impl.setWaylandIviSurfaceID(waylandIviSurfaceID);
        LOG_HL_RENDERER_API1(status, waylandIviSurfaceID);
        return status;
    }

    uint32_t DisplayConfig::getWaylandIviSurfaceID() const
    {
        return impl.getWaylandIviSurfaceID();
    }

    status_t DisplayConfig::setWaylandDisplay(const char* waylandDisplay)
    {
        return impl.setWaylandDisplay(waylandDisplay);
    }

    const char* DisplayConfig::getWaylandDisplay() const
    {
        return impl.getWaylandDisplay();
    }

    status_t DisplayConfig::setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit)
    {
        const status_t status = impl.setIntegrityRGLDeviceUnit(rglDeviceUnit);
        LOG_HL_RENDERER_API1(status, rglDeviceUnit);
        return status;
    }

    uint32_t DisplayConfig::getIntegrityRGLDeviceUnit() const
    {
        return impl.getIntegrityRGLDeviceUnit();
    }

    status_t DisplayConfig::setWindowIviVisible()
    {
        const status_t status = impl.setWindowIviVisible();
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::keepEffectsUploaded(bool enable)
    {
        const status_t status = impl.keepEffectsUploaded(enable);
        LOG_HL_RENDERER_API1(status, enable);
        return status;
    }

    status_t DisplayConfig::setGPUMemoryCacheSize(uint64_t size)
    {
        const status_t status = impl.setGPUMemoryCacheSize(size);
        LOG_HL_RENDERER_API1(status, size);
        return status;
    }

    status_t DisplayConfig::setResizable(bool resizable)
    {
        const status_t status = impl.setResizable(resizable);
        LOG_HL_RENDERER_API_NOARG(status);
        return status;
    }

    status_t DisplayConfig::setClearColor(float red, float green, float blue, float alpha)
    {
        const status_t status = impl.setClearColor(red, green, blue, alpha);
        LOG_HL_RENDERER_API4(status, red, green, blue, alpha);
        return status;
    }

    status_t DisplayConfig::setOffscreen(bool offscreenFlag)
    {
        const status_t status = impl.setOffscreen(offscreenFlag);
        LOG_HL_RENDERER_API1(status, offscreenFlag);
        return status;
    }

    status_t DisplayConfig::setWindowsWindowHandle(void* hwnd)
    {
        const status_t status = impl.setWindowsWindowHandle(hwnd);
        LOG_HL_RENDERER_API1(status, LOG_API_GENERIC_PTR_STRING(hwnd));
        return status;
    }

    void* DisplayConfig::getWindowsWindowHandle() const
    {
        return impl.getWindowsWindowHandle();
    }
}
