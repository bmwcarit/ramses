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
#include "RamsesFrameworkTypesImpl.h"

namespace ramses
{
    DisplayConfig::DisplayConfig()
        : DisplayConfig(0, nullptr)
    {
    }

    DisplayConfig::DisplayConfig(int32_t argc, char const* const* argv)
        : StatusObject(*new DisplayConfigImpl(argc, argv))
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

    void DisplayConfig::registerOptions(CLI::App& cli)
    {
        impl.registerOptions(cli);
    }

    status_t DisplayConfig::setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height)
    {
        const status_t status = impl.setWindowRectangle(x, y, width, height);
        LOG_HL_RENDERER_API4(status, x, y, width, height);
        return status;
    }

    status_t DisplayConfig::getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const
    {
        return impl.getWindowRectangle(x, y, width, height);
    }

    status_t DisplayConfig::setWindowFullscreen(bool fullscreen)
    {
        const status_t status = impl.setFullscreen(fullscreen);
        LOG_HL_RENDERER_API1(status, fullscreen);
        return status;
    }

    bool DisplayConfig::isWindowFullscreen() const
    {
        return impl.isFullscreen();
    }

    status_t DisplayConfig::setWindowBorderless(bool borderless)
    {
        const status_t status = impl.setBorderless(borderless);
        LOG_HL_RENDERER_API1(status, borderless);
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

    status_t DisplayConfig::setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID)
    {
        const status_t status = impl.setWaylandIviLayerID(waylandIviLayerID);
        LOG_HL_RENDERER_API1(status, waylandIviLayerID);
        return status;
    }

    waylandIviLayerId_t DisplayConfig::getWaylandIviLayerID() const
    {
        return impl.getWaylandIviLayerID();
    }

    status_t DisplayConfig::setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID)
    {
        const status_t status = impl.setWaylandIviSurfaceID(waylandIviSurfaceID);
        LOG_HL_RENDERER_API1(status, waylandIviSurfaceID.getValue());
        return status;
    }

    waylandIviSurfaceId_t DisplayConfig::getWaylandIviSurfaceID() const
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

    status_t DisplayConfig::setAsyncEffectUploadEnabled(bool enabled)
    {
        const status_t status = impl.setAsyncEffectUploadEnabled(enabled);
        LOG_HL_RENDERER_API1(status, enabled);
        return status;
    }

    void* DisplayConfig::getAndroidNativeWindow() const
    {
        return impl.getAndroidNativeWindow();
    }

    status_t DisplayConfig::setAndroidNativeWindow(void* nativeWindowPtr)
    {
        return impl.setAndroidNativeWindow(nativeWindowPtr);
    }

    status_t DisplayConfig::setWindowIviVisible()
    {
        const status_t status = impl.setWindowIviVisible(true);
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

    status_t DisplayConfig::setDepthStencilBufferType(EDepthBufferType depthBufferType)
    {
        const auto status = impl.setDepthStencilBufferType(depthBufferType);
        LOG_HL_RENDERER_API1(status, depthBufferType);
        return status;
    }

    status_t DisplayConfig::setX11WindowHandle(unsigned long x11WindowHandle)
    {
        const status_t status = impl.setX11WindowHandle(x11WindowHandle);
        LOG_HL_RENDERER_API1(status, x11WindowHandle);
        return status;
    }

    unsigned long DisplayConfig::getX11WindowHandle() const
    {
        return impl.getX11WindowHandle();
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

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketName(const char* socketname)
    {
        return impl.setWaylandEmbeddedCompositingSocketName(socketname);
    }

    const char *DisplayConfig::getWaylandEmbeddedCompositingSocketName() const
    {
        return impl.getWaylandEmbeddedCompositingSocketName();
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketGroup(const char* groupname)
    {
        return impl.setWaylandEmbeddedCompositingSocketGroup(groupname);
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketFD(int socketFileDescriptor)
    {
        return impl.setWaylandEmbeddedCompositingSocketFD(socketFileDescriptor);
    }

    status_t DisplayConfig::setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions)
    {
        return impl.setWaylandEmbeddedCompositingSocketPermissions(permissions);
    }

    status_t DisplayConfig::setPlatformRenderNode(const char* renderNode)
    {
        return impl.setPlatformRenderNode(renderNode);
    }

    status_t DisplayConfig::setSwapInterval(int32_t interval)
    {
        return impl.setSwapInterval(interval);
    }

    status_t DisplayConfig::setScenePriority(sceneId_t sceneId, int32_t priority)
    {
        return impl.setScenePriority(sceneId, priority);
    }

    status_t DisplayConfig::setResourceUploadBatchSize(uint32_t batchSize)
    {
        return impl.setResourceUploadBatchSize(batchSize);
    }
}
