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
#include "Utils/CommandLineParser.h"

namespace ramses
{
    class DisplayConfigImpl : public StatusObjectImpl
    {
    public:
        DisplayConfigImpl(int32_t argc, char const* const* argv);

        status_t setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height);
        status_t getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const;
        status_t setFullscreen(bool fullscreen);
        bool     isFullscreen() const;
        status_t setBorderless(bool borderless);
        status_t setMultiSampling(uint32_t numSamples);
        status_t getMultiSamplingSamples(uint32_t& numSamples) const;
        status_t enableWarpingPostEffect();
        status_t setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID);
        waylandIviSurfaceId_t getWaylandIviSurfaceID() const;
        status_t setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID);
        waylandIviLayerId_t getWaylandIviLayerID() const;
        status_t setWaylandDisplay(const char* waylandDisplay);
        const char* getWaylandDisplay() const;
        status_t setIntegrityRGLDeviceUnit(uint32_t rglDeviceUnit);
        uint32_t getIntegrityRGLDeviceUnit() const;
        void* getMobilePlatformNativeWindow() const;
        status_t setMobilePlatformNativeWindow(void * nativeWindowPtr);
        status_t setWindowIviVisible(bool visible);
        status_t setResizable(bool resizable);
        status_t keepEffectsUploaded(bool enable);
        status_t setGPUMemoryCacheSize(uint64_t size);
        status_t setClearColor(float red, float green, float blue, float alpha);
        status_t setOffscreen(bool offscreenFlag);
        status_t setDepthStencilBufferType(EDepthBufferType depthBufferType);
        status_t setX11WindowHandle(unsigned long x11WindowHandle);
        unsigned long getX11WindowHandle() const;
        status_t setWindowsWindowHandle(void* hwnd);
        void*    getWindowsWindowHandle() const;
        status_t setAsyncEffectUploadEnabled(bool enabled);

        status_t setWaylandEmbeddedCompositingSocketGroup(const char* groupname);
        const char* getWaylandSocketEmbeddedGroup() const;

        status_t setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        uint32_t getWaylandSocketEmbeddedPermissions() const;

        status_t setWaylandEmbeddedCompositingSocketName(const char* socketname);
        const char* getWaylandEmbeddedCompositingSocketName() const;

        status_t setWaylandEmbeddedCompositingSocketFD(int fd);
        int getWaylandSocketEmbeddedFD() const;

        status_t setPlatformRenderNode(const char* renderNode);
        const char* getPlatformRenderNode() const;

        status_t setSwapInterval(int32_t interval);
        int32_t  getSwapInterval() const;

        status_t setScenePriority(sceneId_t sceneId, int32_t priority);
        int32_t getScenePriority(sceneId_t) const;

        virtual status_t validate() const override;

        //impl methods
        const ramses_internal::DisplayConfig& getInternalDisplayConfig() const;

    private:
        ramses_internal::DisplayConfig m_internalConfig;
    };
}

#endif
