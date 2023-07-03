//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#ifndef RAMSES_DISPLAYCONFIGIMPL_H
#define RAMSES_DISPLAYCONFIGIMPL_H

#include "ramses-renderer-api/Types.h"
#include "RendererLib/DisplayConfig.h"
#include "StatusObjectImpl.h"
#include "DataTypesImpl.h"

namespace CLI
{
    class App;
}

namespace ramses
{
    class DisplayConfigImpl : public StatusObjectImpl
    {
    public:
        DisplayConfigImpl();

        status_t setDeviceType(EDeviceType deviceType);
        EDeviceType getDeviceType() const;
        status_t setWindowType(EWindowType windowType);
        EWindowType getWindowType() const;
        status_t setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height);
        status_t getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const;
        status_t setFullscreen(bool fullscreen);
        bool     isFullscreen() const;
        status_t setBorderless(bool borderless);
        status_t setMultiSampling(uint32_t numSamples);
        status_t getMultiSamplingSamples(uint32_t& numSamples) const;
        status_t setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID);
        waylandIviSurfaceId_t getWaylandIviSurfaceID() const;
        status_t setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID);
        waylandIviLayerId_t getWaylandIviLayerID() const;
        status_t setWaylandDisplay(std::string_view waylandDisplay);
        std::string_view getWaylandDisplay() const;
        void* getAndroidNativeWindow() const;
        status_t setAndroidNativeWindow(void * nativeWindowPtr);
        void* getIOSNativeWindow() const;
        status_t setIOSNativeWindow(void * nativeWindowPtr);
        status_t setWindowIviVisible(bool visible);
        status_t setResizable(bool resizable);
        status_t keepEffectsUploaded(bool enable);
        status_t setGPUMemoryCacheSize(uint64_t size);
        status_t setClearColor(const vec4f& color);
        status_t setOffscreen(bool offscreenFlag);
        status_t setDepthStencilBufferType(EDepthBufferType depthBufferType);
        status_t setX11WindowHandle(unsigned long x11WindowHandle);
        unsigned long getX11WindowHandle() const;
        status_t setWindowsWindowHandle(void* hwnd);
        void*    getWindowsWindowHandle() const;
        status_t setAsyncEffectUploadEnabled(bool enabled);

        status_t setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname);
        std::string_view getWaylandSocketEmbeddedGroup() const;

        status_t setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        uint32_t getWaylandSocketEmbeddedPermissions() const;

        status_t setWaylandEmbeddedCompositingSocketName(std::string_view socketname);
        std::string_view getWaylandEmbeddedCompositingSocketName() const;

        status_t setWaylandEmbeddedCompositingSocketFD(int fd);
        int getWaylandSocketEmbeddedFD() const;

        status_t setPlatformRenderNode(std::string_view renderNode);
        std::string_view getPlatformRenderNode() const;

        status_t setSwapInterval(int32_t interval);
        int32_t  getSwapInterval() const;

        status_t setScenePriority(sceneId_t sceneId, int32_t priority);
        int32_t getScenePriority(sceneId_t) const;

        status_t setResourceUploadBatchSize(uint32_t batchSize);
        uint32_t getResourceUploadBatchSize() const;

        status_t validate() const override;

        //impl methods
        const ramses_internal::DisplayConfig& getInternalDisplayConfig() const;

    private:
        ramses_internal::DisplayConfig m_internalConfig;
    };
}

#endif
