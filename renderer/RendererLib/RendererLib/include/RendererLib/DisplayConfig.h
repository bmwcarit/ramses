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
#include "RendererAPI/EDeviceType.h"
#include "RendererAPI/EWindowType.h"
#include "SceneAPI/SceneId.h"
#include "SceneAPI/TextureEnums.h"
#include "DataTypesImpl.h"

#include <unordered_map>
#include <string>
#include <string_view>

namespace ramses_internal
{
    class DisplayConfig
    {
    public:
        DisplayConfig() {}

        [[nodiscard]] EDeviceType getDeviceType() const;
        void setDeviceType(EDeviceType deviceType);

        [[nodiscard]] EWindowType getWindowType() const;
        void setWindowType(EWindowType windowType);

        [[nodiscard]] bool getFullscreenState() const;
        void setFullscreenState(bool state);


        [[nodiscard]] bool getBorderlessState() const ;
        void setBorderlessState(bool state);

        [[nodiscard]] uint32_t getAntialiasingSampleCount() const;
        void setAntialiasingSampleCount(uint32_t samples);

        [[nodiscard]] uint32_t getDesiredWindowWidth() const;
        void setDesiredWindowWidth(uint32_t width);

        [[nodiscard]] uint32_t getDesiredWindowHeight() const;
        void setDesiredWindowHeight(uint32_t height);

        [[nodiscard]] int32_t getWindowPositionX() const;
        void setWindowPositionX(int32_t posx);

        [[nodiscard]] int32_t getWindowPositionY() const;
        void setWindowPositionY(int32_t posy);

        [[nodiscard]] WaylandIviLayerId getWaylandIviLayerID() const;
        void setWaylandIviLayerID(WaylandIviLayerId waylandIviLayerID);

        [[nodiscard]] WaylandIviSurfaceId getWaylandIviSurfaceID() const;
        void setWaylandIviSurfaceID(WaylandIviSurfaceId waylandIviSurfaceID);

        void setWaylandDisplay(std::string_view waylandDisplay);
        [[nodiscard]] std::string_view getWaylandDisplay() const;

        void setWindowsWindowHandle(WindowsWindowHandle hwnd);
        [[nodiscard]] WindowsWindowHandle getWindowsWindowHandle() const;

        void setX11WindowHandle(X11WindowHandle x11WindowHandle);
        [[nodiscard]] X11WindowHandle getX11WindowHandle() const;

        [[nodiscard]] AndroidNativeWindowPtr getAndroidNativeWindow() const;
        void setAndroidNativeWindow(AndroidNativeWindowPtr nativeWindowPtr);

        [[nodiscard]] bool getStartVisibleIvi() const;
        void setStartVisibleIvi(bool startVisible);

        [[nodiscard]] bool getKeepEffectsUploaded() const;
        void setKeepEffectsUploaded(bool enable);

        [[nodiscard]] bool isResizable() const;
        void setResizable(bool resizable);

        [[nodiscard]] uint64_t getGPUMemoryCacheSize() const;
        void setGPUMemoryCacheSize(uint64_t size);

        void setClearColor(const glm::vec4& clearColor);
        [[nodiscard]] const glm::vec4& getClearColor() const;

        void setDepthStencilBufferType(ERenderBufferType depthStencilBufferType);
        [[nodiscard]] ERenderBufferType getDepthStencilBufferType() const;

        void setAsyncEffectUploadEnabled(bool enabled);
        [[nodiscard]] bool isAsyncEffectUploadEnabled() const;

        void setWaylandEmbeddedCompositingSocketName(std::string_view socket);
        [[nodiscard]] std::string_view getWaylandSocketEmbedded() const;

        void setWaylandEmbeddedCompositingSocketGroup(std::string_view groupNameForSocketPermissions);
        [[nodiscard]] std::string_view getWaylandSocketEmbeddedGroup() const;

        void setWaylandEmbeddedCompositingSocketFD(int fd);
        [[nodiscard]] int getWaylandSocketEmbeddedFD() const;

        bool setWaylandEmbeddedCompositingSocketPermissions(uint32_t permissions);
        [[nodiscard]] uint32_t getWaylandSocketEmbeddedPermissions() const;

        void setPlatformRenderNode(std::string_view renderNode);
        [[nodiscard]] std::string_view getPlatformRenderNode() const;

        void setSwapInterval(int32_t interval);
        [[nodiscard]] int32_t getSwapInterval() const;

        void setScenePriority(SceneId sceneId, int32_t priority);
        [[nodiscard]] int32_t getScenePriority(SceneId sceneId) const;
        [[nodiscard]] const std::unordered_map<SceneId, int32_t>& getScenePriorities() const;

        void setResourceUploadBatchSize(uint32_t batchSize);
        [[nodiscard]] uint32_t getResourceUploadBatchSize() const;

        bool operator==(const DisplayConfig& other) const;
        bool operator!=(const DisplayConfig& other) const;

    private:
        EDeviceType m_deviceType = EDeviceType::GLES_3_0;

        static_assert(SupportedWindowTypes.size() > 0, "No window types supported for build configuration");
        EWindowType m_windowType = *SupportedWindowTypes.begin();

        bool m_fullscreen = false;
        bool m_borderless = false;
        bool m_resizable = false;

        uint32_t m_desiredWindowWidth = 1280;
        uint32_t m_desiredWindowHeight = 480;
        int32_t m_windowPositionX = 0;
        int32_t m_windowPositionY = 0;

        WaylandIviLayerId m_waylandIviLayerID;
        WaylandIviSurfaceId m_waylandIviSurfaceID;
        WindowsWindowHandle m_windowsWindowHandle;
        X11WindowHandle m_x11WindowHandle;
        AndroidNativeWindowPtr m_androidNativeWindowPtr;
        bool m_startVisibleIvi = false;
        std::string m_waylandDisplay;

        uint32_t m_antiAliasingSamples = 1;

        bool m_keepEffectsUploaded = true;
        uint64_t m_gpuMemoryCacheSize = 0u;
        glm::vec4 m_clearColor{ 0.f, 0.f, 0.f, 1.0f };
        ERenderBufferType m_depthStencilBufferType = ERenderBufferType_DepthStencilBuffer;
        bool m_asyncEffectUploadEnabled = true;

        std::string m_waylandSocketEmbedded;
        std::string m_waylandSocketEmbeddedGroupName;
        uint32_t m_waylandSocketEmbeddedPermissions = 0;
        int m_waylandSocketEmbeddedFD = -1;
        std::string m_platformRenderNode;

        int32_t m_swapInterval = -1;
        std::unordered_map<SceneId, int32_t> m_scenePriorities;
        uint32_t m_resourceUploadBatchSize = 10u;
    };
}

#endif
