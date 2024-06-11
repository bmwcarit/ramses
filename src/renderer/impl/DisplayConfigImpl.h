//  -------------------------------------------------------------------------
//  Copyright (C) 2015 BMW Car IT GmbH
//  -------------------------------------------------------------------------
//  This Source Code Form is subject to the terms of the Mozilla Public
//  License, v. 2.0. If a copy of the MPL was not distributed with this
//  file, You can obtain one at https://mozilla.org/MPL/2.0/.
//  -------------------------------------------------------------------------

#pragma once

#include "ramses/renderer/Types.h"
#include "internal/RendererLib/DisplayConfigData.h"
#include "impl/DataTypesImpl.h"

namespace CLI
{
    class App;
}

namespace ramses::internal
{
    class ValidationReportImpl;

    class DisplayConfigImpl
    {
    public:
        DisplayConfigImpl();

        [[nodiscard]] bool setDeviceType(EDeviceType deviceType);
        [[nodiscard]] EDeviceType getDeviceType() const;
        [[nodiscard]] bool setWindowType(EWindowType windowType);
        [[nodiscard]] EWindowType getWindowType() const;
        [[nodiscard]] bool setWindowTitle(std::string_view windowTitle);
        [[nodiscard]] std::string_view getWindowTitle() const;
        [[nodiscard]] bool setWindowRectangle(int32_t x, int32_t y, uint32_t width, uint32_t height);
        [[nodiscard]] bool getWindowRectangle(int32_t& x, int32_t& y, uint32_t& width, uint32_t& height) const;
        [[nodiscard]] bool setFullscreen(bool fullscreen);
        [[nodiscard]] bool isFullscreen() const;
        [[nodiscard]] bool setMultiSampling(uint32_t numSamples);
        [[nodiscard]] bool getMultiSamplingSamples(uint32_t& numSamples) const;
        [[nodiscard]] bool setWaylandIviSurfaceID(waylandIviSurfaceId_t waylandIviSurfaceID);
        [[nodiscard]] waylandIviSurfaceId_t getWaylandIviSurfaceID() const;
        [[nodiscard]] bool setWaylandIviLayerID(waylandIviLayerId_t waylandIviLayerID);
        [[nodiscard]] waylandIviLayerId_t getWaylandIviLayerID() const;
        [[nodiscard]] bool setWaylandDisplay(std::string_view waylandDisplay);
        [[nodiscard]] std::string_view getWaylandDisplay() const;
        [[nodiscard]] void* getAndroidNativeWindow() const;
        [[nodiscard]] bool setAndroidNativeWindow(void * nativeWindowPtr);
        [[nodiscard]] IOSNativeWindowPtr getIOSNativeWindow() const;
        [[nodiscard]] bool setIOSNativeWindow(IOSNativeWindowPtr nativeWindowPtr);
        [[nodiscard]] bool setWindowIviVisible(bool visible);
        [[nodiscard]] bool setResizable(bool resizable);
        [[nodiscard]] bool setGPUMemoryCacheSize(uint64_t size);
        [[nodiscard]] bool setClearColor(const vec4f& color);
        [[nodiscard]] bool setDepthStencilBufferType(EDepthBufferType depthBufferType);
        [[nodiscard]] bool setX11WindowHandle(X11WindowHandle x11WindowHandle);
        [[nodiscard]] X11WindowHandle getX11WindowHandle() const;
        [[nodiscard]] bool setWindowsWindowHandle(void* hwnd);
        [[nodiscard]] void*    getWindowsWindowHandle() const;
        [[nodiscard]] bool setAsyncEffectUploadEnabled(bool enabled);

        [[nodiscard]] bool setWaylandEmbeddedCompositingSocketGroup(std::string_view groupname);
        [[nodiscard]] std::string_view getWaylandSocketEmbeddedGroup() const;

        [[nodiscard]] bool setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        [[nodiscard]] uint32_t getWaylandSocketEmbeddedPermissions() const;

        [[nodiscard]] bool setWaylandEmbeddedCompositingSocketName(std::string_view socketname);
        [[nodiscard]] std::string_view getWaylandEmbeddedCompositingSocketName() const;

        [[nodiscard]] bool setWaylandEmbeddedCompositingSocketFD(int fd);
        [[nodiscard]] int getWaylandSocketEmbeddedFD() const;

        [[nodiscard]] bool setPlatformRenderNode(std::string_view renderNode);
        [[nodiscard]] std::string_view getPlatformRenderNode() const;

        [[nodiscard]] bool setSwapInterval(int32_t interval);
        [[nodiscard]] int32_t  getSwapInterval() const;

        [[nodiscard]] bool setScenePriority(sceneId_t sceneId, int32_t priority);
        [[nodiscard]] int32_t getScenePriority(sceneId_t sceneId) const;

        [[nodiscard]] bool setResourceUploadBatchSize(uint32_t batchSize);
        [[nodiscard]] uint32_t getResourceUploadBatchSize() const;

        void validate(ValidationReportImpl& report) const;

        //impl methods
        [[nodiscard]] const ramses::internal::DisplayConfigData& getInternalDisplayConfig() const;

    private:
        ramses::internal::DisplayConfigData m_internalConfig;
    };
}
